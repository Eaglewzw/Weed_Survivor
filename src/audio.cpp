#include "audio.h"

// 非阻塞音符队列：旋律逐音播放，不用 delay()
#define MAX_NOTES 8

struct Note {
  uint16_t freq;
  uint16_t dur;
};

static Note noteQueue[MAX_NOTES];
static uint8_t noteCount = 0;
static unsigned long noteEndAt = 0;
static unsigned long beepGate = 0;

static void pushNote(int freq, int dur) {
  if (noteCount >= MAX_NOTES) return;
  noteQueue[noteCount].freq = (uint16_t)freq;
  noteQueue[noteCount].dur = (uint16_t)dur;
  noteCount++;
}

void beep(int freq, int duration) {
  unsigned long now = millis();
  if (now < beepGate) return;
  tone(WIO_BUZZER, freq, duration);
  beepGate = now + duration + 20;
  noteEndAt = now + duration;
}

static void musicTick();

// 每帧调用：当前音符播完后弹队列中的下一个
void audioTick() {
  if (noteCount > 0) {
    unsigned long now = millis();
    if (now >= noteEndAt) {
      tone(WIO_BUZZER, noteQueue[0].freq, noteQueue[0].dur);
      noteEndAt = now + noteQueue[0].dur + 10;
      for (uint8_t i = 1; i < noteCount; i++) noteQueue[i - 1] = noteQueue[i];
      noteCount--;
    }
  }
  musicTick();
}

// ==================== 芯片音乐引擎 ====================
// 16 步循环，每步最多 3 个音符（琶音和弦），单音蜂鸣器按时间片轮播

#define N_C4  262
#define N_D4  294
#define N_EB4 311
#define N_E4  330
#define N_F4  349
#define N_FS4 370
#define N_G4  392
#define N_AB4 415
#define N_A4  440
#define N_BB4 466
#define N_B4  494
#define N_C5  523
#define N_D5  587
#define N_EB5 622
#define N_E5  659
#define N_F5  698
#define N_FS5 740
#define N_G5  784
#define N_AB5 831
#define N_A5  880
#define N_B5  988
#define N_C6  1047

struct Step {
  uint16_t f1, f2, f3;  // 0 = 休止
  uint16_t dur;         // 本步总时长 ms
};

// 菜单：C 大调轻快琶音（C - Am - F - G）
static const Step TRACK_MENU[] = {
  {N_C5, N_E5, N_G5, 190}, {N_C5, N_E5, N_G5, 190},
  {N_A4, N_C5, N_E5, 190}, {N_A4, N_C5, N_E5, 190},
  {N_F4, N_A4, N_C5, 190}, {N_F4, N_A4, N_C5, 190},
  {N_G4, N_B4, N_D5, 190}, {N_G4, N_B4, N_D5, 190},
  {N_C6, N_G5, N_E5, 190}, {N_C5, N_E5, N_G5, 190},
  {N_A5, N_E5, N_C5, 190}, {N_A4, N_C5, N_E5, 190},
  {N_F5, N_C5, N_A4, 190}, {N_F4, N_A4, N_C5, 190},
  {N_G5, N_D5, N_B4, 190}, {N_G4, N_B4, N_D5, 190},
};

// 战斗：E 小调低音行进（低音脉冲 + 五度强调）
static const Step TRACK_BATTLE[] = {
  {N_E4, N_B4, 0, 140}, {N_E4, 0, 0, 140},
  {N_E4, N_B4, 0, 140}, {N_G4, 0, 0, 140},
  {N_E4, N_B4, 0, 140}, {N_E4, 0, 0, 140},
  {N_E4, N_B4, 0, 140}, {N_A4, 0, 0, 140},
  {N_D4, N_A4, 0, 140}, {N_D4, 0, 0, 140},
  {N_D4, N_A4, 0, 140}, {N_FS4, 0, 0, 140},
  {N_E4, N_B4, 0, 140}, {N_E4, 0, 0, 140},
  {N_B4, N_E5, 0, 140}, {N_D5, 0, 0, 140},
};

// BOSS：A 小调紧张固定音型（小二度冲突）
static const Step TRACK_BOSS[] = {
  {N_A4, N_AB4, 0, 160}, {N_A4, 0, 0, 160},
  {N_E5, 0, 0, 160},     {N_A4, N_AB4, 0, 160},
  {N_A4, 0, 0, 160},     {N_D5, 0, 0, 160},
  {N_C5, N_B4, 0, 160},  {N_A4, 0, 0, 160},
  {N_A4, N_AB4, 0, 160}, {N_A4, 0, 0, 160},
  {N_F5, 0, 0, 160},     {N_E5, 0, 0, 160},
  {N_D5, N_EB5, 0, 160}, {N_D5, 0, 0, 160},
  {N_B4, 0, 0, 160},     {N_E5, 0, 0, 160},
};

static const Step* MUSIC_TRACKS[] = { TRACK_MENU, TRACK_BATTLE, TRACK_BOSS };
static const uint8_t MUSIC_LENS[] = { 16, 16, 16 };

static const Step* curTrack = NULL;
static uint8_t curTrackLen = 0;
static uint8_t stepIdx = 0;
static uint8_t subIdx = 0;
static unsigned long nextNoteAt = 0;
static unsigned long stepEndAt = 0;

void musicPlay(uint8_t trackId) {
  if (trackId > 2) return;
  if (curTrack == MUSIC_TRACKS[trackId]) return;
  curTrack = MUSIC_TRACKS[trackId];
  curTrackLen = MUSIC_LENS[trackId];
  stepIdx = 0;
  subIdx = 0;
  nextNoteAt = 0;
}

void musicStop() {
  curTrack = NULL;
}

static void musicTick() {
  if (curTrack == NULL || noteCount > 0) return;  // 旋律播放时让位
  unsigned long now = millis();
  if (now < nextNoteAt) return;
  const Step& s = curTrack[stepIdx];
  if (subIdx == 0) stepEndAt = now + s.dur;
  uint16_t f = (subIdx == 0) ? s.f1 : (subIdx == 1) ? s.f2 : s.f3;
  uint8_t total = (s.f1 != 0) + (s.f2 != 0) + (s.f3 != 0);
  subIdx++;
  if (subIdx < total) {
    nextNoteAt = now + (stepEndAt - now) / (total - subIdx + 1);
  } else {
    subIdx = 0;
    stepIdx++;
    if (stepIdx >= curTrackLen) stepIdx = 0;
    nextNoteAt = stepEndAt + 15;
  }
  if (f > 0) {
    int dur = s.dur / total;
    if (dur > 180) dur = 180;
    if (dur < 40) dur = 40;
    tone(WIO_BUZZER, f, dur);
  }
}

void soundShoot()   { beep(800, 40); }
void soundKill(int combo) {
  int f = 150 + combo * 25;
  if (f > 900) f = 900;
  beep(f, 50);
}
void soundGem(int streak) {
  int f = 350 + streak * 50;
  if (f > 900) f = 900;
  beep(f, 30);
}
void soundLevelUp() { pushNote(400, 80); pushNote(600, 80); pushNote(800, 100); }
void soundHurt()    { beep(100, 80); }
void soundEvolve()  { pushNote(500, 100); pushNote(800, 100); pushNote(1200, 150); }
void soundExplode() { beep(80, 120); }
void soundHeartbeat() { beep(110, 60); pushNote(85, 70); }
void soundBossRoar()   { pushNote(70, 150); pushNote(50, 300); }
