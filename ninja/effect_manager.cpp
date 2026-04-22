// effect_manager.cpp
#include "effect_manager.h"
#include <stdlib.h> // 需要用到 rand()
#include <graphics.h>

// 1. 构造函数
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
        shakeIntensity *= 0.9f;
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

    // ==================================================
    // 💥 修复点：找回丢失的粒子运动法则！ 💥
    // ==================================================
    for (auto it = particles.begin(); it != particles.end(); )
    {
        // 让粒子根据速度飞出去！
        it->pos.x += it->velocity.x;
        it->pos.y += it->velocity.y;

        // 空气阻力摩擦
        it->velocity.x *= 0.85f;
        it->velocity.y *= 0.85f;

        it->life--;

        if (it->life <= 0)
        {
            it = particles.erase(it);
        }
        else {
            ++it;
        }
    }

    // --- day07：更新记忆残影生命周期 ---
    for (auto it = echoes.begin(); it != echoes.end(); ) {
        it->life--;
        if (it->life <= 0) {
            it = echoes.erase(it);
        }
        else {
            ++it;
        }
    }

	// --- day09：更新波纹生命周期和扩散状态 ---
    UpdateRipples();
}

// ==================================================
// 💥 视觉大升级：渲染狂暴金色速度线 💥
// ==================================================
void EffectManager::DrawParticles()
{
    for (const auto& p : particles)
    {
        float sizeRatio = (float)p.life / p.maxLife;

        // 绝美色彩渐变：刺眼黄 -> 冷却红
        int r = 255;
        int g = (int)(200 * sizeRatio);
        int b = (int)(50 * sizeRatio);
        setlinecolor(RGB(r, g, b));

        // 线条粗细渐变：由粗变细
        int thickness = (int)(4.0f * sizeRatio) + 1;
        setlinestyle(PS_SOLID, thickness);

        // 速度拖尾算法：速度越快，这根线拉得越长
        float tailX = p.pos.x - p.velocity.x * 2.0f;
        float tailY = p.pos.y - p.velocity.y * 2.0f;

        // 画线！
        line((int)p.pos.x, (int)p.pos.y, (int)tailX, (int)tailY);
    }

    // 画完后把线条粗细恢复正常
    setlinestyle(PS_SOLID, 1);
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
    hitStopFrames = 6;

    // ==================================================
    // 💥 视觉大升级：生成 50 个高爆火花 💥
    // ==================================================
    for (int i = 0; i < 50; i++)
    {
        Particle p;
        p.pos = hitPos;

        // 爆炸初速度翻倍！
        p.velocity.x = (rand() % 600 - 300) / 10.0f;
        p.velocity.y = (rand() % 600 - 300) / 10.0f;

        // 寿命变长
        p.life = 15 + rand() % 15;
        p.maxLife = p.life;

        particles.push_back(p);
    }
}

// --- day07：生成新的记忆残影 ---
void EffectManager::AddMemoryEcho(Vector2D pos, float radius) {
    MemoryEcho echo;
    echo.pos = pos;
    echo.radius = radius;
    echo.life = 60;
    echo.maxLife = 60;
    echoes.push_back(echo);
}

// --- day07：渲染记忆残影 ---
void EffectManager::DrawEchoes() {
    for (const auto& e : echoes) {
        float ratio = (float)e.life / e.maxLife;

        int r = 0;
        int g = (int)(255 * ratio);
        int b = (int)(255 * ratio);

        setfillcolor(RGB(r, g, b));
        setlinecolor(RGB(r, g, b));

        fillcircle((int)e.pos.x, (int)e.pos.y, (int)e.radius);
    }

	// --- day09：在渲染残影的基础上，叠加渲染波纹！ ---
    DrawRipples();
}

// --- day07：清空所有残影 ---
void EffectManager::ClearEchoes() {
    echoes.clear();
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

// --- day09：生成波纹的接口 ---
void EffectManager::AddDynamicRipple(Vector2D pos, float maxRadius, COLORREF color) {
    // 寿命设置为 45 帧（0.75秒），初始半径为 5
    dynamicRipples.push_back({ pos.x, pos.y, 5.0f, maxRadius, 45, 45, color });
}

// --- day09：渲染波纹 ---
void EffectManager::UpdateRipples() {
    for (auto it = dynamicRipples.begin(); it != dynamicRipples.end(); ) {

        // 💥 核心数学：Ease-out 缓动扩散！
        // 距离目标半径越近，扩得越慢，产生非常平滑的声波阻力感
        it->currentRadius += (it->maxRadius - it->currentRadius) * 0.15f;

        it->timer--; // 寿命减少

        if (it->timer <= 0) {
            it = dynamicRipples.erase(it); // 死掉就从内存中删掉
        }
        else {
            ++it;
        }
    }
}

// --- day09：渲染波纹 ---
void EffectManager::DrawRipples() {
    for (auto& r : dynamicRipples) {

        // 算一下剩余寿命的百分比 (1.0 -> 0.0)
        float alpha = (float)r.timer / r.maxTimer;

        // 让原本的颜色根据 alpha 变暗（模拟渐渐消失）
        int red = (int)(GetRValue(r.color) * alpha);
        int green = (int)(GetGValue(r.color) * alpha);
        int blue = (int)(GetBValue(r.color) * alpha);

        // 💥 关键：画空心圆！
        setlinecolor(RGB(red, green, blue));
        setlinestyle(PS_SOLID, 2); // 线条加粗一点，2 像素宽

        // 用 circle 画空心圆，绝对不用 solidcircle！
        circle((int)r.x, (int)r.y, (int)r.currentRadius);
    }
}
