#ifndef ENEMY_H
#define ENEMY_H

#include "globals.h"

// ==================== 敌人槽位 ====================
int findFreeEnemySlot();
int8_t findBoss();

// ==================== BOSS攻击 ====================
void addBossAoE(float x, float y, float r, float maxR, uint8_t typ, uint16_t col);
void bossProjectile(int8_t bossIdx, float angle, float speed);

// ==================== 生成敌人 ====================
void spawnEnemy();
void spawnElite();
void spawnBoss();

#endif
