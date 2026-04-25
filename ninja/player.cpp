/******************************************************************************
 *
 * [引擎核心逻辑] PLAYER.CPP
 *
 * @desc : 盲剑客控制器的具体实现。所有硬编码参数均已提炼至 player.h 的常量区。
 *
 ******************************************************************************/
#include "player.h"
#include <math.h>
#include <iostream>

 // ==========================================
 // ▶ 1. 生命周期与状态机
 // ==========================================
Player::Player(float startX, float startY) : Entity(startX, startY)
{
    speed = PLAYER_BASE_SPEED;
    rotationAngle = 0.0f;

    // 蓄力状态
    isCharging = false;
    chargePower = 0.0f;

    // 刀光状态
    isSlashing = false;
    slashFrames = 0;
    auraTime = 0.0f;
    justSlashed = false;

    // 波纹状态
    isRippleActive = false;
    rippleRadius = 0.0f;

    // 眩晕状态
    stunTimer = 0;
}

void Player::Update(const ExMessage& msg)
{
    // ==========================================
    // 第一步：视觉动画层衰减 (不受硬直影响)
    // ==========================================
    if (isSlashing) {
        slashFrames--;
        if (slashFrames <= 0) {
            isSlashing = false;
        }
    }

    if (isRippleActive) {
        rippleRadius += RIPPLE_SPREAD_SPEED;
        if (rippleRadius > RIPPLE_MAX_RADIUS) {
            isRippleActive = false;
        }
    }

    // ==========================================
    // 第二步：物理层拦截 (眩晕罚站)
    // ==========================================
    if (stunTimer > 0) {
        stunTimer--;
        return;
    }

    // ==========================================
    // 第三步：玩家控制层 (正常状态)
    // ==========================================

    // --- 1. 左键蓄力与一闪攻击 ---
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        isCharging = true;

        if (chargePower < PLAYER_MAX_CHARGE)
        {
            chargePower += PLAYER_CHARGE_RATE;
        }
        auraTime += 0.2f + (chargePower / PLAYER_MAX_CHARGE) * 0.8f;
    }
    else
    {
        if (isCharging == true)
        {
            // 检测到松手，执行一闪
            float dashDistance = chargePower * 1.0f;

            slashStart = position;

            position.x += cos(rotationAngle) * dashDistance;
            position.y += sin(rotationAngle) * dashDistance;

            slashEnd = position;

            isSlashing = true;
            slashFrames = SLASH_EFFECT_FRAMES;
            justSlashed = true; // 发出震屏引线

            // 状态清零
            chargePower = 0.0f;
            auraTime = 0.0f;
        }
        isCharging = false;
    }

    // --- 2. 右键触发波纹探测 ---
    if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) && !isRippleActive) {
        isRippleActive = true;
        rippleRadius = 0.0f;
        rippleCenter = position;
    }

    // --- 3. 处理移动与转动 ---
    if (!isCharging)
    {
        if (GetAsyncKeyState('W') & 0x8000) position.y -= speed;
        if (GetAsyncKeyState('S') & 0x8000) position.y += speed;
        if (GetAsyncKeyState('A') & 0x8000) position.x -= speed;
        if (GetAsyncKeyState('D') & 0x8000) position.x += speed;

        if (msg.message == WM_MOUSEMOVE)
        {
            float dx = msg.x - position.x;
            float dy = msg.y - position.y;
            rotationAngle = atan2(dy, dx);
        }
    }
}

// ==========================================
// ▶ 2. 视觉渲染表现
// ==========================================
void Player::Draw()
{
    // 1. 绘制眩晕警告色
    if (stunTimer > 0) {
        setfillcolor(RGB(200, 50, 50));
    }
    else {
        setfillcolor(WHITE);
    }

    // 2. 绘制蓄力 UI
    if (isCharging)
    {
        float previewDistance = chargePower * 1.0f;
        int targetX = (int)(position.x + cos(rotationAngle) * previewDistance);
        int targetY = (int)(position.y + sin(rotationAngle) * previewDistance);

        // 轨迹虚线
        setlinecolor(YELLOW);
        setlinestyle(PS_DASH, 1);
        line((int)position.x, (int)position.y, targetX, targetY);

        // 落点 X 标记
        setlinestyle(PS_SOLID, 2);
        line(targetX - 5, targetY - 5, targetX + 5, targetY + 5);
        line(targetX - 5, targetY + 5, targetX + 5, targetY - 5);

        // 狂暴呼吸光晕
        setlinecolor(RED);
        float wobbleAmplitude = 5.0f + (chargePower * 0.02f);
        int currentRadius = (int)(25.0f + sin(auraTime) * wobbleAmplitude);
        int lineWidth = (int)(1 + (chargePower / 100.0f));

        setlinestyle(PS_SOLID, lineWidth);
        circle((int)position.x, (int)position.y, currentRadius);
    }

    // 3. 绘制本体与朝向线
    solidcircle((int)position.x, (int)position.y, 15);

    int lineEndX = (int)(position.x + cos(rotationAngle) * 30.0f);
    int lineEndY = (int)(position.y + sin(rotationAngle) * 30.0f);
    setlinecolor(RED);
    setlinestyle(PS_SOLID, 3);
    line((int)position.x, (int)position.y, lineEndX, lineEndY);

    // 4. 绘制一闪刀光残影
    if (isSlashing) {
        setlinecolor(WHITE);
        setlinestyle(PS_SOLID, slashFrames);
        line((int)slashStart.x, (int)slashStart.y, (int)slashEnd.x, (int)slashEnd.y);
    }

    // 5. 绘制声呐波纹
    if (isRippleActive) {
        setlinecolor(WHITE);
        setlinestyle(PS_SOLID, 2);
        circle((int)rippleCenter.x, (int)rippleCenter.y, (int)rippleRadius);
    }
}

// ==========================================
// ▶ 3. 状态干预接口
// ==========================================
bool Player::GetIsJustSlashed()
{
    if (justSlashed) {
        justSlashed = false;
        return true;
    }
    return false;
}

void Player::Stun() {
    if (stunTimer > 0) return;

    stunTimer = STUN_DURATION_FRAMES;

    // 强行打断蓄力状态
    isCharging = false;
    chargePower = 0.0f;
    justSlashed = false;
}