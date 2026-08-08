#include <Arduino.h>
#include "render.h"
#include "utils.h"
#include "player.h"

// ==================== 背景 ====================
void drawBackground() {
  gameSpr.fillScreen(0x0000);
  gameSpr.fillRect(0, PLAY_AREA_Y, SW, PLAY_AREA_H, 0x1A3A);

  // 草丛点 — 使用确定性伪随机，适配大世界
  int seed = (int)(cameraX * 1000 + cameraY);
  for (int i = 0; i < 80; i++) {
    uint32_t h = (seed + i * 2654435761UL);
    int wx = (int)cameraX + (h % SW);
    int wy = (int)cameraY + ((h >> 8) % PLAY_AREA_H);
    int sx = wx - (int)cameraX;
    int sy = wy - (int)cameraY + PLAY_AREA_Y;
    if (sx >= 0 && sx < SW && sy >= PLAY_AREA_Y && sy < PLAY_AREA_BOT) {
      gameSpr.drawPixel(sx, sy, 0x2A6A);
    }
  }
}

// ==================== 玩家 — Vampire Survivors 像素风格 ====================
//
//  调色板 (4色):
//    帽: 0xCB20 (亮稻草)  0xBA00 (暗稻草)
//    肤: 0xFE69 (肤色)
//    衣: 0x34B0 (蓝衬衫)  0x2960 (绿裤)
//    靴: 0x5A40 (棕靴)
//
//  精灵尺寸: 12w x 18h px (不含武器)
//  动画帧: 行走(双腿交替+身体弹跳) / 攻击(手臂前伸+武器亮起)

