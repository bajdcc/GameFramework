// CCGameFramework.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ui/window/Window.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    auto window = std::make_shared<Window>(nullptr, _T("CC_GameFramework_CLS"), _T("CC GameFramework Window"), hInstance);
    window->Run();
    return 0;
}