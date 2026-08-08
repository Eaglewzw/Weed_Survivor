#include <Arduino.h>
#include "src/utils.h"

// ==================== RNG (LCG) ====================
uint32_t rngNext() {
  rngState = rngState * 1103515245 + 12345;
  return rngState;
}

float rngFloat() {
  return (float)(rngNext() & 0x7FFFFFFF) / 2147483647.0f;
}

int rngInt(int min, int max) {
  return min + (int)(rngFloat() * (max - min + 1));
}

// ==================== 数学工具 ====================
float dist2(float x1, float y1, float x2, float y2) {
  float dx = x2 - x1, dy = y2 - y1;
  return dx * dx + dy * dy;
}

float distF(float x1, float y1, float x2, float y2) {
  return sqrtf(dist2(x1, y1, x2, y2));
}

float angleTo(float x1, float y1, float x2, float y2) {
  return atan2f(y2 - y1, x2 - x1);
}

float clampF(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

// ==================== 相机 ====================
void updateCamera() {
  cameraX = player.x - SW / 2;
  cameraY = player.y - PLAY_AREA_H / 2;
  cameraX = clampF(cameraX, 0, WORLD_W - SW);
  cameraY = clampF(cameraY, 0, WORLD_H - PLAY_AREA_H);
}

int toScreenX(float worldX) {
  return (int)(worldX - cameraX + shakeX);
}

int toScreenY(float worldY) {
  return (int)(worldY - cameraY) + PLAY_AREA_Y + shakeY;
}
