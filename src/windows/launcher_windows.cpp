// SPDX-License-Identifier: GPL-3.0-only
// Local Windows launcher. No game data is embedded.
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <algorithm>
#pragma comment(lib,"comctl32.lib")
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
static fs::path app;
static HWND romBox;
static HWND themeBox;
static int selectedTheme=0;
static const wchar_t *themeCodes[]={L"clean",L"arcade",L"crt",L"cabinet"};
static HWND brightnessSlider,contrastSlider,brightnessLabel,contrastLabel;
static int brightness=0,contrast=100;
static HFONT font;
static std::wstring rom;
static const wchar_t *defaultRom=L"";
static void SaveRom() {
    WritePrivateProfileStringW(L"jogo",L"rom",rom.c_str(),(app/L"launcher.ini").c_str());
    SetWindowTextW(romBox,rom.c_str());
}
static std::wstring Quote(const std::wstring &s) {
    std::wstring out=L"\"";unsigned slashes=0;
    for(wchar_t c:s) {
        if(c==L'\\') {++slashes;continue;}
        out.append(c==L'\"'?slashes*2+1:slashes,L'\\');slashes=0;out+=c;
    }
    out.append(slashes*2,L'\\');return out+L"\"";
}
static void Choose(HWND window) {
    wchar_t file[32768]={};
    OPENFILENAMEW ofn{sizeof(ofn)};ofn.hwndOwner=window;ofn.lpstrFile=file;ofn.nMaxFile=32768;
    ofn.lpstrFilter=L"ROM do jogo (*.zip)\0*.zip\0Todos os arquivos\0*.*\0";
    ofn.lpstrTitle=L"Escolha seu sfz2al.zip";ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if(GetOpenFileNameW(&ofn)) {rom=file;SaveRom();}
}
static void Play(HWND window,bool practice) {
    if(!fs::is_regular_file(rom)) {Choose(window);if(!fs::is_regular_file(rom))return;}
    const fs::path game=app/L"sfz2al-game.exe";
    if(!fs::is_regular_file(game)) {MessageBoxW(window,L"O executável do jogo não está nesta pasta.",L"SFZ2AL",MB_OK|MB_ICONERROR);return;}
    fs::create_directories(app/L"user");
    fs::path settings=app/L"user"/(practice?L"treino.ini":L"jogo.ini");
    if(!fs::exists(settings)) {
        std::error_code err;fs::copy_file(app/L"controles.ini",settings,fs::copy_options::none,err);
        if(err) {MessageBoxW(window,L"Não foi possível preparar a configuração de controles.",L"SFZ2AL",MB_OK|MB_ICONERROR);return;}
    }
    std::vector<std::wstring> args={game.wstring(),L"--rom",rom,L"--cpu",L"native",L"--sound",L"z80",
        L"--theme",themeCodes[selectedTheme],
        L"--brightness",std::to_wstring(brightness),L"--contrast",std::to_wstring(contrast),
        L"--renderer",L"sdl",L"--scale",L"3",L"--settings",settings.wstring(),L"--eeprom",(app/L"user/board.eeprom").wstring(),
        L"--features",practice?L"infinite_time,refill_health":L"-infinite_time,-refill_health"};
    std::wstring cmd;for(const auto &s:args) {if(!cmd.empty())cmd+=L' ';cmd+=Quote(s);}
    STARTUPINFOW si{sizeof(si)};PROCESS_INFORMATION pi{};
    if(!CreateProcessW(game.c_str(),cmd.data(),nullptr,nullptr,FALSE,0,nullptr,app.c_str(),&si,&pi)) {
        MessageBoxW(window,L"Não foi possível abrir o jogo.",L"SFZ2AL",MB_OK|MB_ICONERROR);return;
    }
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
}
static HWND Control(HWND w,const wchar_t *type,const wchar_t *text,DWORD style,int x,int y,int width,int height,int id=0) {
    HWND c=CreateWindowW(type,text,WS_CHILD|WS_VISIBLE|style,x,y,width,height,w,(HMENU)(INT_PTR)id,nullptr,nullptr);
    SendMessageW(c,WM_SETFONT,(WPARAM)font,TRUE);return c;
}
#include "controls_window.hpp"
static void UpdateImageControls(bool save) {
    SendMessageW(brightnessSlider,TBM_SETPOS,TRUE,brightness+50);
    SendMessageW(contrastSlider,TBM_SETPOS,TRUE,contrast);
    SetWindowTextW(brightnessLabel,(std::to_wstring(brightness)+L" %").c_str());
    SetWindowTextW(contrastLabel,(std::to_wstring(contrast)+L" %").c_str());
    if(save) {
        bool ok=WritePrivateProfileStringW(L"video",L"brightness",std::to_wstring(brightness).c_str(),(app/L"launcher.ini").c_str())!=0;
        ok=WritePrivateProfileStringW(L"video",L"contrast",std::to_wstring(contrast).c_str(),(app/L"launcher.ini").c_str())!=0&&ok;
        if(!ok)MessageBoxW(GetParent(brightnessSlider),L"Não foi possível guardar os ajustes nesta pasta.",L"Imagem",MB_OK|MB_ICONERROR);
    }
}
static LRESULT CALLBACK Proc(HWND w,UINT m,WPARAM wp,LPARAM lp) {
    if(m==WM_CREATE) {
        font=CreateFontW(-18,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        Control(w,L"STATIC",L"Street Fighter Zero 2 Alpha",0,24,20,570,30);
        Control(w,L"STATIC",L"Versão experimental para Windows — Ásia 960826",0,24,55,570,26);
        Control(w,L"STATIC",L"Arquivo do jogo",0,24,102,570,24);
        romBox=Control(w,L"EDIT",rom.c_str(),WS_BORDER|ES_READONLY|ES_AUTOHSCROLL,24,130,570,30);
        Control(w,L"BUTTON",L"Escolher ZIP…",WS_TABSTOP,24,174,160,38,1);
        Control(w,L"STATIC",L"Tema da tela",0,24,232,160,26);
        themeBox=Control(w,L"COMBOBOX",L"",WS_TABSTOP|CBS_DROPDOWNLIST,200,226,394,160,5);
        for(auto name:{L"Imagem limpa",L"Fliperama",L"TV de tubo",L"Gabinete clássico"})SendMessageW(themeBox,CB_ADDSTRING,0,(LPARAM)name);
        SendMessageW(themeBox,CB_SETCURSEL,selectedTheme,0);
        Control(w,L"STATIC",L"Brilho da imagem",0,24,283,160,26);
        brightnessSlider=Control(w,TRACKBAR_CLASSW,L"",WS_TABSTOP|TBS_HORZ,200,275,310,36,10);
        SendMessageW(brightnessSlider,TBM_SETRANGE,TRUE,MAKELPARAM(0,100));
        brightnessLabel=Control(w,L"STATIC",L"",0,530,283,80,26);
        Control(w,L"STATIC",L"Contraste",0,24,327,160,26);
        contrastSlider=Control(w,TRACKBAR_CLASSW,L"",WS_TABSTOP|TBS_HORZ,200,319,310,36,11);
        SendMessageW(contrastSlider,TBM_SETRANGE,TRUE,MAKELPARAM(50,150));
        contrastLabel=Control(w,L"STATIC",L"",0,530,327,80,26);
        Control(w,L"BUTTON",L"Restaurar imagem",WS_TABSTOP,200,363,210,34,6);
        Control(w,L"BUTTON",L"Jogar",WS_TABSTOP|BS_DEFPUSHBUTTON,24,422,180,52,2);
        Control(w,L"BUTTON",L"Treino",WS_TABSTOP,218,422,180,52,3);
        Control(w,L"BUTTON",L"Controles",WS_TABSTOP,412,422,182,52,4);
        Control(w,L"STATIC",L"Brilho e contraste também mudam no jogo aberto.\r\nF8: temas (se livre). F11: tela cheia. Esc: opções.\r\nTreino: tempo infinito e reposição de vida em repouso.",0,24,494,570,78);
        UpdateImageControls(false);
        return 0;
    }
    if(m==WM_HSCROLL) {
        if((HWND)lp==brightnessSlider)brightness=int(SendMessageW(brightnessSlider,TBM_GETPOS,0,0))-50;
        else if((HWND)lp==contrastSlider)contrast=int(SendMessageW(contrastSlider,TBM_GETPOS,0,0));
        else return 0;
        UpdateImageControls(true);return 0;
    }
    if(m==WM_COMMAND) {
        switch(LOWORD(wp)) {
        case 1:Choose(w);break;
        case 2:Play(w,false);break;
        case 3:Play(w,true);break;
        case 4:OpenControls(w);break;
        case 5:if(HIWORD(wp)==CBN_SELCHANGE) {
            selectedTheme=int(SendMessageW(themeBox,CB_GETCURSEL,0,0));
            if(selectedTheme<0||selectedTheme>3)selectedTheme=0;
            if(!WritePrivateProfileStringW(L"video",L"theme",themeCodes[selectedTheme],(app/L"launcher.ini").c_str()))
                MessageBoxW(w,L"O tema vale nesta sessão, mas não foi possível guardar a escolha nesta pasta.",L"Tema da tela",MB_OK|MB_ICONWARNING);
        }break;
        case 6:brightness=0;contrast=100;UpdateImageControls(true);break;
        }return 0;
    }
    if(m==WM_DESTROY) {DeleteObject(font);PostQuitMessage(0);return 0;}
    return DefWindowProcW(w,m,wp,lp);
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,LPWSTR commandLine,int show) {
    SetProcessDPIAware();wchar_t path[32768];GetModuleFileNameW(nullptr,path,32768);app=fs::path(path).parent_path();
    wchar_t saved[32768];GetPrivateProfileStringW(L"jogo",L"rom",defaultRom,saved,32768,(app/L"launcher.ini").c_str());rom=saved;
    wchar_t theme[32];GetPrivateProfileStringW(L"video",L"theme",L"clean",theme,32,(app/L"launcher.ini").c_str());
    for(int i=0;i<4;++i)if(std::wstring(theme)==themeCodes[i])selectedTheme=i;
    brightness=std::clamp(int(GetPrivateProfileIntW(L"video",L"brightness",0,(app/L"launcher.ini").c_str())),-50,50);
    contrast=std::clamp(int(GetPrivateProfileIntW(L"video",L"contrast",100,(app/L"launcher.ini").c_str())),50,150);
    INITCOMMONCONTROLSEX common{sizeof(common),ICC_BAR_CLASSES};InitCommonControlsEx(&common);
    WNDCLASSW wc{};wc.lpfnWndProc=Proc;wc.hInstance=instance;wc.lpszClassName=L"SFZ2ALLocalLauncher";
    wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassW(&wc);
    HWND w=CreateWindowW(wc.lpszClassName,L"SFZ2AL — Iniciar",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,640,625,nullptr,nullptr,instance,nullptr);
    ShowWindow(w,show);if(std::wstring(commandLine)==L"--controls")OpenControls(w);
    MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){if(CaptureKey(msg))continue;if(!IsDialogMessageW(controlsWindow?controlsWindow:w,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}}
    return 0;
}
