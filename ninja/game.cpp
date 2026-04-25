/******************************************************************************
 *
 * [引擎核心逻辑] GAME.CPP
 *
 * @desc : 游戏全局导演的具体实现。全面升级为 std::vector 驱动的多敌同屏架构。
 *
 ******************************************************************************/
#include "game.h"
#include <graphics.h>
#include <iostream>
#include <math.h>

 // ==========================================
 // ▶ 1. 底层物理算法
 // ==========================================
bool Game::CheckCollision(Vector2D A, Vector2D B, Vector2D center, float r)
{
    Vector2D AB = { B.x - A.x, B.y - A.y };
    Vector2D AC = { center.x - A.x, center.y - A.y };

    float lengthSq = AB.x * AB.x + AB.y * AB.y;

    if (lengthSq == 0.0f) {
        float dx = center.x - A.x;
        float dy = center.y - A.y;
        return (dx * dx + dy * dy) <= (r * r);
    }

    float t = (AC.x * AB.x + AC.y * AB.y) / lengthSq;

    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    float closestX = A.x + t * AB.x;
    float closestY = A.y + t * AB.y;

    float dx = center.x - closestX;
    float dy = center.y - closestY;
    float distSq = dx * dx + dy * dy;

    return distSq <= (r * r);
}

// ==========================================
// ▶ 2. 生命周期与轮回系统
// ==========================================
Game::Game() : player(400.0f, 300.0f), isRunning(true), isGameOver(false)
{
    Vector2D playerSpawn;
    std::vector<Vector2D> enemySpawns;

    // 💥 载入地图，解析多敌出生点
    if (levelMap.LoadLevel("assets/levels/level1.txt", playerSpawn, enemySpawns)) {
        player.SetPosition(playerSpawn);
        spawnPosPlayer = playerSpawn;

        // 动态生成敌军大队！
        for (const Vector2D& pos : enemySpawns) {
            Enemy newEnemy(pos.x, pos.y);
            newEnemy.ResetPatrolBounds();
            enemies.push_back(newEnemy);
            spawnPosEnemies.push_back(pos); // 记住每一个敌人的出生点
        }
    }
    else {
        printf("【警告】地图文件加载失败！请检查文件路径！\n");
    }
}

