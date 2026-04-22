// map.cpp
#include "map.h"
#include <graphics.h>
#include <math.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

Map::Map() {
    tileSize = 40;
    int cols = 800 / tileSize; // 20 列
    int rows = 600 / tileSize; // 15 行

    // 仅仅初始化一张“全白纸”网格，没有任何墙壁。
    // 真正的墙壁，全靠稍后 game.cpp 调用 LoadLevel() 来填满！
    grid.resize(rows, std::vector<Tile>(cols, { 0, 0, 300 }));
}

void Map::Update() {
    // --- day06&07：让地图的记忆随着时间流逝 ---
    for (int r = 0; r < grid.size(); ++r) {
        for (int c = 0; c < grid[0].size(); ++c) {
            if (grid[r][c].memoryTimer > 0) {
                grid[r][c].memoryTimer--; // 倒计时递减
            }
        }
    }
}

void Map::Draw() {
    for (int r = 0; r < grid.size(); ++r) {
        for (int c = 0; c < grid[0].size(); ++c) {

            // 只有当瓦片是墙壁(type==1)，且记忆没消失(timer>0)时，才画出来！
            if (grid[r][c].type == 1 && grid[r][c].memoryTimer > 0) {

                // 计算渐变比例 (从 1.0 慢慢降到 0.0)
                float ratio = (float)grid[r][c].memoryTimer / grid[r][c].maxMemory;

                // 墙壁最亮时是深灰色 (80)，渐渐变暗到纯黑 (0)
                int colorValue = (int)(80 * ratio);

                setfillcolor(RGB(colorValue, colorValue, colorValue));
                // 边框稍微亮一点点，勾勒砖块的轮廓
                setlinecolor(RGB(colorValue + 20, colorValue + 20, colorValue + 20));

                int x = c * tileSize;
                int y = r * tileSize;
                fillrectangle(x, y, x + tileSize, y + tileSize);
            }
        }
    }
}

void Map::ScanByRipple(Vector2D center, float rippleRadius) {
    // 粗略过滤：只检查波纹附近的格子，省 CPU
    // 加上括号，拒绝 Windows 宏替换！
    int startC = (std::max)(0, (int)((center.x - rippleRadius) / tileSize));
    int endC = (std::min)((int)grid[0].size() - 1, (int)((center.x + rippleRadius) / tileSize));
    int startR = (std::max)(0, (int)((center.y - rippleRadius) / tileSize));
    int endR = (std::min)((int)grid.size() - 1, (int)((center.y + rippleRadius) / tileSize));
    for (int r = startR; r <= endR; r++) {
        for (int c = startC; c <= endC; c++) {

            // 找格子中心点，算距离
            float tileCenterX = c * tileSize + tileSize / 2.0f;
            float tileCenterY = r * tileSize + tileSize / 2.0f;

            float dx = center.x - tileCenterX;
            float dy = center.y - tileCenterY;
            float dist = sqrt(dx * dx + dy * dy);

            // 如果格子中心刚好落在波纹环的厚度范围内（给 15 像素容错）
            if (abs(dist - rippleRadius) < 15.0f) {
                // 点亮这个格子！注入 300 帧（5秒）的记忆
                grid[r][c].memoryTimer = 300;
                grid[r][c].maxMemory = 300;
            }
        }
    }
}

