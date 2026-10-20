// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "ra_client.hpp"
namespace online {
inline HWND window=nullptr,userBox=nullptr,passwordBox=nullptr,message=nullptr,listing=nullptr,enable=nullptr,login=nullptr,refresh=nullptr;
inline std::unique_ptr<sfz_ra::Client> client;
inline fs::path folder;
inline std::string game;
inline std::wstring lastList,lastStatus;
inline bool Enabled(){return GetPrivateProfileIntW(L"retroachievements",L"enabled",0,(app/L"launcher.ini").c_str())!=0;}
inline void StopSending(){std::error_code ec;fs::remove(folder/L"online.enabled",ec);WritePrivateProfileStringW(L"retroachievements",L"enabled",L"0",(app/L"launcher.ini").c_str());if(enable)SendMessageW(enable,BM_SETCHECK,BST_UNCHECKED,0);}
inline std::wstring Field(HWND control){int n=GetWindowTextLengthW(control);std::wstring text(size_t(n)+1,0);GetWindowTextW(control,text.data(),n+1);text.resize(n);return text;}
inline void Update(){
    if(!client)return;client->Idle();
    std::wstring state=sfz_ra::Wide(client->status);
    if(!client->username.empty())state=L"Conta: "+sfz_ra::Wide(client->username)+L"\r\n"+state;
    if(state!=lastStatus){SetWindowTextW(message,state.c_str());lastStatus=state;}
    bool pending=client->Pending();EnableWindow(login,!pending);EnableWindow(refresh,!pending);EnableWindow(enable,client->Connected()&&!client->Failed());
    const auto entries=client->List();int unlocked=0;for(auto &a:entries)unlocked+=a.unlocked;
    std::wstring text;
    if(!entries.empty())text=std::to_wstring(unlocked)+L" / "+std::to_wstring(entries.size())+L" conquistas — Softcore\r\n\r\n";
    for(const auto &a:entries){text+=a.unsupported?L"[Indisponível] ":a.unlocked?L"[Conquistada] ":L"[Bloqueada] ";text+=sfz_ra::Wide(a.title)+L" — "+std::to_wstring(a.points)+L" pontos\r\n"+sfz_ra::Wide(a.description)+L"\r\n\r\n";}
    if(entries.empty())text=L"Entre na sua conta para consultar as conquistas deste jogo.\r\n\r\nA consulta não concede conquistas.\r\n\r\nPara ganhá-las, ative a opção abaixo, feche o jogo se estiver aberto e inicie uma nova sessão por JOGAR.\r\n\r\nEsta integração é experimental e usa apenas Softcore.\r\n\r\nA galeria local funciona sem conta e mantém um progresso separado.";
    if(text!=lastList){SetWindowTextW(listing,text.c_str());lastList=text;}
}
inline void Reconnect(){
    client.reset();client=std::make_unique<sfz_ra::Client>(folder,true);lastList.clear();lastStatus.clear();
    if(!client->Resume(game))client->status="Desconectado. Use sua conta RetroAchievements.";
    Update();
}
inline LRESULT CALLBACK Proc(HWND w,UINT m,WPARAM wp,LPARAM lp){
    if(m==WM_DRAWITEM&&wp){const auto &d=*reinterpret_cast<DRAWITEMSTRUCT*>(lp);auto box=d.rcItem;ui::Fill(d.hDC,box,ui::panel);
        wchar_t label[100];GetWindowTextW(d.hwndItem,label,100);ui::Text(d.hDC,label,box,font,(d.itemState&ODS_DISABLED)?ui::muted:ui::ink,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        if(d.itemState&ODS_FOCUS){InflateRect(&box,-3,-3);DrawFocusRect(d.hDC,&box);}return TRUE;}
    LRESULT styled;if(ui::Message(w,m,wp,lp,styled))return styled;
    if(m==WM_CREATE){window=w;
        Control(w,L"STATIC",L"RETROACHIEVEMENTS  /  SOFTCORE EXPERIMENTAL",0,22,15,708,26);
        Control(w,L"STATIC",L"Usuário",0,22,57,90,26);userBox=Control(w,L"EDIT",L"",WS_BORDER|WS_TABSTOP|ES_AUTOHSCROLL,116,53,250,30,10);SendMessageW(userBox,EM_SETLIMITTEXT,128,0);
        Control(w,L"STATIC",L"Senha",0,22,94,90,26);passwordBox=Control(w,L"EDIT",L"",WS_BORDER|WS_TABSTOP|ES_PASSWORD|ES_AUTOHSCROLL,116,90,250,30,11);SendMessageW(passwordBox,EM_SETLIMITTEXT,256,0);
        login=Control(w,L"BUTTON",L"Entrar",WS_TABSTOP,386,53,145,30,1);
        Control(w,L"BUTTON",L"Sair da conta",WS_TABSTOP,548,53,180,30,2);
        refresh=Control(w,L"BUTTON",L"Atualizar lista",WS_TABSTOP,386,90,145,30,3);
        Control(w,L"STATIC",L"A senha não é salva. O token é protegido pelo Windows.",0,22,133,706,26);
        message=Control(w,L"STATIC",L"",0,22,166,706,51);
        listing=Control(w,L"EDIT",L"",WS_BORDER|ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL|WS_TABSTOP,22,224,706,284,20);
        enable=Control(w,L"BUTTON",L"Ativar Softcore ao jogar: enviar conquistas para minha conta",BS_AUTOCHECKBOX|WS_TABSTOP,22,520,706,30,4);
        SendMessageW(enable,BM_SETCHECK,Enabled()?BST_CHECKED:BST_UNCHECKED,0);
        Control(w,L"STATIC",L"Treino não envia conquistas. Alterações de vida/tempo pausam o serviço.\r\nInternet necessária. A conta pode ser criada em retroachievements.org.",0,22,556,706,52);
        Control(w,L"BUTTON",L"Fechar",WS_TABSTOP,582,618,146,36,5);
        Reconnect();SetTimer(w,1,80,nullptr);return 0;
    }
    if(m==WM_TIMER){Update();return 0;}
    if(m==WM_COMMAND){switch(LOWORD(wp)){
        case 1:{auto user=sfz_ra::Utf8(Field(userBox));auto secret=Field(passwordBox);
            if(user.empty()||secret.empty()){MessageBoxW(w,L"Digite seu usuário e senha nesta janela.",L"RetroAchievements",MB_OK);return 0;}
            StopSending();client.reset();client=std::make_unique<sfz_ra::Client>(folder,true);client->Login(user,sfz_ra::Utf8(secret),game);
            SecureZeroMemory(secret.data(),secret.size()*sizeof(wchar_t));SetWindowTextW(passwordBox,L"");lastStatus.clear();Update();break;}
        case 2:{StopSending();if(!sfz_ra::Forget(folder)){MessageBoxW(w,L"Envio desativado, mas não foi possível remover o token desta pasta.",L"RetroAchievements",MB_OK|MB_ICONERROR);return 0;}
            WritePrivateProfileStringW(L"retroachievements",L"enabled",L"0",(app/L"launcher.ini").c_str());SendMessageW(enable,BM_SETCHECK,BST_UNCHECKED,0);
            Reconnect();SetWindowTextW(passwordBox,L"");SetWindowTextW(userBox,L"");MessageBoxW(w,L"Conta desconectada. Se o jogo estava aberto, feche-o para encerrar também aquela sessão.",L"RetroAchievements",MB_OK);break;}
        case 3:Reconnect();break;
        case 4:{bool checked=SendMessageW(enable,BM_GETCHECK,0,0)==BST_CHECKED;
            if(checked){std::error_code ec;fs::create_directories(folder,ec);std::ofstream flag(folder/L"online.enabled");flag<<"SFZ2AL-SOFTCORE-OPT-IN\n";flag.close();if(!flag.good()){StopSending();MessageBoxW(w,L"Não foi possível ativar o envio nesta pasta.",L"RetroAchievements",MB_OK|MB_ICONERROR);return 0;}}
            else StopSending();
            if(!WritePrivateProfileStringW(L"retroachievements",L"enabled",checked?L"1":L"0",(app/L"launcher.ini").c_str()))MessageBoxW(w,L"Não foi possível salvar a preferência nesta pasta.",L"RetroAchievements",MB_OK|MB_ICONERROR);break;}
        case 5:DestroyWindow(w);break;
        }return 0;}
    if(m==WM_DESTROY){KillTimer(w,1);client.reset();window=nullptr;return 0;}
    return DefWindowProcW(w,m,wp,lp);
}
inline void Open(HWND owner){
    if(window){ShowWindow(window,SW_RESTORE);SetForegroundWindow(window);return;}
    folder=app/L"user/retroachievements";game=selectedVersion==1?"sfz2alb":"sfz2al";
    WNDCLASSW wc{};wc.lpfnWndProc=Proc;wc.hInstance=GetModuleHandleW(nullptr);wc.lpszClassName=L"SFZ2ALAchievements";wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.hbrBackground=ui::bgBrush;wc.hIcon=LoadIconW(wc.hInstance,MAKEINTRESOURCEW(1));RegisterClassW(&wc);
    RECT r{0,0,750,676};AdjustWindowRect(&r,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,FALSE);
    window=CreateWindowW(wc.lpszClassName,L"SFZ2AL — Conquistas online",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,owner,nullptr,wc.hInstance,nullptr);
    ui::Caption(window);ShowWindow(window,SW_SHOW);SetFocus(userBox);
}
inline void Close(){if(window)DestroyWindow(window);}
}
