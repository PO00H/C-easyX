/******************************************************************************
 *
 * [引擎核心图纸] ENEMY.H
 *
 * @desc : 敌军基类设计。已接入 Map 全局寻路支持。
 * Day 13 纯净重构：引入 PANIC_LISTEN 状态，暴露动态视野与朝向接口供外部渲染。
 *
 ******************************************************************************/
#pragma once
#include "entity.h"

 // 💥 前向声明：告诉编译器存在 Map 类，避免头文件循环包含
class Map;

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

// 💥 Day13：新增 PANIC_LISTEN 恐慌倾听状态
enum class EnemyState { PATROL, ALERT, CHASE, PANIC_LISTEN };

class Enemy : public Entity {
protected:
    bool isAlive;
    bool hasBeenScanned;

    EnemyState currentState;
    int stateTimer;
    float speed;
    float facingX;
    float facingY;

    // 💥 Day13：动态视力值（灯灭时归零）
    float currentVisionRadius;

    float patrolStartX;
    float patrolEndX;
    int moveDirection;

public:
    Enemy(float startX, float startY);

    void Reset(Vector2D startPos);

    // 注入 GPS 与地图光照系统
    virtual void UpdateAI(Vector2D playerPos, bool isPlayerMakingNoise, Map& levelMap);

    void Draw() override; // 保持你原有的空实现
    void ResetPatrolBounds();
    void Update(const ExMessage& msg) override {}

    // --- 状态接口 ---
    EnemyState GetCurrentState() const { return currentState; }
    bool GetIsAlive() const { return isAlive; }
    void SetIsAlive(bool alive) { isAlive = alive; }
    bool GetHasBeenScanned() const { return hasBeenScanned; }
    void SetHasBeenScanned(bool scanned) { hasBeenScanned = scanned; }
    float GetRadius() const { return ENEMY_COLLISION_RAD; }

    // --- 💥 Day13 暴露给外部渲染器的数据接口 ---
    float GetFacingX() const { return facingX; }
    float GetFacingY() const { return facingY; }
    float GetCurrentVisionRadius() const { return currentVisionRadius; }
};