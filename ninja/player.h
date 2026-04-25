/******************************************************************************
 *
 * [引擎核心图纸] PLAYER.H
 *
 * @desc : 盲剑客实体类。继承自基础 Entity，掌管玩家的核心输入响应、
 * 蓄力一闪状态机、主动声呐波纹，以及撞墙眩晕的惩罚逻辑。
 *
 *=============================================================================
 * [ 函数导航 (Function Map) ]
 *
 * ▶ 1. 生命周期与状态机 (Lifecycle & FSM)
 * ├─ Player()              : 初始化主角各项体征数值
 * ├─ Update()              : 处理衰减、眩晕拦截与键鼠输入
 * └─ Draw()                : 绘制主角、预测轨迹线与心跳光晕
 *
 * ▶ 2. 导演查询接口 (Director I/O)
 * ├─ GetIsJustSlashed()    : (阅后即焚) 查询是否刚刚触发了一闪
 * ├─ GetSlashStart/End()   : 查询刀光的起点与终点，用于碰撞检测
 * └─ GetIsRippleActive()   : 查询声呐波纹状态
 *
 * ▶ 3. 状态干预接口 (State Modifiers)
 * ├─ SetSlashEnd()         : 允许外部因撞墙等因素强行截短刀光
 * ├─ Stun()                : 触发眩晕惩罚并打断所有蓄力状态
 * └─ ResetStun()           : 强制解除眩晕 (用于战败轮回重置)
 *
 ******************************************************************************/
#pragma once
#include "entity.h"

 // ==========================================
 // [ 主角专属常量配置 ] 
 // ==========================================
const float PLAYER_BASE_SPEED = 4.0f;
const float PLAYER_MAX_CHARGE = 400.0f;
const float PLAYER_CHARGE_RATE = 6.0f;
const float RIPPLE_MAX_RADIUS = 200.0f;
const float RIPPLE_SPREAD_SPEED = 15.0f;
const int   STUN_DURATION_FRAMES = 40;
const int   SLASH_EFFECT_FRAMES = 8;
const float PLAYER_COLLISION_RAD = 20.0f;

class Player : public Entity {
private:
    float speed;
    float rotationAngle;      // 朝向鼠标的角度

    // --- 蓄力系统 ---
    bool isCharging;          // 是否正在蓄力
    float chargePower;        // 当前蓄力值

    // --- 刀光残影系统 ---
    bool isSlashing;          // 屏幕上需不需要画刀光
    int slashFrames;          // 刀光存活倒计时
    Vector2D slashStart;      // 一闪的起点
    Vector2D slashEnd;        // 一闪的终点
    bool justSlashed;         // 一次性信号：刚触发了一闪
    float auraTime;           // 呼吸光晕的跳动时间

    // --- 声呐探测系统 ---
    bool isRippleActive;      // 波纹是否正在扩散
    Vector2D rippleCenter;    // 波纹发射的圆心
    float rippleRadius;       // 波纹当前半径

    // --- 眩晕惩罚系统 ---
    int stunTimer;            // 眩晕倒计时

public:
    Player(float startX, float startY);

    void Update(const ExMessage& msg) override;
    void Draw();

    // --- 导演查询接口 ---
    bool GetIsJustSlashed();
    Vector2D GetSlashStart() const { return slashStart; }
    Vector2D GetSlashEnd() const { return slashEnd; }

    bool GetIsRippleActive() const { return isRippleActive; }
    Vector2D GetRippleCenter() const { return rippleCenter; }
    float GetRippleRadius() const { return rippleRadius; }

    float GetRadius() const { return PLAYER_COLLISION_RAD; }
    bool IsStunned() const { return stunTimer > 0; }

    // --- 状态干预接口 ---
    void SetSlashEnd(Vector2D newEnd) { slashEnd = newEnd; }
    void Stun();
    void ResetStun() { stunTimer = 0; }
};