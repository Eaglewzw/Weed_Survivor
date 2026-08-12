#include <Arduino.h>
#include "ui.h"
#include "utils.h"
#include "player.h"
#include "enemy.h"

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

  // 武器栏像素图标（复用开屏动画画法）
  const uint16_t iconCol[3] = {0x07E0, 0xFFE0, 0x07FF};
  int wx = 190;
  for (uint8_t i = 0; i < player.weaponCount; i++) {
    WeaponState& w = player.weapons[i];
    uint16_t borderColor = w.evolved ? 0xFC00 : 0x8410;
    gameSpr.drawRect(wx, BOTTOM_BAR_Y + 2, 30, 24, borderColor);
    uint16_t ic = w.evolved ? TFT_YELLOW : iconCol[w.weaponId];
    int cx = wx + 11, cy = BOTTOM_BAR_Y + 9;
    gameSpr.drawCircle(cx, cy, 7, ic);
    switch (w.weaponId) {
      case 0:  // 叶片
        for (int p = 0; p < 3; p++) {
          float aa = 0.8f + p * 2.094f;
          gameSpr.drawLine(cx, cy, cx + (int)(cosf(aa) * 6), cy + (int)(sinf(aa) * 6), ic);
        }
        break;
      case 1:  // 旋风镰刀
        gameSpr.drawLine(cx, cy, cx + 6, cy - 4, ic);
        gameSpr.drawLine(cx, cy, cx - 5, cy - 5, ic);
        break;
      case 2:  // 光环
        gameSpr.drawCircle(cx, cy, 3, ic);
        break;
    }
    gameSpr.setTextColor(w.evolved ? TFT_YELLOW : TFT_WHITE, 0x0000);
    snprintf(buf, 10, "%d", w.level);
    gameSpr.drawString(buf, wx + 23, BOTTOM_BAR_Y + 16);
    wx += 34;
  }

  // ===== 右上角小地图（世界 960x600 → 40x25） =====
  int mmX = SW - 44, mmY = PLAY_AREA_Y + 4, mmW = 40, mmH = 25;
  gameSpr.fillRect(mmX, mmY, mmW, mmH, 0x1082);
  gameSpr.drawRect(mmX, mmY, mmW, mmH, 0x8410);
  if (bossActive) {
    int8_t bi = findBoss();
    if (bi >= 0 && (millis() / 250) % 2 == 0) {
      int bx = mmX + (int)(enemies[bi].x * mmW / (float)WORLD_W);
      int by = mmY + (int)(enemies[bi].y * mmH / (float)WORLD_H);
      gameSpr.fillRect(bx - 1, by - 1, 3, 3, 0xF800);
    }
  }
  for (int i = 0; i < MAX_CHESTS; i++) {
    if (!chests[i].alive) continue;
    int cx = mmX + (int)(chests[i].x * mmW / (float)WORLD_W);
    int cy = mmY + (int)(chests[i].y * mmH / (float)WORLD_H);
    if (cx > mmX + mmW - 2) cx = mmX + mmW - 2;
    if (cy > mmY + mmH - 2) cy = mmY + mmH - 2;
    gameSpr.fillRect(cx, cy, 2, 2, 0xFD20);
  }
  {
    int px = mmX + (int)(player.x * mmW / (float)WORLD_W);
    int py = mmY + (int)(player.y * mmH / (float)WORLD_H);
    if (px > mmX + mmW - 2) px = mmX + mmW - 2;
    if (py > mmY + mmH - 2) py = mmY + mmH - 2;
    gameSpr.fillRect(px, py, 2, 2, 0x07FF);
  }
}

// ==================== 升级面板 ====================
static const char* weaponDescs[]  = {"Rapid leaf shots", "Orbiting scythe", "Toxic aura ring"};
static const char* passiveDescs[] = {"+20% damage", "-15% cooldown", "+15% move spd", "+30 max HP", "+15% armor"};

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
    char desc[50];

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
    gameSpr.drawString(buf, 18, y + 8);

    // 描述行
    switch (opt.type) {
      case 0: {
        int8_t wi = findWeapon(opt.id);
        int lv = (wi >= 0) ? player.weapons[wi].level : 1;
        snprintf(desc, 50, "%s   Lv%d -> Lv%d", weaponDescs[opt.id], lv, lv + 1);
        break;
      }
      case 1: snprintf(desc, 50, "NEW!  %s", weaponDescs[opt.id]); break;
      case 2: {
        int8_t pi = findPassive(opt.id);
        int lv = (pi >= 0) ? player.passives[pi].level : 1;
        snprintf(desc, 50, "%s   Lv%d -> Lv%d", passiveDescs[opt.id], lv, lv + 1);
        break;
      }
      case 3: snprintf(desc, 50, "NEW!  %s", passiveDescs[opt.id]); break;
      case 4: snprintf(desc, 50, "EVOLVE!  %s", weaponDescs[opt.id]); break;
      case 5: snprintf(desc, 50, "Restore half HP now"); break;
      default: desc[0] = 0; break;
    }
    gameSpr.setTextSize(1);
    gameSpr.setTextColor(sel ? TFT_WHITE : 0xAD55, bg);
    gameSpr.drawString(desc, 18, y + 34);
  }

  gameSpr.setTextSize(1);
  gameSpr.setTextColor(TFT_CYAN, 0x1082);
  gameSpr.drawString("A/B/C choose   Stick: move + press OK", 46, 226);
}

