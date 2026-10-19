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
#include "launcher_style.hpp"
#include "game_preview.hpp"

namespace fs = std::filesystem;
static fs::path app;
static HWND romBox;
static HWND themeBox;
static int selectedTheme=0;
static const wchar_t *themeCodes[]={L"clean",L"arcade",L"crt",L"cabinet"};
static HWND brightnessSlider,contrastSlider,brightnessLabel,contrastLabel;
static int brightness=0,contrast=100;
static HWND soundToggle,soundSlider,soundLabel;
static HWND previewToggle;
static GamePreview preview;
static HBITMAP heroCache;
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
    preview.Pause(true);SetWindowTextW(previewToggle,L"Retomar prévia");
    ui::Sound(menu_audio::Cue::Launch);
}
static HWND Control(HWND w,const wchar_t *type,const wchar_t *text,DWORD style,int x,int y,int width,int height,int id=0) {
    if(std::wstring(type)==L"BUTTON"&&((style&BS_TYPEMASK)==BS_PUSHBUTTON||(style&BS_TYPEMASK)==BS_DEFPUSHBUTTON))
        style=(style&~BS_TYPEMASK)|BS_OWNERDRAW;
    HWND c=CreateWindowW(type,text,WS_CHILD|WS_VISIBLE|style,x,y,width,height,w,(HMENU)(INT_PTR)id,nullptr,nullptr);
    SendMessageW(c,WM_SETFONT,(WPARAM)font,TRUE);ui::Skin(c);return c;
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
    LRESULT styled;if(ui::Message(w,m,wp,lp,styled))return styled;
    if(m==WM_PAINT) {
        PAINTSTRUCT p;HDC dc=BeginPaint(w,&p);RECT r;GetClientRect(w,&r);
        HDC memory=CreateCompatibleDC(dc);
        bool create=heroCache==nullptr;if(create)heroCache=CreateCompatibleBitmap(dc,r.right,r.bottom);
        auto previous=SelectObject(memory,heroCache);if(create)ui::PaintHero(memory);
        preview.Draw(memory);
        BitBlt(dc,0,0,r.right,r.bottom,memory,0,0,SRCCOPY);
        SelectObject(memory,previous);DeleteDC(memory);EndPaint(w,&p);return 0;
    }
    if(m==WM_CREATE) {
        font=CreateFontW(-18,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        Control(w,L"STATIC",L"SEU JOGO",0,24,184,550,24);
        romBox=Control(w,L"EDIT",rom.c_str(),WS_BORDER|ES_READONLY|ES_AUTOHSCROLL,24,211,424,32);
        Control(w,L"BUTTON",L"Escolher ZIP",WS_TABSTOP,460,211,114,32,1);
        Control(w,L"STATIC",L"Tema da tela",0,24,270,160,26);
        themeBox=Control(w,L"COMBOBOX",L"",WS_TABSTOP|CBS_DROPDOWNLIST,200,264,374,160,5);
        for(auto name:{L"Imagem limpa",L"Fliperama",L"TV de tubo",L"Gabinete clássico"})SendMessageW(themeBox,CB_ADDSTRING,0,(LPARAM)name);
        SendMessageW(themeBox,CB_SETCURSEL,selectedTheme,0);
        Control(w,L"STATIC",L"Brilho da imagem",0,24,316,160,26);
        brightnessSlider=Control(w,TRACKBAR_CLASSW,L"",WS_TABSTOP|TBS_HORZ,200,308,270,36,10);
        SendMessageW(brightnessSlider,TBM_SETRANGE,TRUE,MAKELPARAM(0,100));
        brightnessLabel=Control(w,L"STATIC",L"",0,494,316,80,26);
        Control(w,L"STATIC",L"Contraste",0,24,360,160,26);
        contrastSlider=Control(w,TRACKBAR_CLASSW,L"",WS_TABSTOP|TBS_HORZ,200,352,270,36,11);
        SendMessageW(contrastSlider,TBM_SETRANGE,TRUE,MAKELPARAM(50,150));
        contrastLabel=Control(w,L"STATIC",L"",0,494,360,80,26);
        Control(w,L"BUTTON",L"Restaurar imagem",WS_TABSTOP,200,400,210,34,6);
        Control(w,L"BUTTON",L"JOGAR",WS_TABSTOP|BS_DEFPUSHBUTTON,24,466,174,52,2);
        Control(w,L"BUTTON",L"Treino",WS_TABSTOP,212,466,174,52,3);
        Control(w,L"BUTTON",L"Controles",WS_TABSTOP,400,466,174,52,4);
        Control(w,L"STATIC",L"F8: temas (se livre). F11: tela cheia. Esc: opções.\r\nBrilho e contraste também mudam no jogo aberto.\r\nTreino: tempo infinito e reposição de vida em repouso.",0,24,530,550,72);
        soundToggle=Control(w,L"BUTTON",L"Sons do menu",WS_TABSTOP|BS_AUTOCHECKBOX,24,623,190,30,7);
        SendMessageW(soundToggle,BM_SETCHECK,ui::soundEnabled?BST_CHECKED:BST_UNCHECKED,0);
        Control(w,L"STATIC",L"Volume",0,230,626,70,26);
        soundSlider=Control(w,TRACKBAR_CLASSW,L"",WS_TABSTOP|TBS_HORZ,310,618,220,36,12);
        SendMessageW(soundSlider,TBM_SETRANGE,TRUE,MAKELPARAM(0,100));SendMessageW(soundSlider,TBM_SETPOS,TRUE,ui::volume);
        soundLabel=Control(w,L"STATIC",(std::to_wstring(ui::volume)+L" %").c_str(),0,552,626,80,26);
        bool ready=preview.Start(app/L"user/previews");
        previewToggle=Control(w,L"BUTTON",ready?L"Pausar prévia":L"Prévia não preparada",WS_TABSTOP,620,552,252,34,8);
        EnableWindow(previewToggle,ready);
        SetTimer(w,2,20,nullptr);
        UpdateImageControls(false);
        return 0;
    }
    if(m==WM_TIMER&&wp==2) {
        if(!IsIconic(w)&&preview.Tick()){RECT r{602,190,890,594};InvalidateRect(w,&r,FALSE);}
        return 0;
    }
    if(m==WM_HSCROLL) {
        if((HWND)lp==soundSlider) {
            int value=int(SendMessageW(soundSlider,TBM_GETPOS,0,0));
            if(value!=ui::volume){ui::volume=value;ui::RebuildSounds();SetWindowTextW(soundLabel,(std::to_wstring(value)+L" %").c_str());WritePrivateProfileStringW(L"interface",L"volume",std::to_wstring(value).c_str(),(app/L"launcher.ini").c_str());}
            if(LOWORD(wp)==TB_ENDTRACK)ui::Sound(menu_audio::Cue::Confirm);
            return 0;
        }
        if((HWND)lp==brightnessSlider)brightness=int(SendMessageW(brightnessSlider,TBM_GETPOS,0,0))-50;
        else if((HWND)lp==contrastSlider)contrast=int(SendMessageW(contrastSlider,TBM_GETPOS,0,0));
        else return 0;
        UpdateImageControls(true);if(LOWORD(wp)==TB_ENDTRACK)ui::Sound(menu_audio::Cue::Confirm);return 0;
    }
    if(m==WM_COMMAND) {
        switch(LOWORD(wp)) {
        case 1:ui::Sound(menu_audio::Cue::Confirm);Choose(w);break;
        case 2:Play(w,false);break;
        case 3:Play(w,true);break;
        case 4:ui::Sound(menu_audio::Cue::Confirm);OpenControls(w);break;
        case 5:if(HIWORD(wp)==CBN_SELCHANGE) {
            selectedTheme=int(SendMessageW(themeBox,CB_GETCURSEL,0,0));
            ui::Sound(menu_audio::Cue::Confirm);
            if(selectedTheme<0||selectedTheme>3)selectedTheme=0;
            if(!WritePrivateProfileStringW(L"video",L"theme",themeCodes[selectedTheme],(app/L"launcher.ini").c_str()))
                MessageBoxW(w,L"O tema vale nesta sessão, mas não foi possível guardar a escolha nesta pasta.",L"Tema da tela",MB_OK|MB_ICONWARNING);
        }break;
        case 6:brightness=0;contrast=100;UpdateImageControls(true);ui::Sound(menu_audio::Cue::Back);break;
        case 7:ui::soundEnabled=SendMessageW(soundToggle,BM_GETCHECK,0,0)==BST_CHECKED;
            WritePrivateProfileStringW(L"interface",L"sounds",ui::soundEnabled?L"1":L"0",(app/L"launcher.ini").c_str());
            if(ui::soundEnabled)ui::Sound(menu_audio::Cue::Confirm);else PlaySoundW(nullptr,nullptr,0);break;
        case 8:preview.Pause(!preview.Paused());SetWindowTextW(previewToggle,preview.Paused()?L"Retomar prévia":L"Pausar prévia");
            ui::Sound(menu_audio::Cue::Confirm);InvalidateRect(w,nullptr,FALSE);break;
        }return 0;
    }
    if(m==WM_DESTROY) {KillTimer(w,2);preview.Close();DeleteObject(heroCache);DeleteObject(font);ui::Close();PostQuitMessage(0);return 0;}
    return DefWindowProcW(w,m,wp,lp);
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,LPWSTR commandLine,int show) {
    SetProcessDPIAware();wchar_t path[32768];GetModuleFileNameW(nullptr,path,32768);app=fs::path(path).parent_path();
    wchar_t saved[32768];GetPrivateProfileStringW(L"jogo",L"rom",defaultRom,saved,32768,(app/L"launcher.ini").c_str());rom=saved;
    wchar_t theme[32];GetPrivateProfileStringW(L"video",L"theme",L"clean",theme,32,(app/L"launcher.ini").c_str());
    for(int i=0;i<4;++i)if(std::wstring(theme)==themeCodes[i])selectedTheme=i;
    brightness=std::clamp(int(GetPrivateProfileIntW(L"video",L"brightness",0,(app/L"launcher.ini").c_str())),-50,50);
    contrast=std::clamp(int(GetPrivateProfileIntW(L"video",L"contrast",100,(app/L"launcher.ini").c_str())),50,150);
    ui::soundEnabled=GetPrivateProfileIntW(L"interface",L"sounds",1,(app/L"launcher.ini").c_str())!=0;
    ui::volume=std::clamp(int(GetPrivateProfileIntW(L"interface",L"volume",25,(app/L"launcher.ini").c_str())),0,100);
    ui::Init();
    INITCOMMONCONTROLSEX common{sizeof(common),ICC_BAR_CLASSES};InitCommonControlsEx(&common);
    WNDCLASSW wc{};wc.lpfnWndProc=Proc;wc.hInstance=instance;wc.lpszClassName=L"SFZ2ALLocalLauncher";
    wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.hbrBackground=ui::bgBrush;RegisterClassW(&wc);
    RECT size{0,0,920,710};AdjustWindowRect(&size,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,FALSE);
    HWND w=CreateWindowW(wc.lpszClassName,L"SFZ2AL — Iniciar",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,size.right-size.left,size.bottom-size.top,nullptr,nullptr,instance,nullptr);
    ui::Caption(w);ShowWindow(w,show);if(std::wstring(commandLine)==L"--controls")OpenControls(w);
    MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){if(CaptureKey(msg))continue;if(!IsDialogMessageW(controlsWindow?controlsWindow:w,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}}
    return 0;
}
