#pragma once
#include <cstdint>
#include <unordered_set>

class EnemyCounter
{
public:

    static EnemyCounter& GetInstance()
    {
        static EnemyCounter instance;
        return instance;
    }

    // ó›êœï\é¶ëŒâû
    void ResetTotal()
    {
        m_totalHit = 0;
        m_totalKill = 0;
    }

    bool IsWaveActive() const { return m_waveTotal > 0; }

    void OnEnemyHitTotal() { ++m_totalHit; }
    void OnEnemyKilledTotal() { ++m_totalKill; }

    uint32_t GetHitCount()  const { return m_totalHit; }
    uint32_t GetKillCount() const { return m_totalKill; }

    // WaveèÓïÒ
    void StartWave(int totalEnemies);

    void RegisterEnemy(uint32_t id);
    void UnregisterEnemy(uint32_t id);

    void ResetWave();

    int  GetAliveCount()  const;
    int  GetWaveKilled()  const;
    int  GetWaveTotal()   const;
    bool IsWaveCleared()  const;

private:
    EnemyCounter() = default;

    // ó›êœ
    uint32_t m_totalHit = 0;
    uint32_t m_totalKill = 0;

    // Wave
    int m_waveTotal = 0;
    int m_waveKilled = 0;
    std::unordered_set<uint32_t> m_aliveIds;
};