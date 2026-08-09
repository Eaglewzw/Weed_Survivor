/**
 * 割草幸存者 - Wio Terminal 版
 * 吸血鬼幸存者风格割草游戏，大世界+邪恶怪物
 * 320x240 LCD，五向摇杆移动，按钮选择升级
 */

#include "src/globals.h"
#include "src/utils.h"
#include "src/audio.h"
#include "src/player.h"
#include "src/enemy.h"
#include "src/game.h"
#include "src/render.h"
#include "src/ui.h"

// ==================== Arduino Setup ====================
void setup() {
  // 背光
  pinMode(LCD_BACKLIGHT, OUTPUT);
  digitalWrite(LCD_BACKLIGHT, HIGH);

  // 初始化屏幕
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  gameSpr.createSprite(SW, SH);

  // 初始化按钮
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);

  // 蜂鸣器
  pinMode(WIO_BUZZER, OUTPUT);

  // 初始化游戏
  memset(enemies, 0, sizeof(enemies));
  memset(gems, 0, sizeof(gems));
  memset(projectiles, 0, sizeof(projectiles));
  memset(slowZones, 0, sizeof(slowZones));
  memset(bossAoEs, 0, sizeof(bossAoEs)); bossAoECount = 0;
  enemyCount = 0; gemCount = 0; projectileCount = 0; slowZoneCount = 0;
  gameTime = 0; killCount = 0;
  upgrading = false; gameOver = false;

  initPlayer();
  rngState = micros();
  updateCamera();

  lastFrameTime = millis();

  // ===== 启动画面 =====
  tft.fillScreen(0x0000);

  // 底部草地
  for (int i = 0; i < 30; i++) {
    int gx = (i * 23 + 10) % SW;
    tft.fillTriangle(gx, 220, gx - 8, 240, gx + 8, 240, 0x0A40);
    tft.fillTriangle(gx + 6, 215, gx - 2, 240, gx + 12, 240, 0x06A0);
  }

  unsigned long startTime = millis();
  bool pressAVisible = true;
  uint16_t lastTitleColor = 0;
  int lastIconR[3] = {0, 0, 0};
  float lastIconA[3] = {0, 0, 0};

  while (digitalRead(WIO_KEY_A) == HIGH) {
    unsigned long t = millis() - startTime;

    // 彩虹粒子背景
    for (int i = 0; i < 12; i++) {
      int px = (rngNext() % SW);
      int py = (rngNext() % 200);
      uint16_t pc = tft.color565(
        (uint8_t)(128 + sinf(t * 0.003f + i * 0.7f) * 127),
        (uint8_t)(128 + sinf(t * 0.003f + i * 0.7f + 2.1f) * 127),
        (uint8_t)(128 + sinf(t * 0.003f + i * 0.7f + 4.2f) * 127)
      );
      tft.drawPixel(px, py, pc);
    }

    // 标题大字：彩虹呼吸
    float hue = fmodf(t * 0.0004f, 1.0f);
    float rh = hue; if (rh > 1.0f) rh -= 1.0f;
    float gh = hue + 0.33f; if (gh > 1.0f) gh -= 1.0f;
    float bh = hue + 0.66f; if (bh > 1.0f) bh -= 1.0f;
    uint16_t tc = tft.color565(
      (uint8_t)((sinf(rh * 6.283f) * 0.5f + 0.5f) * 255),
      (uint8_t)((sinf(gh * 6.283f) * 0.5f + 0.5f) * 255),
      (uint8_t)((sinf(bh * 6.283f) * 0.5f + 0.5f) * 255)
    );

    if (tc != lastTitleColor) {
      tft.fillRect(0, 10, SW, 60, 0x0000);
      tft.setTextSize(4);
      tft.setTextColor(tc, 0x0000);
      tft.drawString("WEED", 30, 15);
      tft.setTextSize(2);
      tft.setTextColor(0xFFE0, 0x0000);
      tft.drawString("SURVIVOR", 75, 55);
      lastTitleColor = tc;
    }

    // 武器图标旋转动画
    int iconY = 130;
    uint16_t iconColors[] = {0x07E0, 0xFFE0, 0x07FF};
    const char* iconNames[] = {"Leaf", "Cyclone", "Aura"};
    for (int i = 0; i < 3; i++) {
      int cx = 80 + i * 80;
      float r = 10 + sinf((t + i * 600) * 0.005f) * 4;
      float a = (t + i * 400) * 0.004f;
      int ir = (int)r;

      if (ir != lastIconR[i] || fabsf(a - lastIconA[i]) > 0.05f) {
        tft.fillRect(cx - 18, iconY - 18, 36, 42, 0x0000);

        tft.drawCircle(cx, iconY, ir, iconColors[i]);
        tft.drawCircle(cx, iconY, ir - 1, iconColors[i]);
        if (i == 0) {
          for (int p = 0; p < 3; p++) {
            float aa = a + p * 2.094f;
            tft.drawLine(cx, iconY, cx + cosf(aa) * 12, iconY + sinf(aa) * 12, iconColors[i]);
          }
        } else if (i == 1) {
          tft.drawLine(cx, iconY, cx + cosf(a) * 14, iconY + sinf(a) * 14, iconColors[i]);
          tft.drawLine(cx, iconY, cx + cosf(a + 1.5f) * 10, iconY + sinf(a + 1.5f) * 10, iconColors[i]);
        } else {
          tft.drawCircle(cx, iconY, 8 + sinf((t + 200) * 0.004f) * 3, iconColors[i]);
        }

        tft.setTextSize(1);
        tft.setTextColor(iconColors[i], 0x0000);
        tft.drawString(iconNames[i], cx - 12, iconY + 14);
        lastIconR[i] = ir;
        lastIconA[i] = a;
      }
    }

    // "Press A" 闪烁
    bool show = ((t / 500) % 2 == 0);
    if (show != pressAVisible) {
      pressAVisible = show;
      if (show) {
        tft.setTextSize(2);
        tft.setTextColor(0xFC00, 0x0000);
        tft.drawString("Press A", 100, 175);
        tft.setTextSize(1);
      } else {
        tft.fillRect(100, 175, 110, 24, 0x0000);
      }
    }

    delay(30);
  }
  delay(200);
  beep(600, 100);
}

