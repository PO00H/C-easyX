/******************************************************************************
 *
 * [引擎核心逻辑] MAP.CPP
 *
 * @desc : 地图系统的具体实现。彻底去除了魔法数字，全部引用 common.h 宏定义。
 *
 ******************************************************************************/
#include "map.h"
#include <graphics.h>
#include <math.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <queue>

 // ==========================================
 // ▶ 1. 生命周期与渲染
 // ==========================================
Map::Map() {
    grid.resize(MAP_ROWS, std::vector<Tile>(MAP_COLS, { 0, 0, DEFAULT_MEMORY }));
}

void Map::Update() {
    for (int r = 0; r < MAP_ROWS; ++r) {
        for (int c = 0; c < MAP_COLS; ++c) {
            if (grid[r][c].memoryTimer > 0) {
                grid[r][c].memoryTimer--;
            }
        }
    }
}

void Map::Draw() {
    for (int r = 0; r < MAP_ROWS; ++r) {
        for (int c = 0; c < MAP_COLS; ++c) {
            if (grid[r][c].type == 1 && grid[r][c].memoryTimer > 0) {
                float ratio = (float)grid[r][c].memoryTimer / grid[r][c].maxMemory;
                int colorValue = (int)(80 * ratio);

                setfillcolor(RGB(colorValue, colorValue, colorValue));
                setlinecolor(RGB(colorValue + 20, colorValue + 20, colorValue + 20));

                int x = c * TILE_SIZE;
                int y = r * TILE_SIZE;
                fillrectangle(x, y, x + TILE_SIZE, y + TILE_SIZE);
            }
        }
    }
}

// ==========================================
// ▶ 2. 核心传感器与物理
// ==========================================
void Map::ScanByRipple(Vector2D center, float rippleRadius) {
    int startC = (std::max)(0, (int)((center.x - rippleRadius) / TILE_SIZE));
    int endC = (std::min)(MAP_COLS - 1, (int)((center.x + rippleRadius) / TILE_SIZE));
    int startR = (std::max)(0, (int)((center.y - rippleRadius) / TILE_SIZE));
    int endR = (std::min)(MAP_ROWS - 1, (int)((center.y + rippleRadius) / TILE_SIZE));

    for (int r = startR; r <= endR; r++) {
        for (int c = startC; c <= endC; c++) {
            Vector2D tilePos(c * TILE_SIZE + TILE_SIZE / 2.0f, r * TILE_SIZE + TILE_SIZE / 2.0f);

            Vector2D diff = center - tilePos;
            float dist = sqrt(diff.x * diff.x + diff.y * diff.y);

            if (abs(dist - rippleRadius) < 15.0f) {
                grid[r][c].memoryTimer = DEFAULT_MEMORY;
                grid[r][c].maxMemory = DEFAULT_MEMORY;
            }
        }
    }
}

// --- day08：完美贴墙与物理挤出算法 ---
Vector2D Map::ResolveCollision(Vector2D pos, float radius, bool& outHit) {
    outHit = false;
    Vector2D newPos = pos;

    int startC = (std::max)(0, (int)((newPos.x - radius) / TILE_SIZE));
    int endC = (std::min)(MAP_COLS - 1, (int)((newPos.x + radius) / TILE_SIZE));
    int startR = (std::max)(0, (int)((newPos.y - radius) / TILE_SIZE));
    int endR = (std::min)(MAP_ROWS - 1, (int)((newPos.y + radius) / TILE_SIZE));

    for (int r = startR; r <= endR; r++) {
        for (int c = startC; c <= endC; c++) {

            if (grid[r][c].type == 1) {
                float rectLeft = c * TILE_SIZE;
                float rectRight = rectLeft + TILE_SIZE;
                float rectTop = r * TILE_SIZE;
                float rectBottom = rectTop + TILE_SIZE;

                float closestX = (std::max)(rectLeft, (std::min)(newPos.x, rectRight));
                float closestY = (std::max)(rectTop, (std::min)(newPos.y, rectBottom));

                float dx = newPos.x - closestX;
                float dy = newPos.y - closestY;
                float distSq = dx * dx + dy * dy;

                if (distSq < radius * radius) {
                    outHit = true;

                    if (distSq > 0.0001f) {
                        float dist = sqrt(distSq);
                        float overlap = radius - dist;

                        newPos.x += (dx / dist) * overlap;
                        newPos.y += (dy / dist) * overlap;
                    }
                    else {
                        float distLeft = newPos.x - rectLeft;
                        float distRight = rectRight - newPos.x;
                        float distTop = newPos.y - rectTop;
                        float distBottom = rectBottom - newPos.y;

                        float minDist = distLeft;
                        int minAxis = 0;

                        if (distRight < minDist) { minDist = distRight; minAxis = 1; }
                        if (distTop < minDist) { minDist = distTop; minAxis = 2; }
                        if (distBottom < minDist) { minDist = distBottom; minAxis = 3; }

                        float pushBias = 0.1f;
                        if (minAxis == 0)      newPos.x = rectLeft - radius - pushBias;
                        else if (minAxis == 1) newPos.x = rectRight + radius + pushBias;
                        else if (minAxis == 2) newPos.y = rectTop - radius - pushBias;
                        else if (minAxis == 3) newPos.y = rectBottom + radius + pushBias;
                    }
                }
            }
        }
    }
    return newPos;
}

