1.
Proton 3d engine impelement irrlicht 1.9.0 with OGLES1.0, OGLES2.0 and OpenGL
You can build on Win32, MacOS, Android, Ios

2.
Rember first run
On Win32
xxxxxx/media/update_media.bat
&
On MacOs
chmod 777 xxxxxx/media/update_media.sh
xxxxxx/media/update_media.sh

3.
If debug c++ in Android, please set
android:installLocation="internalOnly" of AndroidManifest.xml

4.
Switch OGLES1 -> OGLES2
(a.) find App.cpp
irr::video::E_DRIVER_TYPE AppGetOGLESType()
return irr::video::EDT_OGLES1; -> return irr::video::EDT_OGLES2;
(b.) find SharedActivity.java
public AppGLSurfaceView(Context context, SharedActivity _app)
//setEGLContextClientVersion(2); -> setEGLContextClientVersion(2);

5.
Predefined C_GL_MODE of IrrCompileConfig.h
(a) Win32 and MacOs Enable C_GL_MODE
(b) Ios and Android Disable C_GL_MODE

6.
Enable or disable Irrlicht info bar and gui
Find _IRR_COMPILE_WITH_GUI_ of IrrCompileConfig.h
Enable or disable this define.