void drawPlayer() {
  int px = toScreenX(player.x), py = toScreenY(player.y);

  // ---- 动画相位 ----
  float w = playerWalkPhase;
  float bob = 0;
  int legL = 0, legR = 0;
  int armSwing = 0;

  if (playerMoving) {
    bob = sinf(w * 2.0f) * 1.0f;
    legL = (int)(sinf(w) * 2);
    legR = (int)(sinf(w + 3.14159f) * 2);
    armSwing = (int)(sinf(w) * 1);
  } else {
    bob = sinf(w) * 0.3f;
  }

  int by = py + (int)bob;

  // ---- 状态 ----
  float faceA = player.facingAngle;
  bool hurtVis = (playerHurtFlash > 0 && ((int)(playerHurtFlash * 12) % 2 == 0));
  bool invBlink = (player.invincibleTimer > 0 && ((int)(player.invincibleTimer * 10) % 2 == 0));
  if (invBlink) return;
  bool attacking = (playerAttackFlash > 0.05f);

  // ---- 受伤色调替换 ----
  uint16_t colHat   = hurtVis ? 0xD000 : 0xCB20;
  uint16_t colHatD  = hurtVis ? 0xA000 : 0xBA00;
  uint16_t colSkin  = hurtVis ? 0xF800 : 0xFE69;
  uint16_t colShirt = hurtVis ? 0xD000 : 0x34B0;
  uint16_t colPants = hurtVis ? 0xC000 : 0x2960;
  uint16_t colBoot  = hurtVis ? 0xA000 : 0x5A40;

  // ============================================================
  //  攻击闪光 (像素风: 方块光环而非圆环)
  // ============================================================
  if (attacking) {
    // 大方块光环
    gameSpr.fillRect(px - 8, by - 8, 17, 2, 0xFFFF);
    gameSpr.fillRect(px - 8, by + 6, 17, 2, 0xFFFF);
    gameSpr.fillRect(px - 8, by - 6, 2, 13, 0xFFFF);
    gameSpr.fillRect(px + 7, by - 6, 2, 13, 0xFFFF);
  } else if (playerAttackFlash > 0) {
    // 淡出小方块
    gameSpr.drawRect(px - 6, by - 6, 13, 13, 0xFFFF);
  }

  // ============================================================
  //  帽子 (像素块风格 — 宽檐草帽)
  // ============================================================
  // 帽顶 (2层方块)
  gameSpr.fillRect(px - 4, by - 13, 9, 3, colHat);
  gameSpr.fillRect(px - 3, by - 14, 7, 2, colHat);
  // 帽身
  gameSpr.fillRect(px - 5, by - 10, 11, 4, colHatD);
  // 帽檐 (最宽)
  gameSpr.fillRect(px - 8, by - 7, 17, 2, colHat);
  gameSpr.fillRect(px - 7, by - 6, 15, 1, colHatD);
  // 红色帽带
  gameSpr.fillRect(px - 4, by - 11, 9, 1, 0xF800);

  // ============================================================
  //  头部 (方块像素)
  // ============================================================
  // 头发上沿
  gameSpr.fillRect(px - 3, by - 9, 7, 2, 0x6A40);
  // 脸部 (5h x 7w 方块)
  gameSpr.fillRect(px - 3, by - 7, 7, 5, colSkin);
  // 耳朵 (两侧各1px)
  gameSpr.fillRect(px - 4, by - 5, 1, 3, colSkin);
  gameSpr.fillRect(px + 4, by - 5, 1, 3, colSkin);

  // 眼睛 (像素风: 2x2 白方块 + 1px 瞳孔)
  int ex = (int)(cosf(faceA) * 1.5f);
  int ey = (int)(sinf(faceA) * 1.5f);
  gameSpr.fillRect(px - 3 + ex, by - 5 + ey, 2, 2, 0xFFFF);
  gameSpr.fillRect(px + 2 + ex, by - 5 + ey, 2, 2, 0xFFFF);
  gameSpr.drawPixel(px - 2 + ex, by - 4 + ey, 0x0000);
  gameSpr.drawPixel(px + 3 + ex, by - 4 + ey, 0x0000);
  // 高光 (1px)
  gameSpr.drawPixel(px - 2 + ex, by - 5 + ey, 0xFFFF);
  gameSpr.drawPixel(px + 3 + ex, by - 5 + ey, 0xFFFF);

  // 腮红 (1px 像素点)
  gameSpr.drawPixel(px - 4, by - 3, 0xFACF);
  gameSpr.drawPixel(px + 4, by - 3, 0xFACF);

  // 嘴 (1px)
  if (hurtVis) {
    gameSpr.drawPixel(px,     by - 2, 0x0000);
    gameSpr.drawPixel(px - 1, by - 1, 0x0000);
    gameSpr.drawPixel(px + 1, by - 1, 0x0000);
  } else if (attacking) {
    gameSpr.fillRect(px - 1, by - 2, 2, 2, 0x0000);
  } else {
    gameSpr.fillRect(px - 1, by - 2, 3, 1, 0x0000);
  }

  // ============================================================
  //  身体 (像素方块躯干)
  // ============================================================
  // 脖子
  gameSpr.fillRect(px - 1, by - 2, 3, 1, colSkin);
  // 躯干 (宽肩)
  gameSpr.fillRect(px - 4, by - 1, 9, 2, colShirt);
  gameSpr.fillRect(px - 4, by + 1, 9, 3, colShirt);
  // 背带裤吊带
  gameSpr.drawLine(px - 2, by - 1, px - 2, by + 3, colPants);
  gameSpr.drawLine(px + 2, by - 1, px + 2, by + 3, colPants);
  // 腰带
  gameSpr.fillRect(px - 4, by + 3, 9, 1, 0x6A40);

  // ============================================================
  //  腿部 (像素方块短腿)
  // ============================================================
  gameSpr.fillRect(px - 3 + legL, by + 4, 3, 3, colPants);
  gameSpr.fillRect(px + 1 + legR, by + 4, 3, 3, colPants);
  // 靴子
  gameSpr.fillRect(px - 4 + legL, by + 6, 4, 2, colBoot);
  gameSpr.fillRect(px     + legR, by + 6, 4, 2, colBoot);
  gameSpr.fillRect(px - 3 + legL, by + 7, 4, 1, 0x4200);
  gameSpr.fillRect(px + 1 + legR, by + 7, 4, 1, 0x4200);

  // ============================================================
  //  手臂 + 武器 (Vampire Survivors 风格: 武器在身前)
  // ============================================================
  // 左臂 (后方, 摆动幅度小)
  int laX = px - 4;
  int laY = by;
  int lhX = laX - 1 - armSwing;
  int lhY = laY + 3;
  gameSpr.drawLine(laX, laY, lhX, lhY, colShirt);
  gameSpr.fillRect(lhX - 1, lhY - 1, 2, 2, 0x0520);

  // 右臂 / 攻击臂 (前方, 持武器)
  int raX = px + 4;
  int raY = by;
  int rhX = raX + 1 + armSwing;
  int rhY = raY + 3;

  if (attacking) {
    // 攻击帧: 手臂完全前伸
    rhX = px + (int)(cosf(faceA) * 6);
    rhY = by - 1 + (int)(sinf(faceA) * 5);
    gameSpr.drawLine(raX, raY, rhX, rhY, colShirt);
    gameSpr.fillRect(rhX - 1, rhY - 1, 2, 2, 0x0520);

    // ---- 魔法叶片武器 (像素风) ----
    int wx = px + (int)(cosf(faceA) * 9);
    int wy = by - 2 + (int)(sinf(faceA) * 7);
    // 叶片主体 (3x5 绿色方块)
    gameSpr.fillRect(wx - 1, wy - 2, 3, 5, 0x07E0);
    gameSpr.drawRect(wx - 2, wy - 3, 5, 7, 0x06C0);
    // 叶脉中线
    gameSpr.drawLine(wx, wy - 2, wx, wy + 2, 0x06C0);
    // 武器闪光 (白色像素边框)
    gameSpr.drawPixel(wx - 2, wy - 3, 0xFFFF);
    gameSpr.drawPixel(wx + 2, wy - 3, 0xFFFF);
    gameSpr.drawPixel(wx, wy - 4, 0xFFFF);
  } else if (playerAttackFlash > 0) {
    // 攻击收尾帧: 手臂半伸
    rhX = raX + 2;
    rhY = raY + 2;
    gameSpr.drawLine(raX, raY, rhX, rhY, colShirt);
    gameSpr.fillRect(rhX - 1, rhY - 1, 2, 2, 0x0520);
  } else {
    // 待机帧: 手臂自然下垂
    gameSpr.drawLine(raX, raY, rhX, rhY, colShirt);
    gameSpr.fillRect(rhX - 1, rhY - 1, 2, 2, 0x0520);
  }

  // ============================================================
  //  受伤特效 (像素方块红框 + 泪滴)
  // ============================================================
  if (hurtVis) {
    gameSpr.drawRect(px - 7, by - 9, 15, 18, 0xF800);
    gameSpr.drawRect(px - 6, by - 8, 13, 16, 0xF800);
    // 像素泪滴
    gameSpr.drawPixel(px - 6, by - 7, 0x07FF);
    gameSpr.drawPixel(px - 5, by - 5, 0x07FF);
    gameSpr.drawPixel(px + 6, by - 6, 0x07FF);
    gameSpr.drawPixel(px + 5, by - 4, 0x07FF);
  }
}

