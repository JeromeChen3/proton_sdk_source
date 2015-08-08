/*
 *  App.cpp
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */
#include "PlatformPrecomp.h"
#include "App.h"
#include "Entity/EntityUtils.h"
#include "Irrlicht/IrrlichtManager.h"
#include "FileSystem/FileSystemZip.h"
#include "GUI/MainMenu.h"

//by jesse stone
App* g_pApp = NULL;


irr::video::E_DRIVER_TYPE AppGetOGLESType()
{
	return irr::video::EDT_OGLES2;
}

////////////////////////////////////////
#ifdef __MACH__
	#if (TARGET_OS_IPHONE == 1)
        //it's an iPhone or iPad
		#include "Audio/AudioManagerOS.h"
		AudioManagerOS*		g_audioManager = NULL;
	#else
        //it's being compiled as a native OSX app
		#include "Audio/AudioManagerDenshion.h"
		AudioManagerDenshion*	g_audioManager = NULL; //dummy with no sound
	#endif
#else
	#include "Audio/AudioManagerSDL.h"
	#include "Audio/AudioManagerAndroid.h"

	#if defined RT_WEBOS || defined RT_USE_SDL_AUDIO
		AudioManagerSDL*		g_audioManager = NULL; //sound in windows and WebOS
	#elif defined ANDROID_NDK
		AudioManagerAndroid*	g_audioManager = NULL; //sound for android
	#else
		#include "Audio/AudioManagerAudiere.h"
		AudioManagerAudiere*	g_audioManager = NULL;  //Use Audiere for audio
	#endif
#endif

AudioManager* GetAudioManager()
{
#if defined __APPLE__
    #if (TARGET_OS_IPHONE == 1)
        g_audioManager = AudioManagerOS::GetAudioManager();
    #else
        g_audioManager = AudioManagerDenshion::GetAudioManager();
    #endif
#elif defined RT_WEBOS || defined RT_USE_SDL_AUDIO
	g_audioManager = AudioManagerSDL::GetAudioManager();
#elif defined ANDROID_NDK
	g_audioManager = AudioManagerAndroid::GetAudioManager();
#else
	g_audioManager = AudioManagerAudiere::GetAudioManager();
#endif
    
	return g_audioManager;
}

void FreeAudioManager()
{
#if defined __APPLE__
    #if (TARGET_OS_IPHONE == 1)
        AudioManagerOS::Free();
    #else
        AudioManagerDenshion::Free();
    #endif
#elif defined RT_WEBOS || defined RT_USE_SDL_AUDIO
	AudioManagerSDL::Free();
#elif defined ANDROID_NDK
	AudioManagerAndroid::Free();
#else
	AudioManagerAudiere::Free();
#endif
    
	g_audioManager = NULL;
}

//////////////////App////////////////////////
App::App()
{
	m_initagain		= 0;
	m_connect_set   = 0;

	m_bDidPostInit	= false;
	m_MenuEntity	= NULL;
}

App::~App()
{
	if( m_connect_set )
	{
		m_connect_set = 0;
				
		m_sig_unloadSurfaces.disconnect(boost::bind(&App::OnUnloadSurfaces, this));
		m_sig_loadSurfaces.disconnect(boost::bind(&App::OnReLoadSurfaces, this));
	}
		
	FreeAudioManager();

	Entity::Free();
	FileManager::Free();
	ResourceManager::Free();
	IrrlichtManager::Free();
	MessageManager::Free();
}

bool App::Init()
{
    bool bFileExisted;
	
    if( m_connect_set == 0 )
	{
		m_connect_set = 1;
				
		m_sig_unloadSurfaces.connect(1, boost::bind(&App::OnUnloadSurfaces, this));
		m_sig_loadSurfaces.connect(1, boost::bind(&App::OnReLoadSurfaces, this));
	}
	
	if (GetEmulatedPlatformID() == PLATFORM_ID_IOS)
	{
		SetLockedLandscape(true); //we don't allow portrait mode for this game.  Android doesn't count
		//because its landscape mode is addressed like portrait mode when set in the manifest.
	}
	
	if (GetEmulatedPlatformID() == PLATFORM_ID_WEBOS && IsIPADSize)
	{
		LogMsg("Special handling for touchpad landscape mode..");
		SetLockedLandscape(false);
		SetupScreenInfo(GetPrimaryGLX(), GetPrimaryGLY(), ORIENTATION_PORTRAIT);
	}
	
	SetManualRotationMode(true);
	
	if ( !BaseApp::IsInitted() )
	{
		BaseApp::Init();
			    
		m_varDB.Load("save.dat", &bFileExisted);
		LogMsg("Save path is %s", GetSavePath().c_str());
	    
		//preload audio
		if( GetAudioManager() )
		{
			GetAudioManager()->Preload("audio/click.wav");
			//GetAudioManager()->Play("audio/real.mp3",1,1); //ios 128bps mp3
			//GetAudioManager()->Play("audio/real.ogg",1,1); //android play ogg
		}
		
		if( IrrlichtManager::GetIrrlichtManager()->Init() )
		{	
#ifdef	_IRR_COMPILE_WITH_GUI_
			SetFPSVisible(true);
#else
			SetFPSVisible(false);
#endif
	    
			if( GetFPSVisible() )
			{
				IrrlichtDevice*         device  = IrrlichtManager::GetIrrlichtManager()->GetDevice();
				gui::IGUIEnvironment*   gui     = device->getGUIEnvironment();
				gui::IGUIStaticText*    text    = gui->addStaticText(L"FPS:", core::rect<s32>(0,0,GetScreenSizeX()-1,10), true, false, NULL, 0x10000, true);
		        
				text->setOverrideColor(video::SColor(255,0,0,0));
				text->setBackgroundColor(video::SColor(255,255,255,255));
			}
		}
	}
    
	return true;
}

