// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "controls_config.hpp"

static HWND controlsWindow, playerChoice, deviceChoice, controlsStatus, padStatus;
static HWND bindingRows[port::kBoundActionCount], captureRows[port::kBoundActionCount];
static port::Bindings draft;
static int currentPlayer=0,currentDevice=0,capturing=-1;
static bool captureReleased=false, sdlReady=false;
static SDL_Gamepad *pads[2]{};
static const wchar_t *actionNames[]={L"Cima",L"Baixo",L"Esquerda",L"Direita",L"Soco fraco",L"Soco médio",L"Soco forte",L"Chute fraco",L"Chute médio",L"Chute forte",L"Iniciar",L"Crédito",L"3 socos",L"3 chutes"};
static std::vector<port::PadInput> padOptions;
static std::wstring Wide(const char *s) {
    if(!s)return L"";
    int n=MultiByteToWideChar(CP_UTF8,0,s,-1,nullptr,0);
    std::wstring out(n,L'\0');MultiByteToWideChar(CP_UTF8,0,s,-1,out.data(),n);if(n)out.pop_back();return out;
}
static std::wstring PadLabel(port::PadInput p) {
    if(p.axis==SDL_GAMEPAD_AXIS_LEFT_TRIGGER)return L"Gatilho esquerdo (LT / L2)";
    if(p.axis==SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)return L"Gatilho direito (RT / R2)";
    switch(p.button) {
    case SDL_GAMEPAD_BUTTON_SOUTH:return L"Inferior (A / Cruz)";
    case SDL_GAMEPAD_BUTTON_EAST:return L"Direito (B / Círculo)";
    case SDL_GAMEPAD_BUTTON_WEST:return L"Esquerdo (X / Quadrado)";
    case SDL_GAMEPAD_BUTTON_NORTH:return L"Superior (Y / Triângulo)";
    case SDL_GAMEPAD_BUTTON_BACK:return L"Voltar / Select";
    case SDL_GAMEPAD_BUTTON_START:return L"Start / Options";
    case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:return L"Ombro esquerdo (LB / L1)";
    case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:return L"Ombro direito (RB / R1)";
    case SDL_GAMEPAD_BUTTON_DPAD_UP:return L"Direcional: cima";
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN:return L"Direcional: baixo";
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT:return L"Direcional: esquerda";
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:return L"Direcional: direita";
    default:return p.button<0?L"Sem botão":Wide(SDL_GetGamepadStringForButton(SDL_GamepadButton(p.button)));
    }
}
static void RefreshBindings() {
    for(int a=0;a<port::kBoundActionCount;++a) {
        if(currentDevice==0)SetWindowTextW(bindingRows[a],draft.key[currentPlayer][a]==SDL_SCANCODE_UNKNOWN?L"Sem tecla":Wide(SDL_GetScancodeName(draft.key[currentPlayer][a])).c_str());
        else {
            int selected=0;for(size_t i=0;i<padOptions.size();++i)if(padOptions[i]==draft.pad[currentPlayer][a])selected=int(i);
            SendMessageW(captureRows[a],CB_SETCURSEL,selected,0);
        }
        ShowWindow(bindingRows[a],currentDevice==0?SW_SHOW:SW_HIDE);
        ShowWindow(captureRows[a],currentDevice==1?SW_SHOW:SW_HIDE);
    }
}
static void CancelCapture() {capturing=-1;SetWindowTextW(controlsStatus,L"Escolha uma ação. Entradas repetidas trocam de lugar no mesmo jogador.");}
static void BeginCapture(int a) {
    ui::Sound(menu_audio::Cue::Confirm);
    capturing=a;captureReleased=false;
    std::wstring text=std::wstring(actionNames[a])+L": pressione "+std::wstring(currentDevice?L"um botão ou gatilho. Esc cancela.":L"uma tecla. Esc cancela; Delete remove a tecla.");
    SetWindowTextW(controlsStatus,text.c_str());SetFocus(controlsWindow);
}
static bool CaptureKey(MSG &msg) {
    if(!controlsWindow||capturing<0||(msg.message!=WM_KEYDOWN&&msg.message!=WM_SYSKEYDOWN))return false;
    if(msg.hwnd!=controlsWindow&&!IsChild(controlsWindow,msg.hwnd))return false;
    if(msg.wParam==VK_ESCAPE){CancelCapture();return true;}
    if(currentDevice)return true;
    auto key=msg.wParam==VK_DELETE?SDL_SCANCODE_UNKNOWN:controls::FromWindowsKey(msg.wParam,msg.lParam);
    if(key==SDL_SCANCODE_UNKNOWN&&msg.wParam!=VK_DELETE){SetWindowTextW(controlsStatus,L"Tecla não reconhecida. Tente outra; Esc cancela.");return true;}
    if(!controls::AssignKey(draft,currentPlayer,capturing,key)){SetWindowTextW(controlsStatus,L"F1 e F11 são reservadas para o menu e tela cheia. Escolha outra tecla.");return true;}
    CancelCapture();RefreshBindings();
    if(key!=SDL_SCANCODE_UNKNOWN)for(int a=0;a<port::kBoundActionCount;++a)if(draft.key[1-currentPlayer][a]==key)SetWindowTextW(controlsStatus,L"Atenção: o outro jogador também usa essa tecla.");
    return true;
}
static void PollPads() {
    if(!sdlReady)return;
    SDL_Event event;while(SDL_PollEvent(&event)){}SDL_UpdateGamepads();
    int count=0;auto ids=SDL_GetGamepads(&count);
    for(int p=0;p<2;++p) {
        SDL_JoystickID id=p<count?ids[p]:0;
        if(pads[p]&&(!SDL_GamepadConnected(pads[p])||SDL_GetGamepadID(pads[p])!=id)){SDL_CloseGamepad(pads[p]);pads[p]=nullptr;}
        if(!pads[p]&&id)pads[p]=SDL_OpenGamepad(id);
    }
    SDL_free(ids);
    std::wstring status=L"Controle do jogador "+std::to_wstring(currentPlayer+1)+L": ";
    status+=pads[currentPlayer]?Wide(SDL_GetGamepadName(pads[currentPlayer])):L"não conectado (escolha os botões pela lista)";
    wchar_t previous[1024]{};GetWindowTextW(padStatus,previous,1024);
    if(status!=previous)SetWindowTextW(padStatus,status.c_str());
    if(capturing<0||!currentDevice||GetForegroundWindow()!=controlsWindow||!pads[currentPlayer])return;
    port::PadInput pressed;
    for(auto option:padOptions) {
        bool down=option.button>=0?SDL_GetGamepadButton(pads[currentPlayer],SDL_GamepadButton(option.button)):option.axis>=0&&SDL_GetGamepadAxis(pads[currentPlayer],SDL_GamepadAxis(option.axis))>16000;
        if(down){pressed=option;break;}
    }
    if(pressed==port::PadInput{}){captureReleased=true;return;}
    if(captureReleased){controls::AssignPad(draft,currentPlayer,capturing,pressed);CancelCapture();RefreshBindings();}
}
static LRESULT CALLBACK ControlsProc(HWND w,UINT m,WPARAM wp,LPARAM lp) {
    LRESULT styled;if(ui::Message(w,m,wp,lp,styled))return styled;
    if(m==WM_CREATE) {
        controlsWindow=w;draft=controls::Load(app);currentPlayer=0;currentDevice=0;capturing=-1;
        Control(w,L"STATIC",L"Configure os dois jogadores",0,20,15,610,25);
        playerChoice=Control(w,L"COMBOBOX",L"",WS_TABSTOP|CBS_DROPDOWNLIST,20,48,190,120,20);
        deviceChoice=Control(w,L"COMBOBOX",L"",WS_TABSTOP|CBS_DROPDOWNLIST,225,48,210,120,21);
        for(auto name:{L"Jogador 1",L"Jogador 2"})SendMessageW(playerChoice,CB_ADDSTRING,0,(LPARAM)name);
        for(auto name:{L"Teclado",L"Controle"})SendMessageW(deviceChoice,CB_ADDSTRING,0,(LPARAM)name);
        SendMessageW(playerChoice,CB_SETCURSEL,0,0);SendMessageW(deviceChoice,CB_SETCURSEL,0,0);
        padStatus=Control(w,L"STATIC",L"",0,20,87,610,42);
        padOptions={{}};for(int i=0;i<SDL_GAMEPAD_BUTTON_COUNT;++i)padOptions.push_back({i,-1});
        padOptions.push_back({-1,SDL_GAMEPAD_AXIS_LEFT_TRIGGER});padOptions.push_back({-1,SDL_GAMEPAD_AXIS_RIGHT_TRIGGER});
        for(int a=0;a<port::kBoundActionCount;++a) {
            int y=135+a*31;
            Control(w,L"STATIC",actionNames[a],0,20,y+4,145,25);
            bindingRows[a]=Control(w,L"BUTTON",L"",WS_TABSTOP,170,y,310,28,100+a);
            captureRows[a]=Control(w,L"COMBOBOX",L"",WS_TABSTOP|CBS_DROPDOWNLIST|WS_VSCROLL,170,y,310,270,200+a);
            for(auto option:padOptions)SendMessageW(captureRows[a],CB_ADDSTRING,0,(LPARAM)PadLabel(option).c_str());
            Control(w,L"BUTTON",L"Capturar",WS_TABSTOP,492,y,125,28,300+a);
        }
        controlsStatus=Control(w,L"STATIC",L"",0,20,576,600,46);
        Control(w,L"STATIC",L"O 1º controle é do jogador 1; o 2º é do jogador 2.\r\nO analógico esquerdo mantém o movimento. Reinicie o jogo após salvar.",0,20,627,600,48);
        Control(w,L"BUTTON",L"Restaurar padrões",WS_TABSTOP,20,687,190,36,30);
        Control(w,L"BUTTON",L"Cancelar",WS_TABSTOP,360,687,120,36,31);
        Control(w,L"BUTTON",L"Salvar",WS_TABSTOP|BS_DEFPUSHBUTTON,495,687,120,36,32);
        // Win32 owns keyboard dispatch, including the remapping capture hook.
        SDL_SetHint(SDL_HINT_WINDOWS_ENABLE_MESSAGELOOP,"0");
        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");sdlReady=SDL_Init(SDL_INIT_GAMEPAD);
        if(!sdlReady)SetWindowTextW(padStatus,L"Captura de controle indisponível. Configure os botões pelas listas.");
        CancelCapture();RefreshBindings();SetTimer(w,1,50,nullptr);return 0;
    }
    if(m==WM_TIMER){PollPads();return 0;}
    if(m==WM_COMMAND) {
        int id=LOWORD(wp),notification=HIWORD(wp);
        if((id==20||id==21)&&notification==CBN_SELCHANGE){CancelCapture();currentPlayer=int(SendMessageW(playerChoice,CB_GETCURSEL,0,0));currentDevice=int(SendMessageW(deviceChoice,CB_GETCURSEL,0,0));RefreshBindings();}
        else if(id>=100&&id<100+port::kBoundActionCount)BeginCapture(id-100);
        else if(id>=300&&id<300+port::kBoundActionCount)BeginCapture(id-300);
        else if(id>=200&&id<200+port::kBoundActionCount&&notification==CBN_SELCHANGE){CancelCapture();int choice=int(SendMessageW(captureRows[id-200],CB_GETCURSEL,0,0));if(choice>=0)controls::AssignPad(draft,currentPlayer,id-200,padOptions[choice]);RefreshBindings();}
        else if(id==30){auto defaults=controls::Defaults();for(int a=0;a<port::kBoundActionCount;++a)if(currentDevice)draft.pad[currentPlayer][a]=defaults.pad[currentPlayer][a];else draft.key[currentPlayer][a]=defaults.key[currentPlayer][a];CancelCapture();RefreshBindings();}
        else if(id==31){ui::Sound(menu_audio::Cue::Back);SendMessageW(w,WM_CLOSE,0,0);}
        else if(id==32){std::wstring error;if(controls::Save(app,draft,error)){ui::Sound(menu_audio::Cue::Confirm);DestroyWindow(w);}else MessageBoxW(w,error.c_str(),L"Controles",MB_OK|MB_ICONERROR);}
        return 0;
    }
    if(m==WM_CLOSE){DestroyWindow(w);return 0;}
    if(m==WM_DESTROY){KillTimer(w,1);for(auto &pad:pads)if(pad){SDL_CloseGamepad(pad);pad=nullptr;}if(sdlReady)SDL_Quit();sdlReady=false;controlsWindow=nullptr;EnableWindow(GetWindow(w,GW_OWNER),TRUE);SetForegroundWindow(GetWindow(w,GW_OWNER));return 0;}
    return DefWindowProcW(w,m,wp,lp);
}
static void OpenControls(HWND owner) {
    if(controlsWindow){SetForegroundWindow(controlsWindow);return;}
    WNDCLASSW wc{};wc.lpfnWndProc=ControlsProc;wc.hInstance=GetModuleHandleW(nullptr);wc.lpszClassName=L"SFZ2ALControls";wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.hbrBackground=ui::bgBrush;RegisterClassW(&wc);
    RECT r{0,0,640,742};AdjustWindowRect(&r,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,FALSE);
    EnableWindow(owner,FALSE);
    HWND w=CreateWindowExW(WS_EX_APPWINDOW,wc.lpszClassName,L"SFZ2AL — Controles",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,owner,nullptr,wc.hInstance,nullptr);
    if(w){ui::Caption(w);ShowWindow(w,SW_SHOW);RedrawWindow(w,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);}else EnableWindow(owner,TRUE);
}