// ==================== 敌人绘制 — 邪恶恐怖风格 ====================
void drawEnemy(int idx) {
  Enemy& e = enemies[idx];
  int ex = toScreenX(e.x), ey = toScreenY(e.y);
  int r = (e.type == ENEMY_ELITE) ? 12 : (e.type == ENEMY_THORN) ? 7 : (e.type == ENEMY_DANDELION) ? 8 : 6;

  // 视口裁剪
  if (ex < -20 || ex > SW + 20 || ey < PLAY_AREA_Y - 20 || ey > PLAY_AREA_BOT + 20) return;

  switch (e.type) {

    // ========== ENEMY_WEED — 扭曲邪芽 ==========
    case ENEMY_WEED: {
      // 不规则暗色身体 — 多个重叠圆制造扭曲感
      gameSpr.fillCircle(ex, ey, r, 0x0240);
      gameSpr.fillCircle(ex + 2, ey - 1, r - 1, 0x0260);
      gameSpr.fillCircle(ex - 2, ey + 1, r - 2, 0x0200);
      // 荆棘小突起
      for (int i = 0; i < 5; i++) {
        float ta = i * 1.2566f + e.x * 0.3f;
        int tx = ex + cosf(ta) * (r + 2);
        int ty = ey + sinf(ta) * (r + 2);
        gameSpr.drawLine(ex + cosf(ta) * (r - 1), ey + sinf(ta) * (r - 1), tx, ty, 0x8A40);
      }
      // 血红眼睛 — 不对称
      gameSpr.fillCircle(ex - 2, ey - 1, 2, 0xF800);
      gameSpr.fillCircle(ex + 3, ey - 1, 2, 0xF800);
      gameSpr.fillCircle(ex - 2, ey - 1, 1, 0x0000);
      gameSpr.fillCircle(ex + 3, ey - 1, 1, 0x0000);
      // 牙齿裂口
      for (int t = 0; t < 3; t++) {
        gameSpr.fillTriangle(ex - 2 + t * 2, ey + 2, ex - 1 + t * 2, ey + 5, ex + t * 2, ey + 2, 0xFFFF);
      }
      // 黑色液滴
      gameSpr.fillCircle(ex + 1, ey + 3, 1, 0x0000);
      break;
    }

    // ========== ENEMY_DANDELION — 白骨幽灵 ==========
    case ENEMY_DANDELION: {
      // 苍白骷髅色身体
      gameSpr.fillCircle(ex, ey, r, 0xBD97);
      gameSpr.drawCircle(ex, ey, r, 0x9CD3);
      // 暗黑眼窝
      gameSpr.fillCircle(ex - 3, ey - 2, 3, 0x2104);
      gameSpr.fillCircle(ex + 3, ey - 2, 3, 0x2104);
      // 血红瞳孔
      gameSpr.fillCircle(ex - 3, ey - 2, 1, 0xF800);
      gameSpr.fillCircle(ex + 3, ey - 2, 1, 0xF800);
      // 鼻腔（倒三角）
      gameSpr.fillTriangle(ex, ey, ex - 2, ey + 3, ex + 2, ey + 3, 0x2104);
      // 指骨般触须 — 6根浮动骨片
      for (int i = 0; i < 6; i++) {
        float a = i * 1.0472f + gameTime * 0.7f;
        int bx1 = ex + cosf(a) * (r - 1);
        int by1 = ey + sinf(a) * (r - 1);
        int bx2 = ex + cosf(a) * (r + 6);
        int by2 = ey + sinf(a) * (r + 6);
        // 骨节线
        gameSpr.drawLine(bx1, by1, bx2, by2, 0xD69A);
        // 骨节末端膨大
        gameSpr.fillCircle(bx2, by2, 2, 0xEEEC);
        gameSpr.fillCircle(bx2, by2, 1, 0xBD97);
      }
      // 暗影底部
      gameSpr.fillCircle(ex, ey + r - 1, r - 2, 0x630C);
      break;
    }

    // ========== ENEMY_THORN — 血棘穿刺者 ==========
    case ENEMY_THORN: {
      // 暗红核心
      gameSpr.fillCircle(ex, ey, r, 0x9000);
      gameSpr.fillCircle(ex, ey, r - 1, 0xA800);
      // 8根长刺
      for (int i = 0; i < 8; i++) {
        float a = i * 0.7854f;
        int x1 = ex + cosf(a) * (r - 2);
        int y1 = ey + sinf(a) * (r - 2);
        int x2 = ex + cosf(a) * (r + 5);
        int y2 = ey + sinf(a) * (r + 5);
        // 主刺
        gameSpr.drawLine(x1, y1, x2, y2, 0xC800);
        // 倒钩
        int bx = ex + cosf(a) * (r + 2);
        int by = ey + sinf(a) * (r + 2);
        gameSpr.fillTriangle(bx, by,
          bx + cosf(a - 1.2f) * 4, by + sinf(a - 1.2f) * 4,
          bx + cosf(a + 1.2f) * 4, by + sinf(a + 1.2f) * 4, 0xF800);
      }
      // 发光核心 — 黄橙
      gameSpr.fillCircle(ex, ey, r / 2, 0xFC40);
      gameSpr.fillCircle(ex, ey, r / 2 - 1, 0xFD80);
      // 核心中的竖瞳
      gameSpr.fillCircle(ex, ey, 2, 0xF800);
      gameSpr.fillCircle(ex, ey, 1, 0x0000);
      // 外环尖牙
      for (int i = 0; i < 6; i++) {
        float a = i * 1.0472f + gameTime * 0.4f;
        int tx = ex + cosf(a) * (r + 2);
        int ty = ey + sinf(a) * (r + 2);
        gameSpr.drawLine(ex + cosf(a) * (r - 2), ey + sinf(a) * (r - 2), tx, ty, 0xF800);
      }
      break;
    }

    // ========== ENEMY_ELITE — 恶魔领主芽 ==========
    case ENEMY_ELITE: {
      int er = 12;
      // 暗紫黑身体 — 多层
      gameSpr.fillCircle(ex, ey, er, 0x3008);
      gameSpr.fillCircle(ex, ey, er - 2, 0x480C);
      gameSpr.drawCircle(ex, ey, er, 0x1804);
      // 内圈暗纹
      gameSpr.drawCircle(ex, ey, er - 4, 0x2006);
      gameSpr.drawCircle(ex, ey, er - 5, 0x2006);

      // 巨大恶魔角
      gameSpr.fillTriangle(ex - 9, ey - 6, ex - 15, ey - er - 8, ex - 3, ey - 8, 0x6320);
      gameSpr.fillTriangle(ex + 3, ey - 8, ex + 15, ey - er - 8, ex + 9, ey - 6, 0x6320);
      // 角尖红芒
      gameSpr.fillCircle(ex - 15, ey - er - 8, 2, 0xF800);
      gameSpr.fillCircle(ex + 15, ey - er - 8, 2, 0xF800);

      // 荆棘王冠
      for (int i = 0; i < 5; i++) {
        float a = i * 1.2566f - 1.5708f;
        int cx1 = ex + cosf(a) * (er - 2);
        int cy1 = ey + sinf(a) * (er - 2);
        int cx2 = ex + cosf(a) * (er + 8);
        int cy2 = ey + sinf(a) * (er + 8);
        gameSpr.drawLine(cx1, cy1, cx2, cy2, 0xA800);
        gameSpr.fillTriangle(cx2, cy2,
          cx2 + cosf(a - 0.4f) * 4, cy2 + sinf(a - 0.4f) * 4,
          cx2 + cosf(a + 0.4f) * 4, cy2 + sinf(a + 0.4f) * 4, 0xF800);
      }

      // 血红巨眼 — 左侧
      gameSpr.fillCircle(ex - 5, ey - 3, 4, 0xF800);
      gameSpr.fillCircle(ex - 5, ey - 3, 3, 0xFC20);
      // 竖瞳
      gameSpr.fillRect(ex - 6, ey - 5, 2, 4, 0x0000);
      // 右侧
      gameSpr.fillCircle(ex + 5, ey - 3, 4, 0xF800);
      gameSpr.fillCircle(ex + 5, ey - 3, 3, 0xFC20);
      gameSpr.fillRect(ex + 4, ey - 5, 2, 4, 0x0000);

      // 锯齿大口
      for (int t = 0; t < 6; t++) {
        int mx = ex - 8 + t * 3;
        int my = ey + 2 + (t % 2) * 3;
        gameSpr.fillTriangle(mx, my, mx + 1, my + 5, mx + 3, my, 0x0000);
        gameSpr.fillTriangle(mx, my, mx + 1, my + 5, mx + 3, my, 0xFFFF); // 白牙
      }

      // 暗紫光环
      gameSpr.drawCircle(ex, ey, er + 4, 0x8018);
      gameSpr.drawCircle(ex, ey, er + 5, 0x480C);

      // 底部暗影触须
      for (int tn = 0; tn < 3; tn++) {
        float ta = tn * 2.0944f + 0.5f;
        int tx1 = ex + cosf(ta) * (er - 2);
        int ty1 = ey + sinf(ta) * (er - 2);
        int tx2 = ex + cosf(ta) * (er + 10);
        int ty2 = ey + sinf(ta) * (er + 10);
        gameSpr.drawLine(tx1, ty1, tx2, ty2, 0x3008);
        gameSpr.fillCircle(tx2, ty2, 2, 0x480C);
      }
      break;
    }

    // ========== ENEMY_BOSS — 杂草梦魇 ==========
    case ENEMY_BOSS: {
      int br = 30;
      // 主体 — 腐绿暗紫混合
      gameSpr.fillCircle(ex, ey, br, 0x2260);
      gameSpr.fillCircle(ex, ey, br - 3, 0x1A40);
      gameSpr.drawCircle(ex, ey, br, 0x0140);
      gameSpr.drawCircle(ex, ey, br - 2, 0x0140);
      // 腐烂纹理
      for (int i = 0; i < 8; i++) {
        float fa = i * 0.7854f + 0.3f;
        int fx = ex + cosf(fa) * (br - 8);
        int fy = ey + sinf(fa) * (br - 8);
        gameSpr.fillCircle(fx, fy, 4 + (i % 3), 0x2A60);
      }
      // 暗色内核
      gameSpr.fillCircle(ex, ey, br - 10, 0x0100);

      // 12根血棘 — 缓慢旋转
      for (int s = 0; s < 12; s++) {
        float sa = s * 0.5236f + gameTime * 0.3f;
        int sx1 = ex + cosf(sa) * (br - 5);
        int sy1 = ey + sinf(sa) * (br - 5);
        int sx2 = ex + cosf(sa) * (br + 14);
        int sy2 = ey + sinf(sa) * (br + 14);
        // 主刺
        gameSpr.drawLine(sx1, sy1, sx2, sy2, 0xC800);
        gameSpr.drawLine(sx1 - 1, sy1, sx2, sy2, 0xA800);
        // 倒钩
        gameSpr.fillTriangle(sx2, sy2,
          sx2 + cosf(sa - 0.35f) * 7, sy2 + sinf(sa - 0.35f) * 7,
          sx2 + cosf(sa + 0.35f) * 7, sy2 + sinf(sa + 0.35f) * 7, 0xF800);
        // 血滴
        gameSpr.fillCircle(sx2 + cosf(sa) * 2, sy2 + sinf(sa) * 2, 2, 0xF800);
      }

      // 骷髅面孔区域 — 苍白内圈
      gameSpr.fillCircle(ex, ey - 2, 14, 0xBD97);
      gameSpr.fillCircle(ex, ey - 2, 12, 0xD69A);
      // 眼眶 — 大而空洞
      gameSpr.fillCircle(ex - 7, ey - 7, 6, 0x0000);
      gameSpr.fillCircle(ex + 7, ey - 7, 6, 0x0000);
      // 血红眼球
      gameSpr.fillCircle(ex - 7, ey - 7, 4, 0xF800);
      gameSpr.fillCircle(ex + 7, ey - 7, 4, 0xF800);
      // 竖瞳
      gameSpr.fillRect(ex - 8, ey - 9, 2, 4, 0x0000);
      gameSpr.fillRect(ex + 6, ey - 9, 2, 4, 0x0000);
      // 眼白高光
      gameSpr.fillCircle(ex - 5, ey - 9, 1, 0xFFFF);
      gameSpr.fillCircle(ex + 9, ey - 9, 1, 0xFFFF);

      // 鼻腔
      gameSpr.fillTriangle(ex, ey - 2, ex - 3, ey + 2, ex + 3, ey + 2, 0x0000);

      // 锯齿裂口大嘴
      for (int t = 0; t < 10; t++) {
        int mx = ex - 12 + t * 3;
        int my = ey + 4 + (t % 2) * 4;
        gameSpr.fillTriangle(mx, my, mx + 1, my + 7, mx + 3, my, 0xFFFF);
      }
      // 嘴角血痕
      gameSpr.fillCircle(ex - 14, ey + 8, 2, 0xF800);
      gameSpr.fillCircle(ex + 14, ey + 8, 2, 0xF800);

      // 巨大恶魔角
      gameSpr.fillTriangle(ex - 14, ey - br + 6, ex - 24, ey - br - 18, ex - 6, ey - br + 8, 0x6A40);
      gameSpr.fillTriangle(ex + 6, ey - br + 8, ex + 24, ey - br - 18, ex + 14, ey - br + 6, 0x6A40);
      // 角环纹
      gameSpr.drawLine(ex - 16, ey - br, ex - 12, ey - br - 4, 0x8A60);
      gameSpr.drawLine(ex + 12, ey - br - 4, ex + 16, ey - br, 0x8A60);
      // 角尖血光
      gameSpr.fillCircle(ex - 24, ey - br - 18, 3, 0xF800);
      gameSpr.fillCircle(ex + 24, ey - br - 18, 3, 0xF800);

      // 8条触手 — 从底部伸出
      for (int tn = 0; tn < 8; tn++) {
        float ta = tn * 0.7854f + gameTime * 0.25f;
        int tx1 = ex + cosf(ta) * (br - 4);
        int ty1 = ey + sinf(ta) * (br - 4);
        int tx2 = ex + cosf(ta) * (br + 22);
        int ty2 = ey + sinf(ta) * (br + 22);
        // 触手主茎
        gameSpr.drawLine(tx1, ty1, tx2, ty2, 0x03C0);
        gameSpr.drawLine(tx1 + 1, ty1, tx2 + 1, ty2, 0x0260);
        // 触手末端 — 带刺球
        gameSpr.fillCircle(tx2, ty2, 4, 0x4A0);
        gameSpr.fillCircle(tx2, ty2, 2, 0xF800);
        // 吸盘小点
        int mx3 = (tx1 + tx2) / 2, my3 = (ty1 + ty2) / 2;
        gameSpr.fillCircle(mx3, my3, 2, 0x06E0);
      }

      // 外圈暗紫光环
      gameSpr.drawCircle(ex, ey, br + 8, 0x8018);
      gameSpr.drawCircle(ex, ey, br + 9, 0x600E);
      // 第二圈光环 — 脉动
      float pulse = 1.0f + sinf(gameTime * 3.0f) * 0.15f;
      int pr = (int)((br + 14) * pulse);
      gameSpr.drawCircle(ex, ey, pr, 0x400C);

      // 第三只眼 — 额头
      gameSpr.fillCircle(ex, ey - 18, 4, 0xF800);
      gameSpr.fillCircle(ex, ey - 18, 2, 0xFC40);
      gameSpr.fillCircle(ex, ey - 18, 1, 0x0000);

      // 黑烟粒子
      for (int sm = 0; sm < 6; sm++) {
        float sAng = sm * 1.0472f + gameTime * 0.5f;
        int sDist = br + 12 + (sm * 3) % 8;
        int sPx = ex + cosf(sAng) * sDist;
        int sPy = ey + sinf(sAng) * sDist;
        gameSpr.fillCircle(sPx, sPy, 2, 0x0000);
      }
      break;
    }
  }

  // ===== 血量条（世界空间偏移） =====
  if (e.hp < e.maxHp) {
    float ratio = e.hp / e.maxHp;
    int barW = r * 2;
    if (e.type == ENEMY_BOSS) barW = 40;
    int barY = ey - r - 8;
    gameSpr.fillRect(ex - barW / 2, barY, barW, 3, 0x3000);
    uint16_t barColor = (ratio > 0.5f) ? 0x04C0 : (ratio > 0.25f) ? 0xCC40 : 0xC400;
    gameSpr.fillRect(ex - barW / 2, barY, (int)(barW * ratio), 3, barColor);
  }
}

