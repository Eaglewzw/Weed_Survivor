#include <Arduino.h>
#include "player.h"
#include "utils.h"
#include "audio.h"
#include "enemy.h"

// ==================== 玩家初始化 ====================
void initPlayer() {
  player.x = WORLD_W / 2;
  player.y = WORLD_H / 2;
  player.hp = 200;
  player.maxHp = 200;
  player.exp = 0;
  player.expToNext = 12;
  player.speed = PLAYER_SPEED;
  player.damageMult = 1.0f;
  player.cooldownMult = 1.0f;
  player.pickupRange = 100;
  player.armor = 0;
  player.regen = 0;
  player.invincibleTimer = 0;
  player.level = 1;
  player.facingAngle = 0;
  player.weaponCount = 0;
  player.passiveCount = 0;

  WeaponState& w = player.weapons[0];
  w.weaponId = 0;
  w.level = 1;
  w.evolved = false;
  float cd, dmg, ep, ep2;
  getWeaponStats(0, 1, false, cd, dmg, ep, ep2);
  w.cooldownMax = cd;
  w.cooldown = 0;
  w.damage = dmg;
  w.extraParam = ep;
  w.extraParam2 = ep2;
  player.weaponCount = 1;
}

// ==================== 玩家受伤 ====================
void hurtPlayer(float damage) {
  if (player.invincibleTimer > 0) return;
  float actual = damage * (1.0f - player.armor);
  player.hp -= actual;
  player.invincibleTimer = INVINCIBLE_TIME;
  playerHurtFlash = 0.25f;
  soundHurt();
  if (player.hp <= 0) {
    player.hp = 0;
    gameOver = true;
    musicStop();
  }
}

// ==================== 玩家获得经验 ====================
void addExp(float amount) {
  player.exp += amount;
  while (player.exp >= player.expToNext) {
    player.exp -= player.expToNext;
    player.level++;
    player.expToNext = 8 + player.level * 4;
    if (player.level >= 50 && !bossActive) { spawnBoss(); }
    soundLevelUp();
    generateUpgrades();
    if (upgradeOptionCount == 0) {
      upgradeDirty = false;
      return;
    }
    upgrading = true;
    upgradeDirty = true;
    upgradeSelection = 0;
    return;
  }
}

// ==================== 武器数据 ====================
void getWeaponStats(uint8_t weaponId, uint8_t level, bool evolved,
                    float& cd, float& dmg, float& ep, float& ep2) {
  switch (weaponId) {
    case 0:
      cd = 0.35f * powf(0.85f, level - 1);
      dmg = 24 * powf(1.2f, level - 1);
      ep = 1 + (level - 1) / 2;
      ep2 = (level - 1) / 2;
      if (evolved) { dmg *= 2.5f; ep *= 2; cd *= 0.7f; ep2 += 2; }
      break;
    case 1:
      cd = 0;
      dmg = 28 * powf(1.2f, level - 1);
      ep = 30 + (level - 1) * 5;
      ep2 = 5.0f + (level - 1) * 0.6f;
      if (evolved) { dmg *= 2.5f; ep *= 1.5f; ep2 *= 1.3f; }
      break;
    case 2:
      cd = 0.15f * powf(0.9f, level - 1);
      dmg = 20 * powf(1.3f, level - 1);
      ep = 40 + (level - 1) * 5;
      ep2 = 0;
      if (evolved) { dmg *= 2.5f; ep *= 2.0f; cd *= 0.7f; }
      break;
  }
}

uint8_t getWeaponMaxLevel(uint8_t weaponId) { return 5; }

uint8_t getWeaponEvolvePassive(uint8_t weaponId) {
  const uint8_t map[] = {0, 2, 4};
  return map[weaponId];
}

const char* getEvolveName(uint8_t weaponId) {
  const char* names[] = {"Storm Leaf", "Blood Scythe", "Toxic Aura"};
  return names[weaponId];
}

// ==================== 被动数据 ====================
uint8_t getPassiveMaxLevel(uint8_t passiveId) { return 5; }

// ==================== 查找武器/被动 ====================
int8_t findWeapon(uint8_t weaponId) {
  for (uint8_t i = 0; i < player.weaponCount; i++) {
    if (player.weapons[i].weaponId == weaponId) return i;
  }
  return -1;
}

int8_t findPassive(uint8_t passiveId) {
  for (uint8_t i = 0; i < player.passiveCount; i++) {
    if (player.passives[i].passiveId == passiveId) return i;
  }
  return -1;
}

