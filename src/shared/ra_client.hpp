// SPDX-License-Identifier: GPL-3.0-only
// Windows host for the unmodified RetroAchievements rc_client runtime.
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>
#include <wincrypt.h>
#include "rc_client.h"
#include "md5.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>
#pragma comment(lib,"winhttp.lib")
#pragma comment(lib,"crypt32.lib")

namespace sfz_ra {
namespace fs=std::filesystem;
inline std::wstring Wide(const std::string &text){int n=MultiByteToWideChar(CP_UTF8,0,text.data(),int(text.size()),nullptr,0);std::wstring w(n,0);if(n)MultiByteToWideChar(CP_UTF8,0,text.data(),int(text.size()),w.data(),n);return w;}
inline std::string Utf8(const std::wstring &text){int n=WideCharToMultiByte(CP_UTF8,0,text.data(),int(text.size()),nullptr,0,nullptr,nullptr);std::string s(n,0);if(n)WideCharToMultiByte(CP_UTF8,0,text.data(),int(text.size()),s.data(),n,nullptr,nullptr);return s;}
inline std::string Hash(const std::string &game){
    if(game!="sfz2al"&&game!="sfz2alb")return {};
    md5_state_t state;md5_byte_t bytes[16];md5_init(&state);md5_append(&state,reinterpret_cast<const md5_byte_t*>(game.data()),int(game.size()));md5_finish(&state,bytes);
    std::string text;for(auto b:bytes){text+="0123456789abcdef"[b>>4];text+="0123456789abcdef"[b&15];}return text;
}
// FBNeo exposes CpsRamFF as a little-endian word-swapped 64 KiB buffer.
// Our board keeps the same RAM in 68000 byte order. RA addresses are offsets.
inline uint32_t ReadMemory(const uint8_t *ram,size_t size,uint32_t address,uint8_t *output,uint32_t count){
    if(size!=65536||address>=size||!output||!ram)return 0;
    count=uint32_t(std::min<size_t>(count,size-address));
    for(uint32_t i=0;i<count;++i)output[i]=ram[(address+i)^1];return count;
}
inline bool SaveAccount(const fs::path &folder,const std::string &user,const std::string &token){
    if(user.empty()||token.empty()||user.size()>128||token.size()>512)return false;
    std::string plain=user+'\n'+token;DATA_BLOB source{DWORD(plain.size()),reinterpret_cast<BYTE*>(plain.data())},encrypted{};
    const bool ok=CryptProtectData(&source,L"SFZ2AL RetroAchievements",nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&encrypted)!=0;
    SecureZeroMemory(plain.data(),plain.size());if(!ok)return false;
    std::error_code ec;fs::create_directories(folder,ec);bool saved=false;
    if(!ec){const auto temp=folder/L"account.tmp";std::ofstream out(temp,std::ios::binary);out.write(reinterpret_cast<char*>(encrypted.pbData),encrypted.cbData);out.close();
        if(out.good())saved=MoveFileExW(temp.c_str(),(folder/L"account.bin").c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;}
    SecureZeroMemory(encrypted.pbData,encrypted.cbData);LocalFree(encrypted.pbData);return saved;
}
inline bool LoadAccount(const fs::path &folder,std::string &user,std::string &token){
    user.clear();token.clear();std::ifstream in(folder/L"account.bin",std::ios::binary|std::ios::ate);if(!in)return false;
    auto length=in.tellg();if(length<=0||length>16384)return false;std::vector<uint8_t> data(size_t(length),0);in.seekg(0);in.read(reinterpret_cast<char*>(data.data()),length);
    if(!in)return false;DATA_BLOB source{DWORD(data.size()),data.data()},plain{};
    if(!CryptUnprotectData(&source,nullptr,nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&plain))return false;
    std::string text(reinterpret_cast<char*>(plain.pbData),plain.cbData);SecureZeroMemory(plain.pbData,plain.cbData);LocalFree(plain.pbData);
    auto split=text.find('\n');bool valid=split>0&&split<128&&split+1<text.size()&&text.size()<700;
    if(valid){user=text.substr(0,split);token=text.substr(split+1);}SecureZeroMemory(text.data(),text.size());return valid;
}
inline bool Forget(const fs::path &folder){const auto path=folder/L"account.bin";return DeleteFileW(path.c_str())!=0||GetLastError()==ERROR_FILE_NOT_FOUND;}
struct Request {std::string url,body,type;};
struct Response {int code=RC_API_SERVER_RESPONSE_RETRYABLE_CLIENT_ERROR;std::string body;};
using Transport=std::function<Response(const Request&)>;
inline Response Https(const Request &request){
    Response reply;const std::wstring url=Wide(request.url);URL_COMPONENTS uri{};uri.dwStructSize=sizeof(uri);uri.dwHostNameLength=uri.dwUrlPathLength=uri.dwExtraInfoLength=DWORD(-1);
    if(!WinHttpCrackUrl(url.c_str(),DWORD(url.size()),0,&uri))return reply;
    const std::wstring host(uri.lpszHostName,uri.dwHostNameLength),path(uri.lpszUrlPath,uri.dwUrlPathLength);
    if(uri.nScheme!=INTERNET_SCHEME_HTTPS||host!=L"retroachievements.org"||uri.nPort!=443||path!=L"/dorequest.php"||uri.dwExtraInfoLength)return reply;
    struct Handle{HINTERNET p=nullptr;~Handle(){if(p)WinHttpCloseHandle(p);}};
    Handle session{WinHttpOpen(L"SFZ2ALWindows/0.4.0 (Windows x64)",WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0)};if(!session.p)return reply;
    WinHttpSetTimeouts(session.p,3000,3000,5000,5000);
    Handle connection{WinHttpConnect(session.p,host.c_str(),443,0)};if(!connection.p)return reply;
    Handle call{WinHttpOpenRequest(connection.p,request.body.empty()?L"GET":L"POST",path.c_str(),nullptr,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE)};if(!call.p)return reply;
    DWORD disable=WINHTTP_DISABLE_REDIRECTS;WinHttpSetOption(call.p,WINHTTP_OPTION_DISABLE_FEATURE,&disable,sizeof(disable));
    std::wstring headers=L"Content-Type: "+Wide(request.type.empty()?"application/x-www-form-urlencoded":request.type)+L"\r\n";
    if(!WinHttpSendRequest(call.p,headers.c_str(),DWORD(-1),request.body.empty()?WINHTTP_NO_REQUEST_DATA:const_cast<char*>(request.body.data()),DWORD(request.body.size()),DWORD(request.body.size()),0)||!WinHttpReceiveResponse(call.p,nullptr))return reply;
    DWORD status=0,length=sizeof(status);if(!WinHttpQueryHeaders(call.p,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&status,&length,WINHTTP_NO_HEADER_INDEX))return reply;
    reply.code=int(status);
    for(;;){DWORD available=0;if(!WinHttpQueryDataAvailable(call.p,&available)){reply.code=-2;break;}if(!available)break;
        if(reply.body.size()+available>8*1024*1024){reply.code=-1;reply.body.clear();break;}
        size_t at=reply.body.size();reply.body.resize(at+available);DWORD read=0;if(!WinHttpReadData(call.p,reply.body.data()+at,available,&read)){reply.code=-2;break;}reply.body.resize(at+read);}
    return reply;
}
struct Achievement {uint32_t id=0,points=0;bool unlocked=false,unsupported=false;std::string title,description;};
class Client {
    struct Job {std::future<Response> task;rc_client_server_callback_t callback;void *data;};
    std::vector<Job> jobs;
    std::array<uint8_t,65536> zero{};
    const uint8_t *memory=nullptr;size_t memorySize=0;
    rc_client_t *client=nullptr;Transport transport;fs::path folder;
    std::string hash;bool closing=false,store=false,loaded=false,failed=false,suspended=false;
    std::string notice;uint64_t noticeUntil=0;
    uint64_t policyAt=0;fs::file_time_type accountTime{};
    static Client &Self(rc_client_t *c){return *static_cast<Client*>(rc_client_get_userdata(c));}
    static uint32_t RC_CCONV Read(uint32_t address,uint8_t *buffer,uint32_t count,rc_client_t *c){auto &self=Self(c);return ReadMemory(self.memory?self.memory:self.zero.data(),self.memory?self.memorySize:self.zero.size(),address,buffer,count);}
    static void RC_CCONV Server(const rc_api_request_t *r,rc_client_server_callback_t callback,void *data,rc_client_t *c){
        auto &self=Self(c);if(self.closing){rc_api_server_response_t reply{"",0,-1};callback(&reply,data);return;}
        Request request{r->url?r->url:"",r->post_data?r->post_data:"",r->content_type?r->content_type:""};
        auto send=self.transport;
        self.jobs.push_back({std::async(std::launch::async,[request=std::move(request),send]()mutable{Response response;try{response=send?send(request):Https(request);}catch(...){response.code=-2;}
            if(!request.body.empty())SecureZeroMemory(request.body.data(),request.body.size());return response;}),callback,data});
    }
    static void RC_CCONV LoginDone(int result,const char*,rc_client_t *c,void*){
        auto &self=Self(c);if(self.closing||self.suspended)return;if(result!=RC_OK){self.failed=true;self.status="Não foi possível entrar. Confira usuário, senha e conexão.";return;}
        const auto *user=rc_client_get_user_info(c);if(!user){self.failed=true;return;}
        self.username=user->username;self.status="Conta conectada — Softcore";
        if(self.store&&!SaveAccount(self.folder,user->username,user->token)){self.failed=true;self.status="Conectado, mas não foi possível proteger o token nesta pasta.";}
        if(!self.hash.empty())rc_client_begin_load_game(c,self.hash.c_str(),GameDone,nullptr);
    }
    static void RC_CCONV GameDone(int result,const char*,rc_client_t *c,void*){
        auto &self=Self(c);const auto *game=rc_client_get_game_info(c);
        if(self.closing||self.suspended){rc_client_unload_game(c);return;}
        if(result!=RC_OK||!game||game->id!=11832||game->console_id!=27){self.status="Conquistas indisponíveis para esta sessão.";self.failed=true;rc_client_unload_game(c);return;}
        self.loaded=true;self.status="Softcore conectado — Street Fighter Zero 2 Alpha";self.Notify("RETROACHIEVEMENTS - SOFTCORE CONECTADO");
    }
    static void RC_CCONV Event(const rc_client_event_t *event,rc_client_t *c){auto &self=Self(c);
        if(event->type==RC_CLIENT_EVENT_ACHIEVEMENT_TRIGGERED&&event->achievement)self.Notify("CONQUISTA: "+std::string(event->achievement->title)+" (+"+std::to_string(event->achievement->points)+")");
        else if(event->type==RC_CLIENT_EVENT_DISCONNECTED)self.Notify("RA: SEM CONEXAO - ENVIO PENDENTE");
        else if(event->type==RC_CLIENT_EVENT_RECONNECTED)self.Notify("RA: CONQUISTAS SINCRONIZADAS");
        else if(event->type==RC_CLIENT_EVENT_SERVER_ERROR){self.status="O servidor recusou uma operação. Confira seu progresso online.";self.Notify("RA: ERRO NO SERVIDOR");}
        else if(event->type==RC_CLIENT_EVENT_GAME_COMPLETED)self.Notify("RETROACHIEVEMENTS: TODAS AS CONQUISTAS!");
    }
public:
    std::string status="Desconectado",username;
    explicit Client(fs::path where={},bool spectator=false,Transport send={}):transport(std::move(send)),folder(std::move(where)){
        client=rc_client_create(Read,Server);rc_client_set_userdata(client,this);rc_client_set_host(client,"https://retroachievements.org");
        rc_client_set_hardcore_enabled(client,0);rc_client_set_spectator_mode_enabled(client,spectator?1:0);
        rc_client_set_allow_background_memory_reads(client,0);rc_client_set_event_handler(client,Event);
    }
    Client(const Client&)=delete;Client &operator=(const Client&)=delete;
    ~Client(){closing=true;rc_client_logout(client);while(!jobs.empty())Pump(true);rc_client_destroy(client);}
    void Notify(const std::string &text){notice=text;noticeUntil=GetTickCount64()+6500;}
    std::string Notice()const{return GetTickCount64()<noticeUntil?notice:std::string{};}
    bool Connected()const{return rc_client_get_user_info(client)!=nullptr;}
    bool Loaded()const{return loaded;}
    bool Failed()const{return failed;}
    bool Pending()const{return !jobs.empty();}
    void Memory(const uint8_t *data,size_t size){memory=data;memorySize=size;}
    void Login(const std::string &user,std::string password,const std::string &game){
        hash=Hash(game);store=true;status="Conectando...";failed=false;
        rc_client_begin_login_with_password(client,user.c_str(),password.c_str(),LoginDone,nullptr);
        if(!password.empty())SecureZeroMemory(password.data(),password.size());
    }
    bool Resume(const std::string &game){std::string user,token;if(!LoadAccount(folder,user,token))return false;
        std::error_code ec;accountTime=fs::last_write_time(folder/L"account.bin",ec);
        hash=Hash(game);status="Conectando...";failed=false;store=false;rc_client_begin_login_with_token(client,user.c_str(),token.c_str(),LoginDone,nullptr);SecureZeroMemory(token.data(),token.size());return true;}
    void Pump(bool wait=false){
        for(size_t i=0;i<jobs.size();){if(!wait&&jobs[i].task.wait_for(std::chrono::seconds(0))!=std::future_status::ready){++i;continue;}
            Job job=std::move(jobs[i]);jobs.erase(jobs.begin()+i);Response response=job.task.get();
            rc_api_server_response_t data{response.body.c_str(),response.body.size(),response.code};job.callback(&data,job.data);}
    }
    void Idle(){Pump();rc_client_idle(client);}
    void Frame(){Pump();if(loaded&&!failed)rc_client_do_frame(client);else rc_client_idle(client);}
    void Reset(){if(loaded)rc_client_reset(client);}
    void CheckOptIn(){
        if(folder.empty()||GetTickCount64()<policyAt)return;policyAt=GetTickCount64()+1000;
        std::error_code ec;const auto current=fs::last_write_time(folder/L"account.bin",ec);
        if(ec||current!=accountTime||!fs::is_regular_file(folder/L"online.enabled",ec))Suspend();
    }
    void Suspend(){if(suspended)return;suspended=true;failed=true;loaded=false;rc_client_unload_game(client);status="Conquistas pausadas nesta sessão. Reinicie o jogo pelo iniciador.";Notify("RA PAUSADO: REINICIE O JOGO PARA VOLTAR");}
    std::vector<Achievement> List()const{
        std::vector<Achievement> result;if(!loaded)return result;
        auto *list=rc_client_create_achievement_list(client,RC_CLIENT_ACHIEVEMENT_CATEGORY_PROMOTED,RC_CLIENT_ACHIEVEMENT_LIST_GROUPING_LOCK_STATE);
        if(list){for(uint32_t b=0;b<list->num_buckets;++b)for(uint32_t i=0;i<list->buckets[b].num_achievements;++i){const auto *a=list->buckets[b].achievements[i];
            result.push_back({a->id,a->points,(a->unlocked&RC_CLIENT_ACHIEVEMENT_UNLOCKED_CASUAL)!=0,a->state==RC_CLIENT_ACHIEVEMENT_STATE_DISABLED,a->title?a->title:"",a->description?a->description:""});}rc_client_destroy_achievement_list(list);}
        return result;
    }
    std::vector<uint8_t> SaveProgress()const{
        if(!loaded||failed||!Connected())return {};const auto n=rc_client_progress_size(client);if(n>4*1024*1024)return {};
        std::string identity=username+'\n'+hash+'\n';std::vector<uint8_t> data(identity.begin(),identity.end());const auto at=data.size();data.resize(at+n);
        if(rc_client_serialize_progress_sized(client,data.data()+at,n)!=RC_OK)return {};return data;
    }
    bool LoadProgress(const std::vector<uint8_t> &data){
        if(!loaded||failed){Suspend();return false;}const std::string identity=username+'\n'+hash+'\n';
        if(data.size()<=identity.size()||!std::equal(identity.begin(),identity.end(),data.begin())||rc_client_deserialize_progress_sized(client,data.data()+identity.size(),data.size()-identity.size())!=RC_OK){Suspend();return false;}return true;
    }
};
}
