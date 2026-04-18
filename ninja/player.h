// player.h
#pragma once
// 1：引入头文件
#include "entity.h" 

// 2：继承 Entity
class Player : public Entity {
private:
    // 3：删除了 Vector2D position; 因为它现在在 protected 里

    float speed;
    float rotationAngle; // 朝向鼠标的角度

    // --- day01：蓄力系统相关变量 ---
    bool isCharging;      // 开关：是否正在蓄力
    float chargePower;    // 能量条：当前蓄力值
    float maxChargePower; // 能量上限：最大蓄力值限制

    // --- day02：刀光残影系统 ---
    bool isSlashing;      // 开关：现在屏幕上需不需要画刀光？
    int slashFrames;      // 计时器：刀光还能在屏幕上存活几帧？
    Vector2D slashStart;  // 记忆：一闪的起点
    Vector2D slashEnd;    // 记忆：一闪的终点

    // --- day02：呼吸光晕特效变量 ---
    float auraTime;       // 记录光晕的跳动时间

    // --- day02：新增接口，供<Game>查询是否刚触发了一闪 ---
    bool justSlashed;

    // --- day06：波纹探测变量 ---
    bool isRippleActive;  // 波纹是否正在扩散？
    Vector2D rippleCenter;// 波纹的圆心（踩地板时的位置）
    float rippleRadius;   // 波纹当前半径
    float rippleSpeed;    // 波纹扩散速度

public:

    Player(float startX, float startY);

    // 更新逻辑（传入鼠标消息）
    void Update(const ExMessage& msg)override;
    // 绘制自己
    void Draw();

	// --- day02：新增接口，供<Game>查询是否刚触发了一闪（用于震动系统） ---
    bool GetIsJustSlashed(); // 新增：告诉导演我拔刀了

	// --- day03：预留接口，供<Game>查询刀光的起点和终点 ---
    // 为了等下的碰撞检测做准备！
    Vector2D GetSlashStart() const { return slashStart; }
    Vector2D GetSlashEnd() const { return slashEnd; }

    // --- day06：给导演准备的接口 ---
    bool GetIsRippleActive() const { return isRippleActive; }
    Vector2D GetRippleCenter() const { return rippleCenter; }
    float GetRippleRadius() const { return rippleRadius; }
};