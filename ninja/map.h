// map.h
#pragma once
#include "common.h"
#include <vector>
#include <string>

// --- day06&07：定义地图瓦片的结构体 ---
struct Tile {
    int type;        // 0: 地板, 1: 墙壁
    int memoryTimer; // 记忆残留倒计时（0表示完全不可见）
    int maxMemory;   // 记忆最大时长（用于计算渐变衰减）
};

class Map {
private:
    int tileSize;
    std::vector<std::vector<Tile>> grid; // 网格地图容器

public:
    Map();

    // 每帧更新瓦片的记忆倒计时
    void Update();

    // 绘制被点亮的地图
    void Draw();

    // --- day06&07：波纹扫描接口 ---
    // 传入波纹圆心和当前半径，点亮碰到的墙壁
    void ScanByRipple(Vector2D center, float rippleRadius);

    // --- day08：完美贴墙停住算法 ---
    // 传入当前坐标和半径，返回一个完美贴着墙壁的“安全坐标”
    // 同时通过 outHit 变量，报告刚才是否发生了一次“撞击”
    Vector2D ResolveCollision(Vector2D pos, float radius, bool& outHit);

    bool LoadLevel(const std::string& filename, Vector2D& outPlayerPos, std::vector<Vector2D>& outEnemySpawns);


};