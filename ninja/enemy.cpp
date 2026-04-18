// enemy.cpp
#include "enemy.h"

// 构造函数：把坐标交给老父亲，自己初始化半径
Enemy::Enemy(float startX, float startY) : Entity(startX, startY)
{
    radius = 20.0f; // 沙袋半径 20 像素
    // --- day06：初始化时，敌人是隐形的 ---
    isVisible = false;
    visibleTimer = 0;
}

void Enemy::Update(const ExMessage& msg)// 敌人的状态
{
    // --- day06：处理隐身倒计时 ---
    if (isVisible) {
        visibleTimer--;
        if (visibleTimer <= 0) {
            isVisible = false; // 时间到，重新隐身进入黑暗
        }
    }
}

void Enemy::Draw()
{
    // 如果死了，就直接退出，不画了
    if (isAlive == false) return;

    // --- day06：如果不可见，直接不画！ ---
    if (!isVisible) return;

    // 为了体现被“声呐”扫出来的感觉，我们可以把颜色改成空灵的青色(CYAN)或白色
    setfillcolor(CYAN);
    solidcircle((int)position.x, (int)position.y, (int)radius);
}

// --- day06：被波纹扫中时调用 ---
void Enemy::Reveal()
{
    isVisible = true;
    visibleTimer = 60; // 显身 60 帧（大约 1 秒）
}