// ==================== 玩家属性重算 ====================
void recalcStats() {
  float atkLv = 0, spdLv = 0, movLv = 0, hpLv = 0, armLv = 0;
  for (uint8_t i = 0; i < player.passiveCount; i++) {
    uint8_t id = player.passives[i].passiveId;
    uint8_t lv = player.passives[i].level;
    switch (id) {
      case 0: atkLv = lv; break;
      case 1: spdLv = lv; break;
      case 2: movLv = lv; break;
      case 3: hpLv = lv; break;
      case 4: armLv = lv; break;
    }
  }
  player.damageMult = 1.0f + atkLv * 0.20f;
  player.cooldownMult = 1.0f - spdLv * 0.15f;
  if (player.cooldownMult < 0.1f) player.cooldownMult = 0.1f;
  player.speed = PLAYER_SPEED * (1.0f + movLv * 0.15f);
  player.pickupRange = 100 * (1.0f + movLv * 0.05f);
  player.maxHp = 200 + hpLv * 30;
  if (player.hp > player.maxHp) player.hp = player.maxHp;
  player.armor = armLv * 0.15f;
  if (player.armor > 0.7f) player.armor = 0.7f;
  player.regen = 0;

  for (uint8_t i = 0; i < player.weaponCount; i++) {
    WeaponState& w = player.weapons[i];
    float cd, dmg, ep, ep2;
    getWeaponStats(w.weaponId, w.level, w.evolved, cd, dmg, ep, ep2);
    w.cooldownMax = cd;
    w.damage = dmg;
    if (w.weaponId == 0) { w.extraParam = ep; w.extraParam2 = ep2; }
    else if (w.weaponId == 1) { w.extraParam2 = ep2; }
    else if (w.weaponId == 2) { w.extraParam = ep; w.extraParam2 = ep2; }
  }
}

// ==================== 武器升级/添加 ====================
bool addWeapon(uint8_t weaponId) {
  if (player.weaponCount >= MAX_WEAPONS) return false;
  if (findWeapon(weaponId) >= 0) return false;
  WeaponState& w = player.weapons[player.weaponCount];
  w.weaponId = weaponId;
  w.level = 1;
  w.evolved = false;
  w.cooldown = 0;
  float cd, dmg, ep, ep2;
  getWeaponStats(weaponId, 1, false, cd, dmg, ep, ep2);
  w.cooldownMax = cd;
  w.damage = dmg;
  w.extraParam = ep;
  w.extraParam2 = ep2;
  if (weaponId == 1) w.extraParam = 0;
  player.weaponCount++;
  return true;
}

bool upgradeWeapon(uint8_t weaponId) {
  int8_t idx = findWeapon(weaponId);
  if (idx < 0) return false;
  WeaponState& w = player.weapons[idx];
  if (w.level >= getWeaponMaxLevel(weaponId)) return false;
  w.level++;
  float cd, dmg, ep, ep2;
  getWeaponStats(weaponId, w.level, w.evolved, cd, dmg, ep, ep2);
  w.cooldownMax = cd;
  w.damage = dmg;
  if (weaponId == 0) { w.extraParam = ep; w.extraParam2 = ep2; }
  else if (weaponId == 1) { w.extraParam2 = ep2; }
  else if (weaponId == 2) { w.extraParam = ep; w.extraParam2 = ep2; }
  return true;
}

bool addPassive(uint8_t passiveId) {
  int8_t idx = findPassive(passiveId);
  if (idx >= 0) {
    if (player.passives[idx].level >= getPassiveMaxLevel(passiveId)) return false;
    player.passives[idx].level++;
  } else {
    if (player.passiveCount >= MAX_PASSIVES) return false;
    player.passives[player.passiveCount].passiveId = passiveId;
    player.passives[player.passiveCount].level = 1;
    player.passiveCount++;
  }
  recalcStats();
  return true;
}

bool evolveWeapon(uint8_t weaponId) {
  int8_t idx = findWeapon(weaponId);
  if (idx < 0) return false;
  WeaponState& w = player.weapons[idx];
  if (w.evolved) return false;
  if (w.level < getWeaponMaxLevel(weaponId)) return false;
  uint8_t needPassive = getWeaponEvolvePassive(weaponId);
  int8_t pi = findPassive(needPassive);
  if (pi < 0 || player.passives[pi].level < getPassiveMaxLevel(needPassive)) return false;
  w.evolved = true;
  float cd, dmg, ep, ep2;
  getWeaponStats(weaponId, w.level, true, cd, dmg, ep, ep2);
  w.cooldownMax = cd;
  w.damage = dmg;
  if (weaponId == 0) { w.extraParam = ep; w.extraParam2 = ep2; }
  else if (weaponId == 1) { w.extraParam2 = ep2; }
  else if (weaponId == 2) { w.extraParam = ep; w.extraParam2 = ep2; }
  soundEvolve();
  return true;
}

