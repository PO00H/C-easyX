#pragma once

// --- day03 entity.h ---
#pragma once
#include "common.h" 
#include <graphics.h> // 需要用到 ExMessage 

class Entity {
protected:
    // --- 核心共用属性 ---
    Vector2D position;
    bool isAlive;      // 记录实体是否存活

public:
    // 构造函数：诞生时必须有坐标，且默认是活着的
    Entity(float startX, float startY) : position({ startX, startY }), isAlive(true) {}

    // 虚析构函数：确保删除子类时，不会内存泄漏
    virtual ~Entity() {}

    // --- 公共访问接口 ---
    Vector2D GetPosition() const { return position; }
    bool GetIsAlive() const { return isAlive; }
    void SetIsAlive(bool state) { isAlive = state; }

    // --- 纯虚函数：强制契约 ---
    // 凡是继承 Entity 的，必须自己实现这两个函数！
    virtual void Update(const ExMessage& msg) = 0;
    virtual void Draw() = 0;

    // --- day08：为了在撞墙时能被物理引擎推出去，增加修改坐标的接口 ---
    void SetPosition(Vector2D newPos) { position = newPos; }
};