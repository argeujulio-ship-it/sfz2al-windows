// SPDX-License-Identifier: GPL-3.0-only
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <algorithm>
#include <string>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <iostream>
#include "launcher_style.hpp"
#include "game_preview.hpp"
// Original solid-color 16x16 red/blue fixture. No game image.
static const unsigned char fixture[]={71,73,70,56,57,97,16,0,16,0,129,0,0,255,0,0,0,0,0,0,0,0,0,0,0,33,255,11,78,69,84,83,67,65,80,69,50,46,48,3,1,0,0,0,33,249,4,0,10,0,0,0,44,0,0,0,0,16,0,16,0,0,8,29,0,1,8,28,72,176,160,193,131,8,19,42,92,200,176,161,195,135,16,35,74,156,72,177,162,197,129,1,1,0,33,249,4,1,10,0,1,0,44,0,0,0,0,16,0,16,0,129,0,0,255,0,0,0,0,0,0,0,0,0,8,29,0,1,8,28,72,176,160,193,131,8,19,42,92,200,176,161,195,135,16,35,74,156,72,177,162,197,129,1,1,0,59};
int main(int argc,char **argv) {
 if(argc!=2)return 2;
 auto folder=std::filesystem::path(argv[1]);std::filesystem::create_directories(folder);
 ui::Init();
 {GamePreview empty;assert(!empty.Start(folder/"absent"));assert(!empty.Available());empty.Close();}
 for(auto name:{"first.gif","second.gif","third.gif"}) {
  std::ofstream file(folder/name,std::ios::binary);file.write(reinterpret_cast<const char*>(fixture),sizeof(fixture));
 }
 GamePreview preview;assert(preview.Start(folder));assert(preview.Available());
 HDC screen=GetDC(nullptr),dc=CreateCompatibleDC(screen);HBITMAP bitmap=CreateCompatibleBitmap(screen,920,710);
 auto old=SelectObject(dc,bitmap);ReleaseDC(nullptr,screen);
 preview.Draw(dc);auto before=GetPixel(dc,746,345);
 auto startFrame=preview.FrameIndex();auto deadline=GetTickCount64()+1500;
 while(preview.FrameIndex()==startFrame&&GetTickCount64()<deadline){Sleep(5);preview.Tick();}
 assert(preview.FrameIndex()!=startFrame);preview.Draw(dc);assert(GetPixel(dc,746,345)!=before);
 preview.Pause(true);auto pausedFrame=preview.FrameIndex();Sleep(150);assert(!preview.Tick());assert(preview.FrameIndex()==pausedFrame);
 preview.Pause(false);auto first=preview.ClipName();deadline=GetTickCount64()+1500;
 while(preview.ClipName()==first&&GetTickCount64()<deadline){Sleep(5);preview.Tick();}
 assert(preview.ClipName()!=first);
 preview.Close();SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);ui::Close();
 std::cout<<"PASS: missing-media fallback, original GIF decode/render, advancing pixels, pause/resume and non-repeating random clips.\n";
}