// --- day08：完美贴墙与物理挤出算法 ---
Vector2D Map::ResolveCollision(Vector2D pos, float radius, bool& outHit) {
    outHit = false;
    Vector2D newPos = pos;

    // 性能优化：只检查主角九宫格范围内的瓦片
    int startC = (std::max)(0, (int)((newPos.x - radius) / tileSize));
    int endC = (std::min)((int)grid[0].size() - 1, (int)((newPos.x + radius) / tileSize));
    int startR = (std::max)(0, (int)((newPos.y - radius) / tileSize));
    int endR = (std::min)((int)grid.size() - 1, (int)((newPos.y + radius) / tileSize));

    for (int r = startR; r <= endR; r++) {
        for (int c = startC; c <= endC; c++) {

            // 只有 type == 1 (墙壁) 才参与物理碰撞
            if (grid[r][c].type == 1) {
                float rectLeft = c * tileSize;
                float rectRight = rectLeft + tileSize;
                float rectTop = r * tileSize;
                float rectBottom = rectTop + tileSize;

                // 1. 找矩形上距离圆心最近的点
                float closestX = (std::max)(rectLeft, (std::min)(newPos.x, rectRight));
                float closestY = (std::max)(rectTop, (std::min)(newPos.y, rectBottom));

                // 2. 算距离平方
                float dx = newPos.x - closestX;
                float dy = newPos.y - closestY;
                float distSq = dx * dx + dy * dy;

                // 3. 最终判决：是否碰到墙壁？
                if (distSq < radius * radius) {
                    outHit = true;

                    if (distSq > 0.0001f) {
                        // 🟢 情况 A：正常撞击（圆心还在墙外）
                        float dist = sqrt(distSq);
                        float overlap = radius - dist;

                        // 正常挤出
                        // 原路挤出：把坐标往外推 overlap 这么多距离
                        newPos.x += (dx / dist) * overlap;
                        newPos.y += (dy / dist) * overlap;
                    }
                    else {
                        // 💥 情况 B：深度穿模（圆心已经完全陷进墙里了！）

                        // 算出圆心到墙壁左、右、上、下四条边的距离
                        float distLeft = newPos.x - rectLeft;
                        float distRight = rectRight - newPos.x;
                        float distTop = newPos.y - rectTop;
                        float distBottom = rectBottom - newPos.y;

                        // 找到距离最近的那条边
                        float minDist = distLeft;
                        int minAxis = 0; // 0:左, 1:右, 2:上, 3:下

                        if (distRight < minDist) { minDist = distRight; minAxis = 1; }
                        if (distTop < minDist) { minDist = distTop; minAxis = 2; }
                        if (distBottom < minDist) { minDist = distBottom; minAxis = 3; }

                        // 强行把主角从最近的边“拔”出来，并完美贴合在墙外！
                        float pushBias = 0.1f; // 💥 给一个 0.1 像素的安全间隙
                        if (minAxis == 0)      newPos.x = rectLeft - radius - pushBias;
                        else if (minAxis == 1) newPos.x = rectRight + radius + pushBias;
                        else if (minAxis == 2) newPos.y = rectTop - radius - pushBias;
                        else if (minAxis == 3) newPos.y = rectBottom + radius + pushBias;
                    }
                }
            }
        }
    }
    return newPos; // 返回被挤出后的安全坐标
}

bool Map::LoadLevel(const std::string& filename, Vector2D& outPlayerPos, std::vector<Vector2D>& outEnemySpawns) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // 如果文件路径写错了，直接报错提醒，防止游戏崩溃
        return false; 
    }

    std::string line;
    int r = 0; // 行号
    
    // ==========================================
    // 💥 修复作用域 Bug：直接向 grid 询问地图的高度和宽度！
    // grid.size() 是行数，grid[0].size() 是列数
    // ==========================================
    int maxRows = grid.size();
    int maxCols = grid[0].size();

    // 一行一行地把记事本里的内容读出来
    while (std::getline(file, line) && r < maxRows) {
        
        // 遍历这一行的每一个字符
        for (int c = 0; c < line.length() && c < maxCols; ++c) {
            
            char symbol = line[c];

            // 1. 遇到 '#'，填上物理墙壁
            if (symbol == '#') {
                grid[r][c].type = 1; 
            }
            // 2. 遇到 '.'，铺上空地板
            else {
                grid[r][c].type = 0; 
            }
            
            // 3. 遇到 'P'，记录下主角该站的像素坐标
            if (symbol == 'P') {
                outPlayerPos.x = c * tileSize + tileSize / 2.0f;
                outPlayerPos.y = r * tileSize + tileSize / 2.0f;
            }
            // 4. 遇到 'E'，把敌人的出生点存起来
            else if (symbol == 'E') {
                Vector2D enemyPos = { c * tileSize + tileSize / 2.0f, r * tileSize + tileSize / 2.0f };
                outEnemySpawns.push_back(enemyPos);
            }
        }
        r++; // 读完一行，行号加 1
    }
    
    file.close();
    return true;
}
