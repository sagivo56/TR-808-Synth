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
}
