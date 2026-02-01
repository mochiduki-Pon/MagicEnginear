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

    // —İÏ•\¦‘Î‰
    void ResetTotal()
    {
        m_totalHit = 0;
        m_totalKill = 0;
    }
    void OnEnemyHitTotal() { ++m_totalHit; }
    void OnEnemyKilledTotal() { ++m_totalKill; }

    uint32_t GetHitCount()  const { return m_totalHit; }
    uint32_t GetKillCount() const { return m_totalKill; }

    // Waveî•ñ
    void StartWave(int totalEnemies);

    void RegisterEnemy(uint32_t id);
    void UnregisterEnemy(uint32_t id);

    int  GetAliveCount()  const;
    int  GetWaveKilled()  const;
    int  GetWaveTotal()   const;
    bool IsWaveCleared()  const;

private:
    EnemyCounter() = default;

    // —İÏ
    uint32_t m_totalHit = 0;
    uint32_t m_totalKill = 0;

    // Wave
    int m_waveTotal = 0;
    int m_waveKilled = 0;
    std::unordered_set<uint32_t> m_aliveIds;
};