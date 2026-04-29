/******************************************************************************
 *
 * [引擎核心图纸] GAME.H
 *
 * @desc  : 游戏全局导演类的头文件声明。统筹物理碰撞、多实体调度、
 * 核心玩法循环（双重雷达、战败轮回）与视觉渲染。
 *
 *=============================================================================
 * [ 函数导航 (Function Map) ]
 *
 * ▶ 1. 生命周期 (Lifecycle)
 * ├─ Game()                : 读取外部关卡并动态生成敌人群体
 * ├─ Run()                 : 启动引擎，接管 16ms 帧率控制
 * └─ ResetGame()           : 战败轮回，重置主角与所有敌人的状态及位置
 *
 * ▶ 2. 底层算法 (Physics & Math)
 * └─ CheckCollision()      : 极速斩杀碰撞检测 (线段与圆的相交判定)
 *
 * ▶ 3. 核心调度 (Core Loop)
 * ├─ Update()              : 单帧逻辑：调度群集AI、环境交互、双重雷达与生死判定
 * └─ Draw()                : 单帧渲染：严格的画家算法图层叠加绘制与 UI
 *
 ******************************************************************************/
#pragma once
#include "player.h"
#include "enemy.h"
#include "effect_manager.h"
#include "map.h"
#include <vector>

class Game {
private:
    // ==========================================
    // [ 演员与系统管家 ]
    // ==========================================
    Player player;                  // 玩家实体 (盲剑客)
    std::vector<Enemy> enemies;     // 敌方群集 (支持多敌同屏)

    EffectManager effectManager;    // 特效与屏幕震动管家
    Map levelMap;                   // 关卡物理地图

    // ==========================================
    // [ 引擎状态锁与记忆 ]
    // ==========================================
    bool isRunning;                 // 游戏主循环开关
    bool isGameOver;                // 战败 (落命) 状态锁

    Vector2D spawnPosPlayer;                 // 记忆：主角出生点
    std::vector<Vector2D> spawnPosEnemies;   // 记忆：所有敌人的出生点

    // ==========================================
    // [ 内部隐秘机制 ]
    // ==========================================
    bool CheckCollision(Vector2D A, Vector2D B, Vector2D center, float r);
    void ResetGame();

public:
    Game();
    void Run();

    void Update();
    void Draw();
};