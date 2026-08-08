#ifndef PLAYER_H
#define PLAYER_H

#include "globals.h"

// ==================== 玩家 ====================
void initPlayer();
void hurtPlayer(float damage);
void addExp(float amount);

// ==================== 武器数据 ====================
void getWeaponStats(uint8_t weaponId, uint8_t level, bool evolved,
                    float& cd, float& dmg, float& ep, float& ep2);
uint8_t getWeaponMaxLevel(uint8_t weaponId);
uint8_t getWeaponEvolvePassive(uint8_t weaponId);
const char* getEvolveName(uint8_t weaponId);

// ==================== 被动数据 ====================
uint8_t getPassiveMaxLevel(uint8_t passiveId);
float getPassivePerLevel(uint8_t passiveId);

// ==================== 查找 ====================
int8_t findWeapon(uint8_t weaponId);
int8_t findPassive(uint8_t passiveId);

// ==================== 属性重算 ====================
void recalcStats();

// ==================== 武器/被动操作 ====================
bool addWeapon(uint8_t weaponId);
bool upgradeWeapon(uint8_t weaponId);
bool addPassive(uint8_t passiveId);
bool evolveWeapon(uint8_t weaponId);

// ==================== 升级系统 ====================
void generateUpgrades();
void applyUpgrade(uint8_t index);

#endif
