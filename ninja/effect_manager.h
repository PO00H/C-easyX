// --- day03:effect_manager.h ---
/*
特效
*/
#pragma once
#include "common.h"
#include <vector> // --- day04：引入动态数组库，用于管理粒子集合 ---


// --- day04：定义单个火花粒子的结构体 ---
struct Particle {
    Vector2D pos;      // 当前位置
    Vector2D velocity; // 速度
    int life;          // 剩余寿命
    int maxLife;       // 初始最大寿命（用来算渐变缩小）
};

// --- day07：定义“记忆残影”结构体 ---
struct MemoryEcho {
    Vector2D pos;    // 残影留下的位置
    float radius;    // 残影的大小
    int life;        // 剩余存活帧数
    int maxLife;     // 初始最大寿命（用于计算色彩渐变）
};


// 这个类专门负责各种酷炫的视觉反馈
class EffectManager 
{
private:
    // --- 震动系统 ---
    float shakeIntensity;
    int shakeTimer;

    // --- 顿帧系统 ---
    int hitStopFrames; // 还要卡顿多少帧？

    // --- day04：声明粒子集合容器 ---
	    // 用一个动态数组（std::vector）来存储所有活着的粒子，
        // 每当生成新粒子时，就往这个数组里添加；
        // 每帧更新时，遍历这个数组，更新每个粒子的状态，并删除那些寿命结束的粒子。
    std::vector<Particle> particles;

    // --- day07：残影百宝箱 ---
    std::vector<MemoryEcho> echoes;

public:
    EffectManager();

    // 每帧更新（处理特效衰减）
    void Update();

    // --- day04：渲染粒子集合 ---
    void DrawParticles();

    // 触发普通挥空特效（轻微震动）
    void PlayMissEffect();

    // 触发斩杀特效（剧烈震动 + 顿帧）
    // --- day04：修改斩杀特效接口，增加击中坐标参数以确定粒子生成位置 ---
    void PlayHitEffect(Vector2D hitPos);
    /*void PlayHitEffect();*/

    // --- day07：残影系统接口 ---
    void AddMemoryEcho(Vector2D pos, float radius); // 添加残影
    void DrawEchoes();                              // 渲染残影
    // --- day07：新增接口，用于在斩杀时打碎记忆 ---
    void ClearEchoes();


    // ============================================================
    // --- 特效查询接口 ---
    // 返回当前的震动偏移量，供导演调用 setorigin
    float GetShakeOffsetX() const;
    float GetShakeOffsetY() const;

    // 正在进行特效
    bool IsHitStopping() const;
};