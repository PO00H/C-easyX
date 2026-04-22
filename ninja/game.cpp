// game.cpp
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
    // ==========================================
    // 💥 覆盖行动：读取外部地图真实数据！
    // ==========================================
    Vector2D playerSpawn;
    std::vector<Vector2D> enemySpawns;

    // 1. 让地图去读取外部文件！(注意路径和你创建的 txt 文件一致)
    if (levelMap.LoadLevel("assets/levels/level1.txt", playerSpawn, enemySpawns)) {

        // 2. 如果读取成功，用记事本里解析出来的坐标【强行覆盖】掉主角和敌人！
        player.SetPosition(playerSpawn);

        if (!enemySpawns.empty()) {
            enemy.SetPosition(enemySpawns[0]); // 目前死牢里先放1个敌人测试
			enemy.ResetPatrolBounds(); // 根据出生点重置巡逻范围
        }
    }
    else {
        // 如果文件没找到，主角就会按照替身坐标 (400,300) 出生，并且控制台报错
        printf("【警告】地图文件加载失败！请检查 assets/levels/level1.txt 路径！\n");
    }
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

//void Game::Update()
//{
//    // 1. 获取消息 (注意这里加了 while，把邮箱里积压的消息读完，防止鼠标延迟)
//    ExMessage msg = { 0 };                          // 拿出一个空的情报档案袋，取名叫 msg
//    while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {} // 把玩家这一帧的操作塞进袋子里
//
//    effectManager.Update();
//
//    // --- day06&07：让地图的记忆衰减齿轮转起来（扣减倒计时） ---
//    levelMap.Update();
//
//    // ==========================================
//    // 💥 2. 真正的顿帧魔法 💥
//    // ==========================================
//        // 如果管家说：“现在正在卡顿！”
//        // 导演直接一拍桌子：return！（直接退出 Update，下面的代码全都不执行了！）
//    if (effectManager.IsHitStopping()) {
//        return;
//    }
//
//    // 3. 更新玩家逻辑 
//    player.Update(msg);                             // 把档案袋递给player看
//    enemy.Update(msg);                              // 把档案袋递给enemy看
//
//    // ==========================================
//    // 💥 day08：墙体物理碰撞与眩晕判定 💥
//    // ==========================================
//    bool isHitWall = false;
//
//    // 1. 让地图算出主角的安全坐标，并更新主角位置
//    Vector2D safePos = levelMap.ResolveCollision(player.GetPosition(), player.GetRadius(), isHitWall);
//    player.SetPosition(safePos);
//
//    // 2. 如果发生了真实撞击，并且主角当前没有在眩晕，就给他一记重锤！
//    if (isHitWall && !player.IsStunned()) {
//        player.Stun();                  // 主角变红，剥夺控制权
//        effectManager.PlayMissEffect(); // 管家：屏幕轻微震动一下，模拟撞击手感！
//    }
//
//    // --- 同理，敌人也不能穿墙，也把它按在墙边（敌人不用眩晕） ---
//    bool enemyHitWall = false;
//    Vector2D enemySafePos = levelMap.ResolveCollision(enemy.GetPosition(), enemy.GetRadius(), enemyHitWall);
//    enemy.SetPosition(enemySafePos);
//
//    // ==========================================
//    // --- day06&07：波纹探测扫描逻辑 ---
//    // ==========================================
//    if (player.GetIsRippleActive()) {
//
//        // 1. 扫描地图墙壁，点亮它们！
//        levelMap.ScanByRipple(player.GetRippleCenter(), player.GetRippleRadius());
//
//        // 2. 扫描敌人（残影逻辑）
//        if (enemy.GetIsAlive()) {
//            float dx = enemy.GetPosition().x - player.GetRippleCenter().x;
//            float dy = enemy.GetPosition().y - player.GetRippleCenter().y;
//            float distance = sqrt(dx * dx + dy * dy);
//
//            if (!enemy.GetHasBeenScanned() && abs(distance - player.GetRippleRadius()) < enemy.GetRadius()) {
//                effectManager.AddMemoryEcho(enemy.GetPosition(), enemy.GetRadius());
//                enemy.SetHasBeenScanned(true);
//            }
//        }
//    }
//    else {
//        // 如果屏幕上没有波纹了（或者波纹结束了），重置所有敌人的防抖锁
//        // 等待玩家下一次踩地板
//        if (!player.GetIsRippleActive()) {
//            enemy.SetHasBeenScanned(false);
//        }
//    }
//
//
//    // ==========================================
//    // 4. --- day03：听取玩家信号，并进行死亡判定！---
//    // ==========================================
//    if (player.GetIsJustSlashed()) {
//
//        bool isHit = CheckCollision(
//            player.GetSlashStart(),
//            player.GetSlashEnd(),
//            enemy.GetPosition(),
//            enemy.GetRadius()
//        );
//
//        if (isHit && enemy.GetIsAlive()) {
//            enemy.SetIsAlive(false);
//            // --- day04：调用特效管理器生成斩杀粒子，传入敌人当前坐标 ---
//            effectManager.PlayHitEffect(enemy.GetPosition());
//
//            // --- day07：斩杀成功！除了爆火花，还要瞬间清空残影！ ---
//            effectManager.ClearEchoes();
//        }
//        else {
//            // --- day04：调用特效管理器生成挥空特效 ---
//            effectManager.PlayMissEffect();
//        }
//    }
//
//
//}

