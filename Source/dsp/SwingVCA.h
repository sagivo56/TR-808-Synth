#pragma once

namespace tr808::dsp
{
//==============================================================================
// Behavioural model of the TR-808 "swing type VCA" (service manual Fig. 12): a
// transistor VCA whose diode gives an asymmetric transfer, adding even-harmonic
// content for the ringing metallic character. The series capacitor (AC coupling)
// is modelled by a DC blocker so the asymmetry doesn't introduce a thump.
// Header-only and stateful (one instance per voice).
//==============================================================================
class SwingVCA
{
public:
    void  reset() noexcept { x1 = 0.0f; y1 = 0.0f; }
    void  setAmount (float a) noexcept { amount = a < 0.0f ? 0.0f : (a > 1.0f ? 1.0f : a); }

    float process (float sig) noexcept
    {
        // Diode: the negative half is attenuated -> asymmetry -> even harmonics.
        const float shaped = sig >= 0.0f ? sig : sig * (1.0f - amount);
        // DC blocker (y = x - x1 + R*y1) = the circuit's AC coupling.
        const float y = shaped - x1 + 0.995f * y1;
        x1 = shaped;
        y1 = y;
        return y;
    }

private:
    float amount = 0.5f, x1 = 0.0f, y1 = 0.0f;
};
}
