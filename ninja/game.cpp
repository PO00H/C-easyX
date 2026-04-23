/******************************************************************************
 *
 * [引擎核心] GAME.CPP  -- (Day 09 封版：FSM 状态机 & 双重雷达系统)
 *
 * @desc  : 游戏全局导演类。统筹物理碰撞、AI 调度、核心玩法循环与视觉渲染。
 *
 *=============================================================================
 * [ 函数导航与功能地图 ]
 *
 * ▶ 1. 生命周期 (Lifecycle)
 * Game()          : 读取外部关卡 (txt)、初始化出生点、重置 AI 巡逻锚点
 * Run()           : 游戏主循环 (16ms)，负责唤醒 Update 和 Draw
 *
 * ▶ 2. 物理与底层算法 (Physics & Math)
 * CheckCollision(): 线段与圆的极速相交判定 (利用向量点乘与投影比例)
 *
 * ▶ 3. 核心运算枢纽 (The Brain)
 * Update()        : 统筹单帧内的所有运算，严格按照以下顺序执行：
 * ├─ a. AI 投喂   : 将主角坐标与噪音信号塞给敌人大脑
 * ├─ b. CCD防穿模 : 高速位移的步进式检测、撞墙眩晕惩罚
 * ├─ c. 主动雷达  : 玩家踩地板 -> 擦过敌人 -> 生成实心残影
 * ├─ d. 被动雷达  : 动态频率运算 -> 生成压迫感空心涟漪
 * └─ e. 斩杀结算  : 验证刀光碰撞，触发火花特效与死亡清理
 *
 * ▶ 4. 视觉呈现 (Rendering)
 * Draw()          : 摄像机调度。处理屏幕摇晃、受击闪红底色及多图层叠加
 * (渲染顺序: 残影底层 -> 地图 -> 实体 -> 粒子顶层)
 *
 ******************************************************************************/
#include "game.h"
#include <graphics.h>
#include <iostream>
#include <math.h>
#include "map.h"
#include "enemy.h"


// --- day03：碰撞检测算法 ---
bool Game::CheckCollision(Vector2D A, Vector2D B, Vector2D center, float r)
{
    // 1. 把点变成向量：算出线段 AB 和向量 AC
    Vector2D AB = { B.x - A.x, B.y - A.y };
    Vector2D AC = { center.x - A.x, center.y - A.y };

    // 2. 算一下线段 AB 到底有多长？（用勾股定理，但不开根号，省 CPU）
    float lengthSq = AB.x * AB.x + AB.y * AB.y;

    // 防御性编程：如果主角根本没动（长度为 0），直接算点 A 到圆心的距离
    if (lengthSq == 0.0f) {
        float dx = center.x - A.x;
        float dy = center.y - A.y;
        return (dx * dx + dy * dy) <= (r * r);
    }

    // 3. 算投影比例 t
    float t = (AC.x * AB.x + AC.y * AB.y) / lengthSq;

    // 4. 把影子强行拉回到线段上
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    // 5. 找到线段上，距离圆心【最近】的那个点
    float closestX = A.x + t * AB.x;
    float closestY = A.y + t * AB.y;

    // 6. 算出圆心到这个点的距离
    float dx = center.x - closestX;
    float dy = center.y - closestY;
    float distSq = dx * dx + dy * dy;

    return distSq <= (r * r);
}

// --- day 09：重置关卡逻辑 ---
void Game::ResetGame() {
    isGameOver = false;

    // 1. 使用记忆中的坐标，而不是写死的 400
    player.SetPosition(spawnPosPlayer);
    player.ResetStun();

    // 2. 使用记忆中的坐标，而不是写死的 600
    enemy.SetIsAlive(true);
    enemy.SetPosition(spawnPosEnemy);
    enemy.ResetPatrolBounds();

    // 3. 抹除所有死亡残留的视觉信号
    effectManager.ClearEchoes();
}

// 构造函数：初始化玩家出生点
Game::Game() : player(400.0f, 300.0f), enemy(600.0f, 300.0f), isRunning(true)
{
    // ==========================================
    // 💥 覆盖行动：读取外部地图真实数据！
    // ==========================================
    Vector2D playerSpawn;
    std::vector<Vector2D> enemySpawns;

    if (levelMap.LoadLevel("assets/levels/level1.txt", playerSpawn, enemySpawns)) {
        player.SetPosition(playerSpawn);
        spawnPosPlayer = playerSpawn; // 👈 存入记忆

        if (!enemySpawns.empty()) {
            enemy.SetPosition(enemySpawns[0]);
            enemy.ResetPatrolBounds(); // 根据出生点重置巡逻范围
            spawnPosEnemy = enemySpawns[0]; // 👈 存入记忆
        }
    }
    else {
        printf("【警告】地图文件加载失败！请检查 assets/levels/level1.txt 路径！\n");
    }
}

