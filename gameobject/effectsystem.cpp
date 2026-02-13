#include	"effectsystem.h"	
#include	"../system/commontypes.h"
#include	"../system/meshmanager.h"
#include	<array>
#include	<iostream>
#include	"player.h"


void EffectSystem::init()
{
    exploding = false;
    lifeMs = 0.0f;
    maxSpLifeMs = 300.0f;
    maxSpRadius = 200.0f;
    color = Color(0.5f, 0.7f, 1, 0.8f);
    pos = Vector3(0, 0, 0);
}

static float Clamp01(float t)
{
    if (t < 0.0f) return 0.0f;
    if (t > 1.0f) return 1.0f;
    return t;
}

void EffectSystem::StartExplosion(const Vector3& p)
{
    pos = p;

    m_exStart = Time::Get().Now();
    exploding = true;
}

void EffectSystem::update(uint64_t)
{
    if (!exploding) return;

    lifeMs = (float)Time::ElapsedMs(m_exStart);

    if (lifeMs >= maxSpLifeMs)
    {
        exploding = false;
        lifeMs = maxSpLifeMs;
    }
}

void EffectSystem::draw(uint64_t)
{
    if (!exploding) return;

    const float t = Clamp01(lifeMs / maxSpLifeMs);
    const float r = maxSpRadius * t;

    Color c = color;
    c.w = (1.0f - t) * color.w;

    SphereDrawerDraw(r, c, pos.x, pos.y, pos.z);
}

void EffectSystem::dispose() {
}
