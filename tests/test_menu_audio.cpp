// SPDX-License-Identifier: GPL-3.0-only
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "menu_audio.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#pragma comment(lib,"winmm.lib")
static unsigned U32(const std::vector<uint8_t> &b,int at){return unsigned(b[at])|(unsigned(b[at+1])<<8)|(unsigned(b[at+2])<<16)|(unsigned(b[at+3])<<24);}
static int Peak(const std::vector<uint8_t> &b) {
    int peak=0;
    for(size_t i=44;i<b.size();i+=2)peak=std::max(peak,std::abs(int(int16_t(unsigned(b[i])|(unsigned(b[i+1])<<8)))));
    return peak;
}
int main(int argc,char **argv) {
    for(int i=0;i<4;++i) {
        auto cue=menu_audio::Cue(i);auto quiet=menu_audio::Wave(cue,0),low=menu_audio::Wave(cue,25),full=menu_audio::Wave(cue,100);
        assert(std::memcmp(full.data(),"RIFF",4)==0&&std::memcmp(full.data()+8,"WAVE",4)==0);
        assert(U32(full,4)==full.size()-8&&U32(full,40)==full.size()-44);
        assert(U32(full,24)==22050&&full[22]==1&&full[34]==16);
        assert(Peak(quiet)==0&&Peak(low)>0&&Peak(full)>Peak(low)&&Peak(full)<12000);
        assert(full[44]==0&&full[45]==0);
        assert(menu_audio::Wave(cue,-10)==quiet&&menu_audio::Wave(cue,150)==full);
    }
    std::cout<<"PASS: four original WAV cues, valid format/duration, soft onset, volume, silence and clamping.\n";
    if(argc>1&&std::strcmp(argv[1],"--playback")==0) {
        auto cue=menu_audio::Wave(menu_audio::Cue::Confirm,25);
        if(!PlaySoundW(reinterpret_cast<LPCWSTR>(cue.data()),nullptr,SND_MEMORY|SND_SYNC|SND_NODEFAULT))return 3;
        std::cout<<"PASS: Windows accepted playback through the audio device.\n";
    }
}
