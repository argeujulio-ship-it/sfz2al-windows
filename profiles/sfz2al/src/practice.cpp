// SPDX-License-Identifier: GPL-3.0-only
// SFZ2AL/SFZ2ALB training integration. Shared fight module by Tommy McLeroy.
#include "game.h"
#include "fight.h"
#include "machine.h"
#include "state.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>

namespace cps2recomp {
namespace {
constexpr uint32_t kOne=0xFF8400,kTwo=0xFF8800,kPlayersIn=0xFF80A8,kChosen=0xFF80AC;
constexpr uint16_t kHeld=fight::kSticks|fight::kButtons;
fight::InputHistory inputs{};
fight::Dummy dummy{};
std::vector<uint8_t> checkpoint;
bool session=false,boot=false,wasFighting=false,choosingTwo=false;
uint16_t heldOver=0;
int bootFrames=0,recordFrames=0;
bool Active(){return FeatureOn("training_mode");}
bool Fighting() {
    return Ram16(0xFF8000) == 2 && Ram16(0xFF8004) == 4 &&
           Ram16(0xFF8008) == 4 && Ram16(0xFF800C) == 2;
}
bool HumanPair(){return Ram8(kPlayersIn)==3&&Ram8(kOne+0xA5)==0&&Ram8(kTwo+0xA5)==0;}
bool Choosing(){return Ram16(0xFF8000)==2&&Ram16(0xFF8004)==2&&Ram8(kPlayersIn)==3;}
bool OnRight(){return int16_t(Ram16(kTwo+0x10))>=int16_t(Ram16(kOne+0x10));}
void ResetSession(){
    checkpoint.clear();inputs.Clear();dummy=fight::Dummy{};
    wasFighting=choosingTwo=false;heldOver=0;recordFrames=0;
}
void Capture(Machine &machine){
    if(!Active()||!Fighting()||!HumanPair())return;
    checkpoint.clear();Archive ar(checkpoint,true);machine.Serialize(ar);
}
void Restart(Machine &machine){
    if(!Active()||checkpoint.empty())return;
    Archive ar(checkpoint,false);machine.Serialize(ar);
    inputs.Clear();dummy.Afresh();wasFighting=true;
}
void Record(Machine &){
    if(!Active()||!Fighting()||!HumanPair())return;
    SetFeature(FeatureOn("dummy_record")?"dummy_replay":"dummy_record",true);recordFrames=0;
}
void Stop(Machine &){if(Active())SetFeature("dummy_stand",true);}
void Start(Machine &machine){
    SetFeature("training_mode",true);ResetSession();session=true;boot=true;bootFrames=0;
    machine.Reset();
    if(!FeatureOn("dummy_stand")&&!FeatureOn("dummy_crouch")&&!FeatureOn("dummy_record")&&
       !FeatureOn("dummy_replay")&&!FeatureOn("dummy_low_guard")&&!FeatureOn("dummy_manual"))SetFeature("dummy_stand",true);
}
void Boot(Machine &machine){
    if(Ram8(kPlayersIn)==3){boot=false;return;}
    machine.players[0]=machine.players[1]=0;
    if(++bootFrames>4000){boot=false;return;}
    const bool press=bootFrames%16<6;
    if(Ram8(0xFF8076)<2)machine.players[0]=press?kCoin:0;
    else if(!(Ram8(kPlayersIn)&1))machine.players[0]=press?kStart:0;
    else machine.players[1]=press?kStart:0;
}
void Before(Machine &machine){
    static uint64_t traceTick=0;++traceTick;
    if(!Active()){
        if(session){ResetSession();session=boot=false;SetHud({});}
        return;
    }
    if(!session)Start(machine);
    if(boot){Boot(machine);return;}
    if(Choosing()&&(Ram8(kChosen)&1)&&!(Ram8(kChosen)&2)){
        const uint16_t held=machine.players[0]&kHeld;
        heldOver=choosingTwo?uint16_t(heldOver&held):held;choosingTwo=true;
        machine.players[1]|=held&~heldOver;machine.players[0]&=kStart|kCoin;return;
    }
    choosingTwo=false;
    if(!Fighting()||!HumanPair())return;
    const bool recording=FeatureOn("dummy_record");if(!recording)recordFrames=0;
    fight::DummyView view;view.fighting=true;view.onRight=OnRight();
    dummy.BeforeFrame(machine.players[0],machine.players[1],view);
    if(FeatureOn("dummy_low_guard"))machine.players[1]=uint16_t((machine.players[1]&~kHeld)|(view.onRight?kRight:kLeft)|kDown);
    if(recording&&++recordFrames>=int(fight::Dummy::kRecordingMost)){SetFeature("dummy_replay",true);recordFrames=0;}
    static const bool trace=std::getenv("SFZ2AL_TRAINING_TRACE")!=nullptr;
    if(trace)std::fprintf(stderr,"TRAIN %llu %04X %04X %d %d %d\n",static_cast<unsigned long long>(traceTick),machine.players[0],machine.players[1],FeatureOn("dummy_record"),FeatureOn("dummy_replay"),OnRight());
}
#ifdef SFZ2AL_ENABLE_LADDER_HARNESS
// Explicit research harness, excluded from the normal Windows build.
// Preserve zero health: restoring a knocked-out actor would obstruct round end.
void LadderProbeFrame(Machine &) {
    if (!Fighting()) return;
    const auto p1 = Ram16(0xFF8450), p2 = Ram16(0xFF8850);
    if (p1 > 0 && p1 <= 144) SetRam16(0xFF8450, 144);
    if (p2 > 0 && p2 <= 144) SetRam16(0xFF8850, 1);
}
#endif
void PracticeFrame(Machine &machine) {
    if (!Fighting()) {
        if(Active()){
            const char *text=boot?"TREINO: PREPARANDO SELECAO DOS DOIS JOGADORES":Choosing()?
                (choosingTwo?"J1 ESCOLHE O ADVERSARIO - SOLTE E CONFIRME":"ESCOLHA SEU PERSONAGEM COM O JOGADOR 1"):
                "TREINO: AGUARDANDO LUTA COM DOIS JOGADORES";
            SetHud({{0.5f,0.82f,0,text}});
        }else if(!Hud().empty())SetHud({});
        wasFighting=false;return;
    }
    if (FeatureOn("infinite_time")) {
        SetRam8(0xFF8109, 99);
        SetRam8(0xFF2807, 0x99);
    }
    if (FeatureOn("refill_health")) {
        for (const uint32_t player : {0xFF8400u, 0xFF8800u}) {
            const auto health = Ram16(player + 0x50);
            if (Ram8(player + 4) == 2 && Ram8(player + 5) == 0 &&
                Ram8(player + 6) == 0 && health > 0 && health < 144) {
                SetRam16(player + 0x50, 144);
            }
        }
    }
    if(!Active()){if(!Hud().empty())SetHud({});return;}
    std::vector<HudLine> lines;
    if(HumanPair()){
        if(!wasFighting){dummy.Afresh();inputs.Clear();Capture(machine);}
        const bool right=OnRight();
        inputs.Push(0,machine.players[0],!right);inputs.Push(1,machine.players[1],right);
        if(FeatureOn("input_history")){inputs.Hud(0,lines,0.29f,0.051f);inputs.Hud(1,lines,0.29f,0.051f);}
        const char *mode=FeatureOn("dummy_record")?"GRAVANDO J2 (CONTROLES DO J1)":FeatureOn("dummy_replay")?"REPETINDO":
            FeatureOn("dummy_low_guard")?"DEFESA BAIXA":FeatureOn("dummy_crouch")?"AGACHADO":FeatureOn("dummy_stand")?"PARADO":"MANUAL";
        lines.push_back({0.5f,0.14f,0,std::string("TREINO - ADVERSARIO: ")+mode});
        dummy.Hud(lines,0.20f);
        for(auto &line:lines)if(line.text=="nothing recorded")line.text="SEM GRAVACAO NESTE SLOT";
        lines.push_back({0.5f,0.89f,0,"BACKSPACE: REINICIAR | \\: GRAVAR/REPETIR | ESC: OPCOES"});
    }else lines.push_back({0.5f,0.84f,0,"TREINO: AGUARDANDO LUTA COM DOIS JOGADORES"});
    wasFighting=HumanPair();SetHud(std::move(lines));
}
void State(Archive &ar){
    if(!ar.Saving()&&ar.version>0&&ar.version<8){ResetSession();session=Active();boot=false;return;}
    ar.Mark(0x53465A54);ar.Pod(session);ar.Pod(boot);ar.Pod(bootFrames);ar.Pod(wasFighting);
    ar.Pod(choosingTwo);ar.Pod(heldOver);ar.Pod(recordFrames);ar.Pod(inputs);dummy.Serialize(ar);ar.Vec(checkpoint);
}
}
void RegisterGame() {
    Game &game = TheGame();
    game.features.push_back({"infinite_time", "Treino", "Tempo infinito",
                             "Mantem o relogio em 99 durante a luta."});
    game.features.push_back({"refill_health", "Treino", "Repor vida em repouso",
                             "Recupera a vida dos dois lutadores ao voltarem ao repouso. Nao impede um nocaute."});
    game.features.push_back({"training_mode","Treino","Sessao de treino","Prepara dois jogadores para escolher personagens. Recursos restritos ao treino.",kFeatureSession});
    game.features.push_back({"input_history","Treino","Historico de comandos","Direcoes numericas (6=frente, 4=tras, 2=baixo) e LP/MP/HP/LK/MK/HK.",kFeatureTraining});
    const size_t first=game.features.size();fight::RegisterDummy({});
    game.features.erase(std::remove_if(game.features.begin()+first,game.features.end(),[](const Feature &f){
        return strcmp(f.name,"dummy_stand")&&strcmp(f.name,"dummy_crouch")&&strcmp(f.name,"dummy_record")&&strcmp(f.name,"dummy_replay")&&strcmp(f.name,"dummy_slot");
    }),game.features.end());
    for(size_t i=first;i<game.features.size();++i){auto &f=game.features[i];f.category="Adversario";
        if(!strcmp(f.name,"dummy_stand")){f.title="Parado";f.description="O jogador 2 fica parado.";}
        if(!strcmp(f.name,"dummy_crouch")){f.title="Agachado";f.description="O jogador 2 segura baixo.";}
        if(!strcmp(f.name,"dummy_record")){f.title="Gravar sequencia";f.description="J1 controla J2 e grava ate 20 segundos. Depois escolha Repetir.";}
        if(!strcmp(f.name,"dummy_replay")){f.title="Repetir sequencia";f.description="Repete a gravacao do slot, com pausa entre repeticoes. Ajusta esquerda/direita ao trocar de lado.";}
        if(!strcmp(f.name,"dummy_slot")){f.title="Slot de gravacao";f.description="Tres slots; preservados pelos estados F5/F9 desta versao.";}
    }
    game.features.push_back({"dummy_low_guard","Adversario","Defesa baixa","Segura baixo e tras. Nao defende ataques altos ou agarramentos.",kFeatureTraining|kFeatureExclusive});
    game.features.push_back({"dummy_manual","Adversario","Controle manual do J2","Deixa o jogador 2 livre para o segundo teclado/controle.",kFeatureTraining|kFeatureExclusive});
    game.pages.push_back({"Treino","","Ferramentas de treino e adversario programavel.",true});
    game.pages.push_back({"Adversario","Treino","Escolha o comportamento e o slot de gravacao.",false});
    game.commands.push_back({"training_start","Treino","Iniciar nova sessao","Reinicia a placa e prepara a selecao dos dois personagens.",Start});
    game.commands.push_back({"training_restart","Treino","Reiniciar situacao","Backspace restaura a luta ao ponto salvo, preservando as gravacoes.",Restart,kKeyRestart});
    game.commands.push_back({"training_checkpoint","Treino","Salvar ponto de reinicio","Guarda a situacao atual da luta para usar com Backspace.",Capture});
    game.commands.push_back({"training_record","Treino","Gravar / reproduzir","Barra invertida inicia gravacao; outro toque termina e repete. J1 controla J2 ao gravar.",Record,kKeyExtra});
    game.commands.push_back({"training_stop","Treino","Parar gravacao/repeticao","Para e coloca o adversario parado. A gravacao continua no slot.",Stop});
    game.beforeFrame=Before;game.frame=PracticeFrame;game.state=State;game.inTraining=Active;game.noRunAhead=Active;
#ifdef SFZ2AL_ENABLE_LADDER_HARNESS
    const char *probe = std::getenv("SFZ2AL_TEST_LADDER");
    if (probe && std::strcmp(probe, "1") == 0) game.beforeFrame = LadderProbeFrame;
#endif
}
}
