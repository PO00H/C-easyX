/******************************************************************************
 *
 * [引擎核心图纸] MAP.H
 *
 * @desc : 游戏世界的地基。集成 BFS 导航基站、环境破坏快照与全局光照状态管理。
 * Day 13 升级：新增 initialGrid 快照重置逻辑与灯具熄灭机制。
 *
 *=============================================================================
 * [ 函数导航 (Function Map) ]
 *
 * ▶ 1. 生命周期与状态管理 (Lifecycle & State)
 * ├─ Map()                 : 初始化地图网格
 * ├─ Update()              : 处理记忆迷雾衰减
 * ├─ ResetMap()            : 💥 (Day13) 将地图恢复至 LoadLevel 时的原始状态
 * └─ Draw()                : 渲染不同材质的渐变墙壁
 *
 * ▶ 2. 核心物理与交互 (Physics & Interaction)
 * ├─ ScanByRipple()        : 声呐波纹点亮环境
 * ├─ ResolveCollision()    : 物理挤出判定 (支持一闪破甲)
 * └─ SlashBamboo()         : 💥 (Day13) 环境破坏：斩断竹林或击碎灯具
 *
 * ▶ 3. 导航与接口 (Nav & I/O)
 * ├─ GetBestDirection()    : BFS 热力图寻路算法
 * ├─ LoadLevel()           : 读取关卡文件并同步备份初始快照
 * └─ GetIsAreaDark()       : 💥 (Day13) 查询当前地图是否处于全灯灭状态
 *
 ******************************************************************************/
#pragma once
#include "common.h"
#include <vector>
#include <string>

class Map {
private:
    // ==========================================
    // [ 地图核心数据 ]
    // ==========================================
    std::vector<std::vector<Tile>> grid;         // 当前运行时的实时地图
    std::vector<std::vector<Tile>> initialGrid;  // 💥 Day13：原始快照，用于 R 键重置

    bool isAreaDark;                             // 💥 Day13：全局光照状态标记

public:
    Map();
    void Update();
    void Draw();

    // --- 环境重置接口 ---
    void ResetMap();

    // --- 物理与感知 ---
    void ScanByRipple(Vector2D center, float rippleRadius);
    Vector2D ResolveCollision(Vector2D pos, float radius, bool& outHit, bool isDashing = false);

    // --- 导航寻路 ---
    Vector2D GetBestDirection(Vector2D startPos, Vector2D targetPos);

    // --- I/O 与交互 ---
    bool LoadLevel(const std::string& filename, Vector2D& outPlayerPos, std::vector<Vector2D>& outEnemySpawns);
    void SlashBamboo(Vector2D slashStart, Vector2D slashEnd);

    // --- 状态查询 ---
    bool GetIsAreaDark() const { return isAreaDark; }
};