// ==================== Arduino Loop ====================
void loop() {
  unsigned long now = millis();
  float dt = (now - lastFrameTime) / 1000.0f;
  if (dt > 0.1f) dt = 0.1f;
  if (dt <= 0) dt = 0.016f;
  lastFrameTime = now;

  // 暂停/继续 (C键)
  static bool cPressed = false;
  bool cNow = (digitalRead(WIO_KEY_C) == LOW);
  if (cNow && !cPressed) {
    if (!paused) {
      paused = true;
      beep(600, 50);
    } else {
      paused = false;
      lastFrameTime = millis();
      beep(800, 50);
    }
  }
  cPressed = cNow;

  if (paused) {
    drawPauseOverlay();
    gameSpr.pushSprite(0, 0);
    return;
  }

  // 游戏结束画面
  static uint8_t lastScreen = 0;

  if (gameWon) {
    if (lastScreen != 1) { screenEnterTime = millis(); lastScreen = 1; }
    drawVictoryScreen();
    if (digitalRead(WIO_KEY_A) == LOW) {
      delay(200);
      memset(enemies, 0, sizeof(enemies));
      memset(gems, 0, sizeof(gems));
      memset(projectiles, 0, sizeof(projectiles));
      memset(slowZones, 0, sizeof(slowZones));
      memset(bossAoEs, 0, sizeof(bossAoEs)); bossAoECount = 0;
      enemyCount = 0; gemCount = 0; projectileCount = 0; slowZoneCount = 0;
      gameTime = 0; killCount = 0;
      upgrading = false; gameOver = false; gameWon = false;
      bossActive = false; bossAttackTimer = 0; paused = false;
      lastScreen = 0;
      initPlayer();
      rngState = micros();
      lastFrameTime = millis();
      tft.fillScreen(0x0000);
    }
    return;
  }

  if (gameOver) {
    if (lastScreen != 2) { screenEnterTime = millis(); lastScreen = 2; }
    drawGameOverScreen();
    if (digitalRead(WIO_KEY_A) == LOW) {
      delay(200);
      memset(enemies, 0, sizeof(enemies));
      memset(gems, 0, sizeof(gems));
      memset(projectiles, 0, sizeof(projectiles));
      memset(slowZones, 0, sizeof(slowZones));
      memset(bossAoEs, 0, sizeof(bossAoEs)); bossAoECount = 0;
      enemyCount = 0; gemCount = 0; projectileCount = 0; slowZoneCount = 0;
      gameTime = 0; killCount = 0;
      upgrading = false; gameOver = false; gameWon = false; bossActive = false; bossAttackTimer = 0;
      paused = false; lastScreen = 0;
      initPlayer();
      rngState = micros();
      lastFrameTime = millis();
      tft.fillScreen(0x0000);
    }
    return;
  }

  // 升级面板
  if (upgrading) {
    static bool upPrev = false, downPrev = false;
    bool upNow = (digitalRead(WIO_5S_UP) == LOW);
    bool downNow = (digitalRead(WIO_5S_DOWN) == LOW);
    if (upNow && !upPrev) {
      upgradeSelection--;
      if (upgradeSelection < 0) upgradeSelection = upgradeOptionCount - 1;
      upgradeDirty = true;
      beep(800, 20);
    }
    if (downNow && !downPrev) {
      upgradeSelection++;
      if (upgradeSelection >= upgradeOptionCount) upgradeSelection = 0;
      upgradeDirty = true;
      beep(800, 20);
    }
    upPrev = upNow; downPrev = downNow;

    drawUpgradePanel();
    gameSpr.pushSprite(0, 0);

    if (digitalRead(WIO_5S_PRESS) == LOW) {
      applyUpgrade(upgradeSelection);
      delay(200);
    }
    return;
  }

  // 更新游戏
  updateGame(dt);

  // 屏幕震动偏移
  shakeX = 0; shakeY = 0;
  if (screenShake > 0) {
    shakeX = (rngNext() % 5) - 2;
    shakeY = (rngNext() % 5) - 2;
  }

  // 绘制
  drawBackground();
  drawSlowZones();
  for (int i = 0; i < MAX_GEMS; i++) if (gems[i].life > 0) drawGem(i);
  for (int i = 0; i < MAX_ENEMIES; i++) if (enemies[i].alive) drawEnemy(i);
  for (int i = 0; i < MAX_BOSS_AOE; i++) if (bossAoEs[i].active) drawBossAoE(i);
  for (int i = 0; i < MAX_PROJECTILES; i++) if (projectiles[i].alive) drawProjectile(i);
  drawScythe();
  drawAura();
  drawPlayer();
  drawUI();
  gameSpr.pushSprite(0, 0);
}
