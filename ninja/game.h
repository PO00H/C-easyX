// game.h
/*
- 控制游戏整体流程
- 管理当前场景
- 调用 `Update()` 和 `Draw()`
- 处理菜单、游戏、暂停等状态切换
*/
#pragma once
#include "player.h"
#include "enemy.h"
#include "effect_manager.h"

class Game {
private:
    Player player;    // 游戏里包含一个玩家
	Enemy enemy;      // 游戏里包含一个沙袋敌人
    bool isRunning;   // 控制游戏主循环的开关



    // --- day03：碰撞检测 ---
        // 参数含义：线段起点 A, 线段终点 B, 圆心 center, 圆的半径 radius
    bool CheckCollision(Vector2D A, Vector2D B, Vector2D center, float r);

	// --- day04：特效系统 ---
    EffectManager effectManager;

public:
    Game();
    void Run();       // 启动游戏主循环
	void Update();    // 更新游戏逻辑
	void Draw();      // 绘制游戏画面
};