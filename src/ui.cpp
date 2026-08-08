#include <Arduino.h>
#include "src/ui.h"
#include "src/utils.h"
#include "src/player.h"
#include "src/enemy.h"

// ==================== 主UI栏 ====================
void drawUI() {
  char buf[30];

  // ===== 顶部 HP 条 =====
  gameSpr.fillRect(0, 0, SW, 22, 0x0000);

  if (bossActive) {
    int8_t bi = findBoss();
    if (bi >= 0) {
      Enemy& bo = enemies[bi];
      float bRatio = bo.hp / bo.maxHp;
      int bBarW = SW - 8;
      int bBarH = 8;
      gameSpr.fillRect(4, 2, bBarW, bBarH, 0x3000);
      gameSpr.fillRect(4, 2, (int)(bBarW * bRatio), bBarH, 0xF800);
      gameSpr.drawRect(4, 2, bBarW, bBarH, 0xFFFF);
      gameSpr.setTextColor(0xFFFF, 0xF800);
      snprintf(buf, 30, "WEED KING  %.0f%%", bRatio * 100);
      gameSpr.drawString(buf, 10, 1);
    }
    float hpRatio = player.hp / player.maxHp;
    if (hpRatio > 1.0f) hpRatio = 1.0f;
    uint16_t hpColor = (hpRatio > 0.5f) ? 0x07E0 : (hpRatio > 0.25f) ? 0xFE00 : 0xF800;
    gameSpr.fillRect(4, 12, SW - 8, 8, 0x3000);
    gameSpr.fillRect(4, 12, (int)((SW - 8) * hpRatio), 8, hpColor);
    gameSpr.drawRect(4, 12, SW - 8, 8, 0xFFFF);
    gameSpr.setTextColor(TFT_WHITE, hpColor);
    snprintf(buf, 20, "HP %d/%d", (int)player.hp, (int)player.maxHp);
    gameSpr.drawString(buf, 10, 11);
  } else {
    int barX = 4, barY = 2, barW = 140, barH = 18;
    gameSpr.fillRect(barX, barY, barW, barH, 0x3000);
    float hpRatio = player.hp / player.maxHp;
    if (hpRatio > 1.0f) hpRatio = 1.0f;
    uint16_t hpColor = (hpRatio > 0.5f) ? 0x07E0 : (hpRatio > 0.25f) ? 0xFE00 : 0xF800;
    gameSpr.fillRect(barX, barY, (int)(barW * hpRatio), barH, hpColor);
    gameSpr.drawRect(barX, barY, barW, barH, 0xFFFF);
    gameSpr.setTextColor(TFT_WHITE, (hpRatio > 0.5f) ? 0x07E0 : (hpRatio > 0.25f) ? 0xFE00 : 0xF800);
    snprintf(buf, 20, "HP %d/%d", (int)player.hp, (int)player.maxHp);
    gameSpr.drawString(buf, barX + 4, barY + 2);
  }

  // 右侧信息
  gameSpr.setTextColor(TFT_YELLOW, 0x0000);
  snprintf(buf, 20, "Lv%d", player.level);
  gameSpr.drawString(buf, 150, 2);
  int mins = (int)gameTime / 60;
  int secs = (int)gameTime % 60;
  snprintf(buf, 20, "%02d:%02d", mins, secs);
  gameSpr.drawString(buf, 200, 2);
  gameSpr.setTextColor(TFT_CYAN, 0x0000);
  snprintf(buf, 20, "K:%d", killCount);
  gameSpr.drawString(buf, 260, 2);

  // ===== 底部 XP 条 =====
  gameSpr.fillRect(0, BOTTOM_BAR_Y, SW, 28, 0x0000);
  int xpBarX = 4, xpBarY = BOTTOM_BAR_Y + 2, xpBarW = 180, xpBarH = 24;
  gameSpr.fillRect(xpBarX, xpBarY, xpBarW, xpBarH, 0x0020);
  float xpRatio = player.exp / player.expToNext;
  if (xpRatio > 1.0f) xpRatio = 1.0f;
  gameSpr.fillRect(xpBarX, xpBarY, (int)(xpBarW * xpRatio), xpBarH, 0x07FF);
  gameSpr.drawRect(xpBarX, xpBarY, xpBarW, xpBarH, 0xFFFF);
  gameSpr.setTextColor(TFT_WHITE, (xpRatio > 0.3f) ? 0x07FF : 0x0020);
  snprintf(buf, 20, "XP %d/%d", (int)player.exp, (int)player.expToNext);
  gameSpr.drawString(buf, xpBarX + 4, xpBarY + 5);

  // 武器图标
  int wx = 190;
  gameSpr.setTextColor(TFT_WHITE, 0x0000);
  for (uint8_t i = 0; i < player.weaponCount; i++) {
    WeaponState& w = player.weapons[i];
    uint16_t borderColor = w.evolved ? 0xFC00 : 0x8410;
    gameSpr.drawRect(wx, BOTTOM_BAR_Y + 2, 30, 24, borderColor);
    gameSpr.setTextColor(w.evolved ? TFT_YELLOW : TFT_WHITE, 0x0000);
    snprintf(buf, 10, "%d", w.weaponId + 1);
    gameSpr.drawString(buf, wx + 4, BOTTOM_BAR_Y + 4);
    snprintf(buf, 10, "L%d", w.level);
    gameSpr.drawString(buf, wx + 4, BOTTOM_BAR_Y + 14);
    wx += 34;
  }
}

