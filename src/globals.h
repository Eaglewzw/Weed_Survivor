#ifndef GLOBALS_H
#define GLOBALS_H

#include "src/types.h"

// ==================== 硬件对象 ====================
extern TFT_eSPI tft;
extern TFT_eSprite gameSpr;

// ==================== 游戏状态 ====================
extern Player player;
extern Enemy enemies[MAX_ENEMIES];
extern Gem gems[MAX_GEMS];
extern Projectile projectiles[MAX_PROJECTILES];
extern SlowZone slowZones[MAX_SLOW_ZONES];
extern BossAoE bossAoEs[MAX_BOSS_AOE];

extern int enemyCount;
extern int gemCount;
extern int projectileCount;
extern int slowZoneCount;
extern uint8_t bossAoECount;

extern float gameTime;
extern int killCount;
extern unsigned long lastFrameTime;

extern bool upgrading;
extern bool upgradeDirty;
extern bool gameWon;
extern bool bossActive;
extern float bossAttackTimer;
extern uint8_t bossAttackMode;
extern float bossAttackPhase;
extern float screenShake;

extern int shakeX, shakeY;
extern bool gameOver;
extern bool paused;
extern unsigned long screenEnterTime;

extern UpgradeOption upgradeOptions[3];
extern uint8_t upgradeOptionCount;
extern int8_t upgradeSelection;

extern uint32_t rngState;

// ==================== 玩家动画 ====================
extern float playerWalkPhase;
extern float playerAttackFlash;
extern float playerHurtFlash;
extern bool  playerMoving;

// ==================== 相机 ====================
extern float cameraX, cameraY;

// ==================== 武器/被动名称 ====================
extern const char* weaponNames[];
extern const char* passiveNames[];

#endif
