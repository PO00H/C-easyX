/******************************************************************************
 *
 * [引擎公共字典] COMMON.H  -- (全局基石)
 *
 * @desc  : 存放全游戏通用的数学结构体、全局常量与枚举。所有底层模块均可无副作用引入。
 * 头文件里只放“规则（图纸）”，绝对不能放“实体（变量）”
 *
 *=============================================================================
 * [ 数据结构地图 ]
 *
 * ▶ 1. 核心数学库
 * Vector2D        : 二维向量结构体，支持基础的矢量加减法运算。
 * * ▶ 2. 游戏全局宏
 * GAME_WIDTH      : 游戏窗口基准宽度 (800)
 * GAME_HEIGHT     : 游戏窗口基准高度 (600)
 *
 ******************************************************************************/
#pragma once

 // ==========================================
 // [ 全局常量宏 ] 
 // ==========================================
#define GAME_WIDTH  800
#define GAME_HEIGHT 600
#define PI          3.1415926535f
// ==========================================
// [ 地图空间规则 ] 
// ==========================================
#define TILE_SIZE       40      // 瓦片大小 (800/20, 600/15)
#define MAP_COLS        20      // 地图横向瓦片数
#define MAP_ROWS        15      // 地图纵向瓦片数
#define DEFAULT_MEMORY  60     // 默认波纹记忆时长 (约 2– 秒)

// ==========================================
// [ 世界瓦片定义 ] 
// ==========================================
struct Tile {
    int type;           // 0: 地板, 1: 墙壁
    int memoryTimer;    // 当前记忆倒计时
    int maxMemory;      // 记忆上限 (用于计算渐变衰减)
};

// ==========================================
// [ 核心数学结构体 ] 
// ==========================================
struct Vector2D {
    float x;
    float y;

    // --- 默认构造函数 (初始化为 0,0) ---
    Vector2D() : x(0.0f), y(0.0f) {}

    // --- 带参构造函数 ---
    Vector2D(float _x, float _y) : x(_x), y(_y) {}

    // 💥 架构师魔法：运算符重载 (让向量支持直接加减！)
    Vector2D operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }

    Vector2D operator-(const Vector2D& other) const {
        return Vector2D(x - other.x, y - other.y);
    }

    Vector2D operator*(float scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }
};

// ==========================================
// [ 特效系统调控台 ] 
// ==========================================
#define FX_MISS_SHAKE_POWER   8.0f    // 挥空：震动强度
#define FX_MISS_SHAKE_TIME    5       // 挥空：震动帧数
#define FX_HIT_SHAKE_POWER    15.0f   // 斩杀：剧烈震动强度
#define FX_HIT_SHAKE_TIME     15      // 斩杀：剧烈震动帧数
#define FX_HIT_STOP_FRAMES    6       // 斩杀：顿帧卡顿时间

#define FX_SPARK_COUNT        50      // 斩杀火花：生成数量
#define FX_SPARK_FRICTION     0.85f   // 斩杀火花：空气阻力
#define FX_ECHO_LIFE          60      // 残影：存活帧数
#define FX_RIPPLE_LIFE        45      // 波纹：存活帧数
#define FX_RIPPLE_START_RAD   5.0f    // 波纹：初始爆炸半径
#define FX_RIPPLE_DAMPING     0.08f   // 波纹：扩散阻尼(越小越粘稠)