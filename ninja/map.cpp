/******************************************************************************
 *
 * [引擎核心逻辑] MAP.CPP
 *
 * @desc : 地图系统的具体实现。Day 13 攻克了“竹林不复位”与“灯具连锁黑暗”逻辑。
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
 // ▶ 1. 生命周期与状态管理
 // ==========================================
Map::Map() {
    grid.resize(MAP_ROWS, std::vector<Tile>(MAP_COLS, { 0, 0, DEFAULT_MEMORY }));
    isAreaDark = false;
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

// 💥 Day13：利用快照实现瞬间重置
void Map::ResetMap() {
    grid = initialGrid; // 结构化拷贝
    isAreaDark = false; // 恢复光明
}

void Map::Draw() {
    for (int r = 0; r < MAP_ROWS; ++r) {
        for (int c = 0; c < MAP_COLS; ++c) {
            if (grid[r][c].type >= 1 && grid[r][c].memoryTimer > 0) {
                float ratio = (float)grid[r][c].memoryTimer / grid[r][c].maxMemory;

                if (grid[r][c].type == 1) { // 砖墙：灰
                    int colorValue = (int)(80 * ratio);
                    setfillcolor(RGB(colorValue, colorValue, colorValue));
                    setlinecolor(RGB(colorValue + 20, colorValue + 20, colorValue + 20));
                }
                else if (grid[r][c].type == 2) { // 竹林：绿
                    int g = (int)(100 * ratio);
                    int rb = (int)(30 * ratio);
                    setfillcolor(RGB(rb, g, rb));
                    setlinecolor(RGB(rb + 20, g + 20, rb + 20));
                }
                else if (grid[r][c].type == 3) { // 灯具：橙
                    int r_col = (int)(120 * ratio);
                    int g_col = (int)(60 * ratio);
                    setfillcolor(RGB(r_col, g_col, 0));
                    setlinecolor(RGB(r_col + 20, g_col + 20, 0));
                }

                int x = c * TILE_SIZE;
                int y = r * TILE_SIZE;
                fillrectangle(x, y, x + TILE_SIZE, y + TILE_SIZE);
            }
        }
    }
}

// ==========================================
// ▶ 2. 核心传感器与物理运算
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

Vector2D Map::ResolveCollision(Vector2D pos, float radius, bool& outHit, bool isDashing) {
    outHit = false;
    Vector2D newPos = pos;

    int startC = (std::max)(0, (int)((newPos.x - radius) / TILE_SIZE));
    int endC = (std::min)(MAP_COLS - 1, (int)((newPos.x + radius) / TILE_SIZE));
    int startR = (std::max)(0, (int)((newPos.y - radius) / TILE_SIZE));
    int endR = (std::min)(MAP_ROWS - 1, (int)((newPos.y + radius) / TILE_SIZE));

    for (int r = startR; r <= endR; r++) {
        for (int c = startC; c <= endC; c++) {
            int tileType = grid[r][c].type;

            // 物理过滤：一闪状态下可以穿透竹林 (type 2)
            if(tileType == 1 || ((tileType == 2 || tileType == 3) && !isDashing)) {
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
                }
            }
        }
    }
    return newPos;
}

// ==========================================
// ▶ 3. AI 导航寻路 (BFS 算法)
// ==========================================
Vector2D Map::GetBestDirection(Vector2D startPos, Vector2D targetPos) {
    int startR = (int)(startPos.y / TILE_SIZE), startC = (int)(startPos.x / TILE_SIZE);
    int targetR = (int)(targetPos.y / TILE_SIZE), targetC = (int)(targetPos.x / TILE_SIZE);

    float dx = targetPos.x - startPos.x, dy = targetPos.y - startPos.y;
    float dist = sqrt(dx * dx + dy * dy);

    if (dist < TILE_SIZE || (startR == targetR && startC == targetC)) {
        if (dist > 0) return Vector2D(dx / dist, dy / dist);
        return Vector2D(0, 0);
    }

    std::vector<std::vector<int>> heatMap(MAP_ROWS, std::vector<int>(MAP_COLS, 9999));
    std::queue<std::pair<int, int>> q;

    heatMap[targetR][targetC] = 0;
    q.push({ targetR, targetC });

    int dr[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
    int dc[] = { 0, 0, -1, 1, -1, 1, -1, 1 };

    while (!q.empty()) {
        auto curr = q.front(); q.pop();
        int r = curr.first, c = curr.second;

        if (r == startR && c == startC) break;

        for (int i = 0; i < 8; i++) {
            int nr = r + dr[i], nc = c + dc[i];
            if (nr >= 0 && nr < MAP_ROWS && nc >= 0 && nc < MAP_COLS) {
                // BFS 路径只考虑非墙体格子
                if (grid[nr][nc].type == 0 && heatMap[nr][nc] == 9999) {
                    heatMap[nr][nc] = heatMap[r][c] + 1;
                    q.push({ nr, nc });
                }
            }
        }
    }

    int minHeat = heatMap[startR][startC];
    int bestR = startR, bestC = startC;

    for (int i = 0; i < 8; i++) {
        int nr = startR + dr[i], nc = startC + dc[i];
        if (nr >= 0 && nr < MAP_ROWS && nc >= 0 && nc < MAP_COLS) {
            if (grid[nr][nc].type == 0 && heatMap[nr][nc] < minHeat) {
                minHeat = heatMap[nr][nc];
                bestR = nr; bestC = nc;
            }
        }
    }

    Vector2D bestTarget(bestC * TILE_SIZE + TILE_SIZE / 2.0f, bestR * TILE_SIZE + TILE_SIZE / 2.0f);
    Vector2D moveDir = bestTarget - startPos;
    float moveLen = sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);

    return (moveLen > 0) ? Vector2D(moveDir.x / moveLen, moveDir.y / moveLen) : Vector2D(dx / dist, dy / dist);
}

// ==========================================
// ▶ 4. 外部接口与交互逻辑
// ==========================================
bool Map::LoadLevel(const std::string& filename, Vector2D& outPlayerPos, std::vector<Vector2D>& outEnemySpawns) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    int r = 0;
    while (std::getline(file, line) && r < MAP_ROWS) {
        for (int c = 0; c < line.length() && c < MAP_COLS; ++c) {
            char symbol = line[c];
            if (symbol == '#') grid[r][c].type = 1;
            else if (symbol == 'B') grid[r][c].type = 2;
            else if (symbol == 'L') grid[r][c].type = 3;
            else grid[r][c].type = 0;

            if (symbol == 'P') {
                outPlayerPos = { c * TILE_SIZE + TILE_SIZE / 2.0f, r * TILE_SIZE + TILE_SIZE / 2.0f };
            }
            else if (symbol == 'E') {
                outEnemySpawns.push_back({ c * TILE_SIZE + TILE_SIZE / 2.0f, r * TILE_SIZE + TILE_SIZE / 2.0f });
            }
        }
        r++;
    }
    file.close();

    // 💥 Day13：载入成功后，立即保存初始快照
    initialGrid = grid;
    isAreaDark = false;
    return true;
}

void Map::SlashBamboo(Vector2D slashStart, Vector2D slashEnd) {
    float dx = slashEnd.x - slashStart.x, dy = slashEnd.y - slashStart.y;
    float distance = sqrt(dx * dx + dy * dy);
    if (distance <= 0.001f) return;

    float dirX = dx / distance, dirY = dy / distance;
    float step = TILE_SIZE / 2.0f;

    for (float currentDist = 0; currentDist <= distance; currentDist += step) {
        int c = (int)((slashStart.x + dirX * currentDist) / TILE_SIZE);
        int r = (int)((slashStart.y + dirY * currentDist) / TILE_SIZE);

        if (r >= 0 && r < MAP_ROWS && c >= 0 && c < MAP_COLS) {
            // 💥 Day13：同时支持斩断竹林(2)与击碎灯具(3)
            if (grid[r][c].type == 2 || grid[r][c].type == 3) {
                if (grid[r][c].type == 3) isAreaDark = true; // 熄灯触发！
                grid[r][c].type = 0;
                grid[r][c].memoryTimer = DEFAULT_MEMORY;
            }
        }
    }
}