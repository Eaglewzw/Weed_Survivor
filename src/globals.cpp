#include <Arduino.h>
#include "src/globals.h"

// ==================== 硬件对象 ====================
TFT_eSPI tft;
TFT_eSprite gameSpr = TFT_eSprite(&tft);

// ==================== 游戏状态 ====================
Player player;
Enemy enemies[MAX_ENEMIES];
Gem gems[MAX_GEMS];
Projectile projectiles[MAX_PROJECTILES];
SlowZone slowZones[MAX_SLOW_ZONES];
BossAoE bossAoEs[MAX_BOSS_AOE];

int enemyCount = 0;
int gemCount = 0;
int projectileCount = 0;
int slowZoneCount = 0;
uint8_t bossAoECount = 0;

float gameTime = 0;
int killCount = 0;
unsigned long lastFrameTime = 0;

bool upgrading = false;
bool upgradeDirty = true;
bool gameWon = false;
bool bossActive = false;
float bossAttackTimer = 0;
uint8_t bossAttackMode = 0;
float bossAttackPhase = 0;
float screenShake = 0;

int shakeX = 0, shakeY = 0;
bool gameOver = false;
bool paused = false;
unsigned long screenEnterTime = 0;

UpgradeOption upgradeOptions[3];
uint8_t upgradeOptionCount = 0;
int8_t upgradeSelection = 0;

uint32_t rngState = 12345;

// ==================== 玩家动画 ====================
float playerWalkPhase = 0;
float playerAttackFlash = 0;
float playerHurtFlash = 0;
bool  playerMoving = false;

// ==================== 相机 ====================
float cameraX = 0, cameraY = 0;

// ==================== 武器/被动名称 ====================
const char* weaponNames[] = {"Magic Leaf", "Cyclone", "Aura"};
const char* passiveNames[] = {"ATK+", "SPD+", "MOV+", "HP+", "ARMOR"};
