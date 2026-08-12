#ifndef CONFIG_H
#define CONFIG_H

// ==================== 游戏世界 ====================
#define WORLD_W  960     // 3x screen width
#define WORLD_H  600     // large play area
#define SW       320
#define SH       240

// ==================== 游戏区域（屏幕空间） ====================
#define PLAY_AREA_Y     22
#define PLAY_AREA_H     190
#define PLAY_AREA_BOT   (PLAY_AREA_Y + PLAY_AREA_H)  // 212
#define UI_BAR_H        14
#define BOTTOM_BAR_Y    212

// ==================== 实体上限 ====================
#define MAX_ENEMIES      120
#define MAX_GEMS         30
#define MAX_PROJECTILES  30
#define MAX_WEAPONS      4
#define MAX_PASSIVES     5
#define MAX_BOSS_AOE     12
#define MAX_SLOW_ZONES   8
#define MAX_PARTICLES    64
#define MAX_FLOATERS     16
#define MAX_CHESTS       6

// ==================== 玩家参数 ====================
#define PLAYER_RADIUS    6
#define PLAYER_SPEED     140.0f
#define INVINCIBLE_TIME  0.4f

#endif
