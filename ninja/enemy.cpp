// enemy.cpp
#include "enemy.h"
#include <graphics.h>
#include <math.h>

Enemy::Enemy(float startX, float startY) : Entity(startX, startY)
{
    isAlive = true;
    hasBeenScanned = false;

    // ==========================================
    // 💥 唤醒 AI 大脑：初始化属性
    // ==========================================
    currentState = EnemyState::PATROL; // 出生时默认在巡逻
    stateTimer = 0;
    speed = 1.5f;                      // 敌人的走路速度
    moveDirection = 1;                 // 默认先向右走

    // 设定巡逻范围（以出生点为中心，左右溜达 100 像素）
    patrolStartX = startX - 100.0f;
    patrolEndX = startX + 100.0f;

    // 初始化脖子朝向
    facingX = 1.0f; // 默认面朝右边
    facingY = 0.0f;
}

// 💥 这是敌人全新的思考中枢！
void Enemy::UpdateAI(Vector2D playerPos, bool isPlayerMakingNoise)
{
    if (!isAlive) return;

    // 1. 算距离和归一化向量
    float dx = playerPos.x - position.x;
    float dy = playerPos.y - position.y;
    float distanceToPlayer = sqrt(dx * dx + dy * dy);

    float dirToPlayerX = 0, dirToPlayerY = 0;
    if (distanceToPlayer > 0) {
        dirToPlayerX = dx / distanceToPlayer;
        dirToPlayerY = dy / distanceToPlayer;
    }

    // ==========================================
    // 💥 1. 思考阶段：我接下来【想】看哪边？
    // ==========================================
    float targetFacingX = facingX;
    float targetFacingY = facingY;

    switch (currentState) {
    case EnemyState::PATROL:
    {
        // 行走逻辑
        position.x += speed * moveDirection;
        if (position.x > patrolEndX) moveDirection = -1;
        if (position.x < patrolStartX) moveDirection = 1;

        // 巡逻时，心里想看的方向就是走路的方向
        targetFacingX = (float)moveDirection;
        targetFacingY = 0.0f;

        bool isHeard = (isPlayerMakingNoise && distanceToPlayer < 250.0f);
        bool isSeen = false;

        if (distanceToPlayer < 150.0f) { // 视力半径 150
            // 用点乘判断是否在视野扇形内
            float dotProduct = (facingX * dirToPlayerX) + (facingY * dirToPlayerY);
            if (dotProduct > 0.707f) {
                isSeen = true;
            }
        }

        if (isHeard || isSeen) {
            currentState = EnemyState::ALERT;
            stateTimer = 60;
        }
        break;
    }

    case EnemyState::ALERT:
    {
        stateTimer--;

        // 警觉时，心里想看的方向是【玩家发声的位置】
        targetFacingX = dirToPlayerX;
        targetFacingY = dirToPlayerY;

        if (stateTimer <= 0) {
            float dotProduct = (facingX * dirToPlayerX) + (facingY * dirToPlayerY);
            if (distanceToPlayer < 300.0f && dotProduct > 0.5f) {
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
            position.x += dirToPlayerX * (speed * 1.5f);
            position.y += dirToPlayerY * (speed * 1.5f);

            // 追逐时，眼睛死死盯住玩家
            targetFacingX = dirToPlayerX;
            targetFacingY = dirToPlayerY;
        }

        if (distanceToPlayer > 400.0f) {
            currentState = EnemyState::PATROL;
            patrolStartX = position.x - 100.0f;
            patrolEndX = position.x + 100.0f;
        }
        break;
    }
    }

    // ==========================================
    // 💥 2. 肌肉执行阶段：平滑转动脖子！(Lerp 魔法)
    // ==========================================
    facingX = facingX + (targetFacingX - facingX) * 0.1f;
    facingY = facingY + (targetFacingY - facingY) * 0.1f;

    // 归一化：保证脖子不会越拉越长
    float length = sqrt(facingX * facingX + facingY * facingY);
    if (length > 0) {
        facingX /= length;
        facingY /= length;
    }
}

void Enemy::ResetPatrolBounds() {
    // 强制把当前的真实物理坐标，当做新的巡逻中心点！
    patrolStartX = position.x - 100.0f;
    patrolEndX = position.x + 100.0f;
}

void Enemy::Draw()
{
    // ==========================================
    // 💥 极致硬核：敌人本体是绝对隐形的！
    // 所有的视觉反馈，全部交给波纹扫过时留下的“静态残影 (Echo)”！
    // ==========================================

    // 什么都不画，真盲人！直接 return！
    return;
}