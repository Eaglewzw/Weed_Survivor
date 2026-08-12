#include "effects.h"
#include "utils.h"

// 3x5 像素数字字体（用于伤害飘字）
static const uint8_t DIGITS[10][5] = {
  {0x7, 0x5, 0x5, 0x5, 0x7}, // 0
  {0x2, 0x6, 0x2, 0x2, 0x7}, // 1
  {0x7, 0x1, 0x7, 0x4, 0x7}, // 2
  {0x7, 0x1, 0x7, 0x1, 0x7}, // 3
  {0x5, 0x5, 0x7, 0x1, 0x1}, // 4
  {0x7, 0x4, 0x7, 0x1, 0x7}, // 5
  {0x7, 0x4, 0x7, 0x5, 0x7}, // 6
  {0x7, 0x1, 0x1, 0x1, 0x1}, // 7
  {0x7, 0x5, 0x7, 0x5, 0x7}, // 8
  {0x7, 0x5, 0x7, 0x1, 0x7}  // 9
};

static void addParticle(float x, float y, float vx, float vy, float life, uint16_t color) {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].life > 0) continue;
    Particle& p = particles[i];
    p.x = x; p.y = y; p.vx = vx; p.vy = vy;
    p.life = life; p.maxLife = life; p.color = color;
    return;
  }
}

// ==================== 死亡粒子爆发 ====================
void spawnDeathBurst(float x, float y, uint8_t enemyType) {
  uint16_t base;
  int n;
  switch (enemyType) {
    case ENEMY_DANDELION: base = 0xE73C; n = 6;  break;
    case ENEMY_THORN:     base = 0xF800; n = 7;  break;
    case ENEMY_ELITE:     base = 0xB81F; n = 12; break;
    case ENEMY_BOSS:      base = 0x07E0; n = 40; break;
    default:              base = 0x07E0; n = 5;  break;
  }
  for (int i = 0; i < n; i++) {
    float a = rngFloat() * 6.28318f;
    float sp = 25.0f + rngFloat() * 70.0f;
    uint16_t c = (rngFloat() < 0.25f) ? 0xFFFF : base;
    addParticle(x, y, cosf(a) * sp, sinf(a) * sp, 0.3f + rngFloat() * 0.35f, c);
  }
}

// ==================== 伤害飘字 ====================
void spawnFloater(float x, float y, uint16_t value) {
  if (value > 999) value = 999;
  for (int i = 0; i < MAX_FLOATERS; i++) {
    if (floaters[i].life > 0) continue;
    floaters[i].x = x;
    floaters[i].y = y - 10.0f;
    floaters[i].value = value;
    floaters[i].life = 0.6f;
    return;
  }
}

// ==================== 特效更新 ====================
void updateEffects(float dt) {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    Particle& p = particles[i];
    if (p.life <= 0) continue;
    p.life -= dt;
    if (p.life <= 0) continue;
    p.x += p.vx * dt;
    p.y += p.vy * dt;
    float damp = 1.0f - 4.0f * dt;
    if (damp < 0) damp = 0;
    p.vx *= damp;
    p.vy *= damp;
  }
  for (int i = 0; i < MAX_FLOATERS; i++) {
    Floater& f = floaters[i];
    if (f.life <= 0) continue;
    f.life -= dt;
    f.y -= 45.0f * dt;
  }
}

// ==================== 像素数字绘制 ====================
static void drawDigit(int x, int y, uint8_t digit, uint16_t color) {
  for (int row = 0; row < 5; row++) {
    uint8_t bits = DIGITS[digit][row];
    for (int col = 0; col < 3; col++) {
      if (bits & (1 << (2 - col))) {
        gameSpr.fillRect(x + col * 2, y + row * 2, 2, 2, color);
      }
    }
  }
}

static void drawNumberCentered(int cx, int y, uint16_t value, uint16_t color) {
  char buf[4];
  snprintf(buf, 4, "%u", value);
  int len = strlen(buf);
  int w = len * 6 + (len - 1); // 每数字6px宽+1px间距（2x缩放）
  int x = cx - w / 2;
  for (int i = 0; i < len; i++) {
    drawDigit(x + i * 7 + 1, y + 1, buf[i] - '0', 0x0000);
    drawDigit(x + i * 7, y, buf[i] - '0', color);
  }
}

// ==================== 特效绘制 ====================
void drawEffects() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    Particle& p = particles[i];
    if (p.life <= 0) continue;
    int sx = toScreenX(p.x), sy = toScreenY(p.y);
    if (sx < 0 || sx >= SW || sy < PLAY_AREA_Y || sy >= PLAY_AREA_BOT) continue;
    int s = (p.life > p.maxLife * 0.5f) ? 2 : 1;
    gameSpr.fillRect(sx, sy, s, s, p.color);
  }
  for (int i = 0; i < MAX_FLOATERS; i++) {
    Floater& f = floaters[i];
    if (f.life <= 0) continue;
    int sx = toScreenX(f.x), sy = toScreenY(f.y);
    if (sx < -30 || sx > SW + 30 || sy < PLAY_AREA_Y - 12 || sy > PLAY_AREA_BOT + 12) continue;
    uint16_t c = (f.value >= 100) ? 0xFFE0 : 0xFFFF;
    drawNumberCentered(sx, sy, f.value, c);
  }
}
