// player.cpp----------------------------------------------
/*
- 实现 WASD 移动
- 实现鼠标方向朝向
- 实现蓄力与突进状态
- 预留波纹探测接口
*/

#include "player.h"
#include <math.h> // 用于 atan2 计算角度
#include <iostream>

Player::Player(float startX, float startY) : Entity(startX, startY)
{
    speed = 4.0f;
    rotationAngle = 0.0f;
    //===============================================
    // day01 初始化蓄力状态
    isCharging = false;
    chargePower = 0.0f;
    maxChargePower = 400.0f; // 随便定个上限，决定了你能飞多远
    //===============================================
    // day02 
    // 初始化刀光状态
    isSlashing = false;
    slashFrames = 0;
	// 初始化向心力特效
             // 简单版
             // ringRadius = 0.0f; // 初始半径设为 0
    auraTime = 0.0f;
	// 初始化一闪触发状态
    justSlashed = false;
    //===============================================
    // --- day06：波纹初始化 ---
    isRippleActive = false;
    rippleRadius = 0.0f;
    rippleSpeed = 15.0f; // 波纹扩散得快一点
    //===============================================
    // --- day08：初始化不眩晕 ---
    stunTimer = 0; 
}

void Player::Update(const ExMessage& msg)
{
    // ==========================================
    // 💥 第一步：视觉动画层（必须放在最前面！）
    // 即使主角被撞晕了，射出去的波纹和留在空中的刀光也应该自然消散
    // ==========================================

    // 1. 刀光计时器自动衰减
    if (isSlashing) {
        slashFrames--;
        if (slashFrames <= 0) {
            isSlashing = false;
        }
    }

    // 2. 更新波纹半径（扩散）
    if (isRippleActive) {
        rippleRadius += rippleSpeed;
        if (rippleRadius > 200.0f) { // 扩散到屏幕外就消失
            isRippleActive = false;
        }
    }

    // ==========================================
    // 💥 第二步：物理层拦截（剥夺肉体控制权）
    // ==========================================
    if (stunTimer > 0) {
        stunTimer--;
        return; // 拦截！此时主角被罚站，大脑宕机，下面的按键监听统统不执行！
    }


    // ==========================================
    // 💥 第三步：玩家控制层（正常自由状态下才会执行）
    // ==========================================

    // --- 1. 左键蓄力与一闪攻击 ---
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        isCharging = true; // 进入蓄力状态！

        // 能量与特效同步上涨
        if (chargePower < maxChargePower)
        {
            chargePower += 6.0f;          // 蓄力速度
        }
        // 呼吸光晕时间更新 (蓄力越满，心跳越快)
        auraTime += 0.2f + (chargePower / maxChargePower) * 0.8f;
    }
    else
    {
        // 如果没有按住左键，但刚才在蓄力（说明这一瞬间松手了）
        if (isCharging == true)
        {
            printf("【Player内部】：检测到松手，一闪触发！\n");

            // --- 一闪突进与刀光记录 ---
            float dashDistance = chargePower * 1.0f;

            slashStart.x = position.x;
            slashStart.y = position.y;

            position.x += cos(rotationAngle) * dashDistance;
            position.y += sin(rotationAngle) * dashDistance;

            slashEnd.x = position.x;
            slashEnd.y = position.y;

            isSlashing = true;
            slashFrames = 8;

            // --- 拉响<震动>警报！ ---
            justSlashed = true;

            // --- 所有状态清零 ---
            chargePower = 0.0f;
            auraTime = 0.0f;
        }
        // 彻底回归自由状态
        isCharging = false;
    }

    // --- 2. 右键触发波纹探测 ---
    // 0x08 对应鼠标右键 (VK_RBUTTON)
    if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) && !isRippleActive) {
        isRippleActive = true;
        rippleRadius = 0.0f;
        rippleCenter = position; // 波纹圆心就是玩家当前位置
    }

    // --- 3. 处理移动与转动 (没在蓄力时才能走动) ---
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
void Player::Draw() 
{
    // --- day08：眩晕视觉表现（变红警告） ---
    if (stunTimer > 0) {
        setfillcolor(RGB(200, 50, 50)); // 眩晕时身体变成暗红色
    }
    else {
        setfillcolor(WHITE); // 正常是白色
    }


    // --- day01 新增视觉反馈：画一个代表蓄力大小的空心圆 ---
    if (isCharging) 
    {
        // 1. 预言家逻辑：算出如果此时此刻松手，主角会飞到哪里？
        // 这个公式和我们刚才写在 Update 里的位移公式一模一样
        float previewDistance = chargePower * 1.0f;
        int targetX = (int)(position.x + cos(rotationAngle) * (previewDistance + 0));
        int targetY = (int)(position.y + sin(rotationAngle) * (previewDistance + 0));

        // 2. 拿起画笔：设置为黄色虚线
        setlinecolor(YELLOW);
        // PS_DASH 是 EasyX 专门用来画虚线的代号，1 是线条粗细
        setlinestyle(PS_DASH, 1);

        // 3. 画出连接玩家和落点的虚线
        line((int)position.x, (int)position.y, targetX, targetY);

        // 4. 加点细节：在落点位置画一个醒目的“叉叉 (X)”
        // 先把画笔换回实线 (PS_SOLID)，稍微加粗一点 (2像素)
        setlinestyle(PS_SOLID, 2);
        // 画叉叉的左上到右下 "\"
        line(targetX - 5, targetY - 5, targetX + 5, targetY + 5);
        // 画叉叉的左下到右上 "/"
        line(targetX - 5, targetY + 5, targetX + 5, targetY - 5);

        // ==========================================
        // 🌀 新增：画出向心力收缩圈 🌀
        // ==========================================
        // 
                      //// 简单版
                        //// 我们换一支青色(CYAN)的笔，代表风压或者气流，和黄色的瞄准线区分开
                        //setlinecolor(CYAN);
                        //setlinestyle(PS_SOLID, 1); // 1 像素细线，显得轻盈

                        //// 画出一个以玩家为中心，半径不断变化的圆
                        //circle((int)position.x, (int)position.y, (int)ringRadius);
        // ==========================================
        // 💓 新增：画出呼吸感光晕 💓
        // ==========================================
        // 我们选一个充满危险气息的红色（或者黄色）
        setlinecolor(RED);

        // 【数学魔法开始】
        // 基础半径是 25。
        // sin(auraTime) 会在 -1 到 1 之间波动。
        // 波动幅度：基础幅度 5 + 随着蓄力增加的额外幅度。
        float wobbleAmplitude = 5.0f + (chargePower * 0.02f);
        int currentRadius = (int)(25.0f + sin(auraTime) * wobbleAmplitude);

        // 为了体现“能量狂暴”，随着蓄力增加，线条变粗
        int lineWidth = (int)(1 + (chargePower / 100.0f));
        setlinestyle(PS_SOLID, lineWidth);

        // 画出这个“活着”的圆
        circle((int)position.x, (int)position.y, currentRadius);
    }
    // 画一个白色的圆代表玩家
    // setfillcolor(WHITE);
    solidcircle((int)position.x, (int)position.y, 15);

    // 画一条红线代表玩家的“视线”或“刀刃方向”
    int lineEndX = (int)(position.x + cos(rotationAngle) * 30.0f);
    int lineEndY = (int)(position.y + sin(rotationAngle) * 30.0f);

    setlinecolor(RED);
    // 设置线条粗细为 3
    setlinestyle(PS_SOLID, 3);
    line((int)position.x, (int)position.y, lineEndX, lineEndY);


	// --- day02：如果刀光还在，就画出它！ ---
    // --- 绘制刀光残影 ---
    if (isSlashing) {
        // 画一条极粗的白色实线，代表撕裂空气的刀光
        setlinecolor(WHITE);
        // 根据剩余帧数动态改变线条粗细（一开始最粗，慢慢变细，更有动感）
        setlinestyle(PS_SOLID, slashFrames);

        line((int)slashStart.x, (int)slashStart.y, (int)slashEnd.x, (int)slashEnd.y);
    }

    // --- day06：画出声呐波纹 ---
    if (isRippleActive) {
        setlinecolor(WHITE);
        setlinestyle(PS_SOLID, 2); // 2像素粗的白线
        circle((int)rippleCenter.x, (int)rippleCenter.y, (int)rippleRadius);
    }
}

// --- day02：新增接口，供<Game>查询是否刚触发了一闪（用于震动系统） ---
bool Player::GetIsJustSlashed()
{
    if (justSlashed == true) {
        justSlashed = false; // 导演一旦听到信号，我立刻把警报关掉，否则地球会一直震！
        return true;
    }
    return false;
}

// --- day08：触发眩晕惩罚 ---
void Player::Stun() {
    // 如果已经在眩晕了，就不要重复重置时间，防止死循环
    if (stunTimer > 0) return;

    stunTimer = 40;

    // ==========================================
    // 💥 修复 Bug 2：重置所有冲刺记忆！
    // ==========================================
    isCharging = false;
    chargePower = 0.0f;
    justSlashed = false;

    // 如果你的 player 类里还有其他控制位移的变量（比如 velocityX/Y）
    // 统统在这里设为 0！让主角彻底“静止”
}
