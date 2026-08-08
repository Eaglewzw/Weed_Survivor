#include <Arduino.h>
#include "src/enemy.h"
#include "src/utils.h"
#include "src/audio.h"
#include "src/player.h"

// ==================== 敌人槽位 ====================
int findFreeEnemySlot() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].alive) return i;
  }
  return -1;
}

int8_t findBoss() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].alive && enemies[i].type == ENEMY_BOSS) return i;
  }
  return -1;
}

// ==================== BOSS范围攻击 ====================
void addBossAoE(float x, float y, float r, float maxR, uint8_t typ, uint16_t col) {
  int slot = -1;
  for (int i = 0; i < MAX_BOSS_AOE; i++) {
    if (!bossAoEs[i].active) { slot = i; break; }
  }
  if (slot < 0) return;
  BossAoE& aoe = bossAoEs[slot];
  aoe.active = true;
  aoe.x = x; aoe.y = y;
  aoe.radius = r; aoe.maxRadius = maxR;
  aoe.timer = 0; aoe.type = typ; aoe.color = col;
  bossAoECount++;
}

void bossProjectile(int8_t bossIdx, float angle, float speed) {
  if (projectileCount >= MAX_PROJECTILES) return;
  int slot = -1;
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (!projectiles[i].alive) { slot = i; break; }
  }
  if (slot < 0) return;
  Projectile& p = projectiles[slot];
  p.alive = true;
  p.x = enemies[bossIdx].x;
  p.y = enemies[bossIdx].y;
  p.vx = cosf(angle) * speed;
  p.vy = sinf(angle) * speed;
  p.damage = 25;
  p.pierce = 0;
  p.traveled = 0;
  p.maxRange = 300;
  p.hitMask = 0;
  projectileCount++;
}

// ==================== 生成敌人（在视口边缘生成，适配大世界） ====================
void spawnEnemy() {
  int slot = findFreeEnemySlot();
  if (slot < 0) return;

  float x, y;
  int side = rngInt(0, 3);
  switch (side) {
    case 0: x = cameraX + rngFloat() * SW;        y = cameraY - 10;              break;
    case 1: x = cameraX + SW + 10;                y = cameraY + rngFloat() * PLAY_AREA_H; break;
    case 2: x = cameraX + rngFloat() * SW;        y = cameraY + PLAY_AREA_H + 10; break;
    case 3: x = cameraX - 10;                     y = cameraY + rngFloat() * PLAY_AREA_H; break;
  }

  float t = gameTime / 60.0f;
  float hpScale = 1.0f + t * 0.08f;
  float spdScale = 1.0f + t * 0.03f;
  float dmgScale = 1.0f + t * 0.05f;

  Enemy& e = enemies[slot];
  e.alive = true;
  e.x = x;
  e.y = y;

  float r = rngFloat();
  if (t < 2) {
    e.type = ENEMY_WEED;
  } else if (t < 5) {
    e.type = (r < 0.7f) ? ENEMY_WEED : ENEMY_DANDELION;
  } else if (t < 10) {
    e.type = (r < 0.4f) ? ENEMY_WEED : (r < 0.7f) ? ENEMY_DANDELION : ENEMY_THORN;
  } else {
    e.type = (r < 0.2f) ? ENEMY_WEED : (r < 0.4f) ? ENEMY_DANDELION : (r < 0.5f) ? ENEMY_THORN : ENEMY_ELITE;
  }

  switch (e.type) {
    case ENEMY_WEED:
      e.hp = 5 * hpScale; e.speed = 55 * spdScale; e.damage = 5 * dmgScale; e.xpValue = 3; break;
    case ENEMY_DANDELION:
      e.hp = 9 * hpScale; e.speed = 40 * spdScale; e.damage = 7 * dmgScale; e.xpValue = 5; break;
    case ENEMY_THORN:
      e.hp = 14 * hpScale; e.speed = 70 * spdScale; e.damage = 11 * dmgScale; e.xpValue = 8; break;
    case ENEMY_ELITE:
      e.hp = 90 * hpScale; e.speed = 30 * spdScale; e.damage = 18 * dmgScale; e.xpValue = 40; break;
  }
  e.maxHp = e.hp;
  enemyCount++;
}

void spawnElite() {
  int slot = findFreeEnemySlot();
  if (slot < 0) return;
  float x, y;
  int side = rngInt(0, 3);
  switch (side) {
    case 0: x = cameraX + rngFloat() * SW;        y = cameraY - 15;              break;
    case 1: x = cameraX + SW + 15;                y = cameraY + rngFloat() * PLAY_AREA_H; break;
    case 2: x = cameraX + rngFloat() * SW;        y = cameraY + PLAY_AREA_H + 15; break;
    case 3: x = cameraX - 15;                     y = cameraY + rngFloat() * PLAY_AREA_H; break;
  }
  float t = gameTime / 60.0f;
  float hpScale = 1.0f + t * 0.08f;
  Enemy& e = enemies[slot];
  e.alive = true; e.x = x; e.y = y; e.type = ENEMY_ELITE;
  e.hp = 100 * hpScale; e.maxHp = e.hp;
  e.speed = 25; e.damage = 40; e.xpValue = 30;
  enemyCount++;
}

void spawnBoss() {
  int slot = findFreeEnemySlot();
  if (slot < 0) return;
  Enemy& e = enemies[slot];
  e.alive = true;
  e.x = player.x;
  e.y = player.y - 200;  // spawn above player in world space
  // Clamp to world
  if (e.y < 40) e.y = 40;
  e.type = ENEMY_BOSS;
  e.hp = player.maxHp * 1000.0f;
  e.maxHp = e.hp;
  e.speed = 8;
  e.damage = 300;
  e.xpValue = 0;
  bossActive = true;
  bossAttackTimer = 3.0f;
  enemyCount++;
  soundEvolve();
}
