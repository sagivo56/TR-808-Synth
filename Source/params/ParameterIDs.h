#pragma once

// ===========================================================================
// Central registry of every APVTS parameter ID.
//
// These string IDs are the contract with the host: automation lanes and saved
// sessions key off them, so once shipped they must stay stable. New voices /
// sequencer params are added here in later milestones (M2+).
// ===========================================================================
namespace ParamIDs
{
    inline constexpr auto masterGain  = "master_gain";
    inline constexpr auto masterDrive = "master_drive";
    inline constexpr auto multiOut    = "multi_out";
    inline constexpr auto accentLevel = "accent_level";

    // Melodic BD Bass voice controls.
    inline constexpr auto bassLevel    = "bdbass_level";
    inline constexpr auto bassTone     = "bdbass_tone";
    inline constexpr auto bassDecay    = "bdbass_decay";
    inline constexpr auto bassPunch    = "bdbass_punch";
    inline constexpr auto bassDrive    = "bdbass_drive";
    inline constexpr auto bassDuckBd   = "bdbass_duck_bd";   // bass ducks the regular BD

    // Parallel FX (global). Per-voice sends use macroId(v, "revsend"/"dlysend").
    inline constexpr auto bassRevSend  = "bdbass_revsend";
    inline constexpr auto bassDlySend  = "bdbass_dlysend";
    inline constexpr auto revPredelay  = "rev_predelay";
    inline constexpr auto revDecay     = "rev_decay";
    inline constexpr auto revBass      = "rev_bass";
    inline constexpr auto revCrossover = "rev_crossover";
    inline constexpr auto revDamp      = "rev_damp";
    inline constexpr auto revDepth     = "rev_depth";
    inline constexpr auto revReturn    = "rev_return";
    inline constexpr auto dlyTime      = "dly_time";
    inline constexpr auto dlyFeedback  = "dly_feedback";
    inline constexpr auto dlyTone      = "dly_tone";
    inline constexpr auto dlyReturn    = "dly_return";
}