void Game::Run() {
    initgraph(800, 600, EX_SHOWCONSOLE);
    BeginBatchDraw();

    while (isRunning) {
        Update();
        Draw();
        Sleep(16);
    }

    EndBatchDraw();
    closegraph();
}

// ==========================================
// 📜 历史足迹：Day 06 - Day 08 的旧版逻辑
// (保留作为参考，不参与运行)
// ==========================================
/*
void Game::Update()
{
    ExMessage msg = { 0 };
    while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {}

    effectManager.Update();
    levelMap.Update();

    if (effectManager.IsHitStopping()) {
        return;
    }

    player.Update(msg);
    enemy.Update(msg);

    bool isHitWall = false;
    Vector2D safePos = levelMap.ResolveCollision(player.GetPosition(), player.GetRadius(), isHitWall);
    player.SetPosition(safePos);

    if (isHitWall && !player.IsStunned()) {
        player.Stun();
        effectManager.PlayMissEffect();
    }

    bool enemyHitWall = false;
    Vector2D enemySafePos = levelMap.ResolveCollision(enemy.GetPosition(), enemy.GetRadius(), enemyHitWall);
    enemy.SetPosition(enemySafePos);

    if (player.GetIsRippleActive()) {
        levelMap.ScanByRipple(player.GetRippleCenter(), player.GetRippleRadius());

        if (enemy.GetIsAlive()) {
            float dx = enemy.GetPosition().x - player.GetRippleCenter().x;
            float dy = enemy.GetPosition().y - player.GetRippleCenter().y;
            float distance = sqrt(dx * dx + dy * dy);

            if (!enemy.GetHasBeenScanned() && abs(distance - player.GetRippleRadius()) < enemy.GetRadius()) {
                effectManager.AddMemoryEcho(enemy.GetPosition(), enemy.GetRadius());
                enemy.SetHasBeenScanned(true);
            }
        }
    }
    else {
        if (!player.GetIsRippleActive()) {
            enemy.SetHasBeenScanned(false);
        }
    }

    if (player.GetIsJustSlashed()) {
        bool isHit = CheckCollision(
            player.GetSlashStart(),
            player.GetSlashEnd(),
            enemy.GetPosition(),
            enemy.GetRadius()
        );

        if (isHit && enemy.GetIsAlive()) {
            enemy.SetIsAlive(false);
            effectManager.PlayHitEffect(enemy.GetPosition());
            effectManager.ClearEchoes();
        }
        else {
            effectManager.PlayMissEffect();
        }
    }
}
*/

