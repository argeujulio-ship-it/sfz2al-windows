// SPDX-License-Identifier: GPL-3.0-only
// Original Portuguese summaries. Main-form move reference; not an exhaustive move list.
#pragma once
#include <array>
namespace gallery {
struct Fighter {const wchar_t *name,*origin,*style,*story,*moves;int x,y;};
inline constexpr std::array<Fighter,18> fighters={{
{L"Ryu",L"Japão",L"Ansatsuken",L"Viaja em busca de adversários e do verdadeiro sentido da luta.",L"Hadouken     ↓ ↘ → + P\r\nShoryuken    → ↓ ↘ + P\r\nTatsumaki    ↓ ↙ ← + K\r\nShinku Hadouken    ↓ ↘ → ↓ ↘ → + P",0,0},
{L"Ken",L"Estados Unidos",L"Ansatsuken",L"Amigo e rival de Ryu, busca aperfeiçoar sua técnica.",L"Hadouken     ↓ ↘ → + P\r\nShoryuken    → ↓ ↘ + P\r\nTatsumaki    ↓ ↙ ← + K\r\nShoryureppa    ↓ ↘ → ↓ ↘ → + P",4,0},
{L"Akuma / Gouki",L"Japão",L"Ansatsuken",L"Entrega-se ao poder do Satsui no Hado e procura desafios extremos.",L"Gou Hadouken    ↓ ↘ → + P\r\nGou Shoryuken    → ↓ ↘ + P\r\nTatsumaki    ↓ ↙ ← + K\r\nShun Goku Satsu    LP, LP, →, LK, HP\r\nExige barra completa.",1,3},
{L"Nash / Charlie",L"Estados Unidos",L"Combate militar",L"Investiga a Shadaloo e enfrenta sua ameaça como militar.",L"Sonic Boom    [←] → + P\r\nSomersault Shell    [↓] ↑ + K\r\nSonic Break    [←] → ← → + P\r\nRepetir P lança outros projéteis.",0,2},
{L"Chun-Li",L"China",L"Kung fu",L"Combate a Shadaloo e busca respostas sobre o desaparecimento do pai.",L"Kikoken    ← ↙ ↓ ↘ → + P\r\nHyakuretsukyaku    K repetidamente\r\nTenshokyaku    [↓] ↑ + K\r\nSenretsukyaku    [←] → ← → + K",2,0},
{L"Adon",L"Tailândia",L"Muay Thai",L"Deseja superar Sagat e provar a força de seu Muay Thai.",L"Jaguar Kick    ↓ ↘ → + K\r\nRising Jaguar    → ↓ ↘ + K\r\nJaguar Tooth    → ↘ ↓ ↙ ← + K\r\nJaguar Varied Assault    ↓ ↘ → ↓ ↘ + P",1,0},
{L"Sodom",L"Estados Unidos",L"Luta com jitte",L"Ex-integrante da Mad Gear, persegue seu ideal de guerreiro japonês.",L"Jigoku Scrape    ↓ ↘ → + P\r\nButsumetsu Buster    360° + P (perto)\r\nDaikyo Burning    ↓ ↘ → ↓ ↘ → + P",3,2},
{L"Guy",L"Estados Unidos",L"Bushinryu",L"Herdeiro do Bushinryu, luta para manter o equilíbrio contra o mal.",L"Hozanto    ↓ ↙ ← + P\r\nBushin Senpukyaku    ↓ ↙ ← + K\r\nHayagake    ↓ ↘ → + K\r\nBushin Hassoken    ↓ ↘ → ↓ ↘ → + P",3,0},
{L"Birdie",L"Reino Unido",L"Briga de rua",L"Procura vantagens e uma posição dentro da organização Shadaloo.",L"Bull Head    [←] → + P\r\nBull Horn    segure P ou K, depois solte\r\nMurderer Chain    360° + K\r\nThe Birdie    [←] → ← → + P",1,2},
{L"Rose",L"Itália",L"Soul Power",L"Usa o Soul Power para enfrentar o poder sombrio de Bison.",L"Soul Spark    ← ↙ ↓ ↘ → + P\r\nSoul Reflect    ↓ ↙ ← + P\r\nSoul Spiral    ↓ ↘ → + K\r\nAura Soul Spark    ↓ ↙ ← ↓ ↙ ← + P",2,2},
{L"Bison / Vega",L"Desconhecida",L"Psycho Power",L"Lidera a Shadaloo e usa o Psycho Power para ampliar seu domínio.",L"Psycho Shot    ↓ ↘ → + P\r\nDouble Knee Press    [←] → + K\r\nHead Press    [↓] ↑ + K\r\nKnee Press Nightmare    [←] → ← → + K",2,3},
{L"Sagat",L"Tailândia",L"Muay Thai",L"Marcado pela derrota para Ryu, procura um novo confronto com o rival.",L"Tiger Shot    ↓ ↘ → + P\r\nTiger Uppercut    → ↓ ↘ + P\r\nTiger Crush    → ↓ ↘ + K\r\nTiger Cannon    ↓ ↘ → ↓ ↘ → + P",4,2},
{L"Dan",L"Hong Kong",L"Saikyo",L"Cria o estilo Saikyo e deseja provar que ele merece respeito.",L"Gadoken    ↓ ↘ → + P\r\nKoryuken    → ↓ ↘ + P\r\nDankukyaku    ↓ ↙ ← + K\r\nShinku Gadoken    ↓ ↘ → ↓ ↘ → + P",3,3},
{L"Sakura",L"Japão",L"Estilo inspirado em Ryu",L"Inspirada por Ryu, procura seu ídolo e aprende lutando.",L"Hadouken    ↓ ↘ → + P\r\nShouoken    → ↓ ↘ + P\r\nShunpukyaku    ↓ ↙ ← + K\r\nShinku Hadouken    ↓ ↘ → ↓ ↘ → + P",2,1},
{L"Rolento",L"Estados Unidos",L"Combate militar",L"Ex-membro da Mad Gear, persegue o projeto de uma sociedade militar.",L"Patriot Circle    ↓ ↘ → + P\r\nMekong Delta Attack    PPP, depois P\r\nStinger    → ↓ ↘ + K, depois K\r\nTake No Prisoners    ↓ ↘ → ↓ ↘ → + K",3,1},
{L"Dhalsim",L"Índia",L"Yoga",L"Luta por seu povo, conciliando o combate com sua vida espiritual.",L"Yoga Fire    ↓ ↘ → + P\r\nYoga Flame    → ↘ ↓ ↙ ← + P\r\nYoga Teleport    → ↓ ↘ + PPP ou KKK\r\nYoga Inferno    ↓ ↘ → ↓ ↘ → + P",0,1},
{L"Zangief",L"Rússia",L"Luta livre",L"Defende com orgulho a força e a tradição da luta livre.",L"Spinning Piledriver    360° + P (perto)\r\nDouble Lariat    PPP\r\nBanishing Flat    → ↓ ↘ + P\r\nFinal Atomic Buster    720° + P (perto)",4,1},
{L"Gen",L"China",L"Mantis / Crane",L"Mestre de dois estilos, procura um adversário à altura de sua técnica.",L"Trocar estilo    PPP / KKK\r\nMantis — Hyakurenko    P repetidamente\r\nMantis — Gekiro    → ↓ ↘ + K\r\nMantis — Zan'ei    ↓ ↘ → ↓ ↘ → + P",1,1}
}};
}
