#ifndef TYPES_H
#define TYPES_H

#include <TFT_eSPI.h>
#include "config.h"

// ==================== 敌人类型 ====================
enum EnemyType {
  ENEMY_WEED = 0,
  ENEMY_DANDELION = 1,
  ENEMY_THORN = 2,
  ENEMY_ELITE = 3,
  ENEMY_BOSS = 4
};

// ==================== 数据结构 ====================

struct Enemy {
  float x, y;
  float hp, maxHp;
  float speed;
  float damage;
  uint8_t type;
  bool alive;
  uint8_t xpValue;
};

struct Gem {
  float x, y;
  uint8_t value;
  float life;
};

struct Projectile {
  float x, y;
  float vx, vy;
  float damage;
  int8_t pierce;
  float traveled;
  float maxRange;
  bool alive;
  uint8_t hitMask;
};

struct WeaponState {
  uint8_t level;
  uint8_t weaponId;
  bool evolved;
  float cooldown;
  float cooldownMax;
  float damage;
  float extraParam;
  float extraParam2;
};

struct PassiveState {
  uint8_t level;
  uint8_t passiveId;
};

struct Player {
  float x, y;
  float hp, maxHp;
  float exp, expToNext;
  float speed;
  float damageMult;
  float cooldownMult;
  float pickupRange;
  float armor;
  float regen;
  float invincibleTimer;
  uint8_t level;
  float facingAngle;

  WeaponState weapons[MAX_WEAPONS];
  uint8_t weaponCount;
  PassiveState passives[MAX_PASSIVES];
  uint8_t passiveCount;
};

struct UpgradeOption {
  uint8_t type;
  uint8_t id;
};

struct BossAoE {
  bool active;
  float x, y;
  float radius;
  float maxRadius;
  float timer;
  uint8_t type;
  uint16_t color;
};

struct SlowZone {
  float x, y;
  float life;
};

struct Candidate {
  uint8_t type;
  uint8_t id;
  int priority;
};

#endif
