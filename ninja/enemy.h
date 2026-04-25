/******************************************************************************
 *
 * [引擎核心图纸] ENEMY.H
 *
 * @desc : 敌军基类设计。已接入 Map 全局寻路支持，彻底解决死角卡墙问题。
 *
 ******************************************************************************/
#pragma once
#include "entity.h"

 // 💥 前向声明：告诉编译器存在 Map 类，避免头文件循环包含
class Map;

// (省略常量定义和 Enum，保持你原有的配置)
const float ENEMY_BASE_SPEED = 1.5f;
const float ENEMY_CHASE_MULT = 1.5f;
const float ENEMY_PATROL_RADIUS = 100.0f;
const float ENEMY_HEAR_RADIUS = 250.0f;
const float ENEMY_SIGHT_RADIUS = 150.0f;
const float ENEMY_SIGHT_ANGLE = 0.707f;
const int   ENEMY_ALERT_FRAMES = 60;
const float ENEMY_LOSE_RADIUS = 400.0f;
const float ENEMY_COLLISION_RAD = 20.0f;
const float ENEMY_TURN_SPEED = 0.1f;

enum class EnemyState { PATROL, ALERT, CHASE };

class Enemy : public Entity {
protected:
    bool isAlive;
    bool hasBeenScanned;

    EnemyState currentState;
    int stateTimer;
    float speed;
    float facingX;
    float facingY;
    float patrolStartX;
    float patrolEndX;
    int moveDirection;

public:
    Enemy(float startX, float startY);

    // 💥 重置大脑接口
    void Reset(Vector2D startPos);

    // 💥 注入 GPS：把 Map 的引用传进来
    virtual void UpdateAI(Vector2D playerPos, bool isPlayerMakingNoise, Map& levelMap);

    void Draw() override;
    void ResetPatrolBounds();
    void Update(const ExMessage& msg) override {}

    EnemyState GetCurrentState() const { return currentState; }
    bool GetIsAlive() const { return isAlive; }
    void SetIsAlive(bool alive) { isAlive = alive; }
    bool GetHasBeenScanned() const { return hasBeenScanned; }
    void SetHasBeenScanned(bool scanned) { hasBeenScanned = scanned; }
    float GetRadius() const { return ENEMY_COLLISION_RAD; }
};