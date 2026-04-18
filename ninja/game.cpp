// game.cpp
#include "game.h"
#include <graphics.h>
#include <iostream>
#include <math.h>


// --- day03：碰撞检测算法 ---
bool Game::CheckCollision(Vector2D A, Vector2D B, Vector2D center, float r)
{
    // 1. 把点变成向量：算出线段 AB 和向量 AC
    Vector2D AB = { B.x - A.x, B.y - A.y };
    Vector2D AC = { center.x - A.x, center.y - A.y };

    // 2. 算一下线段 AB 到底有多长？（用勾股定理，但不开根号，省 CPU）
	    // lengthSq 是 AB 长度的平方，等于 AB.x^2 + AB.y^2
    float lengthSq = AB.x * AB.x + AB.y * AB.y;

    // 防御性编程：如果主角根本没动（长度为 0），直接算点 A 到圆心的距离
    if (lengthSq == 0.0f) {
        float dx = center.x - A.x;
        float dy = center.y - A.y;
        return (dx * dx + dy * dy) <= (r * r);
    }

    // 3. 算投影比例 t
        // 利用向量点乘，算出圆心 C 在线段 AB 上的“影子”落在哪。
        // t 的值是一个百分比：
        // 如果 t=0，说明影子落在 A 点。
        // 如果 t=1，说明影子落在 B 点。
        // 如果 t=0.5，说明影子在线段正中间。
    float t = (AC.x * AB.x + AC.y * AB.y) / lengthSq;

    // 4. 把影子强行拉回到线段上
    // 如果 t < 0，说明影子落在了 A 的左边外面，那最近的点就是 A 本身。
    // 如果 t > 1，说明影子落在了 B 的右边外面，那最近的点就是 B 本身。
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    // 5. 找到线段上，距离圆心【最近】的那个致命点 (closestX, closestY)
    float closestX = A.x + t * AB.x;
    float closestY = A.y + t * AB.y;

    // 6. 最后的判决：算出圆心到这个致命点的距离
    float dx = center.x - closestX;
    float dy = center.y - closestY;
    float distSq = dx * dx + dy * dy;

    // 如果最近距离小于等于半径，说明刀刃切进肉里了！斩杀成功！
    return distSq <= (r * r);
}

// 构造函数：初始化玩家出生点在屏幕正中央 (400, 300)
Game::Game() : player(400.0f, 300.0f), enemy(600.0f, 300.0f), isRunning(true)
{

}

void Game::Run() {
    // 1. 初始化窗口
    initgraph(800, 600, EX_SHOWCONSOLE);
    BeginBatchDraw();

    // 2. 游戏主循环 (只负责调度，不负责具体干活)
    while (isRunning) {

        Update(); // 👈 叫醒导演！让他去执行 Game::Update() 里的震动和玩家逻辑！
        Draw();   // 👈 叫醒摄影师！让他去执行 Game::Draw() 里的画面摇晃逻辑！

        Sleep(16);
    }

    // 3. 游戏结束
    EndBatchDraw();
    closegraph();
}

void Game::Update()
{
    // 1. 获取消息 (注意这里加了 while，把邮箱里积压的消息读完，防止鼠标延迟)
    ExMessage msg = { 0 };                          // 拿出一个空的情报档案袋，取名叫 msg
    while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {} // 把玩家这一帧的操作塞进袋子里

    effectManager.Update();

    // ==========================================
    // 💥 2. 真正的顿帧魔法 💥
    // ==========================================
        // 如果管家说：“现在正在卡顿！”
        // 导演直接一拍桌子：return！（直接退出 Update，下面的代码全都不执行了！）
    if (effectManager.IsHitStopping()) {
        return;
    }

    // 3. 更新玩家逻辑 
    player.Update(msg);                             // 把档案袋递给player看
    enemy.Update(msg);                              // 把档案袋递给enemy看

    // ==========================================
    // 💥 day06：波纹探测扫描逻辑 💥
    // ==========================================
    if (player.GetIsRippleActive() && enemy.GetIsAlive()) {
        // 1. 计算波纹圆心到敌人的距离
        float dx = enemy.GetPosition().x - player.GetRippleCenter().x;
        float dy = enemy.GetPosition().y - player.GetRippleCenter().y;
        float distance = sqrt(dx * dx + dy * dy);

        // 2. 判断波纹边缘是否碰到了敌人
        // 如果 距离 与 波纹半径 的差值，小于敌人的半径，说明波纹切过敌人的身体了！
        if (abs(distance - player.GetRippleRadius()) < enemy.GetRadius()) {
            enemy.Reveal(); // 扫到了！让他显身！
        }
    }


    // ==========================================
    // 4. --- day03：听取玩家信号，并进行死亡判定！---
    // ==========================================
    if (player.GetIsJustSlashed()) {

        bool isHit = CheckCollision(
            player.GetSlashStart(),
            player.GetSlashEnd(),
            enemy.GetPosition(),
            enemy.GetRadius()
        );

        if (isHit && enemy.GetIsAlive()) {
            enemy.SetIsAlive(false);
            // --- day04：调用特效管理器生成斩杀粒子，传入敌人当前坐标 ---
            effectManager.PlayHitEffect(enemy.GetPosition());
        }
        else {
            // --- day04：调用特效管理器生成挥空特效 ---
            effectManager.PlayMissEffect();
        }
    }


}

void Game::Draw()
{
    // --- day03：直接找effect_manager要这一帧的摇晃偏移量 ---

    float offsetX = effectManager.GetShakeOffsetX();
    float offsetY = effectManager.GetShakeOffsetY();

    setorigin((int)offsetX, (int)offsetY);
    cleardevice();

    enemy.Draw();
    player.Draw();

    // --- day04：在顶层图层渲染粒子特效 ---
    effectManager.DrawParticles();

    setorigin(0, 0);
    FlushBatchDraw();
}
