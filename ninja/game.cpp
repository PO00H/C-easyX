/******************************************************************************
 *
 * [引擎核心逻辑] GAME.CPP
 *
 * @desc : 游戏全局导演的具体实现。全面升级为 std::vector 驱动的多敌同屏架构。
 * Day 13 重构：权力下放，物理与破甲逻辑已完美封装至 Player 类。
 *
 ******************************************************************************/
#include "game.h"
#include <graphics.h>
#include <iostream>
#include <math.h>

 // ==========================================
 // ▶ 0. 语义化常量定义 (去魔法数字)
 // ==========================================
constexpr int   HEARTBEAT_INTERVAL_FRAMES = 15;    // 狂暴心跳特效间隔 (约 0.25 秒)
constexpr int   FOOTSTEP_INTERVAL_FRAMES = 60;    // 警觉脚步特效间隔 (约 1 秒)
constexpr float ENEMY_HEAR_DISTANCE = 150.0f;// 敌人产生脚步波纹的警觉距离

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

    if (levelMap.LoadLevel("assets/levels/level1.txt", playerSpawn, enemySpawns)) {
        player.SetPosition(playerSpawn);
        spawnPosPlayer = playerSpawn;

        for (const Vector2D& pos : enemySpawns) {
            Enemy newEnemy(pos.x, pos.y);
            newEnemy.ResetPatrolBounds();
            enemies.push_back(newEnemy);
            spawnPosEnemies.push_back(pos);
        }
    }
    else {
        printf("【警告】地图文件加载失败！请检查文件路径！\n");
    }
}

void Game::ResetGame() {
    isGameOver = false;
    player.SetPosition(spawnPosPlayer);
    player.ResetStun();
    levelMap.ResetMap();

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
    static int globalTimer = 0;
    globalTimer++;

    // --- 输入监听 ---
    ExMessage msg = { 0 };
    while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
        if (isGameOver && msg.message == WM_KEYDOWN && msg.vkcode == 'R') {
            ResetGame();
        }
    }

    effectManager.Update();
    levelMap.Update();

    if (effectManager.IsHitStopping()) return;
    if (isGameOver) return;

    // ==========================================
    // --- 1. 主角逻辑 (权力下放至 Player) ---
    // ==========================================
    player.Update(msg, levelMap);

    bool hasSlashed = player.GetIsJustSlashed();
    if (hasSlashed) {
        effectManager.PlayShake(); // 一闪成功时触发全局震动
    }

    // ==========================================
    // --- 2. 敌军群集逻辑 ---
    // ==========================================
    bool isNoiseActive = player.GetIsRippleActive();
    for (Enemy& e : enemies) {
        if (!e.GetIsAlive()) continue;

        // A. AI 投喂
        e.UpdateAI(player.GetPosition(), isNoiseActive, levelMap);

        // B. 敌人防穿模
        bool enemyHitWall = false;
        Vector2D enemySafePos = levelMap.ResolveCollision(e.GetPosition(), e.GetRadius(), enemyHitWall);
        e.SetPosition(enemySafePos);

        // C. 主动探测雷达 (实心残影) - 💥 恢复之前丢失的代码！
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

        // D. 被动压迫雷达 (产生心跳/脚步涟漪) - 💥 恢复之前丢失的代码！
        float distToPlayer = sqrt(pow(e.GetPosition().x - player.GetPosition().x, 2) + pow(e.GetPosition().y - player.GetPosition().y, 2));

        if (e.GetCurrentState() == EnemyState::CHASE) {
            if (globalTimer % HEARTBEAT_INTERVAL_FRAMES == 0) {
                effectManager.AddDynamicRipple(e.GetPosition(), 30.0f, RGB(255, 50, 50));
            }
        }
        else if (distToPlayer < ENEMY_HEAR_DISTANCE) {
            if ((globalTimer + (int)e.GetPosition().x) % FOOTSTEP_INTERVAL_FRAMES == 0) {
                effectManager.AddDynamicRipple(e.GetPosition(), 60.0f, RGB(50, 150, 255));
            }
        }

        // E. 极速斩杀判定
        if (hasSlashed) {
            bool isHit = CheckCollision(player.GetSlashStart(), player.GetSlashEnd(), e.GetPosition(), e.GetRadius());
            if (isHit) {
                e.SetIsAlive(false);
                effectManager.PlayHitEffect(e.GetPosition());
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

    levelMap.Draw();
    effectManager.DrawEchoes();

    for (Enemy& e : enemies) {
        if (e.GetIsAlive()) e.Draw();
    }

    player.Draw();
    effectManager.DrawParticles();

    setorigin(0, 0);

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