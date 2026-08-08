#include <Arduino.h>
#include "game.h"
#include "utils.h"
#include "audio.h"
#include "player.h"
#include "enemy.h"

// ==================== 投射物槽位 ====================
int findFreeProjectileSlot() {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (!projectiles[i].alive) return i;
  }
  return -1;
}

void fireProjectile(float x, float y, float angle, float speed, float damage, int8_t pierce, float range) {
  int slot = findFreeProjectileSlot();
  if (slot < 0) return;
  Projectile& p = projectiles[slot];
  p.x = x; p.y = y;
  p.vx = cosf(angle) * speed;
  p.vy = sinf(angle) * speed;
  p.damage = damage;
  p.pierce = pierce;
  p.traveled = 0;
  p.maxRange = range;
  p.alive = true;
  p.hitMask = 0;
  projectileCount++;
}

// ==================== 伤害敌人 ====================
void damageEnemy(int idx, float damage) {
  if (idx < 0 || idx >= MAX_ENEMIES || !enemies[idx].alive) return;
  Enemy& e = enemies[idx];
  float armor = (e.type == ENEMY_THORN) ? 0.15f : (e.type == ENEMY_ELITE) ? 0.2f : (e.type == ENEMY_BOSS) ? 0.4f : 0;
  float actual = damage * (1.0f - armor);
  e.hp -= actual;
  if (e.hp <= 0) {
    e.alive = false;
    enemyCount--;
    killCount++;
    if (e.type == ENEMY_BOSS) {
      bossActive = false;
      gameWon = true;
      soundEvolve();
    } else {
      spawnGem(e.x, e.y, e.xpValue);
      soundKill();
    }
  }
}

// ==================== 经验宝石 ====================
int findFreeGemSlot() {
  for (int i = 0; i < MAX_GEMS; i++) {
    if (gems[i].life <= 0) return i;
  }
  return -1;
}

void spawnGem(float x, float y, uint8_t value) {
  int slot = findFreeGemSlot();
  if (slot < 0) return;
  gems[slot].x = x;
  gems[slot].y = y;
  gems[slot].value = value;
  gems[slot].life = 15.0f;
  gemCount++;
}

