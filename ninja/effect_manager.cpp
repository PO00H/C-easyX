/******************************************************************************
 *
 * [引擎核心逻辑] EFFECT_MANAGER.CPP
 *
 * @desc : 游戏所有视觉特效的具体实现。死代码(Dead Code)已全部剔除，
 * 所有硬编码替换为 common.h 中的 FX_ 宏以支持快速手感调优。
 *
 ******************************************************************************/
#include "effect_manager.h"
#include <stdlib.h> 
#include <graphics.h>

 // ==========================================
 // ▶ 1. 生命周期与衰减控制
 // ==========================================
EffectManager::EffectManager() {
    shakeIntensity = 0.0f;
    shakeTimer = 0;
    hitStopFrames = 0;
}

void EffectManager::Update() {
    // 1. 屏幕震动衰减
    if (shakeTimer > 0) {
        shakeTimer--;
        shakeIntensity *= 0.9f;
    }
    else {
        shakeIntensity = 0.0f;
    }

    // 2. 顿帧衰减
    if (hitStopFrames > 0) {
        hitStopFrames--;
    }

    // 3. 火花粒子物理运算 (位移与空气阻力)
    for (auto it = particles.begin(); it != particles.end(); ) {
        it->pos.x += it->velocity.x;
        it->pos.y += it->velocity.y;

        it->velocity.x *= FX_SPARK_FRICTION;
        it->velocity.y *= FX_SPARK_FRICTION;

        it->life--;

        if (it->life <= 0) {
            it = particles.erase(it);
        }
        else {
            ++it;
        }
    }

    // 4. 记忆残影衰减
    for (auto it = echoes.begin(); it != echoes.end(); ) {
        it->life--;
        if (it->life <= 0) {
            it = echoes.erase(it);
        }
        else {
            ++it;
        }
    }

    // 5. 波纹物理扩散
    UpdateRipples();
}

void EffectManager::UpdateRipples() {
    for (auto it = dynamicRipples.begin(); it != dynamicRipples.end(); ) {
        // 核心数学：Ease-out 缓动扩散算法，产生声波粘稠感
        it->currentRadius += (it->maxRadius - it->currentRadius) * FX_RIPPLE_DAMPING;

        it->timer--;

        if (it->timer <= 0) {
            it = dynamicRipples.erase(it);
        }
        else {
            ++it;
        }
    }
}

// ==========================================
// ▶ 2. 打击反馈系统接口
// ==========================================
void EffectManager::PlayMissEffect() {
    shakeIntensity = FX_MISS_SHAKE_POWER;
    shakeTimer = FX_MISS_SHAKE_TIME;
}

void EffectManager::PlayHitEffect(Vector2D hitPos) {
    shakeIntensity = FX_HIT_SHAKE_POWER;
    shakeTimer = FX_HIT_SHAKE_TIME;
    hitStopFrames = FX_HIT_STOP_FRAMES;

    // 生成高爆火花
    for (int i = 0; i < FX_SPARK_COUNT; i++) {
        Particle p;
        p.pos = hitPos;

        p.velocity.x = (rand() % 600 - 300) / 10.0f;
        p.velocity.y = (rand() % 600 - 300) / 10.0f;

        p.life = 15 + rand() % 15;
        p.maxLife = p.life;

        particles.push_back(p);
    }
}

float EffectManager::GetShakeOffsetX() const {
    if (shakeTimer <= 0) return 0.0f;
    return (rand() % 200 - 100) / 100.0f * shakeIntensity;
}

float EffectManager::GetShakeOffsetY() const {
    if (shakeTimer <= 0) return 0.0f;
    return (rand() % 200 - 100) / 100.0f * shakeIntensity;
}

bool EffectManager::IsHitStopping() const {
    return hitStopFrames > 0;
}

// ==========================================
// ▶ 3. 记忆与雷达系统接口
// ==========================================
void EffectManager::AddMemoryEcho(Vector2D pos, float radius) {
    MemoryEcho echo;
    echo.pos = pos;
    echo.radius = radius;
    echo.life = FX_ECHO_LIFE;
    echo.maxLife = FX_ECHO_LIFE;
    echoes.push_back(echo);
}

void EffectManager::AddDynamicRipple(Vector2D pos, float maxRadius, COLORREF color) {
    dynamicRipples.push_back({ pos.x, pos.y, FX_RIPPLE_START_RAD, maxRadius, FX_RIPPLE_LIFE, FX_RIPPLE_LIFE, color });
}

void EffectManager::ClearEchoes() {
    echoes.clear();
}

// ==========================================
// ▶ 4. 渲染管线
// ==========================================
void EffectManager::DrawParticles() {
    for (const auto& p : particles) {
        float sizeRatio = (float)p.life / p.maxLife;

        int r = 255;
        int g = (int)(200 * sizeRatio);
        int b = (int)(50 * sizeRatio);
        setlinecolor(RGB(r, g, b));

        int thickness = (int)(4.0f * sizeRatio) + 1;
        setlinestyle(PS_SOLID, thickness);

        // 速度拖尾拉伸算法
        float tailX = p.pos.x - p.velocity.x * 2.0f;
        float tailY = p.pos.y - p.velocity.y * 2.0f;

        line((int)p.pos.x, (int)p.pos.y, (int)tailX, (int)tailY);
    }
    setlinestyle(PS_SOLID, 1);
}

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

    DrawRipples();
}

void EffectManager::DrawRipples() {
    for (auto& r : dynamicRipples) {
        float alpha = (float)r.timer / r.maxTimer;

        int baseR = GetRValue(r.color);
        int baseG = GetGValue(r.color);
        int baseB = GetBValue(r.color);

        // 霓虹发光 (Neon Glow) 多层渲染
        int layers = 3;
        for (int i = 0; i < layers; i++) {
            float layerAlpha = alpha * (1.0f - (i * 0.3f));
            if (layerAlpha <= 0.0f) continue;

            int red = (int)(baseR * layerAlpha);
            int green = (int)(baseG * layerAlpha);
            int blue = (int)(baseB * layerAlpha);

            setlinecolor(RGB(red, green, blue));

            int thickness = (i == 0) ? 1 : 2;
            setlinestyle(PS_SOLID, thickness);

            int drawRadius = (int)(r.currentRadius - i * 2.0f);
            if (drawRadius > 0) {
                circle((int)r.x, (int)r.y, drawRadius);
            }
        }
    }
}