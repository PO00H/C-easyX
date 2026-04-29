/******************************************************************************
 *
 * [引擎核心逻辑] PLAYER.CPP
 *
 * @desc : 盲剑客控制器的具体实现。
 * Day 13 修复：引入 CCD (连续碰撞检测) 算法，彻底解决一闪高速穿模问题。
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

    isCharging = false;
    chargePower = 0.0f;

    isSlashing = false;
    slashFrames = 0;
    auraTime = 0.0f;
    justSlashed = false;

    isRippleActive = false;
    rippleRadius = 0.0f;

    stunTimer = 0;
}

// 💥 核心调度：结合 Map 物理系统的 Update
void Player::Update(const ExMessage& msg, Map& levelMap)
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
        if (chargePower < PLAYER_MAX_CHARGE) {
            chargePower += PLAYER_CHARGE_RATE;
        }
        auraTime += 0.2f + (chargePower / PLAYER_MAX_CHARGE) * 0.8f;
    }
    else
    {
        if (isCharging == true)
        {
            // 检测到松手，发起一闪突进
            float dashDistance = chargePower * 1.0f;
            slashStart = position;

            // ---------------------------------------------------------
            // 💥 修复核心：CCD (Continuous Collision Detection) 连续碰撞算法
            // ---------------------------------------------------------
            float moveDx = cos(rotationAngle) * dashDistance;
            float moveDy = sin(rotationAngle) * dashDistance;

            // 将高速移动切片，每步最多走 5 个像素，绝不漏掉任何一堵墙
            int steps = (int)(dashDistance / 5.0f) + 1;
            float stepX = moveDx / steps;
            float stepY = moveDy / steps;

            bool hitWall = false;
            for (int i = 0; i < steps; i++) {
                position.x += stepX;
                position.y += stepY;

                // 传入 true：破甲模式（无视竹林，但会被铁墙挡住）
                position = levelMap.ResolveCollision(position, PLAYER_COLLISION_RAD, hitWall, true);

                if (hitWall) {
                    break; // 只要撞到不可破坏的墙，立刻刹车！
                }
            }

            slashEnd = position;

            // ---------------------------------------------------------
            // 💥 Day13 环境破坏：将突进轨迹上的竹林直接斩碎为平地
            levelMap.SlashBamboo(slashStart, slashEnd);

            // 启动刀光与震屏引线
            isSlashing = true;
            slashFrames = SLASH_EFFECT_FRAMES;
            justSlashed = true;

            // 如果最后 hitWall 是 true，触发眩晕惩罚！
            if (hitWall) {
                Stun();
            }

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

    // --- 3. 处理常规移动与转动 ---
    if (!isCharging)
    {
        if (GetAsyncKeyState('W') & 0x8000) position.y -= speed;
        if (GetAsyncKeyState('S') & 0x8000) position.y += speed;
        if (GetAsyncKeyState('A') & 0x8000) position.x -= speed;
        if (GetAsyncKeyState('D') & 0x8000) position.x += speed;

        // 常规移动步子很小，无需 CCD，直接单次检测即可
        // 传入 false，代表没有在突进，会被竹林无情挡住
        bool hitWall = false;
        position = levelMap.ResolveCollision(position, PLAYER_COLLISION_RAD, hitWall, false);

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
    if (stunTimer > 0) {
        setfillcolor(RGB(200, 50, 50));
    }
    else {
        setfillcolor(WHITE);
    }

    if (isCharging)
    {
        float previewDistance = chargePower * 1.0f;
        int targetX = (int)(position.x + cos(rotationAngle) * previewDistance);
        int targetY = (int)(position.y + sin(rotationAngle) * previewDistance);

        setlinecolor(YELLOW);
        setlinestyle(PS_DASH, 1);
        line((int)position.x, (int)position.y, targetX, targetY);

        setlinestyle(PS_SOLID, 2);
        line(targetX - 5, targetY - 5, targetX + 5, targetY + 5);
        line(targetX - 5, targetY + 5, targetX + 5, targetY - 5);

        setlinecolor(RED);
        float wobbleAmplitude = 5.0f + (chargePower * 0.02f);
        int currentRadius = (int)(25.0f + sin(auraTime) * wobbleAmplitude);
        int lineWidth = (int)(1 + (chargePower / 100.0f));

        setlinestyle(PS_SOLID, lineWidth);
        circle((int)position.x, (int)position.y, currentRadius);
    }

    solidcircle((int)position.x, (int)position.y, (int)PLAYER_COLLISION_RAD);

    int lineEndX = (int)(position.x + cos(rotationAngle) * (PLAYER_COLLISION_RAD * 2));
    int lineEndY = (int)(position.y + sin(rotationAngle) * (PLAYER_COLLISION_RAD * 2));
    setlinecolor(RED);
    setlinestyle(PS_SOLID, 3);
    line((int)position.x, (int)position.y, lineEndX, lineEndY);

    if (isSlashing) {
        setlinecolor(WHITE);
        setlinestyle(PS_SOLID, slashFrames);
        line((int)slashStart.x, (int)slashStart.y, (int)slashEnd.x, (int)slashEnd.y);
    }

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

    isCharging = false;
    chargePower = 0.0f;
    justSlashed = false;
}