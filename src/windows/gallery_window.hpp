// SPDX-License-Identifier: GPL-3.0-only
// Local ROM portraits are generated on the user's machine, never embedded.
#pragma once
#include "gallery_data.hpp"
#include <atomic>
#include <fstream>
#include <thread>

namespace gallery {
inline HWND window=nullptr,details=nullptr,status=nullptr,prepare=nullptr,bonus=nullptr;
inline fs::path folder,art,gamePath,romPath;
inline int selected=0;
inline bool reward=false;
inline std::atomic<bool> busy=false,cancel=false;
inline std::thread worker;
inline ULONG_PTR graphicsToken=0;
inline bool Unlocked(int id){
    std::ifstream in(folder/L"progress"/(std::to_wstring(id)+L".clear"));
    std::string marker;std::getline(in,marker);return marker=="SFZ2AL-ARCADE-CLEAR-1";
}
inline int Count(){int n=0;for(int i=0;i<18;++i)n+=Unlocked(i);return n;}
inline int Encoder(CLSID &id){
    UINT n=0,size=0;Gdiplus::GetImageEncodersSize(&n,&size);if(!size)return -1;
    std::vector<unsigned char> buffer(size);auto codecs=reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer.data());
    Gdiplus::GetImageEncoders(n,size,codecs);
    for(UINT i=0;i<n;++i)if(wcscmp(codecs[i].MimeType,L"image/png")==0){id=codecs[i].Clsid;return 0;}return -1;
}
inline bool Ready(){for(int i=0;i<18;++i)if(!fs::is_regular_file(art/(std::to_wstring(i)+L".png")))return false;return true;}
inline void Portrait(Gdiplus::Graphics &g,int id,int x,int y,int width,int height,bool open){
    if(!open){Gdiplus::SolidBrush bg(Gdiplus::Color(255,22,30,47)),silhouette(Gdiplus::Color(255,51,65,85));
        g.FillRectangle(&bg,x,y,width,height);g.FillEllipse(&silhouette,x+width/3,y+height/9,width/3,height/3);
        g.FillEllipse(&silhouette,x+width/7,y+height/2,width*5/7,height*3/5);return;}
    Gdiplus::Image image((art/(std::to_wstring(id)+L".png")).c_str());
    if(image.GetLastStatus()==Gdiplus::Ok){
        Gdiplus::ImageAttributes attributes;
        if(!open){Gdiplus::ColorMatrix matrix={{{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,1,0},{0.07f,0.10f,0.15f,0,1}}};attributes.SetColorMatrix(&matrix);}
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
        g.DrawImage(&image,Gdiplus::Rect(x,y,width,height),0,0,int(image.GetWidth()),int(image.GetHeight()),Gdiplus::UnitPixel,&attributes);
    }else {Gdiplus::SolidBrush brush(Gdiplus::Color(255,24,34,50));g.FillRectangle(&brush,x,y,width,height);}
}
inline bool SaveReward(){
    if(Count()!=18||!Ready())return false;
    Gdiplus::Bitmap image(1440,960,PixelFormat32bppARGB);Gdiplus::Graphics g(&image);
    g.Clear(Gdiplus::Color(255,12,17,29));Gdiplus::SolidBrush ink(Gdiplus::Color(255,255,197,71));
    Gdiplus::FontFamily family(L"Segoe UI");Gdiplus::Font title(&family,38,Gdiplus::FontStyleBold,Gdiplus::UnitPixel),name(&family,20,Gdiplus::FontStyleBold,Gdiplus::UnitPixel);
    g.DrawString(L"STREET FIGHTER ZERO 2 ALPHA",-1,&title,Gdiplus::PointF(40,18),&ink);
    g.DrawString(L"ELENCO COMPLETO — 18 CAMPANHAS CONCLUIDAS",-1,&name,Gdiplus::PointF(40,72),&ink);
    for(int i=0;i<18;++i){int x=36+(i%6)*234,y=120+(i/6)*270;Portrait(g,i,x,y,205,224,true);g.DrawString(fighters[i].name,-1,&name,Gdiplus::PointF(float(x),float(y+230)),&ink);}
    g.DrawString(L"Painel composto localmente com retratos da sua ROM. Arte dos personagens: CAPCOM.",-1,&name,Gdiplus::PointF(36,928),&ink);
    CLSID encoder;return Encoder(encoder)==0&&image.Save((folder/L"recompensa.png").c_str(),&encoder)==Gdiplus::Ok;
}
inline void Refresh(){
    const auto &f=fighters[selected];const bool open=Unlocked(selected);
    std::wstring text=f.name;
    text+=open?L"\r\nCAMPANHA CONCLUIDA\r\n\r\n":L"\r\nHISTÓRIA BLOQUEADA\r\n\r\n";
    if(open)text+=std::wstring(L"Origem: ")+f.origin+L"\r\nEstilo: "+f.style+L"\r\n\r\n"+f.story+L"\r\n\r\n";
    else text+=L"Conclua o Arcade com este personagem para liberar sua imagem e história.\r\n\r\n";
    text+=L"GOLPES PRINCIPAIS\r\n\r\n";text+=f.moves;
    text+=L"\r\n\r\nDireções com o personagem olhando para a direita. Inverta esquerda/direita ao trocar de lado.\r\n\r\nP = soco; K = chute.\r\nL / M / H = fraco / médio / forte.\r\nPPP / KKK = três botões juntos.\r\n[ ] = segure para carregar.\r\n360° / 720° = uma / duas voltas.\r\nSupers exigem barra; PP/KK e PPP/KKK selecionam níveis maiores quando disponíveis.\r\n\r\nReferência da forma principal; não é a lista completa de golpes e variantes.";
    if(!reward){
        // The progress timer also calls Refresh. Replacing identical edit text
        // resets its scroll position, so leave the reader's view untouched.
        int length=GetWindowTextLengthW(details);
        std::wstring current(size_t(length)+1,L'\0');
        GetWindowTextW(details,current.data(),length+1);current.resize(length);
        if(current!=text)SetWindowTextW(details,text.c_str());
    }
    int count=Count();std::wstring progress=std::to_wstring(count)+L" / 18 personagens liberados";
    if(busy)progress+=L" — preparando retratos locais...";
    else if(!Ready())progress+=L" — prepare as imagens usando sua ROM";
    SetWindowTextW(status,progress.c_str());EnableWindow(bonus,count==18&&Ready()&&!busy);
    SetWindowTextW(bonus,count==18?L"Ver recompensa completa":L"Recompensa: complete 18 / 18");
    InvalidateRect(window,nullptr,FALSE);
}
inline bool Run(const std::vector<std::wstring> &args,const fs::path &working){
    std::wstring command;for(const auto &arg:args){if(!command.empty())command+=L' ';command+=Quote(arg);}
    STARTUPINFOW si{sizeof(si)};PROCESS_INFORMATION process{};
    if(!CreateProcessW(args.front().c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,working.c_str(),&si,&process))return false;
    CloseHandle(process.hThread);WaitForSingleObject(process.hProcess,INFINITE);
    DWORD exit=1;GetExitCodeProcess(process.hProcess,&exit);CloseHandle(process.hProcess);return exit==0;
}
inline void BuildImages(){
    if(busy||!fs::is_regular_file(romPath))return;
    if(worker.joinable())worker.join();busy=true;cancel=false;EnableWindow(prepare,FALSE);Refresh();
    const auto target=art,sourceRom=romPath,sourceGame=gamePath,working=app;
    const HWND notify=window;
    worker=std::thread([target,sourceRom,sourceGame,working,notify](){
        bool ok=true;ULONG_PTR token=0;Gdiplus::GdiplusStartupInput startup;
        if(Gdiplus::GdiplusStartup(&token,&startup,nullptr)!=Gdiplus::Ok){PostMessageW(notify,WM_APP+20,0,0);return;}
        try{
            fs::create_directories(target);CLSID encoder;if(Encoder(encoder)!=0)throw std::runtime_error("PNG encoder");
            for(int i=0;i<18&&!cancel;++i){
                const auto scratch=target/(L"prepare-"+std::to_wstring(i));fs::create_directories(scratch);
                const auto movie=scratch/L"select.txt",shot=scratch/L"portrait-000900.png",ram=scratch/L"selection.ram";
                std::ofstream input(movie);input<<"0 0000 0000\n";int tick=650;
                for(int n=0;n<fighters[i].x+fighters[i].y;++n){input<<tick<<" "<<(n<fighters[i].x?"0001":"0004")<<" 0000\n"<<tick+4<<" 0000 0000\n";tick+=12;}input.close();
                const auto prefix=scratch/L"portrait-";
                ok=Run({sourceGame.wstring(),L"--rom",sourceRom.wstring(),L"--headless",L"--cpu",L"native",L"--sound",L"z80",L"--frames",L"900",L"--features",L"training_mode",L"--movie",movie.wstring(),L"--eeprom",(scratch/L"board.eeprom").wstring(),L"--settings",(scratch/L"settings.ini").wstring(),L"--shot-every",L"900:"+prefix.wstring(),L"--dump-ram",L"900:"+ram.wstring()},working);
                std::ifstream bytes(ram,std::ios::binary);bytes.seekg(0x8482);char character=-1;bytes.get(character);
                if(!ok||!bytes||static_cast<unsigned char>(character)!=i)throw std::runtime_error("Selection mismatch");
                bytes.close();
                {Gdiplus::Bitmap image(shot.c_str());
                if(image.GetLastStatus()!=Gdiplus::Ok||image.GetWidth()!=384||image.GetHeight()!=224)throw std::runtime_error("Portrait capture");
                std::unique_ptr<Gdiplus::Bitmap> crop(image.Clone(0,0,114,128,PixelFormat32bppARGB));
                if(!crop||crop->Save((target/(std::to_wstring(i)+L".png")).c_str(),&encoder)!=Gdiplus::Ok)throw std::runtime_error("Portrait save");}
                // Remove only the known generated files; never touch user's ROM or progress.
                for(const auto &name:{L"select.txt",L"portrait-000900.png",L"selection.ram",L"selection.ram.video",L"selection.ram.objects",L"selection.ram.regs",L"board.eeprom",L"settings.ini"}){std::error_code ec;fs::remove(scratch/name,ec);}
                std::error_code ec;fs::remove(scratch,ec);
                PostMessageW(notify,WM_APP+21,i+1,0);
            }
        }catch(...){ok=false;}
        Gdiplus::GdiplusShutdown(token);PostMessageW(notify,WM_APP+20,ok&&!cancel,0);
    });
}
inline LRESULT CALLBACK Proc(HWND w,UINT m,WPARAM wp,LPARAM lp){
    if(m==WM_DRAWITEM&&wp){const auto &d=*reinterpret_cast<DRAWITEMSTRUCT*>(lp);RECT box=d.rcItem;
        ui::Fill(d.hDC,box,ui::panel);FrameRect(d.hDC,&box,ui::panelBrush);
        wchar_t label[160];GetWindowTextW(d.hwndItem,label,160);ui::Text(d.hDC,label,box,font,(d.itemState&ODS_DISABLED)?ui::muted:ui::ink,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        if(d.itemState&ODS_FOCUS){InflateRect(&box,-3,-3);DrawFocusRect(d.hDC,&box);}return TRUE;}
    LRESULT styled;if(ui::Message(w,m,wp,lp,styled))return styled;
    if(m==WM_CREATE){
        window=w;
        status=Control(w,L"STATIC",L"",0,24,51,1000,30);
        details=Control(w,L"EDIT",L"",ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL|WS_BORDER,634,94,382,514);
        prepare=Control(w,L"BUTTON",L"Preparar imagens da ROM",WS_TABSTOP,24,627,232,38,1);
        bonus=Control(w,L"BUTTON",L"Recompensa: complete 18 / 18",WS_TABSTOP,270,627,330,38,2);
        Control(w,L"BUTTON",L"Fechar",WS_TABSTOP,876,627,140,38,3);
        SetTimer(w,1,1500,nullptr);Refresh();return 0;
    }
    if(m==WM_TIMER&&!busy){Refresh();return 0;}
    if(m==WM_PAINT){
        PAINTSTRUCT paint;HDC dc=BeginPaint(w,&paint);HDC mem=CreateCompatibleDC(dc);auto bitmap=CreateCompatibleBitmap(dc,1040,690);auto old=SelectObject(mem,bitmap);
        ui::Fill(mem,{0,0,1040,690},ui::background);
        ui::Text(mem,reward?L"RECOMPENSA — ELENCO COMPLETO":L"GALERIA — OS 18 GUERREIROS",{24,8,1016,47},ui::badgeFont,ui::gold);
        {Gdiplus::Graphics g(mem);
        if(reward){Gdiplus::Image image((folder/L"recompensa.png").c_str());g.DrawImage(&image,24,94,576,384);}
        else for(int i=0;i<18;++i){int x=24+(i%6)*96,y=94+(i/6)*172;bool open=Unlocked(i);
            ui::Fill(mem,{x-2,y-2,x+90,y+162},i==selected?ui::gold:ui::panel);
            ui::Fill(mem,{x,y,x+88,y+160},ui::panel);
            Portrait(g,i,x,y,88,100,open);
            ui::Text(mem,open?fighters[i].name:L"?",{x+2,y+104,x+87,y+132},ui::smallFont,open?ui::ink:ui::muted,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
            ui::Text(mem,open?L"LIBERADO":L"BLOQUEADO",{x,y+134,x+88,y+156},ui::smallFont,open?ui::cyan:ui::muted,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        }}
        BitBlt(dc,0,0,1040,690,mem,0,0,SRCCOPY);SelectObject(mem,old);DeleteObject(bitmap);DeleteDC(mem);EndPaint(w,&paint);return 0;
    }
    if(m==WM_LBUTTONDOWN){int x=LOWORD(lp),y=HIWORD(lp);if(x>=24&&x<600&&y>=94&&y<610){int col=(x-24)/96,row=(y-94)/172;int id=row*6+col;if(id<18){selected=id;reward=false;ui::Sound(menu_audio::Cue::Confirm);Refresh();}}return 0;}
    if(m==WM_KEYDOWN&&(wp==VK_LEFT||wp==VK_RIGHT||wp==VK_UP||wp==VK_DOWN)){selected=(selected+(wp==VK_LEFT?17:wp==VK_RIGHT?1:wp==VK_UP?12:6))%18;reward=false;Refresh();return 0;}
    if(m==WM_COMMAND){switch(LOWORD(wp)){
        case 1:BuildImages();break;
        case 2:if(Count()==18&&Ready()){if(!SaveReward()){MessageBoxW(w,L"Não foi possível salvar a imagem nesta pasta.",L"Galeria",MB_OK|MB_ICONERROR);break;}reward=true;SetWindowTextW(details,L"PARABÉNS!\r\n\r\nVocê concluiu as campanhas dos 18 personagens principais.\r\n\r\nEste painel usa os retratos originais da sua ROM.\r\n\r\nA imagem foi salva em user/gallery/recompensa.png.\r\n\r\nClique em um personagem para voltar às fichas.");InvalidateRect(w,nullptr,FALSE);}break;
        case 3:SendMessageW(w,WM_CLOSE,0,0);break;
        }return 0;}
    if(m==WM_APP+21){SetWindowTextW(status,(L"Preparando retratos locais: "+std::to_wstring(wp)+L" / 18").c_str());return 0;}
    if(m==WM_APP+20){if(worker.joinable())worker.join();busy=false;EnableWindow(prepare,TRUE);Refresh();if(!wp)MessageBoxW(w,L"Não foi possível preparar todos os retratos. Confira a ROM completa da edição selecionada e tente novamente. Seu progresso foi preservado.",L"Galeria",MB_OK|MB_ICONERROR);return 0;}
    if(m==WM_CLOSE){cancel=true;if(worker.joinable())worker.join();busy=false;DestroyWindow(w);return 0;}
    if(m==WM_DESTROY){KillTimer(w,1);window=nullptr;if(graphicsToken)Gdiplus::GdiplusShutdown(graphicsToken);graphicsToken=0;return 0;}
    return DefWindowProcW(w,m,wp,lp);
}
inline void Open(HWND owner){
    if(window){ShowWindow(window,SW_RESTORE);SetForegroundWindow(window);return;}
    Gdiplus::GdiplusStartupInput startup;if(Gdiplus::GdiplusStartup(&graphicsToken,&startup,nullptr)!=Gdiplus::Ok)return;
    folder=app/L"user/gallery";art=folder/versionCodes[selectedVersion];romPath=rom;gamePath=app/(selectedVersion==1?L"sfz2alb-game.exe":L"sfz2al-game.exe");
    reward=false;selected=0;
    WNDCLASSW wc{};wc.lpfnWndProc=Proc;wc.hInstance=GetModuleHandleW(nullptr);wc.lpszClassName=L"SFZ2ALGallery";wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.hbrBackground=ui::bgBrush;wc.hIcon=LoadIconW(wc.hInstance,MAKEINTRESOURCEW(1));RegisterClassW(&wc);
    RECT r{0,0,1040,690};AdjustWindowRect(&r,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,FALSE);
    window=CreateWindowW(wc.lpszClassName,L"SFZ2AL — Galeria",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,owner,nullptr,wc.hInstance,nullptr);
    // WM_CREATE runs before the assignment above; refresh now that window is known.
    ui::Caption(window);Refresh();ShowWindow(window,SW_SHOW);
    if(!Ready()&&fs::is_regular_file(romPath))BuildImages();
}
inline void Close(){if(window)SendMessageW(window,WM_CLOSE,0,0);if(worker.joinable()){cancel=true;worker.join();}}
}
