/******************************************************************************
 *
 * [引擎核心逻辑] ENEMY.CPP
 *
 * @desc : AI 状态机的具体流转实现。
 * Day 13 纯净重构：纯逻辑层实现“环境光照劫持”与“黑暗恐慌模式”。
 *
 ******************************************************************************/
#include "enemy.h"
#include "map.h" 
#include <graphics.h>
#include <math.h>
#include <stdlib.h> // 用于恐慌时的随机转头

Enemy::Enemy(float startX, float startY) : Entity(startX, startY) {
    Reset(Vector2D(startX, startY));
}

void Enemy::Reset(Vector2D startPos) {
    position = startPos;
    isAlive = true;
    hasBeenScanned = false;
    currentState = EnemyState::PATROL;
    stateTimer = 0;
    speed = ENEMY_BASE_SPEED;
    moveDirection = 1;
    facingX = 1.0f;
    facingY = 0.0f;

    // 💥 恢复正常视力
    currentVisionRadius = ENEMY_SIGHT_RADIUS;
    ResetPatrolBounds();
}

void Enemy::ResetPatrolBounds() {
    patrolStartX = position.x - ENEMY_PATROL_RADIUS;
    patrolEndX = position.x + ENEMY_PATROL_RADIUS;
}

void Enemy::UpdateAI(Vector2D playerPos, bool isPlayerMakingNoise, Map& levelMap)
{
    if (!isAlive) return;

    // ---------------------------------------------------------
    // 💥 Day13：最高优先级 - 环境光照劫持机制
    // ---------------------------------------------------------
    if (levelMap.GetIsAreaDark()) {
        currentVisionRadius = 0.0f; // 瞬间致盲

        // 突然灯黑了，强行打断巡逻和警戒，陷入恐慌！
        if (currentState == EnemyState::PATROL || currentState == EnemyState::ALERT) {
            currentState = EnemyState::PANIC_LISTEN;
            stateTimer = 0;
        }
    }
    else {
        currentVisionRadius = ENEMY_SIGHT_RADIUS; // 恢复光明视力
    }

    float dx = playerPos.x - position.x;
    float dy = playerPos.y - position.y;
    float distanceToPlayer = sqrt(dx * dx + dy * dy);

    float dirToPlayerX = 0, dirToPlayerY = 0;
    if (distanceToPlayer > 0) {
        dirToPlayerX = dx / distanceToPlayer;
        dirToPlayerY = dy / distanceToPlayer;
    }

    float targetFacingX = facingX;
    float targetFacingY = facingY;

    // ---------------------------------------------------------
    // 🧠 状态机行为流转
    // ---------------------------------------------------------
    switch (currentState) {
    case EnemyState::PATROL:
    {
        position.x += speed * moveDirection;
        if (position.x > patrolEndX) moveDirection = -1;
        if (position.x < patrolStartX) moveDirection = 1;

        targetFacingX = (float)moveDirection;
        targetFacingY = 0.0f;

        bool isHeard = (isPlayerMakingNoise && distanceToPlayer < ENEMY_HEAR_RADIUS);
        bool isSeen = false;

        // 💥 使用动态视力 currentVisionRadius 替代原本的写死常量
        if (distanceToPlayer < currentVisionRadius) {
            float dotProduct = (facingX * dirToPlayerX) + (facingY * dirToPlayerY);
            if (dotProduct > ENEMY_SIGHT_ANGLE) {
                isSeen = true;
            }
        }

        if (isHeard || isSeen) {
            currentState = EnemyState::ALERT;
            stateTimer = ENEMY_ALERT_FRAMES;
        }
        break;
    }

    case EnemyState::ALERT:
    {
        stateTimer--;
        targetFacingX = dirToPlayerX;
        targetFacingY = dirToPlayerY;

        if (stateTimer <= 0) {
            float dotProduct = (facingX * dirToPlayerX) + (facingY * dirToPlayerY);
            if (distanceToPlayer < ENEMY_HEAR_RADIUS + 50.0f && dotProduct > 0.5f) {
                currentState = EnemyState::CHASE;
            }
            else {
                currentState = EnemyState::PATROL;
            }
        }
        break;
    }

    case EnemyState::CHASE:
    {
        if (distanceToPlayer > 0) {
            Vector2D smartDir = levelMap.GetBestDirection(position, playerPos);

            position.x += smartDir.x * (speed * ENEMY_CHASE_MULT);
            position.y += smartDir.y * (speed * ENEMY_CHASE_MULT);

            targetFacingX = smartDir.x;
            targetFacingY = smartDir.y;
        }

        if (distanceToPlayer > ENEMY_LOSE_RADIUS) {
            currentState = EnemyState::PATROL;
            ResetPatrolBounds();
        }
        break;
    }

    // ---------------------------------------------------------
    // 💥 Day13：新增的瞎子恐慌摸索状态
    // ---------------------------------------------------------
    case EnemyState::PANIC_LISTEN:
    {
        stateTimer++;
        // 行为1：不敢走路，每 30 帧神经质地随机转头看
        if (stateTimer % 30 == 0) {
            targetFacingX = (rand() % 200 - 100) / 100.0f;
            targetFacingY = (rand() % 200 - 100) / 100.0f;
        }

        // 行为2：听觉代偿，只要玩家发出一点声响，无视朝向，瞬间锁定追杀！
        if (isPlayerMakingNoise && distanceToPlayer < ENEMY_HEAR_RADIUS) {
            currentState = EnemyState::CHASE;
        }
        break;
    }
    }

    // 平滑转身插值算法
    facingX = facingX + (targetFacingX - facingX) * ENEMY_TURN_SPEED;
    facingY = facingY + (targetFacingY - facingY) * ENEMY_TURN_SPEED;

    float length = sqrt(facingX * facingX + facingY * facingY);
    if (length > 0) {
        facingX /= length;
        facingY /= length;
    }
}

// 💥 彻底尊重你的架构：将视觉表现完全留给外部渲染器
void Enemy::Draw() {
    return;
}