/*void App::Kill()
{
	Entity::GetEntityManager()->RemoveAllEntities();
	IrrlichtManager::GetIrrlichtManager()->Kill();
	BaseApp::Kill();
}*/

void App::Update()
{
	BaseApp::Update();
    
	if (!m_bDidPostInit)
	{
		m_bDidPostInit = true;
				
		//build a GUI node
		Entity *pGUIEnt = Entity::GetEntityManager()->AddEntity(new Entity("GUI"));
		//by stone, highlevel shader used
		m_MenuEntity = MainMenuCreate(pGUIEnt);
	}
}

void App::Draw()
{
	//this->isNeedInitAgain(); //move to outside
	
	IrrlichtManager::GetIrrlichtManager()->IsRunning(); //let it do its own update tick
	IrrlichtManager::GetIrrlichtManager()->BeginScene(); //turn on irrlicht's 3d mode renderstates
	IrrlichtManager::GetIrrlichtManager()->Render(); //render its scenegraph
	IrrlichtManager::GetIrrlichtManager()->EndScene(); //tell irrlicht to go into 2d mode renderstates
	
	BaseApp::Draw();
}

void App::CheckInitAgain()
{
	irr::IrrlichtDevice*		pdevice = IrrlichtManager::GetIrrlichtManager()->GetDevice();
	irr::video::IVideoDriver*	pdriver = IrrlichtManager::GetIrrlichtManager()->GetDriver();
		
	if( m_initagain )
	{
		m_initagain = 0;

		m_MenuEntity->OnUnLoad();

		if (pdevice->getGUIEnvironment())
			pdevice->getGUIEnvironment()->OnUnLoad();
			
		pdriver->OnUnLoad();

		pdriver->OnAgainDriverInit();
						
		pdriver->OnReLoad();
			
		if (pdevice->getGUIEnvironment())
			pdevice->getGUIEnvironment()->OnReLoad();

		m_MenuEntity->OnReLoad();
	}
}

void App::OnUnloadSurfaces()
{
	irr::video::IVideoDriver* pdriver = IrrlichtManager::GetIrrlichtManager()->GetDriver();
	
	if (pdriver)
	{
		LogMsg("Irrlicht unloading surfaces..");
	}
}

//m_sig_loadSurfaces trigger
void App::OnReLoadSurfaces()
{
	irr::video::IVideoDriver* pdriver = IrrlichtManager::GetIrrlichtManager()->GetDriver();
	
	if (pdriver)
	{
		LogMsg("Irrlicht loading surfaces..");

		m_initagain = 1;
	}
}

Entity*	App::GetMainScene() 
{
	return m_MenuEntity;
}

void App::OnScreenSizeChange()
{
	BaseApp::OnScreenSizeChange();
}

void App::GetServerInfo( string &server, uint32 &port )
{
#if defined (_DEBUG) && defined(WIN32)
	server = "localhost";
	port = 8080;

	//server = "www.rtsoft.com";
	//port = 80;
#else

	server = "rtsoft.com";
	port = 80;
#endif
}

/*int App::GetSpecial()
{
	return m_special; //1 means pirated copy
}*/

Variant * App::GetVar( const string &keyName )
{
	return GetShared()->GetVar(keyName);
}

std::string App::GetVersionString()
{
	return "V0.7";
}

float App::GetVersion()
{
	return 0.7f;
}

int App::GetBuild()
{
	return 1;
}

void App::SaveStuff()
{
	m_varDB.Save("save.dat");
}
void App::OnEnterBackground()
{
	SaveStuff();
	BaseApp::OnEnterBackground();
}

void App::OnEnterForeground()
{
	BaseApp::OnEnterForeground();
}


void App::OnExitApp(VariantList *pVarList)
{
	LogMsg("Exiting the app");
    
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_FINISH_APP;
	BaseApp::GetBaseApp()->AddOSMessage(o);
}

///////////////////////////////////////////////////////
const char* GetAppName() 
{
	return "RT3DApp";
}

//for palm webos, android, App Store
const char * GetBundlePrefix()
{
	const char * bundlePrefix = "com.rtsoft.";
	return bundlePrefix;
}

//applicable to Palm WebOS builds only
const char * GetBundleName()
{
	const char * bundleName = "rt3dapp";
	return bundleName;
}
