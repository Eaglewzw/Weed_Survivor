#include "audio.h"

static unsigned long beepEndTime = 0;

void beep(int freq, int duration) {
  unsigned long now = millis();
  if (now < beepEndTime) return;
  tone(WIO_BUZZER, freq, duration);
  beepEndTime = now + duration + 20;
}

void soundShoot()  { beep(800, 40); }
void soundHit()    { beep(200, 30); }
void soundKill()   { beep(150, 60); }
void soundLevelUp(){ beep(400, 80); delay(80); beep(600, 80); }
void soundHurt()   { beep(100, 80); }
void soundEvolve() { beep(500, 100); delay(100); beep(800, 100); }
void soundExplode(){ beep(80, 120); }
