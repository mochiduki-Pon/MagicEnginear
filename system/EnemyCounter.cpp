#include "EnemyCounter.h"
#include <cassert>
#include <iostream>

//debug—p
//waveî•ñ‚Ì•Û‚Æ•\¦
void EnemyCounter::StartWave(int totalEnemies)
{
    m_waveTotal = totalEnemies;
    m_waveKilled = 0;
    m_aliveIds.clear();
    m_aliveIds.reserve((size_t)totalEnemies);

#ifdef _DEBUG
    std::cout << "[Wave Start] total=" << m_waveTotal << std::endl;
#endif
}

// Enemy‚Ì“o˜^î•ñ‚Æ‘‚«o‚µ
void EnemyCounter::RegisterEnemy(uint32_t id)
{
    auto [it, inserted] = m_aliveIds.insert(id);
    assert(inserted && "Enemy registered twice!");

#ifdef _DEBUG
    std::cout << "[Enemy Registered]"
        << " id=" << id
        << " alive=" << m_aliveIds.size()
        << std::endl;
#endif
}

// Enemy‚Ì“o˜^‰ğœî•ñ‚Æ‘‚«o‚µ
void EnemyCounter::UnregisterEnemy(uint32_t id)
{
    size_t erased = m_aliveIds.erase(id);
    assert(erased == 1 && "Enemy unregistered twice or unknown id!");

    ++m_waveKilled;

#ifdef _DEBUG
    std::cout << "[Enemy Killed]"
        << " id=" << id
        << " alive=" << m_aliveIds.size()
        << " killed=" << m_waveKilled
        << "/" << m_waveTotal
        << std::endl;
#endif
}

void EnemyCounter::ResetWave()
{
    m_waveTotal = 0;
    m_waveKilled = 0;
    m_aliveIds.clear();
}

int EnemyCounter::GetAliveCount() const { return (int)m_aliveIds.size(); }

int EnemyCounter::GetWaveKilled() const { return m_waveKilled; }

int EnemyCounter::GetWaveTotal() const { return m_waveTotal; }

bool EnemyCounter::IsWaveCleared() const { return (m_waveKilled >= m_waveTotal) && m_aliveIds.empty(); }