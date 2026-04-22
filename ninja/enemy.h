// enemy.h
#pragma once
#include "entity.h"

// ==========================================
// 💥 核心：定义敌人的 AI 状态机 (FSM)
// ==========================================
enum class EnemyState {
    PATROL,     // 巡逻：在领地里左右溜达
    ALERT,      // 警觉：听到波纹声音，停下脚步，头上冒问号
    CHASE       // 追击：锁定玩家位置，红着眼冲过来！
};

class Enemy : public Entity {
protected:
    // 💥 注意这里改成了 protected！
    // 这样未来的派生类（近战步卒/远程弓手）才能直接访问和修改这些变量！
    bool isAlive;
    bool hasBeenScanned;

    // --- AI 大脑相关变量 ---
    EnemyState currentState; // 当前处于什么状态？
    int stateTimer;          // 状态倒计时（用于控制警觉发呆的时间）
    float speed;             // 移动速度

    float facingX; // 面朝方向的 X 向量
    float facingY; // 面朝方向的 Y 向量

    // --- 巡逻相关变量 ---
    float patrolStartX;      // 巡逻左边界
    float patrolEndX;        // 巡逻右边界
    int moveDirection;       // 1 代表向右走，-1 代表向左走
    int memoryTimer;    // 记忆倒计时
    int maxMemory;      // 最大记忆时长

public:
    Enemy(float startX, float startY);

    // 💥 加上 virtual！这告诉编译器：“我允许未来的子类重写我的思考方式！”
    virtual void UpdateAI(Vector2D playerPos, bool isPlayerMakingNoise);
    // 💥 新增：重置巡逻锚点
    void ResetPatrolBounds();
    void Draw() override;

    EnemyState GetCurrentState() const { return currentState; }

    // 原有的旧接口保留
    void Update(const ExMessage& msg) override {} // 留空，敌人不需要玩家的键盘输入
    bool GetIsAlive() const { return isAlive; }
    void SetIsAlive(bool alive) { isAlive = alive; }
    bool GetHasBeenScanned() const { return hasBeenScanned; }
    void SetHasBeenScanned(bool scanned) { hasBeenScanned = scanned; }
    float GetRadius() const { return 20.0f; }
    void TriggerMemory() { memoryTimer = 120; } // 3秒记忆
};