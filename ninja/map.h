/******************************************************************************
 *
 * [引擎核心图纸] MAP.H
 *
 * @desc : 游戏世界的地基。在原有网格物理的基础上，新增了基于 BFS
 * 算法的全局导航基站，为 AI 提供避障寻路支持。
 *
 *=============================================================================
 * [ 函数导航 (Function Map) ]
 *
 * ▶ 1. 生命周期与渲染 (Lifecycle & Render)
 * ├─ Map()                 : 初始化地图网格
 * ├─ Update()              : 处理记忆迷雾衰减
 * └─ Draw()                : 渲染渐变墙壁
 *
 * ▶ 2. 核心传感器 (Sensors & Physics)
 * ├─ ScanByRipple()        : 处理主动波纹点亮
 * └─ ResolveCollision()    : 九宫格物理挤出
 *
 * ▶ 3. AI 导航基站 (Navigation)
 * └─ GetBestDirection()    : (新增) 生成 BFS 热力图，输出规避墙壁的最佳向量
 *
 * ▶ 4. 外部接口 (I/O)
 * └─ LoadLevel()           : 读取关卡 txt 文件
 *
 ******************************************************************************/
#pragma once
#include "common.h"
#include <vector>
#include <string>

class Map {
private:
    std::vector<std::vector<Tile>> grid;

public:
    Map();
    void Update();
    void Draw();

    void ScanByRipple(Vector2D center, float rippleRadius);
    Vector2D ResolveCollision(Vector2D pos, float radius, bool& outHit);

    // --- 💥 新增：AI 寻路 GPS 接口 ---
    Vector2D GetBestDirection(Vector2D startPos, Vector2D targetPos);

    bool LoadLevel(const std::string& filename, Vector2D& outPlayerPos, std::vector<Vector2D>& outEnemySpawns);
};