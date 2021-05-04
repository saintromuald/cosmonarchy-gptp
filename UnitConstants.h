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
constexpr s32 HealthMultiplier = 256;

// Shaman
constexpr s32 ShamanHeal = 2;

// Madcap
constexpr s32 MadcapPassiveDecayTime = 0.5 * 24;
constexpr s32 MadcapMaxStacks = 10;
constexpr f64 MadcapAttackSpeedPerStack = 0.05;

// Apostle
constexpr s32 ApostleBloodTitheHealing = 5 * 256;
constexpr s32 ApostleLastBreathEnergyGain = 5 * 256;

// Mines
const s32 MineBurrowTime = 60;
const s32 MineSeekRange = 576;
const s32 MineDetonateRange = 30;
const s32 LobotomyMineLockdownTime = 12; // 4 seconds

// Azazel
constexpr s32 AzazelDefensiveMatrixHeal = 53; // 5 HP per second | 256 * 5 / 24

// Treasury
constexpr s32 TreasuryIncrease = 4;

// Demiurge
constexpr s32 GodsMadeFleshEnergyDrain = 100;

// Lanifect
constexpr s32 AIWebSeekRange = 96;
constexpr s32 AICorsairAllySeekRange = 128;
constexpr s32 AICorsairEnemySeekRange = 128;
constexpr f64 AICorsairLifePercentage = 0.5;
constexpr s32 AIWebMinEnergy = 30;
constexpr s32 AIWebAllyMinCount = 3;
constexpr s32 AIWebEnemyMinCount = 2;
constexpr s32 AIWebTimer = 3 * 24;

// Star Sovereign
constexpr s32 AIStarsovereignAbilSeekRange = 10 * 32;
constexpr s32 AIStarsovereignAbilMinEnemyHealth = 2000;
constexpr s32 AIStarsovereignAbilMaxAllyHealth = 2000;
constexpr f64 AIStarsovereignAbilLifePercentage = 0.5;
constexpr s32 AIStarsovereignAbilTimer = 612;

// Quazilisk
constexpr f64 QuazaliskPassiveFactor = 0.1;

// Vithrilisk
constexpr s32 VithriliskCorrosiveTouchTimer = 10 * 24 + 1;
constexpr f64 VithriliskCorrosiveTouchCooldownIncrease = 0.02;
constexpr f64 VithriliskCorrosiveTouchArmorReduction = 0.2 * 256;

// Zoryusthaleth
constexpr s32 ZoryusthalethReconstitutionHealing = 5 * 256;

// Matraleth
constexpr s32 DefaultArmParasiteTimer = 2 * 24 + 1;

// Akistrokor
constexpr s32 AkistrokorEnthrallingAssaultHeal = 256 * 20 / 24; // 20 HP per second

// Larval Colony
constexpr f64 LarvalColonyMultiplier = 0.75;