// ==================== 经验宝石 ====================
void drawGem(int idx) {
  Gem& g = gems[idx];
  if (g.life <= 0) return;
  int gx = toScreenX(g.x), gy = toScreenY(g.y);
  float alpha = (g.life < 3) ? g.life / 3.0f : 1.0f;
  if (alpha <= 0) return;
  int r = 3 + g.value / 5;
  if (r > 6) r = 6;
  gameSpr.fillCircle(gx, gy, r + 2, 0x06A0);
  gameSpr.fillCircle(gx, gy, r, 0x0FC0);
}

// ==================== 投射物 ====================
void drawProjectile(int idx) {
  Projectile& p = projectiles[idx];
  int px = toScreenX(p.x), py = toScreenY(p.y);
  // BOSS弹幕 — 红色大光球 + 拖尾
  if (p.damage >= 25) {
    gameSpr.fillCircle(px, py, 6, 0xC800);
    gameSpr.fillCircle(px, py, 4, 0xF800);
    gameSpr.fillCircle(px, py, 2, 0xFD20);
    float tailA = atan2f(p.vy, p.vx) + 3.1416f;
    int tx = px + cosf(tailA) * 5, ty = py + sinf(tailA) * 5;
    gameSpr.fillCircle(tx, ty, 3, 0xC800);
    gameSpr.fillCircle(tx, ty, 2, 0xF800);
  } else {
    // 玩家投射物 — 绿色叶片
    gameSpr.fillCircle(px, py, 3, 0x04A0);
    gameSpr.fillCircle(px, py, 2, 0x06C0);
  }
}

