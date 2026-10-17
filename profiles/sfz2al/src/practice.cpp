// SPDX-License-Identifier: GPL-3.0-only
// Local SFZ2AL practice options. Addresses checked against this target's RAM traces.
#include "game.h"
#include <cstdlib>
#include <cstring>

namespace cps2recomp {
namespace {
bool Fighting() {
    return Ram16(0xFF8000) == 2 && Ram16(0xFF8004) == 4 &&
           Ram16(0xFF8008) == 4 && Ram16(0xFF800C) == 2;
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
void PracticeFrame(Machine &) {
    if (!Fighting()) return;
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
}
}
void RegisterGame() {
    Game &game = TheGame();
    game.features.push_back({"infinite_time", "Treino", "Tempo infinito",
                             "Mantem o relogio em 99 durante a luta."});
    game.features.push_back({"refill_health", "Treino", "Repor vida em repouso",
                             "Recupera a vida dos dois lutadores ao voltarem ao repouso. Nao impede um nocaute."});
    game.frame = PracticeFrame;
#ifdef SFZ2AL_ENABLE_LADDER_HARNESS
    const char *probe = std::getenv("SFZ2AL_TEST_LADDER");
    if (probe && std::strcmp(probe, "1") == 0) game.beforeFrame = LadderProbeFrame;
#endif
}
}
