// enemy.cpp
#include "enemy.h"

// 构造函数：把坐标交给老父亲，自己初始化半径
Enemy::Enemy(float startX, float startY) : Entity(startX, startY)
{
    radius = 20.0f; // 沙袋半径 20 像素
    // --- day07：初始化防抖锁 ---
    hasBeenScannedThisWave = false;
}

void Enemy::Update(const ExMessage& msg)// 敌人的状态
{
    
}

void Enemy::Draw()
{
    // --- day07：盲剑客视角下，活着的敌人是绝对隐形的！ ---
    // 彻底留空，或者直接 return;
    // 渲染工作已经全部交给了 EffectManager::DrawEchoes()
    return;
}