// ==================== 升级面板 ====================
void drawUpgradePanel() {
  gameSpr.fillRect(0, 0, SW, SH, 0x1082);

  gameSpr.setTextSize(2);
  gameSpr.setTextColor(TFT_YELLOW, 0x1082);
  gameSpr.drawString("LEVEL UP!", 80, 12);

  for (uint8_t i = 0; i < upgradeOptionCount; i++) {
    int y = 40 + i * 62;
    bool sel = (i == upgradeSelection);

    uint16_t bg   = sel ? 0x3186 : 0x2104;
    uint16_t border = (upgradeOptions[i].type == 4) ? 0xFC00 : (sel ? 0xFFE0 : 0x07E0);
    gameSpr.fillRect(10, y, SW - 20, 56, bg);
    gameSpr.drawRect(10, y, SW - 20, 56, border);
    if (sel) gameSpr.drawRect(9, y - 1, SW - 18, 58, border);

    UpgradeOption& opt = upgradeOptions[i];
    char btn = 'A' + i;
    char buf[50];

    gameSpr.setTextSize(2);
    gameSpr.setTextColor(sel ? TFT_YELLOW : TFT_WHITE, bg);

    switch (opt.type) {
      case 0: snprintf(buf, 50, "[%c] Up %s Lv+1", btn, weaponNames[opt.id]); break;
      case 1: snprintf(buf, 50, "[%c] New %s", btn, weaponNames[opt.id]); break;
      case 2: snprintf(buf, 50, "[%c] Up %s Lv+1", btn, passiveNames[opt.id]); break;
      case 3: snprintf(buf, 50, "[%c] New %s", btn, passiveNames[opt.id]); break;
      case 4: snprintf(buf, 50, "[%c] EVOLVE %s!", btn, getEvolveName(opt.id)); break;
      case 5: snprintf(buf, 50, "[%c] Heal +%.0fHP", btn, player.maxHp * 0.5f); break;
      default: buf[0] = 0; break;
    }
    gameSpr.drawString(buf, 18, y + 14);
  }

  gameSpr.setTextSize(1);
}

// ==================== 暂停画面 ====================
void drawPauseOverlay() {
  gameSpr.fillRect(SW/2 - 60, SH/2 - 30, 120, 60, 0x0000);
  gameSpr.drawRect(SW/2 - 60, SH/2 - 30, 120, 60, 0xFC00);
  gameSpr.setTextSize(2);
  gameSpr.setTextColor(0xFC00, 0x0000);
  gameSpr.drawString("PAUSED", SW/2 - 40, SH/2 - 20);
  gameSpr.setTextSize(1);
  gameSpr.drawString("Press C", SW/2 - 20, SH/2 + 10);
}

// ==================== 胜利画面 ====================
void drawVictoryScreen() {
  unsigned long vt = millis() - screenEnterTime;
  tft.fillScreen(0x0000);
  for (int i = 0; i < 25; i++) {
    int px = (rngNext() % SW);
    int py = (rngNext() % 180);
    uint16_t pc = (rngNext() % 7 == 0) ? 0xF800 : (rngNext() % 3 == 0) ? 0xFFE0 : 0x07FF;
    tft.fillCircle(px, py, 2, pc);
  }
  if ((vt / 400) % 2 == 0) {
    tft.setTextSize(2);
    tft.setTextColor(0xFFE0, 0x0000);
    tft.drawString("VICTORY!", 80, 30);
  }
  tft.setTextSize(3);
  tft.setTextColor(0x07E0, 0x0000);
  tft.drawString("WEEDED", 70, 70);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, 0x0000);
  char buf[40];
  snprintf(buf, 40, "Time: %d:%02d", (int)gameTime / 60, (int)gameTime % 60);
  tft.drawString(buf, 60, 115);
  snprintf(buf, 40, "Lv%d  Kills:%d", player.level, killCount);
  tft.drawString(buf, 60, 145);
  if ((vt / 500) % 2 == 0) {
    tft.setTextColor(0xFC00, 0x0000);
    tft.drawString("Press A", 100, 190);
  }
  tft.setTextSize(1);
}

// ==================== 游戏结束画面 ====================
void drawGameOverScreen() {
  unsigned long gt = millis() - screenEnterTime;

  tft.fillScreen(0x2000);

  for (int i = 0; i < 30; i++) {
    int dx = (rngNext() % SW);
    int dy = (rngNext() % 200);
    tft.fillCircle(dx, dy, 2 + (rngNext() % 3), 0xF800);
  }

  for (int t = 0; t < 10; t++) {
    int tx = 30 + t * 26;
    tft.fillTriangle(tx, 20, tx + 13, 55, tx + 26, 20, 0xFFFF);
  }
  for (int t = 0; t < 10; t++) {
    int tx = 30 + t * 26;
    tft.fillTriangle(tx, 180, tx + 13, 145, tx + 26, 180, 0xFFFF);
  }

  if ((gt / 300) % 2 == 0) {
    tft.setTextSize(3);
    tft.setTextColor(0xF800, 0x2000);
    tft.drawString("DEVOURED!", 40, 65);
  }

  tft.setTextSize(2);
  tft.setTextColor(0xFD20, 0x2000);
  tft.drawString("YOU WERE", 80, 105);

  tft.setTextColor(TFT_WHITE, 0x2000);
  char buf[40];
  snprintf(buf, 40, "Time: %d:%02d", (int)gameTime / 60, (int)gameTime % 60);
  tft.drawString(buf, 60, 185);
  snprintf(buf, 40, "Lv%d  Kills:%d", player.level, killCount);
  tft.drawString(buf, 60, 205);

  if ((gt / 500) % 2 == 0) {
    tft.setTextColor(0xF800, 0x2000);
    tft.drawString("Press A", 105, 230);
  }
  tft.setTextSize(1);
}
