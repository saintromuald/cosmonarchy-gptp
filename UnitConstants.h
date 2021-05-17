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
// - It's pretty annoying that project has to recompile most files after chaning this one. that was the case with Unit already,
//		but we might want to limit the include scope for this one
// - Maybe we should also move aise enums in here, I don't think it ever made sense for it to be in Unit, since it's included everywhere.

// Constants
// Note: I would recommend having unit name as first segment of any related variable name.

// COMMON / SHARED \\

// General
const s32 HealthMultiplier = 256;
const s32 StimpackOffset = 5 * 256;
const s32 StimpackTimer = 37;
const s32 MindControlEnergyGain = 5 * 256;

// Charlatan
const s32 CharlatanHallucinationTimer = 3 * 24;

// TERRAN \\

// Cleric
const s32 ClericHealValue = 107; // roughly 10 HP per second

// Shaman
const s32 ShamanHeal = 2;

// Madcap
const s32 MadcapPassiveDecayTime = 0.5 * 24;
const s32 MadcapMaxStacks = 10;
const f64 MadcapAttackSpeedPerStack = 0.05;

// Apostle
const s32 ApostleLazarusAgentHeal = 25 * 256;
const s32 ApostleBloodTitheHealing = 5 * 256;
const s32 ApostleLastBreathEnergyGain = 5 * 256;
const s32 ApostleAILastBreathTimer = 24;

// Mines
const s32 MineBurrowTime = 60;
const s32 MineSeekRange = 576;
const s32 MineDetonateRange = 30;
const s32 LobotomyMineLockdownTimer = 12; // 4 seconds

// Southpaw
const s32 SouthpawKnockoutDriversTimer = 24 * 2 + 1;

// Madrigal
const s32 MadrigalDirgeTimer = 48 + 1;
const s32 MadrigalDirgeDamage = 5;

// Wyvern
const s32 WyvernBaseAttackCooldown = 30;

// Seraph
const s32 SeraphObservanceDebuffTimer = 5 * 24 + 1;
const s32 SeraphIrradiateTimer = 10;

// Azazel
const s32 AzazelDefensiveMatrixTimer = 90; // roughly 30 seconds
const s32 AzazelDefensiveMatrixHeal = 53; // 5 HP per second | 256 * 5 / 24

// Centaur
const s32 CentaurMissileDistance = 4 * 32;

// Treasury
const s32 TreasuryIncrease = 4;

// PROTOSS \\

// Ecclesiast
const s32 EcclesiastAstralBlessingTimer = 5 * 24 + 1;

// Cantavis
const s32 CantavisHallucinationTimer = 10 * 24 + 1;

// Mind Tyrant
const s32 MindTyrantMaelstromTimer = 16; // 5 seconds

// Demiurge
const s32 GodsMadeFleshEnergyDrain = 100;

// Lanifect
const s32 AIWebSeekRange = 96;
const s32 AICorsairAllySeekRange = 12 * 32;
const s32 AICorsairEnemySeekRange = 4 * 32;
const f64 AICorsairLifePercentage = 0.5;
const s32 AIWebMinEnergy = 30;
const s32 AIWebAllyMinCount = 3;
const s32 AIWebEnemyMinCount = 2;
const s32 AIWebTimer = 3 * 24;

// Didact
const s32 DidactStasisFieldTimer = 35; // roughly 12 seconds

// Star Sovereign
const s32 AIStarsovereignAbilSeekRange = 6 * 32;
const s32 AIStarsovereignAbilMinEnemyHealth = 2000;
const s32 AIStarsovereignAbilMaxAllyHealth = 2000;
const f64 AIStarsovereignAbilLifePercentage = 0.5;
const s32 AIStarsovereignAbilTimer = 612;

// ZERG \\

// Zergling
const s32 ZerglingAdrenalGlandsTimer = 24 * 2 + 1;

// Vorvaling
const s32 VorvalingCaptivatingClawsTimer = 24 + 1;

// Quazilisk
const f64 QuazaliskPassiveFactor = 0.1;

// Vithrilisk
const s32 VithriliskCorrosiveTouchTimer = 10 * 24 + 1;
const f64 VithriliskCorrosiveTouchCooldownIncrease = 0.02;
const f64 VithriliskCorrosiveTouchArmorReduction = 0.2 * 256;

// Zoryusthaleth
const s32 ZoryusthalethReconstitutionHealing = 5 * 256;

// Isthrathaleth
const s32 IsthrathalethPlagueTimer = 30;

// Matraleth
const s32 DefaultArmParasiteTimer = 2 * 24 + 1;
const s32 MatralethAIParasiteTimer = 75;
const s32 MatralethAIEnsnaringBroodTimer = 75;
const s32 MatralethEnsnareTimer = 37;

// Akistrokor
const s32 AkistrokorEnthrallingAssaultHeal = 256 * 20 / 24; // 20 HP per second

// Alkajelisk
const s32 AlkajeliskParasiticSporesTimer = 135; // 135 is 255 - 5 * 24
const s32 AlkajeliskParasiticSporesSeekRange = 8 * 32;

// Larval Colony
const f64 LarvalColonyMultiplier = 0.75;