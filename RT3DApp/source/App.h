/*
 *  App.h
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */

#pragma once

#include "BaseApp.h"
#include "EDriverTypes.h" //by stone

class App: public BaseApp
{
public:
	
	App();
	virtual ~App();
	
	virtual bool	Init();
	//virtual void	Kill();
	virtual void	Draw();
	virtual void	Update();
	virtual void	CheckInitAgain();
	virtual void	OnEnterBackground();
	virtual void	OnEnterForeground();
	virtual void	OnScreenSizeChange();
	
	void			OnUnloadSurfaces();
	void			OnReLoadSurfaces();
	string			GetVersionString();
	float			GetVersion();
	int				GetBuild();
	void			GetServerInfo(string &server, uint32 &port);
	VariantDB*		GetShared() {return &m_varDB;}
	Variant*		GetVar(const string &keyName );
	Variant*		GetVarWithDefault(const string &varName, const Variant &var) {return m_varDB.GetVarWithDefault(varName, var);}
	void			OnExitApp(VariantList *pVarList);
    void            SaveStuff();
	Entity*			GetMainScene();

private:
	bool			m_bDidPostInit;
	//int				m_special;
	int				m_connect_set; //by jesse stone
	int				m_initagain;
	
	VariantDB		m_varDB;		//holds all data we want to save/load
	Entity*			m_MenuEntity;	//by stone, high level shader used
};

extern irr::video::E_DRIVER_TYPE AppGetOGLESType();

extern AudioManager*	GetAudioManager();  //supply this yourself
extern void             FreeAudioManager();

extern const char*		GetAppName();
extern const char*		GetBundleName();
extern const char*		GetBundlePrefix();


