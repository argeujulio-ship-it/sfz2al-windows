// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <windows.h>
#include "bindings.h"
#include "../recomp-common/settings.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

namespace controls {
namespace fs=std::filesystem;
inline port::Bindings Defaults() {
    auto b=port::DefaultBindings();
    const SDL_Scancode p2[]={SDL_SCANCODE_I,SDL_SCANCODE_K,SDL_SCANCODE_J,SDL_SCANCODE_L,
        SDL_SCANCODE_R,SDL_SCANCODE_T,SDL_SCANCODE_Y,SDL_SCANCODE_F,SDL_SCANCODE_G,SDL_SCANCODE_H,
        SDL_SCANCODE_2,SDL_SCANCODE_6,SDL_SCANCODE_UNKNOWN,SDL_SCANCODE_UNKNOWN};
    for(int i=0;i<port::kBoundActionCount;++i)b.key[1][i]=p2[i];
    return b;
}
inline bool AssignKey(port::Bindings &b,int player,int action,SDL_Scancode key) {
    if(key==SDL_SCANCODE_ESCAPE||key==SDL_SCANCODE_F1||key==SDL_SCANCODE_F11)return false;
    const auto old=b.key[player][action];
    if(key!=SDL_SCANCODE_UNKNOWN)for(int i=0;i<port::kBoundActionCount;++i)
        if(i!=action&&b.key[player][i]==key)b.key[player][i]=old;
    b.key[player][action]=key;return true;
}
inline void AssignPad(port::Bindings &b,int player,int action,port::PadInput input) {
    const auto old=b.pad[player][action];
    if(input.button>=0||input.axis>=0)for(int i=0;i<port::kBoundActionCount;++i)
        if(i!=action&&b.pad[player][i]==input)b.pad[player][i]=old;
    b.pad[player][action]=input;
}
inline port::Bindings Load(const fs::path &directory) {
    const auto path=fs::exists(directory/L"user/jogo.ini")?directory/L"user/jogo.ini":directory/L"controles.ini";
    if(fs::exists(path))return port::LoadBindings(path.string());
    return Defaults();
}
// Stage every profile first, then replace it. Unrelated INI lines are kept by
// the same INI editor the game uses. A backup is retained beside each old file.
inline bool Save(const fs::path &directory,const port::Bindings &b,std::wstring &error) {
    struct File {fs::path path,temp,backup;bool existed=false,committed=false;};
    std::vector<File> files;
    try {
        fs::create_directories(directory/L"user");
        for(const auto &rel:{L"controles.ini",L"user/jogo.ini",L"user/treino.ini"}) {
            File f;f.path=directory/rel;f.temp=f.path.wstring()+L".controls.tmp";f.backup=f.path.wstring()+L".controls.bak";
            f.existed=fs::exists(f.path);files.push_back(f);auto &file=files.back();
            if(file.existed)fs::copy_file(file.path,file.temp,fs::copy_options::overwrite_existing);
            else if(fs::exists(directory/L"controles.ini"))fs::copy_file(directory/L"controles.ini",file.temp,fs::copy_options::overwrite_existing);
            else {std::ofstream fresh(file.temp);}
            recomp::Ini ini(file.temp);ini.load();
            for(int p=0;p<2;++p)for(int a=0;a<port::kBoundActionCount;++a) {
                ini.set("keyboard"+std::to_string(p+1),port::kBoundActions[a].name,recomp::KeyText(b.key[p][a]));
                ini.set("gamepad"+std::to_string(p+1),port::kBoundActions[a].name,recomp::PadText(b.pad[p][a]));
            }
            if(!ini.save())throw std::runtime_error("staging failed");
            if(file.existed)fs::copy_file(file.path,file.backup,fs::copy_options::overwrite_existing);
        }
        for(auto &f:files) {
            if(!MoveFileExW(f.temp.c_str(),f.path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
                throw std::runtime_error("replace failed");
            f.committed=true;
        }
        return true;
    } catch(...) {
        for(auto &f:files) {
            std::error_code ec;
            if(f.committed) {
                if(f.existed)fs::copy_file(f.backup,f.path,fs::copy_options::overwrite_existing,ec);
                else fs::remove(f.path,ec);
            }
            fs::remove(f.temp,ec);
        }
        error=L"Não foi possível salvar os controles. Confira se a pasta permite gravação. As cópias anteriores ficam em .controls.bak.";
        return false;
    }
}
inline SDL_Scancode FromWindowsKey(WPARAM vk,LPARAM lp) {
    if(vk>='A'&&vk<='Z')return SDL_Scancode(SDL_SCANCODE_A+vk-'A');
    if(vk>='1'&&vk<='9')return SDL_Scancode(SDL_SCANCODE_1+vk-'1');
    if(vk=='0')return SDL_SCANCODE_0;
    if(vk>=VK_F1&&vk<=VK_F12)return SDL_Scancode(SDL_SCANCODE_F1+vk-VK_F1);
    if(vk>=VK_F13&&vk<=VK_F24)return SDL_Scancode(SDL_SCANCODE_F13+vk-VK_F13);
    if(vk>=VK_NUMPAD1&&vk<=VK_NUMPAD9)return SDL_Scancode(SDL_SCANCODE_KP_1+vk-VK_NUMPAD1);
    switch(vk) {
    case VK_ESCAPE:return SDL_SCANCODE_ESCAPE;case VK_RETURN:return (lp&(1LL<<24))?SDL_SCANCODE_KP_ENTER:SDL_SCANCODE_RETURN;
    case VK_UP:return SDL_SCANCODE_UP;case VK_DOWN:return SDL_SCANCODE_DOWN;case VK_LEFT:return SDL_SCANCODE_LEFT;case VK_RIGHT:return SDL_SCANCODE_RIGHT;
    case VK_SPACE:return SDL_SCANCODE_SPACE;case VK_TAB:return SDL_SCANCODE_TAB;case VK_BACK:return SDL_SCANCODE_BACKSPACE;
    case VK_INSERT:return SDL_SCANCODE_INSERT;case VK_DELETE:return SDL_SCANCODE_DELETE;case VK_HOME:return SDL_SCANCODE_HOME;case VK_END:return SDL_SCANCODE_END;
    case VK_PRIOR:return SDL_SCANCODE_PAGEUP;case VK_NEXT:return SDL_SCANCODE_PAGEDOWN;
    case VK_SHIFT:return ((lp>>16)&255)==0x36?SDL_SCANCODE_RSHIFT:SDL_SCANCODE_LSHIFT;
    case VK_CONTROL:return (lp&(1LL<<24))?SDL_SCANCODE_RCTRL:SDL_SCANCODE_LCTRL;
    case VK_MENU:return (lp&(1LL<<24))?SDL_SCANCODE_RALT:SDL_SCANCODE_LALT;
    case VK_CAPITAL:return SDL_SCANCODE_CAPSLOCK;case VK_NUMLOCK:return SDL_SCANCODE_NUMLOCKCLEAR;
    case VK_NUMPAD0:return SDL_SCANCODE_KP_0;case VK_DECIMAL:return SDL_SCANCODE_KP_PERIOD;
    case VK_ADD:return SDL_SCANCODE_KP_PLUS;case VK_SUBTRACT:return SDL_SCANCODE_KP_MINUS;case VK_MULTIPLY:return SDL_SCANCODE_KP_MULTIPLY;case VK_DIVIDE:return SDL_SCANCODE_KP_DIVIDE;
    case VK_OEM_MINUS:return SDL_SCANCODE_MINUS;case VK_OEM_PLUS:return SDL_SCANCODE_EQUALS;
    case VK_OEM_1:return SDL_SCANCODE_SEMICOLON;case VK_OEM_2:return SDL_SCANCODE_SLASH;case VK_OEM_3:return SDL_SCANCODE_GRAVE;
    case VK_OEM_4:return SDL_SCANCODE_LEFTBRACKET;case VK_OEM_5:return SDL_SCANCODE_BACKSLASH;case VK_OEM_6:return SDL_SCANCODE_RIGHTBRACKET;
    case VK_OEM_7:return SDL_SCANCODE_APOSTROPHE;case VK_OEM_COMMA:return SDL_SCANCODE_COMMA;case VK_OEM_PERIOD:return SDL_SCANCODE_PERIOD;
    case VK_OEM_102:return SDL_SCANCODE_NONUSBACKSLASH;
    default:return SDL_SCANCODE_UNKNOWN;
    }
}
}