// ==================== 生成升级选项 ====================
void generateUpgrades() {
  Candidate pool[20];
  int poolSize = 0;

  for (uint8_t i = 0; i < player.weaponCount; i++) {
    WeaponState& w = player.weapons[i];
    if (w.level < getWeaponMaxLevel(w.weaponId) && !w.evolved) {
      pool[poolSize].type = 0; pool[poolSize].id = w.weaponId; pool[poolSize].priority = 10;
      poolSize++;
    }
    if (w.level >= getWeaponMaxLevel(w.weaponId) && !w.evolved) {
      uint8_t needPassive = getWeaponEvolvePassive(w.weaponId);
      int8_t pi = findPassive(needPassive);
      if (pi >= 0 && player.passives[pi].level >= getPassiveMaxLevel(needPassive)) {
        pool[poolSize].type = 4; pool[poolSize].id = w.weaponId; pool[poolSize].priority = 100;
        poolSize++;
      }
    }
  }

  for (uint8_t wid = 0; wid < 3; wid++) {
    if (findWeapon(wid) < 0 && player.weaponCount < MAX_WEAPONS) {
      pool[poolSize].type = 1; pool[poolSize].id = wid; pool[poolSize].priority = 15;
      poolSize++;
    }
  }

  for (uint8_t i = 0; i < player.passiveCount; i++) {
    if (player.passives[i].level < getPassiveMaxLevel(player.passives[i].passiveId)) {
      pool[poolSize].type = 2; pool[poolSize].id = player.passives[i].passiveId; pool[poolSize].priority = 8;
      poolSize++;
    }
  }

  for (uint8_t pid = 0; pid < 5; pid++) {
    if (findPassive(pid) < 0 && player.passiveCount < MAX_PASSIVES) {
      pool[poolSize].type = 3; pool[poolSize].id = pid; pool[poolSize].priority = 12;
      poolSize++;
    }
  }

  if (player.hp < player.maxHp * 0.7f) {
    pool[poolSize].type = 5; pool[poolSize].id = 0; pool[poolSize].priority = 5;
    poolSize++;
  }

  for (int i = 0; i < poolSize - 1; i++) {
    for (int j = i + 1; j < poolSize; j++) {
      if (pool[j].priority > pool[i].priority) {
        Candidate tmp = pool[i]; pool[i] = pool[j]; pool[j] = tmp;
      }
    }
  }
  int topN = poolSize < 12 ? poolSize : 12;
  if (topN == 0) { upgradeOptionCount = 0; return; }

  int selected[3] = {-1, -1, -1};
  int selCount = 0;
  int attempts = 0;
  while (selCount < 3 && attempts < 30) {
    attempts++;
    int idx = rngInt(0, topN - 1);
    bool dup = false;
    for (int s = 0; s < selCount; s++) {
      if (selected[s] == idx) { dup = true; break; }
    }
    if (!dup) {
      selected[selCount++] = idx;
    }
  }

  for (int i = 0; i < topN; i++) {
    if (pool[i].type == 4) {
      bool found = false;
      for (int s = 0; s < selCount; s++) {
        if (selected[s] == i) { found = true; break; }
      }
      if (!found && selCount > 0) { selected[selCount - 1] = i; }
      break;
    }
  }

  upgradeOptionCount = selCount;
  for (int i = 0; i < selCount; i++) {
    Candidate& c = pool[selected[i]];
    upgradeOptions[i].type = c.type;
    upgradeOptions[i].id = c.id;
  }
}

bool applyUpgrade(uint8_t index) {
  if (index >= upgradeOptionCount) return false;
  UpgradeOption& opt = upgradeOptions[index];
  switch (opt.type) {
    case 0: upgradeWeapon(opt.id); break;
    case 1: addWeapon(opt.id); break;
    case 2: addPassive(opt.id); break;
    case 3: addPassive(opt.id); break;
    case 4: evolveWeapon(opt.id); break;
    case 5: player.hp += player.maxHp * 0.5f;
            if (player.hp > player.maxHp) player.hp = player.maxHp; break;
  }
  recalcStats();
  upgrading = false;
  return true;
}
