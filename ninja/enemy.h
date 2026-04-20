#pragma once

// enemy.h
#pragma once
#include "entity.h"

class Enemy : public Entity {
private:
	// --- day03：碰撞检测相关 ---
    float radius; // 沙袋的大小
    
    // --- day07：声呐防抖锁 ---
    // 防止一道波纹在穿过身体的几帧内，连续触发多个残影
    bool hasBeenScannedThisWave;

public:
    // 构造函数
    Enemy(float startX, float startY);

	// 必须完成基类的契约：实现 Update 和 Draw 两个函数
    void Update(const ExMessage& msg) override;
    void Draw() override;

    // 留一个接口：为了等下的碰撞检测做准备
    float GetRadius() const { return radius; }

    // --- day07：防抖锁访问接口 ---
    bool GetHasBeenScanned() const { return hasBeenScannedThisWave; }
    void SetHasBeenScanned(bool state) { hasBeenScannedThisWave = state; }
};