// ==========================================
// 💥 现行逻辑：Day 09 终极 FSM + 双重雷达版
// ==========================================
void Game::Update()
{
    // --- day 09：Update 里的终焉判定 ---
    ExMessage msg = { 0 };
    while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
        // 如果死了，监听 R 键重生
        if (isGameOver && msg.message == WM_KEYDOWN && msg.vkcode == 'R') {
            ResetGame();
        }
    }

    // ==========================================
    // 💥 修复点：找回丢失的时间齿轮！波纹不扩和屏幕乱抖全是因为少了这三行！
    // ==========================================
    effectManager.Update();
    levelMap.Update();
    if (effectManager.IsHitStopping()) {
        return;
    }

    if (isGameOver) return; // 💥 死了就别计算物理和 AI 了，世界静止

    // 1. 记下移动前的安全坐标
    Vector2D oldPlayerPos = player.GetPosition();

    // 2. 更新玩家逻辑
    player.Update(msg);

    // 3. AI 情报投喂 (噪音判定)
    bool isNoiseActive = player.GetIsRippleActive();
    enemy.UpdateAI(player.GetPosition(), isNoiseActive);

    Vector2D newPlayerPos = player.GetPosition();

    // 4. 小碎步防穿模系统 (CCD)
    bool isHitWall = false;
    Vector2D safePos = oldPlayerPos;
    float moveDx = newPlayerPos.x - oldPlayerPos.x;
    float moveDy = newPlayerPos.y - oldPlayerPos.y;
    float moveDist = sqrt(moveDx * moveDx + moveDy * moveDy);

    if (moveDist > 5.0f) {
        int steps = (int)(moveDist / 5.0f) + 1;
        float stepX = moveDx / steps;
        float stepY = moveDy / steps;

        for (int i = 0; i < steps; i++) {
            safePos.x += stepX;
            safePos.y += stepY;
            bool hit = false;
            safePos = levelMap.ResolveCollision(safePos, player.GetRadius(), hit);
            if (hit) {
                isHitWall = true;
                break;
            }
        }
    }
    else {
        safePos = levelMap.ResolveCollision(newPlayerPos, player.GetRadius(), isHitWall);
    }

    player.SetPosition(safePos);

    // 5. 状态检查与刀光修复
    bool hasSlashed = player.GetIsJustSlashed();
    if (hasSlashed) {
        player.SetSlashEnd(safePos);
    }

    if (isHitWall && hasSlashed && !player.IsStunned()) {
        player.Stun();
        effectManager.PlayMissEffect();
    }

    // 敌人碰撞
    bool enemyHitWall = false;
    Vector2D enemySafePos = levelMap.ResolveCollision(enemy.GetPosition(), enemy.GetRadius(), enemyHitWall);
    enemy.SetPosition(enemySafePos);

    // 6. 主动探测 (实心残影)
    if (player.GetIsRippleActive()) {
        levelMap.ScanByRipple(player.GetRippleCenter(), player.GetRippleRadius());

        if (enemy.GetIsAlive()) {
            float dx = enemy.GetPosition().x - player.GetRippleCenter().x;
            float dy = enemy.GetPosition().y - player.GetRippleCenter().y;
            float distance = sqrt(dx * dx + dy * dy);

            if (!enemy.GetHasBeenScanned() && abs(distance - player.GetRippleRadius()) < enemy.GetRadius()) {
                effectManager.AddMemoryEcho(enemy.GetPosition(), enemy.GetRadius());
                enemy.SetHasBeenScanned(true);
            }
        }
    }
    else {
        enemy.SetHasBeenScanned(false);
    }

    // 7. 被动雷达 (空心波纹：心跳/脚步声)
    if (enemy.GetIsAlive()) {
        float dx = enemy.GetPosition().x - player.GetPosition().x;
        float dy = enemy.GetPosition().y - player.GetPosition().y;
        float distToEnemy = sqrt(dx * dx + dy * dy);

        if (enemy.GetCurrentState() == EnemyState::CHASE) {
            static int chaseTimer = 0;
            chaseTimer++;
            if (chaseTimer >= 15) {
                effectManager.AddDynamicRipple(enemy.GetPosition(), 30.0f, RGB(255, 50, 50));
                chaseTimer = 0;
            }
        }
        else if (distToEnemy < 150.0f) {
            static int tensionTimer = 0;
            tensionTimer++;
            int threshold = (int)((distToEnemy / 150.0f) * 60.0f);
            if (threshold < 15) threshold = 15;
            if (tensionTimer >= threshold) {
                effectManager.AddDynamicRipple(enemy.GetPosition(), 60.0f, RGB(50, 150, 255));
                tensionTimer = 0;
            }
        }
    }

    // 8. 斩杀判定
    if (hasSlashed) {
        bool isHit = CheckCollision(
            player.GetSlashStart(),
            player.GetSlashEnd(),
            enemy.GetPosition(),
            enemy.GetRadius()
        );

        if (isHit && enemy.GetIsAlive()) {
            enemy.SetIsAlive(false);
            effectManager.PlayHitEffect(enemy.GetPosition());
            effectManager.ClearEchoes();
        }
    }
    // --- 死亡判定段 ---
    if (enemy.GetIsAlive()) {
        float dx = enemy.GetPosition().x - player.GetPosition().x;
        float dy = enemy.GetPosition().y - player.GetPosition().y;
        float distSq = dx * dx + dy * dy;
        float catchDist = player.GetRadius() + enemy.GetRadius();

        if (distSq < catchDist * catchDist) {
            isGameOver = true;
            effectManager.PlayMissEffect(); // 借用震屏效果，模拟受击感
            effectManager.ClearEchoes();    // 💥 感官剥夺：瞬间清空所有涟漪
        }
    }
}


void Game::Draw()
{
    float offsetX = effectManager.GetShakeOffsetX();
    float offsetY = effectManager.GetShakeOffsetY();

    setorigin((int)offsetX, (int)offsetY);

    if (player.IsStunned()) {
        setbkcolor(RGB(40, 0, 0));
    }
    else {
        setbkcolor(BLACK);
    }

    cleardevice();

    effectManager.DrawEchoes();
    levelMap.Draw();
    enemy.Draw();
    player.Draw();
    effectManager.DrawParticles();

    setorigin(0, 0);

    if (isGameOver) {
        // 1. 盖一块纯黑的幕布，彻底剥夺视觉！
        setfillcolor(BLACK);
        solidrectangle(0, 0, 800, 600);

        // 2. 准备一个全屏大小的隐形矩形，用来让文字绝对居中
        RECT rect = { 0, 0, 800, 600 };

        // 3. 宣判文字
        settextcolor(RGB(200, 0, 0));
        settextstyle(80, 0, _T("微软雅黑"));
        drawtext(_T("落 命"), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        settextcolor(WHITE);
        settextstyle(20, 0, _T("微软雅黑"));

        // 这里的坐标 (320, 400) 是手动算好的大约在“落命”下方的位置
        outtextxy(320, 400, _T("按 R 键重新轮回..."));
    }

    FlushBatchDraw();
}