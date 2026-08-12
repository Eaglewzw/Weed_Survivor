#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>

void beep(int freq, int duration);
void audioTick();
void soundShoot();
void soundKill(int combo);
void soundGem(int streak);
void soundLevelUp();
void soundHurt();
void soundEvolve();
void soundExplode();
void soundHeartbeat();
void soundBossRoar();

// ==================== 芯片音乐 ====================
#define MUSIC_MENU   0
#define MUSIC_BATTLE 1
#define MUSIC_BOSS   2

void musicPlay(uint8_t trackId);
void musicStop();

#endif