void Game::Update()
{
    ExMessage msg = { 0 };
    while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {}

    effectManager.Update();
    levelMap.Update();

    if (effectManager.IsHitStopping()) {
        return;
    }

    // ==========================================
    // 💥 1. 悄悄记下移动前的安全老坐标！
    // ==========================================
    Vector2D oldPlayerPos = player.GetPosition();

    // 2. 更新玩家的输入和逻辑
    player.Update(msg);

    // ==========================================
    // 💥 唤醒敌人大脑：情报投喂！
    // ==========================================
    // 提问：什么是噪音？
    // 只要玩家的声呐波纹还在扩散，我们就认为玩家正在发出“巨大的噪音”！
    bool isNoiseActive = player.GetIsRippleActive();

    // 把主角的【精确坐标】和【噪音状态】强行塞进敌人的脑子里！
    enemy.UpdateAI(player.GetPosition(), isNoiseActive);
    // 3. 拿到移动（一闪）后的新坐标
    Vector2D newPlayerPos = player.GetPosition();

    // ==========================================
    // 💥 3. 小碎步防穿模系统 (CCD) 💥
    // ==========================================
    bool isHitWall = false;
    Vector2D safePos = oldPlayerPos;

    // 算一下这一帧移动了多远
    float moveDx = newPlayerPos.x - oldPlayerPos.x;
    float moveDy = newPlayerPos.y - oldPlayerPos.y;
    float moveDist = sqrt(moveDx * moveDx + moveDy * moveDy);

    // 如果位移超过 5 像素，说明在高速冲刺（一闪）！
    if (moveDist > 5.0f) {

        // 把冲刺的长距离，切成每 5 像素一小步
        int steps = (int)(moveDist / 5.0f) + 1;
        float stepX = moveDx / steps;
        float stepY = moveDy / steps;

        // 一步一步往前试探（像激光一样扫射）
        for (int i = 0; i < steps; i++) {
            safePos.x += stepX;
            safePos.y += stepY;

            bool hit = false;
            safePos = levelMap.ResolveCollision(safePos, player.GetRadius(), hit);

            // 只要有一小步碰到了墙的正面，立马刹车！
            if (hit) {
                isHitWall = true;
                break;
            }
        }
    }
    else {
        // 如果只是缓慢走路，就按老方法只检测一次
        safePos = levelMap.ResolveCollision(newPlayerPos, player.GetRadius(), isHitWall);
    }

    // 4. 把算出来的绝对安全坐标，强制还给主角
    player.SetPosition(safePos);

    // ==========================================
    // 💥 修复斩杀失效Bug：向主角询问一次信号，并存入局部变量！
    // ==========================================
    bool hasSlashed = player.GetIsJustSlashed();

    // ==========================================
    // 💥 修复刀光穿墙Bug：截断刀光特效！
    // ==========================================
    if (hasSlashed) { // 👈 这里用刚才记下来的变量
        player.SetSlashEnd(safePos);
    }

    // 5. 如果撞墙了，且主角没在眩晕，给予重锤罚站！
    if (isHitWall && hasSlashed && !player.IsStunned()) {
        player.Stun();                  // 只有这种组合才罚站
        effectManager.PlayMissEffect(); // 屏幕震动
    }

    // --- 同理，敌人也不能穿墙 ---
    bool enemyHitWall = false;
    Vector2D enemySafePos = levelMap.ResolveCollision(enemy.GetPosition(), enemy.GetRadius(), enemyHitWall);
    enemy.SetPosition(enemySafePos);


    // ==========================================
    // --- day06&07：波纹探测扫描逻辑 ---
    // ==========================================
    if (player.GetIsRippleActive()) {

        levelMap.ScanByRipple(player.GetRippleCenter(), player.GetRippleRadius());

        if (enemy.GetIsAlive()) {
            float dx = enemy.GetPosition().x - player.GetRippleCenter().x;
            float dy = enemy.GetPosition().y - player.GetRippleCenter().y;
            float distance = sqrt(dx * dx + dy * dy);

            // 1. 旧逻辑：在地上留下一个白色的残影圈（有防抖锁，只留一次）
            if (!enemy.GetHasBeenScanned() && abs(distance - player.GetRippleRadius()) < enemy.GetRadius()) {
                effectManager.AddMemoryEcho(enemy.GetPosition(), enemy.GetRadius());
                enemy.SetHasBeenScanned(true);
            }

            // 2. 新逻辑：只要波纹擦过敌人，立刻注入 300 帧的“发光记忆”！
            // (注意：这个不需要防抖锁，波纹只要还在它身上，它的记忆就一直是满的)
            if (abs(distance - player.GetRippleRadius()) < enemy.GetRadius()) {
                enemy.TriggerMemory(); // 👈 移到这里面来！
            }
        }
    }
    else {
        enemy.SetHasBeenScanned(false);
    }


    // ==========================================
    // 💥 极致氛围：被动听觉雷达 (脚步声与迫近压迫感)
    // ==========================================
    if (enemy.GetIsAlive()) {
        // 先算一下主角和敌人的绝对真实距离
        float dx = enemy.GetPosition().x - player.GetPosition().x;
        float dy = enemy.GetPosition().y - player.GetPosition().y;
        float distToEnemy = sqrt(dx * dx + dy * dy);

        // --- 情况 1：疯狗追击（无视距离，疯狂踩出残影） ---
        if (enemy.GetCurrentState() == EnemyState::CHASE) {
            static int chaseTimer = 0;
            chaseTimer++;
            if (chaseTimer >= 15) {
                // 💥 替换成红色的空心扩散波纹！最大长到 30 像素
                effectManager.AddDynamicRipple(enemy.GetPosition(), 30.0f, RGB(255, 50, 50));
                chaseTimer = 0;
            }
        }
        // --- 情况 2：死神擦肩而过（距离小于 150，且没有在追击） ---
        else if (distToEnemy < 150.0f) {
            static int tensionTimer = 0;
            tensionTimer++;

            int threshold = (int)((distToEnemy / 150.0f) * 60.0f);
            if (threshold < 15) threshold = 15;

            if (tensionTimer >= threshold) {
                // 💥 替换成冰蓝色的空心扩散波纹！最大长到 60 像素
                effectManager.AddDynamicRipple(enemy.GetPosition(), 60.0f, RGB(50, 150, 255));
                tensionTimer = 0;
            }
        }
    }

    // ==========================================
    // --- day03：玩家斩杀与特效逻辑 ---
    // ==========================================
    if (hasSlashed) { // 👈 这里也用刚才记下来的变量！千万别再调用 player.GetIsJustSlashed() 了！

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
}

void Game::Draw()
{
    float offsetX = effectManager.GetShakeOffsetX();
    float offsetY = effectManager.GetShakeOffsetY();

    setorigin((int)offsetX, (int)offsetY);

    // ==========================================
    // 💥 视觉大升级：全屏受击闪红 (Damage Flash)
    // ==========================================
    // 注：如果你在 Player 类里写的是 IsStunned() 就用这个
    // 如果没有这个函数，可以临时去 player.h 里加一句：bool IsStunned() const { return stunTimer > 0; }
    if (player.IsStunned()) {
        setbkcolor(RGB(40, 0, 0)); // 眩晕时，宇宙底色变成压抑的暗红色
    }
    else {
        setbkcolor(BLACK);         // 正常时，宇宙是纯黑的
    }


    cleardevice();

    // 1. 画敌人的残影（最底层）
    effectManager.DrawEchoes();

    // 2. 画被点亮的地图墙壁
    levelMap.Draw();

    // 3. 画敌人和主角（活着的敌人目前是隐形的）
    enemy.Draw();
    player.Draw();

    // 4. 画战斗火花粒子（最顶层）
    effectManager.DrawParticles();

    setorigin(0, 0);
    FlushBatchDraw();
}
