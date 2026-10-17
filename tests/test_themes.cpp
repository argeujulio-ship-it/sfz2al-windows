// SPDX-License-Identifier: GPL-3.0-only
#include "themes.h"
#include "color_adjust.h"
#include <cassert>
#include <filesystem>
#include <iostream>
static Uint32 Pixel(SDL_Surface *s,int x,int y) {
    Uint8 r,g,b,a;assert(SDL_ReadSurfacePixel(s,x,y,&r,&g,&b,&a));
    return (Uint32(r)<<16)|(Uint32(g)<<8)|b;
}
int main(int argc,char **argv) {
    if(argc!=2)return 2;
    port::ColorAdjust adjust;
    const uint32_t colors[]={0xa5000000,0xa5808080,0xa5ffffff,0xa54080c0};
    assert(adjust.Apply(colors,4)==colors);
    adjust.Set(10,100);
    auto adjusted=adjust.Apply(colors,4);
    assert(adjusted[0]==0xa51a1a1a&&adjusted[1]==0xa59a9a9a&&adjusted[2]==0xa5ffffff);
    adjust.Set(0,50);adjusted=adjust.Apply(colors,4);
    assert(adjusted[0]==0xa5404040&&adjusted[1]==0xa5808080&&adjusted[2]==0xa5c0c0c0);
    adjust.Set(0,150);adjusted=adjust.Apply(colors,4);
    assert(adjusted[0]==0xa5000000&&adjusted[1]==0xa5808080&&adjusted[2]==0xa5ffffff);
    adjust.Set(-500,500);adjusted=adjust.Apply(colors,4);assert((adjusted[0]>>24)==0xa5);
    adjust.Set(0,100);assert(adjust.Apply(colors,4)==colors);
    assert(SDL_Init(0));
    std::filesystem::create_directories(argv[1]);
    for(auto size:{SDL_Point{960,720},SDL_Point{1280,720},SDL_Point{640,480},SDL_Point{320,240}}) {
        auto surface=SDL_CreateSurface(size.x,size.y,SDL_PIXELFORMAT_RGBA8888);assert(surface);
        auto renderer=SDL_CreateSoftwareRenderer(surface);assert(renderer);
        {
            port::ThemeOverlay overlay;
            for(auto theme:{port::Theme::Clean,port::Theme::Arcade,port::Theme::Crt,port::Theme::Cabinet,port::Theme::Clean}) {
                auto s=port::ThemePicture(theme,size.x,size.y,384,224,port::Shape::Arcade,port::Scaling::Sharp);
                assert(s.x>=0&&s.y>=0&&s.x+s.w<=size.x+.01f&&s.y+s.h<=size.y+.01f);
                assert(std::abs(s.w/s.h-4.f/3.f)<.002f);
                SDL_SetRenderDrawColor(renderer,0,0,0,255);SDL_RenderClear(renderer);
                SDL_SetRenderDrawColor(renderer,120,160,200,255);SDL_RenderFillRect(renderer,&s);
                overlay.Draw(renderer,theme,size.x,size.y,s);
                SDL_RenderPresent(renderer);
                int cx=int(s.x+s.w/2),cy=int(s.y+s.h/2);
                auto pixel=Pixel(surface,cx,cy);
                if(theme!=port::Theme::Crt)assert(pixel==0x78a0c8);
                else {assert(pixel!=0);assert((pixel&255)<=200);}
                if(theme!=port::Theme::Clean)assert(Pixel(surface,10,10)!=0x78a0c8);
                if(size.x==960&&theme!=port::Theme::Clean) {
                    // Original calibration artwork for screenshots, not game assets.
                    SDL_SetRenderDrawColor(renderer,20,24,41,255);SDL_RenderFillRect(renderer,&s);
                    for(int y=0;y<8;++y)for(int x=0;x<12;++x) {
                        SDL_SetRenderDrawColor(renderer,Uint8(30+x*16),Uint8(35+y*23),Uint8(165+x*5),255);
                        SDL_FRect cell{s.x+x*s.w/12,s.y+y*s.h/8,s.w/12-1,s.h/8-1};SDL_RenderFillRect(renderer,&cell);
                    }
                    overlay.Draw(renderer,theme,size.x,size.y,s);SDL_RenderPresent(renderer);
                    auto filename=std::filesystem::path(argv[1])/(theme==port::Theme::Arcade?"tema_fliperama.png":theme==port::Theme::Cabinet?"tema_gabinete_classico.png":"tema_tv_tubo.png");
                    assert(SDL_SavePNG(surface,filename.string().c_str()));
                }
            }
        }
        SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);
    }
    assert(port::ParseTheme("arcade")==port::Theme::Arcade);
    assert(port::ParseTheme("crt")==port::Theme::Crt);
    assert(port::ParseTheme("cabinet")==port::Theme::Cabinet);
    std::cout<<"PASS: brightness/contrast pixel values, alpha preservation, clamp, reset identity; four themes, 4:3 in four window sizes, cached theme switches.\n";
    SDL_Quit();
}