// game.cpp
void Game::ResetGame() {
    isGameOver = false;
    player.SetPosition(spawnPosPlayer);
    player.ResetStun();

    // 💥 修复：调用 Enemy 的 Reset 方法，彻底清理大脑
    for (int i = 0; i < (int)enemies.size(); i++) {
        enemies[i].Reset(spawnPosEnemies[i]);
    }

    effectManager.ClearEchoes();
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
// ▶ 3. 核心调度与更新逻辑
// ==========================================
void Game::Update()
{
    static int globalTimer = 0; // 引入全局时钟
    globalTimer++;

    // --- 输入监听与轮回判定 ---
    ExMessage msg = { 0 };
    while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
        if (isGameOver && msg.message == WM_KEYDOWN && msg.vkcode == 'R') {
            ResetGame();
        }
    }

    // --- 管家系统更新 ---
    effectManager.Update();
    levelMap.Update();

    if (effectManager.IsHitStopping()) {
        return;
    }

    if (isGameOver) return; // 💥 世界静止

    // --- 1. 主角逻辑与防穿模 ---
    Vector2D oldPlayerPos = player.GetPosition();
    player.Update(msg);
    Vector2D newPlayerPos = player.GetPosition();

    bool isHitWall = false;
    Vector2D safePos = oldPlayerPos;
    float moveDx = newPlayerPos.x - oldPlayerPos.x;
    float moveDy = newPlayerPos.y - oldPlayerPos.y;
    float moveDist = sqrt(moveDx * moveDx + moveDy * moveDy);

    // CCD 步进检测
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

    bool hasSlashed = player.GetIsJustSlashed();
    if (hasSlashed) {
        player.SetSlashEnd(safePos);
    }

    if (isHitWall && hasSlashed && !player.IsStunned()) {
        player.Stun();
        effectManager.PlayMissEffect();
    }

    // --- 2. 敌军群集逻辑运算 (必须加 & 引用) ---
    bool isNoiseActive = player.GetIsRippleActive();

    for (Enemy& e : enemies) {
        if (!e.GetIsAlive()) continue;

        // A. AI 投喂
        e.UpdateAI(player.GetPosition(), isNoiseActive, levelMap);

        // B. 敌人防穿模
        bool enemyHitWall = false;
        Vector2D enemySafePos = levelMap.ResolveCollision(e.GetPosition(), e.GetRadius(), enemyHitWall);
        e.SetPosition(enemySafePos);

        // C. 主动探测雷达 (实心残影)
        if (isNoiseActive) {
            float dx = e.GetPosition().x - player.GetRippleCenter().x;
            float dy = e.GetPosition().y - player.GetRippleCenter().y;
            float distance = sqrt(dx * dx + dy * dy);

            if (!e.GetHasBeenScanned() && abs(distance - player.GetRippleRadius()) < e.GetRadius()) {
                effectManager.AddMemoryEcho(e.GetPosition(), e.GetRadius());
                e.SetHasBeenScanned(true);
            }
        }
        else {
            e.SetHasBeenScanned(false);
        }

        // D. 被动压迫雷达 (产生心跳/脚步涟漪)
        float distToPlayer = sqrt(pow(e.GetPosition().x - player.GetPosition().x, 2) + pow(e.GetPosition().y - player.GetPosition().y, 2));

        if (e.GetCurrentState() == EnemyState::CHASE) {
            // 💥 狂暴心跳：追击时，每 15 帧（约0.25秒）极其稳定地爆发一次红色涟漪
            if (globalTimer % 15 == 0) {
                effectManager.AddDynamicRipple(e.GetPosition(), 30.0f, RGB(255, 50, 50));
            }
        }
        else if (distToPlayer < 150.0f) {
            // 👣 警觉脚步：靠近时，每 60 帧（约1秒）稳定释放一次蓝色涟漪
            // 魔法细节：加上 e.GetPosition().x 来错开不同敌人的频率，防止所有敌人同时亮起，形成错落有致的压迫感！
            if ((globalTimer + (int)e.GetPosition().x) % 60 == 0) {
                effectManager.AddDynamicRipple(e.GetPosition(), 60.0f, RGB(50, 150, 255));
            }
        }

        // E. 斩杀判定
        if (hasSlashed) {
            bool isHit = CheckCollision(player.GetSlashStart(), player.GetSlashEnd(), e.GetPosition(), e.GetRadius());
            if (isHit) {
                e.SetIsAlive(false);
                effectManager.PlayHitEffect(e.GetPosition());
                effectManager.ClearEchoes();
            }
        }

        // F. 终焉判定 (被抓)
        float catchDist = player.GetRadius() + e.GetRadius();
        if (distToPlayer * distToPlayer < catchDist * catchDist) {
            isGameOver = true;
            effectManager.PlayMissEffect();
            effectManager.ClearEchoes();
        }
    }

    // 地图涟漪扫描
    if (isNoiseActive) {
        levelMap.ScanByRipple(player.GetRippleCenter(), player.GetRippleRadius());
    }
}

// ==========================================
// ▶ 4. 视觉渲染
// ==========================================
void Game::Draw()
{
    float offsetX = effectManager.GetShakeOffsetX();
    float offsetY = effectManager.GetShakeOffsetY();
    setorigin((int)offsetX, (int)offsetY);

    if (player.IsStunned()) setbkcolor(RGB(40, 0, 0));
    else setbkcolor(BLACK);
    cleardevice();

    // ==========================================
    // 🟢 修复：严格的画家算法渲染管线 (层级从底到高)
    // ==========================================

    // 层级 1：最底层，画墙壁和环境
    levelMap.Draw();

    // 层级 2：附着在环境上的记忆残影 (现在它会稳稳地压在墙上！)
    effectManager.DrawEchoes();

    // 层级 3：活着的敌人
    for (Enemy& e : enemies) {
        if (e.GetIsAlive()) e.Draw();
    }

    // 层级 4：玩家自己
    player.Draw();

    // 层级 5：最顶层，刀光、火花、动态波纹必须在最上面！
    effectManager.DrawParticles();

    setorigin(0, 0);

    // ... 下面的 战败全屏 UI 保持不变 ...

    // 战败全屏 UI
    if (isGameOver) {
        setfillcolor(BLACK);
        solidrectangle(0, 0, GAME_WIDTH, GAME_HEIGHT);

        RECT rect = { 0, 0, GAME_WIDTH, GAME_HEIGHT };
        settextcolor(RGB(200, 0, 0));
        settextstyle(80, 0, _T("微软雅黑"));
        drawtext(_T("落 命"), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        settextcolor(WHITE);
        settextstyle(20, 0, _T("微软雅黑"));
        outtextxy(GAME_WIDTH / 2 - 80, GAME_HEIGHT / 2 + 100, _T("按 R 键重新轮回..."));
    }

    FlushBatchDraw();
}