#pragma once
#pragma warning(disable: 4455)

// This file is meant to gather constants used for unit behaviors.
// It's primary role is to reduce amounts of bugs which come from partial code editing,
// by removing the need to hard code same "magic numbers" in multiple places of the codebase.
// And also to make it easier to edit and manage game variables in general.
//
// Obviously for now not many things are using it, but I would recommend moving things here as time passes.
// ~ Veeq7

// Notes:
// - It's pretty annoying that project has to recompile most files after chaning this one. that was the case with CUnit already,
//		but we might want to limit the include scope for this one
// - Maybe we should also move aise enums in here, I don't think it ever made sense for it to be in CUnit, since it's included everywhere.

// Constants
// Note: I would recommend having unit name as first segment of any related variable name.

// General
constexpr int HealthMultiplier = 256;

// Shaman
constexpr int ShamanHeal = 2;

// Madcap
constexpr int MadcapPassiveDecayTime = 0.5 * 24;
constexpr int MadcapMaxStacks = 10;
constexpr double MadcapAttackSpeedPerStack = 0.05;

// Apostle
constexpr int ApostleBloodTitheHealing = 5 * 256;
constexpr int ApostleLastBreathEnergyGain = 5 * 256;

// Mines
const int MineBurrowTime = 60;
const int MineSeekRange = 576;
const int MineDetonateRange = 30;

// Azazel
constexpr int AzazelDefensiveMatrixHeal = 53; // 5 HP per second | 256 * 5 / 24

// Treasury
constexpr int TreasuryIncrease = 4;

// Lanifect
constexpr int AIWebSeekRange = 96;
constexpr int AICorsairAllySeekRange = 128;
constexpr int AICorsairEnemySeekRange = 128;
constexpr double AICorsairLifePercentage = 0.5;
constexpr int AIWebMinEnergy = 30;
constexpr int AIWebAllyMinCount = 3;
constexpr int AIWebEnemyMinCount = 2;
constexpr int AIWebTimer = 3 * 24;

// Star Sovereign
constexpr int AIStarsovereignAbilSeekRange = 10 * 32;
constexpr int AIStarsovereignAbilMinEnemyHealth = 2000;
constexpr int AIStarsovereignAbilMaxAllyHealth = 2000;
constexpr double AIStarsovereignAbilLifePercentage = 0.5;
constexpr int AIStarsovereignAbilTimer = 612;

// Quazilisk
constexpr double QuazaliskPassiveFactor = 0.1;

// Vithrilisk
constexpr int VithriliskCorrosiveTouchTimer = 10 * 24 + 1;
constexpr double VithriliskCorrosiveTouchCooldownIncrease = 0.02;
constexpr double VithriliskCorrosiveTouchArmorReduction = 0.2 * 256;

// Zoryusthaleth
constexpr int ZoryusthalethReconstitutionHealing = 5 * 256;

// Matraleth
constexpr int DefaultArmParasiteTimer = 2 * 24 + 1;

// Akistrokor
constexpr int AkistrokorEnthrallingAssaultHeal = 256 * 20 / 24; // 20 HP per second

// Larval Colony
constexpr double LarvalColonyMultiplier = 0.75;