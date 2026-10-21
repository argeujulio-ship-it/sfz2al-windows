// SPDX-License-Identifier: GPL-3.0-only
// Main-form reference. Text and practice instructions authored for this app.
#pragma once
#include "gallery_data.hpp"
#include <string>
namespace gallery {
enum class Page { Story, Simple, Super, Combos };
inline constexpr std::array<const wchar_t*,18> comboRoutes={{
    L"c.MK xx Hadouken\r\nCancele o chute médio agachado com ↓ ↘ → + P.",
    L"c.MP xx Shoryuken\r\nCancele o soco médio agachado com → ↓ ↘ + P.",
    L"c.MK xx Gou Hadouken\r\nCancele o chute médio agachado com ↓ ↘ → + P.",
    L"c.MP xx Sonic Boom\r\nCarregue ↙ antes do soco; finalize com → + P.",
    L"c.MP xx Kikoken\r\nCancele o soco médio agachado com ← ↙ ↓ ↘ → + P.",
    L"c.MP xx Rising Jaguar\r\nCancele o soco médio agachado com → ↓ ↘ + K.",
    L"c.HP xx Jigoku Scrape\r\nCancele o soco forte agachado com ↓ ↘ → + P.",
    L"c.HP xx Hozanto\r\nCancele o soco forte agachado com ↓ ↙ ← + P.",
    L"c.MP xx Bull Head\r\nCarregue ↙ antes do soco; finalize com → + P.",
    L"c.MP xx Soul Spiral\r\nCancele o soco médio agachado com ↓ ↘ → + K.",
    L"c.MK xx Double Knee Press\r\nCarregue ↙ antes do chute; finalize com → + K.",
    L"c.MP xx Tiger Shot\r\nCancele o soco médio agachado com ↓ ↘ → + P.",
    L"c.MP xx Gadoken\r\nBem perto, cancele o soco médio agachado com ↓ ↘ → + P.",
    L"c.HP xx Shouoken\r\nCancele o soco forte agachado com → ↓ ↘ + P.",
    L"c.MP xx Patriot Circle\r\nCancele o soco médio agachado com ↓ ↘ → + P. Repita o movimento para continuar.",
    L"c.LP xx Yoga Fire\r\nBem perto, use ↙ + LP para o soco curto agachado e cancele com ↓ ↘ → + P.",
    L"c.LP > c.LP\r\nPerto do adversário, conecte dois socos fracos agachados. Não é cancelamento para agarrão.",
    L"Mantis: c.MP xx Hyakurenko\r\nUse PPP para Mantis. Cancele o soco médio agachado com P repetidamente."
}};
inline std::wstring MoveSection(int id,bool super){
    const std::wstring all=fighters[id].moves;size_t split=0;
    const int simpleLines=(id==3||id==6)?2:3;
    for(int i=0;i<simpleLines;++i){auto next=all.find(L"\r\n",split);if(next==std::wstring::npos)return super?L"":all;split=next+2;}
    return super?all.substr(split):all.substr(0,split-2);
}
inline const wchar_t *Notation(){return L"Direções para o personagem olhando à direita; inverta ao trocar de lado.\r\n\r\nP = soco; K = chute.\r\nL / M / H = fraco / médio / forte.\r\nLP / MP / HP = socos; LK / MK / HK = chutes.\r\nPPP / KKK = três botões juntos.\r\n[ ] = mantenha a direção para carregar.\r\n360° / 720° = uma / duas voltas.\r\nj. = no salto; c. = agachado.\r\n> = próximo golpe, acertando o tempo.\r\nxx = cancelar a animação no impacto.\r\n\r\nReferência da forma principal; não inclui todas as variantes.";}
inline std::wstring PageText(int id,Page page,bool open){
    const auto &f=fighters[id];std::wstring text=std::wstring(f.name)+L"\r\n\r\n";
    if(page==Page::Story){
        if(open)return text+L"CAMPANHA CONCLUÍDA\r\n\r\nOrigem: "+f.origin+L"\r\nEstilo: "+f.style+L"\r\n\r\n"+f.story;
        return text+L"HISTÓRIA BLOQUEADA\r\n\r\nConclua o Arcade com este personagem para liberar sua imagem e história.\r\n\r\nAs abas de golpes e combos podem ser consultadas desde o início.";
    }
    if(page==Page::Simple)text+=L"GOLPES SIMPLES\r\n\r\nNormais: P ou K; segure ↓ para atacar agachado. No ar, use P ou K durante o salto.\r\n\r\nESPECIAIS SEM BARRA\r\n\r\n"+MoveSection(id,false);
    else if(page==Page::Super)text+=L"SUPER COMBOS\r\n\r\n"+MoveSection(id,true)+L"\r\n\r\nExigem barra. P/K usa o nível 1; dois botões, nível 2; três botões, nível 3, quando o golpe permitir. Shun Goku Satsu exige nível 3.\r\n\r\nSão ataques especiais com gasto de barra, diferentes das sequências da aba Combos.";
    else {
        text+=L"COMBOS PARA PRATICAR\r\n\r\nCANCELAMENTO / LIGAÇÃO\r\n"+std::wstring(comboRoutes[id])+L"\r\n\r\nComece perto de um adversário parado, sem defesa. No cancelamento, complete o movimento durante o impacto do golpe anterior. O botão e a distância podem mudar o resultado.";
        if(id==0||id==1||id==2||id==5||id==7||id==8||id==17){
            text+=L"\r\n\r\nENTRADA PELO ALTO\r\n";
            text+=id==17?L"j.HK > c.HK\r\nAcerte o chute no fim do salto.":L"j.HP > c.HK\r\nAcerte o soco no fim do salto.";
            text+=L" Aterrisse e faça ↓ + HK. Comece com salto vertical perto do adversário, sem atravessar para o outro lado.";
        }
        text+=L"\r\n\r\nConfira o contador de acertos: se ele reiniciar entre os ataques, a sequência não conectou como combo. Comece com dois golpes antes de acrescentar um especial.\r\n\r\nExemplos iniciais: ajuste tempo e distância e confirme a conexão no treino. Não são sequências automáticas nem comandos de um botão.";
    }
    return text+L"\r\n\r\nLEGENDA\r\n\r\n"+Notation();
}
}