// ==================== 暂停统计页 ====================
void drawPauseOverlay() {
  gameSpr.fillRect(0, 0, SW, SH, 0x1082);
  char buf[60];

  gameSpr.setTextSize(2);
  gameSpr.setTextColor(0xFFE0, 0x1082);
  gameSpr.drawString("PAUSED", 8, 6);
  gameSpr.setTextSize(1);
  gameSpr.setTextColor(TFT_WHITE, 0x1082);
  snprintf(buf, 60, "Time %02d:%02d   Lv%d   Kills %d",
           (int)gameTime / 60, (int)gameTime % 60, player.level, killCount);
  gameSpr.drawString(buf, 10, 32);

  // 本局武器
  gameSpr.setTextColor(TFT_YELLOW, 0x1082);
  gameSpr.drawString("WEAPONS", 10, 52);
  gameSpr.setTextColor(TFT_WHITE, 0x1082);
  int ty = 68;
  for (uint8_t i = 0; i < player.weaponCount; i++) {
    WeaponState& w = player.weapons[i];
    snprintf(buf, 40, "%s L%d%s", weaponNames[w.weaponId], w.level, w.evolved ? " EVO" : "");
    gameSpr.drawString(buf, 10, ty);
    ty += 16;
  }

  // DPS
  gameSpr.setTextColor(TFT_YELLOW, 0x1082);
  gameSpr.drawString("DPS", 170, 52);
  gameSpr.setTextColor(TFT_WHITE, 0x1082);
  float dps = (gameTime > 0) ? totalDamageDealt / gameTime : 0;
  snprintf(buf, 20, "%.1f", dps);
  gameSpr.drawString(buf, 170, 68);

  // 杀敌分布
  gameSpr.setTextColor(TFT_YELLOW, 0x1082);
  gameSpr.drawString("KILLS", 10, 140);
  gameSpr.setTextColor(TFT_WHITE, 0x1082);
  snprintf(buf, 60, "Weed %d   Dan %d   Thorn %d", killStats[0], killStats[1], killStats[2]);
  gameSpr.drawString(buf, 10, 156);
  snprintf(buf, 60, "Elite %d   Boss %d", killStats[3], killStats[4]);
  gameSpr.drawString(buf, 10, 172);

  if ((millis() / 500) % 2 == 0) {
    gameSpr.setTextColor(TFT_CYAN, 0x1082);
    gameSpr.drawString("Press C to resume", 109, 220);
  }
}

// ==================== 低血红晕（呼吸脉冲） ====================
void drawLowHpVignette() {
  float hpRatio = player.hp / player.maxHp;
  if (hpRatio >= 0.3f || hpRatio <= 0) return;
  float pulse = 0.5f + 0.5f * cosf(heartbeatTimer * 5.7f);  // 与心跳同步
  int layers = 2 + (int)(pulse * 4);
  for (int i = 0; i < layers; i++) {
    gameSpr.drawRect(i, i, SW - 2 * i, SH - 2 * i, 0x4000);
  }
}

// ==================== BOSS 登场横幅 ====================
void drawBossBanner() {
  if (bossBannerTimer <= 0) return;
  gameSpr.fillRect(0, 0, SW, SH, 0x0000);
  int cx = SW / 2;
  gameSpr.fillTriangle(cx, 40, cx - 22, 78, cx + 22, 78, 0xF800);
  gameSpr.fillTriangle(cx, 48, cx - 12, 70, cx + 12, 70, 0x0000);
  gameSpr.setTextSize(2);
  gameSpr.setTextColor(0xFFE0, 0x0000);
  gameSpr.drawString("THE WEED KING", 56, 100);
  if (((int)(bossBannerTimer * 6.0f)) % 2 == 0) {
    gameSpr.setTextSize(3);
    gameSpr.setTextColor(0xF800, 0x0000);
    gameSpr.drawString("AWAKENS", 76, 135);
  }
  gameSpr.setTextSize(1);
  gameSpr.setTextColor(0x8410, 0x0000);
  gameSpr.drawString("GET READY...", 124, 200);
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