// ==================== 主游戏更新 ====================
void updateGame(float dt) {
  if (gameOver || upgrading) return;

  gameTime += dt;

  // ===== 玩家移动 =====
  float dx = 0, dy = 0;
  if (digitalRead(WIO_5S_UP) == LOW)    dy -= 1;
  if (digitalRead(WIO_5S_DOWN) == LOW)  dy += 1;
  if (digitalRead(WIO_5S_LEFT) == LOW)  dx -= 1;
  if (digitalRead(WIO_5S_RIGHT) == LOW) dx += 1;
  if (dx != 0 || dy != 0) {
    float mag = sqrtf(dx * dx + dy * dy);
    dx /= mag; dy /= mag;
    player.facingAngle = atan2f(dy, dx);
    playerMoving = true;
  } else {
    playerMoving = false;
  }

  float slowMult = 1.0f;
  for (int i = 0; i < slowZoneCount; i++) {
    if (slowZones[i].life > 0 && distF(player.x, player.y, slowZones[i].x, slowZones[i].y) < 35) {
      slowMult = 0.4f; break;
    }
  }
  player.x += dx * player.speed * slowMult * dt;
  player.y += dy * player.speed * slowMult * dt;

  // 夹持到世界边界（而不是屏幕边界）
  player.x = clampF(player.x, PLAYER_RADIUS, WORLD_W - PLAYER_RADIUS);
  player.y = clampF(player.y, PLAYER_RADIUS, WORLD_H - PLAYER_RADIUS);

  // 更新相机
  updateCamera();

  // ===== 玩家动画计时器 =====
  if (playerMoving) {
    playerWalkPhase += dt * 7.0f;  // 步频
  } else {
    // 呼吸待机动画
    playerWalkPhase += dt * 1.5f;
  }
  if (playerAttackFlash > 0) playerAttackFlash -= dt;
  if (playerHurtFlash > 0)   playerHurtFlash -= dt;

  // 无敌计时
  if (player.invincibleTimer > 0) player.invincibleTimer -= dt;

  // 生命恢复
  if (player.regen > 0 && player.hp < player.maxHp) {
    player.hp += player.regen * dt;
    if (player.hp > player.maxHp) player.hp = player.maxHp;
  }

  // ===== 生成敌人 =====
  float t = gameTime / 60.0f;
  float spawnInterval = 0.15f - t * 0.025f;
  if (spawnInterval < 0.03f) spawnInterval = 0.03f;
  static float spawnTimer = 0;
  spawnTimer += dt;
  while (spawnTimer >= spawnInterval && enemyCount < MAX_ENEMIES) {
    spawnTimer -= spawnInterval;
    spawnEnemy();
    if (t > 3 && rngFloat() < (t - 3) * 0.12f) spawnEnemy();
  }

  // 精英怪
  static float eliteTimer = 0;
  eliteTimer += dt;
  float eliteInterval = 20.0f - t * 2.0f;
  if (eliteInterval < 8) eliteInterval = 8;
  if (eliteTimer >= eliteInterval && enemyCount < MAX_ENEMIES) {
    eliteTimer = 0;
    spawnElite();
  }

  // ===== 更新敌人 =====
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].alive) continue;
    Enemy& e = enemies[i];
    float a = angleTo(e.x, e.y, player.x, player.y);
    float spd = e.speed;
    for (int j = 0; j < slowZoneCount; j++) {
      if (slowZones[j].life > 0 && distF(e.x, e.y, slowZones[j].x, slowZones[j].y) < 35) {
        spd *= 0.4f; break;
      }
    }
    e.x += cosf(a) * spd * dt;
    e.y += sinf(a) * spd * dt;
  }

  // ===== BOSS攻击逻辑 =====
  if (bossActive) {
    if (screenShake > 0) screenShake -= dt * 5;
    if (screenShake < 0) screenShake = 0;

    for (int i = 0; i < MAX_BOSS_AOE; i++) {
      if (!bossAoEs[i].active) continue;
      BossAoE& aoe = bossAoEs[i];
      aoe.timer += dt;
      if (aoe.type == 0 || aoe.type == 1) {
        aoe.radius += aoe.maxRadius * 0.5f * dt;
        if (aoe.radius > aoe.maxRadius) aoe.radius = aoe.maxRadius;
      }
      if (aoe.type == 2) {
        if (aoe.timer > 0.6f) {
          aoe.type = 3;
          aoe.timer = 0;
          aoe.radius = aoe.maxRadius;
          aoe.color = 0xF800;
        }
      }
      if (aoe.type == 3) {
        if (aoe.timer > 0.3f) { aoe.active = false; bossAoECount--; }
      }
      if (aoe.type <= 1 && aoe.timer > 2.5f) { aoe.active = false; bossAoECount--; }
      if (aoe.active && aoe.type != 2) {
        float d = distF(player.x, player.y, aoe.x, aoe.y);
        if (d < aoe.radius + PLAYER_RADIUS) {
          hurtPlayer(500);
        }
      }
    }

    bossAttackTimer -= dt;
    if (bossAttackTimer <= 0) {
      bossAttackMode = (bossAttackMode + 1) % 4;
      soundExplode();
      int8_t bi = findBoss();
      if (bi < 0) { bossAttackTimer = 99; }
      else {
        switch (bossAttackMode) {
          case 0: {
            screenShake = 2.0f;
            for (int i = 0; i < 4; i++) {
              float ang = i * 1.5708f;
              addBossAoE(enemies[bi].x + cosf(ang) * 20, enemies[bi].y + sinf(ang) * 20, 10, 80, 0, 0x3A0);
            }
            bossAttackTimer = 3.0f;
            break;
          }
          case 1: {
            screenShake = 3.0f;
            for (int i = 0; i < 3; i++) {
              addBossAoE(enemies[bi].x, enemies[bi].y, 30 + i * 35, 50 + i * 40, 1, 0xC800);
            }
            bossAttackTimer = 3.5f;
            break;
          }
          case 2: {
            screenShake = 2.5f;
            for (int i = 0; i < 5; i++) {
              float px = player.x + cosf(i * 1.2566f) * 50 + rngFloat() * 30 - 15;
              float py = player.y + sinf(i * 1.2566f) * 50 + rngFloat() * 30 - 15;
              if (py > WORLD_H) py = WORLD_H - 20;
              if (py < 0) py = 20;
              addBossAoE(px, py, 0, 25, 2, 0xFC40);
            }
            bossAttackTimer = 2.5f;
            break;
          }
          case 3: {
            screenShake = 4.0f;
            for (int i = 0; i < 8; i++) {
              float ang = i * 0.7854f;
              float sx = player.x + cosf(ang) * 120;
              float sy = player.y + sinf(ang) * 120;
              addBossAoE(sx, sy, 15, 70, 0, 0x4A0);
            }
            bossAttackTimer = 3.0f;
            break;
          }
        }
      }
    }
  }

  // 蒲公英慢速区域
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].alive || enemies[i].type != ENEMY_DANDELION) continue;
    if (rngFloat() < 0.01f && slowZoneCount < MAX_SLOW_ZONES) {
      slowZones[slowZoneCount].x = enemies[i].x;
      slowZones[slowZoneCount].y = enemies[i].y;
      slowZones[slowZoneCount].life = 3.0f;
      slowZoneCount++;
    }
  }

  // 更新慢速区域
  for (int i = 0; i < slowZoneCount; i++) {
    slowZones[i].life -= dt;
  }
  int writeIdx = 0;
  for (int i = 0; i < slowZoneCount; i++) {
    if (slowZones[i].life > 0) {
      if (writeIdx != i) slowZones[writeIdx] = slowZones[i];
      writeIdx++;
    }
  }
  slowZoneCount = writeIdx;

  // ===== 自动攻击 =====
  float nearestDist = 999999;
  int nearestIdx = -1;
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].alive) continue;
    float d = dist2(player.x, player.y, enemies[i].x, enemies[i].y);
    if (d < nearestDist) { nearestDist = d; nearestIdx = i; }
  }

  for (uint8_t wi = 0; wi < player.weaponCount; wi++) {
    WeaponState& w = player.weapons[wi];
    w.cooldown -= dt;

    switch (w.weaponId) {
      case 0: {
        if (w.cooldown <= 0 && nearestIdx >= 0) {
          w.cooldown = w.cooldownMax * player.cooldownMult;
          float a = angleTo(player.x, player.y, enemies[nearestIdx].x, enemies[nearestIdx].y);
          int count = (int)w.extraParam;
          for (int c = 0; c < count; c++) {
            float spread = (count > 1) ? (c - (count - 1) * 0.5f) * 0.15f : 0;
            fireProjectile(player.x, player.y, a + spread, 350, w.damage * player.damageMult, (int8_t)w.extraParam2, 500);
          }
          soundShoot();
          playerAttackFlash = 0.12f;
        }
        break;
      }
      case 1: {
        w.extraParam += w.extraParam2 * dt;
        if (w.extraParam > 6.283185f) w.extraParam -= 6.283185f;
        float cdTmp, dmgTmp, orbitR, avTmp;
        getWeaponStats(w.weaponId, w.level, w.evolved, cdTmp, dmgTmp, orbitR, avTmp);
        float sx = player.x + cosf(w.extraParam) * orbitR;
        float sy = player.y + sinf(w.extraParam) * orbitR;
        for (int i = 0; i < MAX_ENEMIES; i++) {
          if (!enemies[i].alive) continue;
          if (distF(sx, sy, enemies[i].x, enemies[i].y) < 12) {
            damageEnemy(i, w.damage * player.damageMult);
            if (playerAttackFlash < 0.02f) playerAttackFlash = 0.05f;
            if (w.evolved) {
              player.hp += w.damage * 0.15f;
              if (player.hp > player.maxHp) player.hp = player.maxHp;
            }
          }
        }
        break;
      }
      case 2: {
        if (w.cooldown <= 0) {
          w.cooldown = w.cooldownMax * player.cooldownMult;
          playerAttackFlash = 0.1f;
          float radius = w.extraParam;
          for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].alive) continue;
            if (distF(player.x, player.y, enemies[i].x, enemies[i].y) < radius + 8) {
              damageEnemy(i, w.damage * player.damageMult);
            }
          }
        }
        break;
      }
    }
  }

  // ===== 更新投射物 =====
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (!projectiles[i].alive) continue;
    Projectile& p = projectiles[i];
    p.x += p.vx * dt;
    p.y += p.vy * dt;
    p.traveled += sqrtf(p.vx * p.vx + p.vy * p.vy) * dt;
    // 世界边界检查（带外边距）
    if (p.traveled > p.maxRange || p.x < -20 || p.x > WORLD_W + 20 || p.y < -20 || p.y > WORLD_H + 20) {
      p.alive = false;
      projectileCount--;
      continue;
    }
    for (int j = 0; j < MAX_ENEMIES; j++) {
      if (!enemies[j].alive) continue;
      if (p.hitMask & (1 << (j % 8))) continue;
      if (distF(p.x, p.y, enemies[j].x, enemies[j].y) < 10) {
        damageEnemy(j, p.damage);
        p.hitMask |= (1 << (j % 8));
        if (p.pierce == 0) {
          p.alive = false; projectileCount--; break;
        } else if (p.pierce > 0) {
          p.pierce--;
          if (p.pierce <= 0) { p.alive = false; projectileCount--; break; }
        }
      }
    }
  }

  // ===== 玩家碰撞敌人 =====
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].alive) continue;
    Enemy& e = enemies[i];
    if (distF(player.x, player.y, e.x, e.y) < PLAYER_RADIUS + 8) {
      if (e.type == ENEMY_BOSS) {
        player.hp = 0;
        gameOver = true;
        soundHurt();
      } else {
        hurtPlayer(e.damage);
      }
    }
  }

  // BOSS弹幕碰撞玩家
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (!projectiles[i].alive) continue;
    if (distF(player.x, player.y, projectiles[i].x, projectiles[i].y) < 10) {
      hurtPlayer(projectiles[i].damage);
      projectiles[i].alive = false;
      projectileCount--;
    }
  }

  // ===== 更新经验宝石 =====
  for (int i = 0; i < MAX_GEMS; i++) {
    if (gems[i].life <= 0) continue;
    gems[i].life -= dt;
    float d = distF(player.x, player.y, gems[i].x, gems[i].y);
    if (d < player.pickupRange) {
      float a = angleTo(gems[i].x, gems[i].y, player.x, player.y);
      float speed = 30 + (1.0f - d / player.pickupRange) * 200;
      gems[i].x += cosf(a) * speed * dt;
      gems[i].y += sinf(a) * speed * dt;
    }
    if (d < PLAYER_RADIUS + 6) {
      addExp(gems[i].value);
      gems[i].life = 0;
      gemCount--;
    }
  }
}
