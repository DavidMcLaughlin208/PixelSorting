#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    ofGLFWWindowSettings settings;
    settings.setGLVersion(4, 3); //we define the OpenGL version we want to use
    settings.setSize(1280, 800);
    ofCreateWindow(settings);
    HWND hwnd = ofGetWin32Window();
    HICON hMyIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hMyIcon);
    // this kicks off the running of my app
    ofRunApp(new ofApp());

}
