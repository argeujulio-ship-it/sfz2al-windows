// SPDX-License-Identifier: GPL-3.0-only
#include "gallery_progress.h"
#include "gallery_data.hpp"
#include <cassert>
#include <iostream>

int main(int argc,char **argv){
    using namespace port;
    const auto folder=std::filesystem::path(argc>1?argv[1]:"test-gallery");
    GallerySample fight;fight.mode=2;fight.game=4;fight.match=4;fight.round=2;fight.players=1;fight.computer[1]=1;
    for(int id=0;id<18;++id){
        GalleryProgress p;fight.character[0]=uint8_t(id);
        for(int f=0;f<180;++f)assert(p.Observe(fight)==-1);
        auto ending=fight;ending.game=10;assert(p.Observe(ending)==-1);assert(p.Observe(ending)==id);
        assert(p.Observe(ending)==-1);assert(GalleryProgress::Save(folder,id));assert(GalleryProgress::Save(folder,id));
        assert(std::filesystem::is_regular_file(folder/"progress"/(std::to_string(id)+".clear")));
        assert(gallery::fighters[id].x>=0&&gallery::fighters[id].x<5&&gallery::fighters[id].y>=0&&gallery::fighters[id].y<4);
    }
    GalleryProgress p;fight.character[0]=0;auto ending=fight;ending.game=10;
    for(int f=0;f<300;++f)assert(p.Observe(ending)==-1); // Loading an ending or seeing an attract ending.
    for(int variant=0;variant<7;++variant){
        p.Reset();auto bad=fight;
        if(variant==0)bad.training=true;
        if(variant==1)bad.players=0;
        if(variant==2)bad.players=3;
        if(variant==3)bad.computer[0]=1;
        if(variant==4)bad.character[0]=23;
        if(variant==5)bad.test=true;
        if(variant==6)bad.playMode=2;
        for(int f=0;f<300;++f)assert(p.Observe(bad)==-1);
        for(int f=0;f<10;++f)assert(p.Observe(ending)==-1);
    }
    p.Reset();auto second=fight;second.players=2;second.computer[0]=1;second.computer[1]=0;second.character[1]=13;
    for(int f=0;f<180;++f)assert(p.Observe(second)==-1);
    second.game=10;assert(p.Observe(second)==-1);assert(p.Observe(second)==13);
    assert(!GalleryProgress::Save(folder,-1));assert(!GalleryProgress::Save(folder,18));
    std::cout<<"PASS: 18 clears, idempotent persistence, exclusions, P2, ending without fight.\n";
}