// ==================== 镰刀 ====================
void drawScythe() {
  int8_t wi = findWeapon(1);
  if (wi < 0) return;
  WeaponState& w = player.weapons[wi];
  float cd, dmg, orbitR, angVel;
  getWeaponStats(w.weaponId, w.level, w.evolved, cd, dmg, orbitR, angVel);
  float sx = player.x + cosf(w.extraParam) * orbitR;
  float sy = player.y + sinf(w.extraParam) * orbitR;
  int dsx = toScreenX(sx), dsy = toScreenY(sy);
  uint16_t color = w.evolved ? 0xC800 : 0xBBBC;
  gameSpr.fillCircle(dsx, dsy, 6, color);
  gameSpr.drawCircle(dsx, dsy, 6, 0xFFFF);
  float a = w.extraParam;
  gameSpr.drawLine(dsx + cosf(a) * 5, dsy + sinf(a) * 5,
               dsx + cosf(a + 1.5f) * 10, dsy + sinf(a + 1.5f) * 10, 0xCCCC);
}

// ==================== 光环 ====================
void drawAura() {
  int8_t wi = findWeapon(2);
  if (wi < 0) return;
  WeaponState& w = player.weapons[wi];
  float radius = w.extraParam;
  uint16_t color = w.evolved ? 0x07E0 : 0x03A0;
  gameSpr.drawCircle(toScreenX(player.x), toScreenY(player.y), (int)radius, color);
}

