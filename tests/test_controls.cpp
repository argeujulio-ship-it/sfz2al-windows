// SPDX-License-Identifier: GPL-3.0-only
#define NOMINMAX
#include "controls_config.hpp"
#include <cassert>
#include <iostream>
static std::string Read(const std::filesystem::path &p){std::ifstream f(p);return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char **argv) {
    if(argc!=2)return 2;
    auto root=std::filesystem::path(argv[1]);std::filesystem::create_directories(root/"user");
    for(auto name:{"controles.ini","user/jogo.ini","user/treino.ini"}){std::ofstream f(root/name);f<<"; keep comment\n[video]\nscale = 4\n[features]\ninfinite_time = true\n";}
    auto b=controls::Defaults();std::wstring error;
    // Older profiles must keep both new shortcuts unassigned.
    auto legacy=port::LoadBindings((root/"user/jogo.ini").string());
    for(int p=0;p<2;++p)for(int a=12;a<14;++a) {
        assert(legacy.key[p][a]==SDL_SCANCODE_UNKNOWN);
        assert(legacy.pad[p][a]==port::PadInput{});
    }
    controls::AssignKey(b,0,12,SDL_SCANCODE_Q);
    controls::AssignKey(b,0,13,SDL_SCANCODE_W);
    controls::AssignKey(b,1,12,SDL_SCANCODE_U);
    controls::AssignKey(b,1,13,SDL_SCANCODE_O);
    controls::AssignPad(b,0,12,{SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,-1});
    controls::AssignPad(b,0,13,{-1,SDL_GAMEPAD_AXIS_LEFT_TRIGGER});
    controls::AssignPad(b,1,12,{SDL_GAMEPAD_BUTTON_LEFT_STICK,-1});
    controls::AssignPad(b,1,13,{SDL_GAMEPAD_BUTTON_RIGHT_STICK,-1});
    assert(!controls::AssignKey(b,0,4,SDL_SCANCODE_F11));
    assert(b.key[0][4]==SDL_SCANCODE_A);
    assert(controls::AssignKey(b,0,4,SDL_SCANCODE_S));
    assert(b.key[0][5]==SDL_SCANCODE_A);
    controls::AssignKey(b,1,11,SDL_SCANCODE_KP_0);
    controls::AssignPad(b,0,4,{-1,SDL_GAMEPAD_AXIS_RIGHT_TRIGGER});
    controls::AssignPad(b,1,5,{SDL_GAMEPAD_BUTTON_SOUTH,-1});
    assert(controls::Save(root,b,error));
    for(auto name:{"controles.ini","user/jogo.ini","user/treino.ini"}) {
        auto path=root/name;auto loaded=port::LoadBindings(path.string());
        for(int p=0;p<2;++p)for(int a=0;a<port::kBoundActionCount;++a){assert(loaded.key[p][a]==b.key[p][a]);assert(loaded.pad[p][a]==b.pad[p][a]);}
        for(int p=0;p<2;++p) {
            bool held[SDL_SCANCODE_COUNT]{};
            auto keyboardMask=[&](){return port::BoundInputMask([&](int a){auto key=loaded.key[p][a];return key!=SDL_SCANCODE_UNKNOWN&&held[key];});};
            held[loaded.key[p][12]]=true;assert(keyboardMask()==0x70);
            held[loaded.key[p][13]]=true;assert(keyboardMask()==0x3f0);
            held[loaded.key[p][12]]=false;assert(keyboardMask()==0x380);
            held[loaded.key[p][13]]=false;assert(keyboardMask()==0);
            held[loaded.key[p][4]]=true;held[loaded.key[p][12]]=true;assert(keyboardMask()==0x70);
            held[loaded.key[p][12]]=false;assert(keyboardMask()==0x10);
            auto padMask=[&](port::PadInput pressed){return port::BoundInputMask([&](int a){return loaded.pad[p][a]==pressed;});};
            assert(padMask(loaded.pad[p][12])==0x70);
            assert(padMask(loaded.pad[p][13])==0x380);
        }
        auto text=Read(path);assert(text.find("; keep comment")!=std::string::npos);assert(text.find("scale = 4")!=std::string::npos);
    }
    auto original=Read(root/"controles.ini");
    SetFileAttributesW((root/"user/treino.ini").c_str(),FILE_ATTRIBUTE_READONLY);
    controls::AssignKey(b,0,4,SDL_SCANCODE_P);
    assert(!controls::Save(root,b,error));
    assert(Read(root/"controles.ini")==original);
    SetFileAttributesW((root/"user/treino.ini").c_str(),FILE_ATTRIBUTE_NORMAL);
    // Deny replacement of the last profile, after staging can succeed.
    HANDLE lock=CreateFileW((root/"user/treino.ini").c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);
    assert(lock!=INVALID_HANDLE_VALUE);
    auto previousGame=Read(root/"user/jogo.ini");
    assert(!controls::Save(root,b,error));
    assert(Read(root/"controles.ini")==original);
    assert(Read(root/"user/jogo.ini")==previousGame);
    CloseHandle(lock);
    assert(controls::FromWindowsKey(VK_RETURN,1LL<<24)==SDL_SCANCODE_KP_ENTER);
    assert(controls::FromWindowsKey(VK_CONTROL,1LL<<24)==SDL_SCANCODE_RCTRL);
    std::cout<<"PASS: 3P/3K held/release/overlap, both players, keyboard/pad/trigger roundtrip, legacy profiles, swaps, reserved keys, unrelated settings, save failure rollback.\n";
}
