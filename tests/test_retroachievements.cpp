// SPDX-License-Identifier: GPL-3.0-only
// All server replies and credentials below are synthetic. No network is used.
#include "ra_client.hpp"
#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>

using namespace sfz_ra;
static void Settle(Client &client){for(int i=0;i<3000;++i){client.Idle();if(i>20&&!client.Pending())break;std::this_thread::sleep_for(std::chrono::milliseconds(1));}assert(!client.Pending());}
int main(int argc,char **argv){
    auto folder=fs::path(argc>1?argv[1]:"test-ra");
    assert(Hash("sfz2al")=="509cf3b79fa57d7a13217f32efbdc0c4");assert(Hash("sfz2alb")=="0ccb683be8343aed11f3464be25fe439");assert(Hash("sfa2").empty());
    std::array<uint8_t,65536> ram{};uint8_t b[4]{};
    ram[0x8482]=13;ram[0x8451]=144;
    assert(ReadMemory(ram.data(),ram.size(),0x8483,b,1)==1&&b[0]==13);
    assert(ReadMemory(ram.data(),ram.size(),0x8450,b,2)==2&&b[0]==144&&b[1]==0);
    assert(ReadMemory(ram.data(),ram.size(),65535,b,4)==1);assert(ReadMemory(ram.data(),ram.size(),65536,b,1)==0);
    assert(SaveAccount(folder,"UnitUser","SyntheticTokenForOfflineTest"));std::string user,token;
    assert(LoadAccount(folder,user,token)&&user=="UnitUser"&&token=="SyntheticTokenForOfflineTest");
    std::ifstream file(folder/L"account.bin",std::ios::binary);std::string encrypted((std::istreambuf_iterator<char>(file)),{});file.close();assert(encrypted.find(token)==std::string::npos);
    std::atomic<int> awards=0;
    Transport fake=[&](const Request &request){
        assert(request.url=="https://retroachievements.org/dorequest.php");
        if(request.body.find("r=gameid")!=std::string::npos)return Response{200,R"({"Success":true,"GameID":11832})"};
        if(request.body.find("r=login2")!=std::string::npos)return Response{200,R"({"Success":true,"User":"UnitUser","Token":"SyntheticTokenForOfflineTest","Score":0,"SoftcoreScore":0,"Messages":0})"};
        if(request.body.find("r=achievementsets")!=std::string::npos)return Response{200,R"({"Success":true,"GameId":11832,"Title":"Synthetic offline test","ConsoleId":27,"ImageIconUrl":"","RichPresenceGameId":11832,"RichPresencePatch":"","Sets":[{"AchievementSetId":1,"GameId":11832,"Title":null,"Type":"core","ImageIconUrl":"","Achievements":[{"ID":900001,"Title":"Offline validation","Description":"Own test condition; not an official achievement","Flags":3,"Points":1,"MemAddr":"0xH008483=3_0x008450=144_0xH008107=1","Author":"LocalTest","BadgeName":"00000","Created":1,"Modified":1}],"Leaderboards":[]}]})"};
        if(request.body.find("r=awardachievement")!=std::string::npos){assert(request.body.find("h=0")!=std::string::npos);++awards;return Response{200,R"({"Success":true,"Score":0,"SoftcoreScore":1,"AchievementID":900001,"AchievementsRemaining":0})"};}
        if(request.body.find("r=startsession")!=std::string::npos){assert(request.body.find("h=0")!=std::string::npos);return Response{200,R"({"Success":true,"Unlocks":[],"HardcoreUnlocks":[]})"};}
        return Response{200,R"({"Success":true})"};
    };
    {
        Client c(folder,false,fake);ram.fill(0);c.Memory(ram.data(),ram.size());assert(c.Resume("sfz2al"));Settle(c);if(!c.Loaded())std::cerr<<c.status<<'\n';assert(c.Loaded());assert(c.List().size()==1);
        c.Frame();Settle(c);const auto state=c.SaveProgress();assert(!state.empty());
        ram[0x8482]=3;ram[0x8451]=144;ram[0x8106]=1;c.Frame();Settle(c);if(awards!=1)std::cerr<<"awards="<<awards<<" notice="<<c.Notice()<<" status="<<c.status<<" unlocked="<<c.List()[0].unlocked<<'\n';assert(awards==1);assert(c.List()[0].unlocked);
        assert(c.LoadProgress(state));c.Frame();Settle(c);assert(awards==1); // Already earned awards cannot be duplicated by loading.
        c.Suspend();ram.fill(0);c.Frame();ram[0x8482]=3;ram[0x8451]=144;ram[0x8106]=1;c.Frame();Settle(c);assert(awards==1);
    }
    {
        Client spectator(folder,true,fake);ram.fill(0);spectator.Memory(ram.data(),ram.size());assert(spectator.Resume("sfz2alb"));Settle(spectator);assert(spectator.Loaded());spectator.Frame();
        ram[0x8482]=3;ram[0x8451]=144;ram[0x8106]=1;spectator.Frame();Settle(spectator);assert(awards==1);
    }
    {
        Client pending(folder,false,fake);assert(pending.Resume("sfz2al"));pending.CheckOptIn();Settle(pending);assert(!pending.Loaded());
    }
    assert(Forget(folder));assert(!LoadAccount(folder,user,token));
    std::cout<<"PASS: canonical hashes, swapped RAM, DPAPI, mock login/load/Softcore award, state, spectator and suspension. No network.\n";
}