// ==================== 慢速区域 ====================
void drawSlowZones() {
  for (int i = 0; i < slowZoneCount; i++) {
    if (slowZones[i].life <= 0) continue;
    int sx = toScreenX(slowZones[i].x);
    int sy = toScreenY(slowZones[i].y);
    gameSpr.drawCircle(sx, sy, 30, 0xCCCF);
  }
}

// ==================== BOSS范围攻击 ====================
void drawBossAoE(int idx) {
  BossAoE& aoe = bossAoEs[idx];
  if (!aoe.active) return;
  int ax = toScreenX(aoe.x), ay = toScreenY(aoe.y);
  int r = (int)aoe.radius;
  if (aoe.type == 0) {
    gameSpr.drawCircle(ax, ay, r, aoe.color);
    gameSpr.drawCircle(ax, ay, r - 1, aoe.color);
    gameSpr.fillCircle(ax, ay, r / 2, aoe.color);
  } else if (aoe.type == 1) {
    gameSpr.drawCircle(ax, ay, r, aoe.color);
    gameSpr.drawCircle(ax, ay, r + 1, 0xC800);
    gameSpr.drawCircle(ax, ay, r - 1, 0xC800);
    for (int s = 0; s < 8; s++) {
      float sa = s * 0.7854f + aoe.timer * 3;
      int sx = ax + cosf(sa) * r;
      int sy = ay + sinf(sa) * r;
      int sx2 = ax + cosf(sa) * (r + 8);
      int sy2 = ay + sinf(sa) * (r + 8);
      gameSpr.drawLine(sx, sy, sx2, sy2, 0xF800);
    }
  } else if (aoe.type == 2) {
    bool blink = ((int)(aoe.timer * 10) % 2 == 0);
    if (blink) {
      gameSpr.drawCircle(ax, ay, r, 0xFC40);
      gameSpr.drawCircle(ax, ay, r - 2, 0xFC40);
      gameSpr.drawLine(ax - r, ay, ax + r, ay, 0xFC40);
      gameSpr.drawLine(ax, ay - r, ax, ay + r, 0xFC40);
    }
  } else if (aoe.type == 3) {
    gameSpr.fillCircle(ax, ay, r, 0xF800);
    gameSpr.fillCircle(ax, ay, r - 3, 0xFC40);
    for (int s = 0; s < 6; s++) {
      float sa = s * 1.0472f;
      int sx = ax + cosf(sa) * (r - 4);
      int sy = ay + sinf(sa) * (r - 4);
      int sx2 = ax + cosf(sa) * (r + 6);
      int sy2 = ay + sinf(sa) * (r + 6);
      gameSpr.drawLine(sx, sy, sx2, sy2, 0xFFE0);
    }
  }
}
