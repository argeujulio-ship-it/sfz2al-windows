// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <mmsystem.h>
#include <dwmapi.h>
#include "menu_audio.hpp"
#pragma comment(lib,"uxtheme.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"dwmapi.lib")
namespace ui {
inline constexpr COLORREF background=RGB(12,17,29),panel=RGB(22,30,47),ink=RGB(231,235,242),
    muted=RGB(160,173,194),gold=RGB(255,197,71),red=RGB(222,55,62),cyan=RGB(70,197,218);
inline HBRUSH bgBrush,panelBrush;
inline HFONT logoFont,titleFont,smallFont,badgeFont;
inline bool soundEnabled=true;
inline int volume=25;
inline std::array<std::vector<uint8_t>,4> sounds;
inline void RebuildSounds() {
    PlaySoundW(nullptr,nullptr,0);
    for(int i=0;i<4;++i)sounds[i]=menu_audio::Wave(menu_audio::Cue(i),volume);
}
inline void Sound(menu_audio::Cue cue) {
    if(soundEnabled&&volume>0)PlaySoundW(reinterpret_cast<LPCWSTR>(sounds[size_t(cue)].data()),nullptr,SND_MEMORY|SND_ASYNC|SND_NODEFAULT);
}
inline void Init() {
    bgBrush=CreateSolidBrush(background);panelBrush=CreateSolidBrush(panel);
    logoFont=CreateFontW(-47,0,0,0,FW_BLACK,TRUE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Arial Black");
    titleFont=CreateFontW(-62,0,0,0,FW_BLACK,TRUE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Arial Black");
    smallFont=CreateFontW(-14,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    badgeFont=CreateFontW(-27,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    RebuildSounds();
}
inline void Close() {
    PlaySoundW(nullptr,nullptr,0);
    for(auto object:{HGDIOBJ(bgBrush),HGDIOBJ(panelBrush),HGDIOBJ(logoFont),HGDIOBJ(titleFont),HGDIOBJ(smallFont),HGDIOBJ(badgeFont)})DeleteObject(object);
}
inline void Caption(HWND w) {
    BOOL dark=TRUE;DwmSetWindowAttribute(w,20,&dark,sizeof(dark));
    COLORREF color=background;DwmSetWindowAttribute(w,35,&color,sizeof(color));
}
inline void Fill(HDC dc,RECT r,COLORREF color) {auto brush=CreateSolidBrush(color);FillRect(dc,&r,brush);DeleteObject(brush);}
inline void Text(HDC dc,const wchar_t *s,RECT r,HFONT type,COLORREF color,UINT flags=DT_LEFT|DT_VCENTER|DT_SINGLELINE) {
    auto old=SelectObject(dc,type);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,color);DrawTextW(dc,s,-1,&r,flags);SelectObject(dc,old);
}
inline void PolygonFill(HDC dc,const POINT *p,int n,COLORREF color) {
    auto brush=CreateSolidBrush(color);auto old=SelectObject(dc,brush);auto pen=SelectObject(dc,GetStockObject(NULL_PEN));
    Polygon(dc,p,n);SelectObject(dc,old);SelectObject(dc,pen);DeleteObject(brush);
}
inline void PaintHero(HDC dc) {
    Fill(dc,{0,0,920,710},background);
    POINT blue[]={{580,0},{920,0},{920,165},{430,165}};
    PolygonFill(dc,blue,4,RGB(20,37,59));
    for(int i=0;i<7;++i) {
        int x=520+i*57;POINT line[]={{x,10},{x+12,10},{x-95,150},{x-107,150}};
        PolygonFill(dc,line,4,RGB(29,51,74));
    }
    Text(dc,L"ARCADE COLLECTION  /  WINDOWS",{26,10,500,34},smallFont,cyan);
    Text(dc,L"STREET FIGHTER",{29,32,700,91},logoFont,red);
    Text(dc,L"STREET FIGHTER",{26,29,700,88},logoFont,gold);
    Text(dc,L"ZERO 2",{26,80,510,155},titleFont,ink);
    POINT badge[]={{382,101},{513,101},{500,137},{369,137}};
    PolygonFill(dc,badge,4,red);
    Text(dc,L"ALPHA",{381,100,509,138},badgeFont,ink,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    Text(dc,L"ASIA 960826",{718,50,888,80},badgeFont,gold,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
    Text(dc,L"VERSÃO EXPERIMENTAL",{680,91,888,116},smallFont,muted,DT_RIGHT|DT_SINGLELINE);
    Fill(dc,{24,162,890,165},red);Fill(dc,{24,162,305,165},gold);
    // Original fighting-game panel: opposing colors, diagonal energy and a VS badge.
    Fill(dc,{602,190,890,594},panel);
    POINT left[]={{616,206},{743,206},{704,568},{616,568}};
    POINT right[]={{770,206},{876,206},{876,568},{731,568}};
    PolygonFill(dc,left,4,RGB(61,30,44));PolygonFill(dc,right,4,RGB(22,59,79));
    for(int i=0;i<8;++i) {
        int y=263+i*31;POINT slash[]={{618,y},{873,y-28},{873,y-24},{618,y+4}};
        PolygonFill(dc,slash,4,RGB(42,58,76));
    }
    Text(dc,L"PLAYER 1",{618,217,740,247},smallFont,gold);
    Text(dc,L"PLAYER 2",{756,217,877,247},smallFont,cyan,DT_RIGHT|DT_SINGLELINE);
    auto brush=CreateSolidBrush(background);auto old=SelectObject(dc,brush);auto pen=SelectObject(dc,GetStockObject(NULL_PEN));
    Ellipse(dc,686,290,812,416);SelectObject(dc,old);SelectObject(dc,pen);DeleteObject(brush);
    Text(dc,L"VS",{688,304,812,394},titleFont,gold,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    Text(dc,L"READY TO FIGHT",{612,444,880,481},badgeFont,ink,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    Text(dc,L"2 JOGADORES  ·  6 BOTÕES",{612,490,880,520},smallFont,muted,DT_CENTER|DT_SINGLELINE);
    Text(dc,L"ATALHOS 3P / 3K",{612,527,880,553},smallFont,gold,DT_CENTER|DT_SINGLELINE);
    Fill(dc,{24,606,890,607},RGB(45,57,77));
    Text(dc,L"PROJETO INDEPENDENTE  ·  NÃO AFILIADO À CAPCOM",{24,671,890,696},smallFont,muted);
}
inline void DrawButton(const DRAWITEMSTRUCT &d) {
    wchar_t name[128]{};GetWindowTextW(d.hwndItem,name,128);
    bool primary=std::wstring(name)==L"JOGAR"||std::wstring(name)==L"Jogar";
    bool down=(d.itemState&ODS_SELECTED)!=0,focus=(d.itemState&ODS_FOCUS)!=0;
    RECT box=d.rcItem;Fill(d.hDC,box,primary?(down?RGB(216,147,34):gold):(down?RGB(43,55,77):panel));
    auto brush=CreateSolidBrush(focus?cyan:primary?gold:RGB(67,81,105));FrameRect(d.hDC,&box,brush);DeleteObject(brush);
    if(down)OffsetRect(&box,1,1);
    HFONT type=(GetDlgCtrlID(d.hwndItem)==2||GetDlgCtrlID(d.hwndItem)==3)?badgeFont:HFONT(SendMessageW(d.hwndItem,WM_GETFONT,0,0));
    Text(d.hDC,name,box,type,primary?background:ink,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    if(focus){InflateRect(&box,-4,-4);DrawFocusRect(d.hDC,&box);}
}
inline LRESULT CALLBACK ChildProc(HWND w,UINT m,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR) {
    if(m==WM_SETFOCUS)Sound(menu_audio::Cue::Navigate);
    if(m==WM_NCDESTROY)RemoveWindowSubclass(w,ChildProc,1);
    return DefSubclassProc(w,m,wp,lp);
}
inline void Skin(HWND w) {SetWindowTheme(w,L"",L"");SetWindowSubclass(w,ChildProc,1,0);}
inline bool Message(HWND,UINT m,WPARAM wp,LPARAM lp,LRESULT &result) {
    if(m==WM_ERASEBKGND){RECT r;GetClipBox(HDC(wp),&r);FillRect(HDC(wp),&r,bgBrush);result=1;return true;}
    if(m==WM_CTLCOLORSTATIC||m==WM_CTLCOLOREDIT||m==WM_CTLCOLORLISTBOX||m==WM_CTLCOLORBTN) {
        auto dc=HDC(wp);SetTextColor(dc,ink);SetBkColor(dc,background);result=LRESULT(bgBrush);return true;
    }
    if(m==WM_DRAWITEM&&wp) {DrawButton(*reinterpret_cast<DRAWITEMSTRUCT*>(lp));result=TRUE;return true;}
    if(m==WM_NOTIFY) {
        auto n=reinterpret_cast<NMCUSTOMDRAW*>(lp);
        if(n->hdr.code==NM_CUSTOMDRAW) {
            if(n->dwDrawStage==CDDS_PREPAINT){Fill(n->hdc,n->rc,background);result=CDRF_NOTIFYITEMDRAW;return true;}
            if(n->dwDrawStage==CDDS_ITEMPREPAINT) {
                if(n->dwItemSpec==TBCD_CHANNEL)Fill(n->hdc,n->rc,RGB(61,76,101));
                if(n->dwItemSpec==TBCD_THUMB)Fill(n->hdc,n->rc,gold);
                result=CDRF_SKIPDEFAULT;return true;
            }
        }
    }
    return false;
}
}
