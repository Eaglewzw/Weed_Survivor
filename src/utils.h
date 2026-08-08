#ifndef UTILS_H
#define UTILS_H

#include "src/globals.h"

// ==================== RNG ====================
uint32_t rngNext();
float rngFloat();
int rngInt(int min, int max);

// ==================== 数学工具 ====================
float dist2(float x1, float y1, float x2, float y2);
float distF(float x1, float y1, float x2, float y2);
float angleTo(float x1, float y1, float x2, float y2);
float clampF(float v, float lo, float hi);

// ==================== 相机 ====================
void updateCamera();
int toScreenX(float worldX);
int toScreenY(float worldY);

#endif
