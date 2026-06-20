#include "Envelope.h"
#include <cmath>
#include <algorithm>

namespace tr808::dsp
{
void Envelope::prepare (double sr)
{
    sampleRate = sr;
    setAttack (attackMs);
    setDecay (decayMs);
    reset();
}

void Envelope::reset()
{
    stage = Stage::idle;
    level = 0.0f;
}

void Envelope::setAttack (float ms) noexcept
{
    attackMs = std::max (0.0f, ms);
    const double samples = std::max (1.0, (double) attackMs * 0.001 * sampleRate);
    attackInc = (float) (1.0 / samples);
}

void Envelope::setDecay (float ms) noexcept
{
    decayMs = std::max (1.0f, ms);
    const double samples = std::max (1.0, (double) decayMs * 0.001 * sampleRate);
    // Multiplier that takes the level from 1 down to floorLevel over 'decayMs'.
    decayMul   = (float) std::exp (std::log ((double) floorLevel) / samples);
    releaseMul = decayMul;
}

void Envelope::trigger() noexcept
{
    stage = Stage::attack;          // keep current 'level' for click-free retrigger
}

void Envelope::noteOff() noexcept
{
    if (mode == Mode::ar && stage != Stage::idle)
        stage = Stage::release;
}

void Envelope::forceRelease (float ms) noexcept
{
    if (stage == Stage::idle)
        return;

    const double samples = std::max (1.0, (double) std::max (0.1f, ms) * 0.001 * sampleRate);
    releaseMul = (float) std::exp (std::log ((double) floorLevel) / samples);
    stage = Stage::release;
}

float Envelope::processSample() noexcept
{
    switch (stage)
    {
        case Stage::idle:
            return 0.0f;

        case Stage::attack:
            level += attackInc;
            if (level >= 1.0f)
            {
                level = 1.0f;
                stage = (mode == Mode::ad ? Stage::decay : Stage::sustain);
            }
            break;

        case Stage::decay:
            level *= decayMul;
            if (level <= floorLevel) { level = 0.0f; stage = Stage::idle; }
            break;

        case Stage::sustain:
            level = 1.0f;
            break;

        case Stage::release:
            level *= releaseMul;
            if (level <= floorLevel) { level = 0.0f; stage = Stage::idle; }
            break;
    }

    return level;
}
}
