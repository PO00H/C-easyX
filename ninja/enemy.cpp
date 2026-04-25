/******************************************************************************
 *
 * [引擎核心逻辑] ENEMY.CPP
 *
 * @desc : AI 状态机的具体流转实现。剔除死代码，整合 Lerp 平滑转向。
 *
 ******************************************************************************/
#include "enemy.h"
#include "map.h" // 💥 这里必须引入真正的地图头文件
#include <graphics.h>
#include <math.h>

Enemy::Enemy(float startX, float startY) : Entity(startX, startY) {
    Reset(Vector2D(startX, startY)); // 复用 Reset 逻辑
}

// 💥 重生时的大脑清理
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
    ResetPatrolBounds();
}

void Enemy::ResetPatrolBounds() {
    patrolStartX = position.x - ENEMY_PATROL_RADIUS;
    patrolEndX = position.x + ENEMY_PATROL_RADIUS;
}

// 💥 接入导航基站
void Enemy::UpdateAI(Vector2D playerPos, bool isPlayerMakingNoise, Map& levelMap)
{
    if (!isAlive) return;

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

        if (distanceToPlayer < ENEMY_SIGHT_RADIUS) {
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
            // 💥 向 GPS 索要最佳规避路线
            Vector2D smartDir = levelMap.GetBestDirection(position, playerPos);

            position.x += smartDir.x * (speed * ENEMY_CHASE_MULT);
            position.y += smartDir.y * (speed * ENEMY_CHASE_MULT);

            // 让敌人的脸顺着走路的方向看
            targetFacingX = smartDir.x;
            targetFacingY = smartDir.y;
        }

        if (distanceToPlayer > ENEMY_LOSE_RADIUS) {
            currentState = EnemyState::PATROL;
            ResetPatrolBounds();
        }
        break;
    }
    }

    facingX = facingX + (targetFacingX - facingX) * ENEMY_TURN_SPEED;
    facingY = facingY + (targetFacingY - facingY) * ENEMY_TURN_SPEED;

    float length = sqrt(facingX * facingX + facingY * facingY);
    if (length > 0) {
        facingX /= length;
        facingY /= length;
    }
}

void Enemy::Draw() { return; }