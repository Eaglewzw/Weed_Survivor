#ifndef GAME_H
#define GAME_H

#include "src/globals.h"

// ==================== 主更新 ====================
void updateGame(float dt);

// ==================== 投射物 ====================
int findFreeProjectileSlot();
void fireProjectile(float x, float y, float angle, float speed, float damage, int8_t pierce, float range);

// ==================== 伤害/宝石 ====================
void damageEnemy(int idx, float damage);
int findFreeGemSlot();
void spawnGem(float x, float y, uint8_t value);

#endif
