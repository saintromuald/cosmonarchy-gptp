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
constexpr s32 HealthMultiplier = 256;
constexpr s32 StimpackOffset = 5 * 256;
constexpr s32 StimpackTimer = 14;
constexpr s32 MindControlEnergyGain = 5 * 256;
constexpr u32 NeutralHostilePlayerId = 9;

// Factional
constexpr s32 AxitriliskShieldLossSFXTimer = 24 * 6 + 1;

// TERRAN \\

// Cleric
constexpr s32 ClericHealValue = 96; // 9 HP per second

// Shaman
constexpr s32 ShamanHeal = 4;

// Madcap
constexpr s32 MadcapPassiveDecayTime = 0.5 * 24;
constexpr s32 MadcapMaxStacks = 10;

// Apostle
constexpr s32 ApostleLastBreathEnergyGain = 5 * 256;
constexpr s32 ApostleAILazarusAgentTimer = 24;

// Olympian
constexpr s32 OlympianMeleeDistance = 30;

// Hypnagogue
constexpr u32 ThroughTheLookingGlassRange = 6 * 32;

// Mines
constexpr s32 MineBurrowTime = 72;
constexpr s32 MineSeekRange = 576;
constexpr s32 MineDetonateRange = 30;
constexpr s32 LobotomyMineLockdownTimer = 9; // 3 seconds

// Cuirass
constexpr u32 CuirassSelfReviveHP = 50 * 256;
constexpr s32 CuirassSelfHeal = 160; // (15 * 256) / 24 = 160

// Madrigal
constexpr s32 MadrigalDirgeTimer = 48 + 1;
constexpr s32 MadrigalDirgeDamage = 5;

// Wyvern
constexpr s32 WyvernBaseAttackCooldown = 30;

// Seraph
constexpr s32 SeraphObservanceDebuffTimer = 5 * 24 + 1;
constexpr s32 SeraphIrradiateTimer = 10;

// Azazel
constexpr s32 AzazelDefensiveMatrixTimer = 30; // roughly 10 seconds
constexpr s32 AzazelDefensiveMatrixHeal = 107; // ~10 HP per second | 256 * 5 / 24

// Centaur
constexpr s32 CentaurMissileDistance = 4 * 32;

// Reservoir / Addon
constexpr s32 EmergencyDetonationTimer = 5 * 24 + 1;
constexpr s32 ControlledDemolitionTimer = 10 * 24 + 1;

// Treasury
constexpr s32 TreasuryIncrease = 8;

// PROTOSS \\

// Ecclesiast
constexpr s32 EcclesiastAstralBlessingTimer = 5 * 24 + 1;

// Cantavis
constexpr s32 CantavisHallucinatingFieldTimer = 10 * 24 + 1;

// Archon
constexpr s32 ArchonAstralAegisShieldHeal = 50 * 256;

// Mind Tyrant
constexpr s32 MindTyrantMaelstromTimer = 16; // 5 seconds

// Demiurge
constexpr s32 GodsMadeFleshEnergyDrain = 100;

// Lanifect
constexpr s32 AILanifectCheckRange = 3 * 32;
constexpr s32 AILanifectSeekRange = 8 * 32;
constexpr s32 AIWebTimer = 1 * 24;

// Didact
constexpr s32 DidactStasisFieldTimer = 23; // roughly 8 seconds

// Star Sovereign
constexpr s32 AIStarSovereignAbilSeekRange = 6 * 32;
constexpr s32 AIStarSovereignAbilMinEnemyHealth = 2000;
constexpr s32 AIStarSovereignAbilMaxAllyHealth = 2000;
constexpr f64 AIStarSovereignAbilLifePercentage = 0.5;
constexpr s32 AIStarSovereignAbilTimer = 612;

// ZERG \\

// Zethrokor
constexpr s32 ZethrokorAdrenalGlandsTimer = 24 * 2 + 1;

// Quazilisk
constexpr f64 QuazaliskPassiveFactor = 0.1;

// Nathrokor
constexpr s32 NathrokorAdrenalFrenzyTimer = 2 * 24 + 1;
constexpr f64 NathrokorAdrenalFrenzySpeedFactor = 0.01;

// Vorvrokor
constexpr s32 VorvrokorCaptivatingClawsTimer = 24 + 1;

// Vithrilisk
constexpr s32 VithriliskCorrosiveTouchTimer = 5 * 24 + 1;
constexpr f64 VithriliskCorrosiveTouchCooldownIncrease = 0.05;
constexpr f64 VithriliskCorrosiveTouchArmorReduction = 1 * 256;

// Zoryusthaleth
constexpr s32 ZoryusthalethReconstitutionHealing = 5 * 256;

// Isthrathaleth
constexpr s32 IsthrathalethPlagueTimer = 5 * 3;

// Matraleth
constexpr s32 DefaultArmParasiteTimer = 2 * 24 + 1;
constexpr s32 MatralethAIParasiteTimer = 75;
constexpr s32 MatralethAIEnsnaringBroodTimer = 75;
constexpr s32 MatralethEnsnareTimer = 37;
constexpr s32 BroodspawnTimer = 720;

// Akistrokor
constexpr s32 AkistrokorEnthrallingAssaultHeal = 256 * 20 / 24; // 20 HP per second

// Alkajelisk
constexpr s32 AlkajeliskParasiticSporesTimer = 135; // 135 is 255 - 5 * 24
constexpr s32 AlkajeliskParasiticSporesSeekRange = 8 * 32;

// Hatchery
constexpr s32 LarvaSpawnRate = (12 * 24) / 8; // 12 seconds

// Gladius
constexpr s32 BindingPlasmaTimer = 12; // 0.5s
constexpr s32 BindingPlasmaRadius = 3 * 32;