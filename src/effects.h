#ifndef EFFECTS_H
#define EFFECTS_H

#include "globals.h"

void spawnDeathBurst(float x, float y, uint8_t enemyType);
void spawnFloater(float x, float y, uint16_t value);
void updateEffects(float dt);
void drawEffects();

#endif
