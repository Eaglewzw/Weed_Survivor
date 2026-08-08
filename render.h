#ifndef RENDER_H
#define RENDER_H

#include "globals.h"

// ==================== 世界实体绘制 ====================
void drawBackground();
void drawPlayer();
void drawEnemy(int idx);
void drawGem(int idx);
void drawProjectile(int idx);
void drawScythe();
void drawAura();
void drawSlowZones();
void drawBossAoE(int idx);

#endif
