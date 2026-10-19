// SPDX-License-Identifier: GPL-3.0-only
// Plays locally generated animated clips. No game media is embedded.
#pragma once
#include <gdiplus.h>
#include <filesystem>
#include <memory>
#include <random>
#include <vector>
#include <cwctype>
#include "color_adjust.h"
#pragma comment(lib,"gdiplus.lib")
class GamePreview {
    ULONG_PTR token=0;
    std::vector<std::filesystem::path> files;
    std::unique_ptr<Gdiplus::Image> image;
    std::vector<unsigned> delays;
    std::mt19937 random{std::random_device{}()};
    unsigned frame=0,count=0;
    size_t current=SIZE_MAX;
    ULONGLONG due=0;
    bool paused=false;
    int brightness=0,contrast=100;
    port::ColorAdjust color;
    std::unique_ptr<Gdiplus::Bitmap> adjusted;
    bool colorDirty=true;
    Gdiplus::Image *AdjustedFrame() {
        if(brightness==0&&contrast==100)return image.get();
        const int width=int(image->GetWidth()),height=int(image->GetHeight());
        if(!adjusted)adjusted=std::make_unique<Gdiplus::Bitmap>(width,height,PixelFormat32bppARGB);
        if(!colorDirty)return adjusted.get();
        {
            Gdiplus::Graphics canvas(adjusted.get());
            canvas.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
            canvas.DrawImage(image.get(),0,0,width,height);
        }
        Gdiplus::Rect area(0,0,width,height);Gdiplus::BitmapData pixels{};
        if(adjusted->LockBits(&area,Gdiplus::ImageLockModeRead|Gdiplus::ImageLockModeWrite,PixelFormat32bppARGB,&pixels)!=Gdiplus::Ok)
            return image.get();
        for(int y=0;y<height;++y) {
            auto row=reinterpret_cast<uint32_t*>(static_cast<BYTE*>(pixels.Scan0)+ptrdiff_t(y)*pixels.Stride);
            const auto mapped=color.Apply(row,size_t(width));
            std::copy_n(mapped,width,row);
        }
        adjusted->UnlockBits(&pixels);colorDirty=false;return adjusted.get();
    }
    bool Next() {
        if(files.empty())return false;
        size_t choice=std::uniform_int_distribution<size_t>(0,files.size()-1)(random);
        if(files.size()>1&&current<files.size()) {
            choice=std::uniform_int_distribution<size_t>(0,files.size()-2)(random);
            if(choice>=current)++choice;
        }
        adjusted.reset();colorDirty=true;image.reset();count=0;current=choice;
        for(size_t attempt=0;attempt<files.size();++attempt) {
            image=std::make_unique<Gdiplus::Image>(files[current].c_str());
            if(image->GetLastStatus()==Gdiplus::Ok) {
                count=image->GetFrameCount(&Gdiplus::FrameDimensionTime);
                if(count>1)break;
            }
            image.reset();current=(current+1)%files.size();
        }
        if(!image||count<2){image.reset();return false;}
        delays.assign(count,40);
        UINT size=image->GetPropertyItemSize(PropertyTagFrameDelay);
        std::vector<unsigned char> property(size);
        if(size>=sizeof(Gdiplus::PropertyItem)) {
            auto item=reinterpret_cast<Gdiplus::PropertyItem*>(property.data());
            if(image->GetPropertyItem(PropertyTagFrameDelay,size,item)==Gdiplus::Ok&&item->type==PropertyTagTypeLong) {
                auto values=static_cast<const ULONG*>(item->value);
                for(unsigned i=0;i<count&&i<item->length/sizeof(ULONG);++i)delays[i]=std::clamp(unsigned(values[i]*10),20u,1000u);
            }
        }
        frame=0;image->SelectActiveFrame(&Gdiplus::FrameDimensionTime,0);
        due=GetTickCount64()+delays[0];return true;
    }
public:
    bool Start(const std::filesystem::path &folder) {
        Gdiplus::GdiplusStartupInput input;
        if(Gdiplus::GdiplusStartup(&token,&input,nullptr)!=Gdiplus::Ok){token=0;return false;}
        std::error_code error;
        for(auto it=std::filesystem::directory_iterator(folder,error);!error&&it!=std::filesystem::directory_iterator();it.increment(error)) {
            if(!it->is_regular_file(error))continue;
            auto ext=it->path().extension().wstring();
            std::transform(ext.begin(),ext.end(),ext.begin(),[](wchar_t c){return wchar_t(towlower(c));});
            if(ext==L".gif")files.push_back(it->path());
        }
        return Next();
    }
    void Close() {adjusted.reset();image.reset();files.clear();if(token)Gdiplus::GdiplusShutdown(token);token=0;}
    void SetColorAdjust(int light,int gain) {
        light=std::clamp(light,-50,50);gain=std::clamp(gain,50,150);
        if(light==brightness&&gain==contrast)return;
        brightness=light;contrast=gain;color.Set(light,gain);colorDirty=true;
    }
    bool Available() const {return image!=nullptr;}
    unsigned FrameIndex() const {return frame;}
    std::wstring ClipName() const {return current<files.size()?files[current].filename().wstring():L"";}
    bool Paused() const {return paused;}
    void Pause(bool value) {paused=value;due=GetTickCount64()+(delays.empty()?40:delays[frame]);}
    bool Tick() {
        if(paused||!image||GetTickCount64()<due)return false;
        ULONGLONG now=GetTickCount64();
        for(int step=0;step<4&&now>=due;++step) {
            if(++frame>=count){if(!Next()){paused=true;return true;}break;}
            image->SelectActiveFrame(&Gdiplus::FrameDimensionTime,frame);colorDirty=true;due+=delays[frame];
        }
        if(due<now)due=now+delays[frame];
        return true;
    }
    void Draw(HDC dc) {
        if(!image)return; // The original VS artwork is the no-media fallback.
        ui::Fill(dc,{602,190,890,594},ui::panel);
        ui::Text(dc,L"PRÉVIA DO JOGO",{616,204,876,236},ui::smallFont,ui::gold,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        ui::Fill(dc,{614,246,878,445},RGB(0,0,0));
        {
            Gdiplus::Graphics graphics(dc);
            graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBilinear);
            graphics.DrawImage(AdjustedFrame(),616,248,260,195);
        }
        ui::Text(dc,paused?L"PRÉVIA PAUSADA":L"EM DEMONSTRAÇÃO",{616,460,876,488},ui::smallFont,ui::ink,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        ui::Text(dc,L"Cenas aleatórias · sem som",{616,495,876,524},ui::smallFont,ui::muted,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }
};
