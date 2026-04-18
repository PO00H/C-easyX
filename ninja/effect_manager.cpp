// effect_manager.cpp
#include "effect_manager.h"
#include <stdlib.h> // 需要用到 rand()
#include <graphics.h>

// 1. 构造函数：
EffectManager::EffectManager()
{
    shakeIntensity = 0.0f;
    shakeTimer = 0;
    hitStopFrames = 0;
}

// 2. 每帧让特效自然衰减
void EffectManager::Update()
{
    // 处理屏幕震动衰减
    if (shakeTimer > 0) 
    {
        shakeTimer--;
        shakeIntensity *= 0.9f; // 经典的指数衰减
    }
    else 
    {
        shakeIntensity = 0.0f;
    }

    // 处理顿帧衰减
    if (hitStopFrames > 0) 
    {
        hitStopFrames--;
    }

    // --- day04：更新粒子物理运动状态 ---
    for (auto it = particles.begin(); it != particles.end(); ) 
    {
        // 更新坐标
        it->pos.x += it->velocity.x;
        it->pos.y += it->velocity.y;

        // 应用空气阻力（摩擦力阻尼系数 0.85）
        it->velocity.x *= 0.85f;
        it->velocity.y *= 0.85f;

        // 寿命递减
        it->life--;

        // 判定粒子存活状态，回收失效粒子内存
        if (it->life <= 0) 
        {
            it = particles.erase(it);
        }
        else {
            ++it;
        }
    }
}


// --- day04：遍历并渲染所有存活粒子 ---
void EffectManager::DrawParticles() 
{
    setfillcolor(WHITE); // 设置粒子渲染颜色为白色

    for (const auto& p : particles) 
    {
        // 根据生命周期比例计算当前渲染半径
		float sizeRatio = (float)p.life / p.maxLife; // life随着时间减小，sizeRatio从1慢慢变小
        int radius = (int)(4.0f * sizeRatio); // 初始最大半径设为 4 像素

        if (radius > 0) 
        {
            solidcircle((int)p.pos.x, (int)p.pos.y, radius);
        }
    }
}
// 3. 对外接口：挥空特效
void EffectManager::PlayMissEffect()
{
    shakeIntensity = 8.0f;
    shakeTimer = 5;
}

// 4. 对外接口：斩杀特效
void EffectManager::PlayHitEffect(Vector2D hitPos)
{
    shakeIntensity = 15.0f;
    shakeTimer = 15;
    // 💥 顿帧设定：60帧/秒的游戏，卡住 6 帧正好是 0.1 秒
    hitStopFrames = 6;

    // --- day04：实现斩杀特效，触发屏幕震动、顿帧并生成爆炸粒子集 ---
    // 批量生成 30 个随机方向的溅射粒子
    for (int i = 0; i < 30; i++) 
    {
        Particle p;
        p.pos = hitPos;

        // 生成 [-15.0, 15.0] 区间的随机<初始速度>
        p.velocity.x = (rand() % 300 - 150) / 10.0f;
        p.velocity.y = (rand() % 300 - 150) / 10.0f;

        // 生成 [10, 19] 区间的随机<存活帧数>
        p.life = 10 + rand() % 10;
        p.maxLife = p.life;

        particles.push_back(p);
    }
}

// 5. 检查专用接口
float EffectManager::GetShakeOffsetX() const
{
    if (shakeTimer <= 0) return 0.0f;
    return (rand() % 200 - 100) / 100.0f * shakeIntensity;
}

float EffectManager::GetShakeOffsetY() const
{
    if (shakeTimer <= 0) return 0.0f;
    return (rand() % 200 - 100) / 100.0f * shakeIntensity;
}

bool EffectManager::IsHitStopping() const
{
    return hitStopFrames > 0;
}