// map.cpp
#include "map.h"
#include <graphics.h>
#include <math.h>
#include <algorithm>

Map::Map() {
    tileSize = 40;
    int cols = 800 / tileSize; // 20 列
    int rows = 600 / tileSize; // 15 行

    // 初始化网格，所有瓦片默认都是地板(0)，且没有记忆(timer = 0)
    grid.resize(rows, std::vector<Tile>(cols, { 0, 0, 300 }));

    // --- day06&07：设计迷宫（设置 type = 1 为墙壁） ---
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // 四周变成墙壁封锁线
            if (r == 0 || r == rows - 1 || c == 0 || c == cols - 1) {
                grid[r][c].type = 1;
            }
        }
    }
    // 中间的<测试>墙体
    grid[5][5].type = 1; grid[6][5].type = 1; grid[7][5].type = 1;
    grid[10][12].type = 1; grid[10][13].type = 1; grid[10][14].type = 1;
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