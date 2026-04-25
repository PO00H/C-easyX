/******************************************************************************
 *
 * [引擎核心图纸] EFFECT_MANAGER.H
 *
 * @desc : 游戏的视觉灵魂与反馈中枢。掌管屏幕震动、时间顿帧、
 * 高爆粒子物理、记忆残影叠加以及霓虹波纹扩散渲染。
 *
 *=============================================================================
 * [ 函数导航 (Function Map) ]
 *
 * ▶ 1. 生命周期 (Lifecycle)
 * ├─ EffectManager()       : 初始化特效管家状态
 * └─ Update()              : 统筹所有粒子、波纹、残影与震动的时间衰减与物理运动
 *
 * ▶ 2. 打击反馈系统 (Impact Feedback)
 * ├─ PlayMissEffect()      : 挥空反馈（轻微震动）
 * └─ PlayHitEffect()       : 斩杀反馈（剧烈震动 + 顿帧 + 爆炸火花）
 *
 * ▶ 3. 视觉渲染管线 (Rendering Pipeline)
 * ├─ DrawParticles()       : 渲染极速衰减的火花拖尾线
 * ├─ DrawEchoes()          : 渲染实心记忆残影，并叠加调用波纹渲染
 * └─ DrawRipples()         : 内部调用：执行霓虹发光(Neon Glow)叠加渲染
 *
 * ▶ 4. 数据调度接口 (Data IO)
 * ├─ AddMemoryEcho()       : 注入新的记忆残影
 * ├─ AddDynamicRipple()    : 注入新的环境动态波纹
 * ├─ ClearEchoes()         : 强制清空残影(用于战败/斩杀瞬间)
 * └─ GetShakeOffset...()   : 输出震动偏移量，供导演(Game)晃动摄像机
 *
 ******************************************************************************/
#pragma once
#include "common.h"
#include <vector>
#include <graphics.h>

 // ==========================================
 // [ 特效组件结构体 ] 
 // ==========================================
struct Particle {
    Vector2D pos;      // 当前绝对坐标
    Vector2D velocity; // 飞行向量速度
    int life;          // 剩余寿命
    int maxLife;       // 初始最大寿命
};

struct MemoryEcho {
    Vector2D pos;
    float radius;
    int life;
    int maxLife;
};

struct RippleEffect {
    float x, y;
    float currentRadius;
    float maxRadius;
    int timer;
    int maxTimer;
    COLORREF color;
};

// ==========================================
// [ 管家类定义 ] 
// ==========================================
class EffectManager {
private:
    // --- 震动与顿帧系统 ---
    float shakeIntensity;
    int shakeTimer;
    int hitStopFrames;

    // --- 视觉实体容器 ---
    std::vector<Particle> particles;
    std::vector<MemoryEcho> echoes;
    std::vector<RippleEffect> dynamicRipples;

    // 内部私有方法
    void UpdateRipples();
    void DrawRipples();

public:
    EffectManager();

    void Update();

    // --- 接口：打击系统 ---
    void PlayMissEffect();
    void PlayHitEffect(Vector2D hitPos);
    bool IsHitStopping() const;
    float GetShakeOffsetX() const;
    float GetShakeOffsetY() const;

    // --- 接口：记忆与雷达系统 ---
    void AddMemoryEcho(Vector2D pos, float radius);
    void AddDynamicRipple(Vector2D pos, float maxRadius, COLORREF color);
    void ClearEchoes();

    // --- 接口：渲染管线 ---
    void DrawParticles();
    void DrawEchoes();
};