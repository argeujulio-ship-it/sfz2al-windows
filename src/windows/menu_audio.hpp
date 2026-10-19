// SPDX-License-Identifier: GPL-3.0-only
// Original synthesized interface cues; no samples or melodies from the game.
#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>
namespace menu_audio {
enum class Cue { Navigate, Confirm, Back, Launch };
inline std::vector<uint8_t> Wave(Cue cue,int volume) {
    constexpr int rate=22050;
    const double seconds=cue==Cue::Navigate?.045:cue==Cue::Confirm?.12:cue==Cue::Back?.09:.32;
    const int count=int(seconds*rate);
    std::vector<uint8_t> out(44+count*2);
    auto u16=[&](int at,uint16_t n){out[at]=uint8_t(n);out[at+1]=uint8_t(n>>8);};
    auto u32=[&](int at,uint32_t n){u16(at,uint16_t(n));u16(at+2,uint16_t(n>>16));};
    auto tag=[&](int at,const char *s){for(int i=0;i<4;++i)out[at+i]=uint8_t(s[i]);};
    tag(0,"RIFF");u32(4,uint32_t(out.size()-8));tag(8,"WAVE");tag(12,"fmt ");u32(16,16);
    u16(20,1);u16(22,1);u32(24,rate);u32(28,rate*2);u16(32,2);u16(34,16);tag(36,"data");u32(40,count*2);
    double phase=0;
    const double gain=std::max(0,std::min(100,volume))*.003;
    for(int i=0;i<count;++i) {
        double t=double(i)/rate,progress=t/seconds,frequency=0;
        switch(cue) {
        case Cue::Navigate:frequency=780+progress*320;break;
        case Cue::Confirm:frequency=progress<.45?660:990;break;
        case Cue::Back:frequency=660-progress*320;break;
        case Cue::Launch:frequency=progress<.3?440:progress<.6?660:880;break;
        }
        phase+=6.283185307179586*frequency/rate;
        const double envelope=std::min(1.0,t/.004)*std::min(1.0,(seconds-t)/.018)*(1-progress*.5);
        double sample=(std::sin(phase)+.15*std::sin(phase*2))*.85*envelope*gain;
        u16(44+i*2,uint16_t(int16_t(std::lround(sample*32767))));
    }
    return out;
}
}