// ==========================================
// ▶ 3. AI 导航基站 (BFS 广度优先搜索)
// ==========================================
Vector2D Map::GetBestDirection(Vector2D startPos, Vector2D targetPos) {
    int startR = (int)(startPos.y / TILE_SIZE);
    int startC = (int)(startPos.x / TILE_SIZE);
    int targetR = (int)(targetPos.y / TILE_SIZE);
    int targetC = (int)(targetPos.x / TILE_SIZE);

    // 计算直线向量
    float dx = targetPos.x - startPos.x;
    float dy = targetPos.y - startPos.y;
    float dist = sqrt(dx * dx + dy * dy);

    // 优化 1：如果在同一个格子里，或者距离极近，关闭 GPS，直接冲刺！
    if (dist < TILE_SIZE || (startR == targetR && startC == targetC)) {
        if (dist > 0) return Vector2D(dx / dist, dy / dist);
        return Vector2D(0, 0);
    }

    // 优化 2：越界保护
    if (startR < 0 || startR >= MAP_ROWS || startC < 0 || startC >= MAP_COLS ||
        targetR < 0 || targetR >= MAP_ROWS || targetC < 0 || targetC >= MAP_COLS) {
        if (dist > 0) return Vector2D(dx / dist, dy / dist);
        return Vector2D(0, 0);
    }

    // ----------------------------------------------------
    // 💥 核心：构建 BFS 热力图
    // ----------------------------------------------------
    std::vector<std::vector<int>> heatMap(MAP_ROWS, std::vector<int>(MAP_COLS, 9999));
    std::queue<std::pair<int, int>> q;

    // 从【玩家的格子】开始倒水
    heatMap[targetR][targetC] = 0;
    q.push({ targetR, targetC });

    // 索引：     0      1     2     3  |  4       5       6       7
    // 方向：     上     下    左    右  | 左上    右上    左下    右下
    int dr[] = { -1,    1,    0,    0, -1,     -1,      1,      1 }; // 行的变化
    int dc[] = { 0,    0,   -1,    1,  -1,      1,     -1,      1 }; // 列的变化

    while (!q.empty()) {
        auto curr = q.front();
        q.pop();
        int r = curr.first;
        int c = curr.second;

        // 如果水流已经蔓延到了敌人脚下，提前下班！
        if (r == startR && c == startC) break;

        for (int i = 0; i < 8; i++) {
            int nr = r + dr[i];
            int nc = c + dc[i];

            // 如果没越界，并且是空地板，并且还没被水淹过
            if (nr >= 0 && nr < MAP_ROWS && nc >= 0 && nc < MAP_COLS) {
                if (grid[nr][nc].type == 0 && heatMap[nr][nc] == 9999) {
                    heatMap[nr][nc] = heatMap[r][c] + 1; // 距离 + 1
                    q.push({ nr, nc });
                }
            }
        }
    }

    // ----------------------------------------------------
    // 💥 决策：查看敌人四周，寻找热力值最小的格子
    // ----------------------------------------------------
    int minHeat = heatMap[startR][startC]; // 默认是自己脚下的温度
    int bestR = startR, bestC = startC;

    for (int i = 0; i < 8; i++) {
        int nr = startR + dr[i];
        int nc = startC + dc[i];
        if (nr >= 0 && nr < MAP_ROWS && nc >= 0 && nc < MAP_COLS) {
            // 如果旁边格子的温度（距离）更低，且不是死路
            if (grid[nr][nc].type == 0 && heatMap[nr][nc] < minHeat) {
                minHeat = heatMap[nr][nc];
                bestR = nr;
                bestC = nc;
            }
        }
    }

    // 算出指向最佳格子中心点的向量
    Vector2D bestTarget(bestC * TILE_SIZE + TILE_SIZE / 2.0f, bestR * TILE_SIZE + TILE_SIZE / 2.0f);
    Vector2D moveDir = bestTarget - startPos;
    float moveLen = sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);

    if (moveLen > 0) {
        return Vector2D(moveDir.x / moveLen, moveDir.y / moveLen);
    }

    // 兜底方案：如果寻路失败，回退到直线追踪
    if (dist > 0) return Vector2D(dx / dist, dy / dist);
    return Vector2D(0, 0);
}

// ==========================================
// ▶ 3. 外部接口 (I/O)
// ==========================================
bool Map::LoadLevel(const std::string& filename, Vector2D& outPlayerPos, std::vector<Vector2D>& outEnemySpawns) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    int r = 0;

    while (std::getline(file, line) && r < MAP_ROWS) {
        for (int c = 0; c < line.length() && c < MAP_COLS; ++c) {

            char symbol = line[c];

            if (symbol == '#') {
                grid[r][c].type = 1;
            }
            else {
                grid[r][c].type = 0;
            }

            if (symbol == 'P') {
                outPlayerPos.x = c * TILE_SIZE + TILE_SIZE / 2.0f;
                outPlayerPos.y = r * TILE_SIZE + TILE_SIZE / 2.0f;
            }
            else if (symbol == 'E') {
                Vector2D enemyPos = { c * TILE_SIZE + TILE_SIZE / 2.0f, r * TILE_SIZE + TILE_SIZE / 2.0f };
                outEnemySpawns.push_back(enemyPos);
            }
        }
        r++;
    }

    file.close();
    return true;
}