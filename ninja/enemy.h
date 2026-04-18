#pragma once

// enemy.h
#pragma once
#include "entity.h"

class Enemy : public Entity {
private:
	// --- day03：碰撞检测相关 ---
    float radius; // 沙袋的大小
    // --- day06：波纹探测相关 ---
    bool isVisible;     // 当前是否可见？
    int visibleTimer;   // 显身倒计时（帧）

public:
    // 构造函数
    Enemy(float startX, float startY);

	// 必须完成基类的契约：实现 Update 和 Draw 两个函数
    void Update(const ExMessage& msg) override;
    void Draw() override;

    // 留一个接口：为了等下的碰撞检测做准备
    float GetRadius() const { return radius; }

    // --- day06：暴露现形接口：被波纹扫到了！ ---
    void Reveal();
};