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

namespace Massacre {
inline constexpr s32 ResourceReward = 3;
inline constexpr s32 UnitMultiplier = 2;
}

// General
constexpr s32 HealthMultiplier = 256;
constexpr s32 StimpackOffset = 5 * 256;
constexpr s32 StimpackTimer = 21;
constexpr s32 MindControlEnergyGain = 5 * 256;
constexpr u32 NeutralHostilePlayer = 9;

// Factional
constexpr s32 AxitriliskShieldLossSFXTimer = 24 * 6 + 1;

// TERRAN \\

// Cleric
namespace Cleric {
inline constexpr s32 HealAmount = 96;  // 9 HP per second
inline constexpr s32 NanoshieldTimer = 25;
}

// Shaman
namespace NaniteField {
inline constexpr s32 HealAmount = 4;
inline constexpr s32 SupportRange = 4;
inline constexpr s32 VisionaryRange = 6;
}

// Madcap
constexpr s32 MadcapPassiveDecayTime = 0.5 * 24;
constexpr s32 MadcapMaxStacks = 10;

// Apostle
namespace Reclamation {
inline constexpr u32 ReviveStunTimer = 3 * 24;
inline constexpr u32 ReviveHealTimer = 10 * 24;
}

// Olympian
constexpr s32 OlympianMeleeDistance = 30;

// Hypnagogue
namespace ThroughTheLookingGlass {
inline constexpr u32 CastRange = 6 * 32;
inline constexpr u32 TargetSearchRange = 6 * 32;
}

namespace TerranMine {
inline constexpr s32 BurrowTime = 72;
inline constexpr s32 SeekRange = 576;
inline constexpr s32 DetonateRange = 30;
inline constexpr s32 LobotomyTimer = 9; // 3 seconds
}

namespace Cuirass {
inline constexpr u32 SelfReviveHP = 50 * 256;
inline constexpr s32 SelfHeal = 160; // (15 * 256) / 24 = 160
}

namespace Madrigal {
inline constexpr s32 DirgeTimer = 48 + 1;
inline constexpr s32 DirgeDamage = 5;
}

namespace Wyvern {
inline constexpr s32 BaseAttackCooldown = 30;
}

namespace Seraph {
inline constexpr s32 ObservanceDebuffTimer = 5 * 24 + 1;
inline constexpr s32 ObservanceSearchRange = 6 * 32;
inline constexpr s32 IrradiateTimer = 10;
}

namespace Azazel {
inline constexpr s32 DefensiveMatrixTimer = 30; // roughly 10 seconds
inline constexpr s32 DefensiveMatrixHeal = 107; // ~10 HP per second | 256 * 5 / 24
}

namespace Centaur {
inline constexpr s32 MissileDistance = 4 * 32;
}

// Reservoir / Addon
constexpr s32 EmergencyDetonationTimer = 5 * 24 + 1;
constexpr s32 ControlledDemolitionTimer = 10 * 24 + 1;

// PROTOSS \\

namespace ProtossEcclesiast {
inline constexpr s32 AstralBlessingTimer = 5 * 24 + 1;
}

namespace KhalendricAshguard {
inline constexpr s32 IndomitableAnnexCap = 5;
inline constexpr s32 IndomitableAnnexDamageMultiplier = 5;
inline constexpr s32 IndomitableAnnexTimedLife = 10;
}

namespace ProtossCantavis {
inline constexpr s32 HallucinatingFieldTimer = 10 * 24 + 1;
}

namespace ProtossArchon {
inline constexpr s32 AstralAegisRange = 3 * 32;
inline constexpr s32 AstralAegisShieldHeal = 50 * 256;
}

namespace ProtossMindTyrant {
inline constexpr s32 TyrannyMaxStacks = 48;
inline constexpr s32 MaelstromTimer = 16; // 5 seconds
}

namespace ProtossUnar {
inline constexpr s32 SolitaryFocusTravelThreshold = 1 * 32;
inline constexpr s32 SolitaryFocusCap = 4;
}

namespace ProtossVibrance {
inline constexpr s32 WeaponRadiusInner = 64;
inline constexpr s32 WeaponRadiusMedium = 128;
inline constexpr s32 WeaponRadiusOuter = 192;
}

namespace ProtossReactivePayload {
inline constexpr s32 SplitProcDistance = 1 * 32;
inline constexpr s32 SplitPeriod = 4 * 32;
inline constexpr s32 SplitSearchRadius = 3 * 32;
}

namespace ProtossEmpathyCore {
inline constexpr s32 EnergyAmount = 5 * 256;
}

namespace ProtossLanifect {
inline constexpr s32 AICheckRange = 3 * 32;
inline constexpr s32 AISeekRange = 8 * 32;
inline constexpr s32 AIWebTimer = 1 * 24;
}

namespace ProtossDidact {
inline constexpr s32 ShroudOfAdunRange = 4 * 32;
inline constexpr s32 StasisFieldTimer = 23; // roughly 8 seconds
}

namespace ProtossStarSovereign {
inline constexpr s32 AIAbilSeekRange = 6 * 32;
inline constexpr s32 AIAbilMinEnemyHealth = 2000;
inline constexpr s32 AIAbilMaxAllyHealth = 2000;
inline constexpr f64 AIAbilLifePercentage = 0.5;
inline constexpr s32 AIAbilTimer = 612;
}

namespace Protoss {
inline constexpr s32 AstralChargeRadius = 2 * 32;
inline constexpr s32 VengefulMarchDuration = 1 * 24 + 1;
inline constexpr s32 BindingPlasmaTimer = 24 * 0.5;
inline constexpr s32 BindingPlasmaRadius = 3 * 32;
inline constexpr s32 BindingPlasmaSearchRadius = 8 * 32;
}

// ZERG \\

namespace Nathrokor {
inline constexpr s32 AdrenalFrenzyTimer = 2 * 24 + 1;
inline constexpr f64 AdrenalFrenzySpeedFactor = 0.01;
}

namespace Vorvrokor {
inline constexpr s32 CaptivatingClawsTimer = 24 + 1;
}

namespace Axitrilisk {
inline constexpr s32 PerpetualWindTimer = 8 * 24 + 1;
}

namespace Vithrilisk {
inline constexpr s32 CorrosiveTouchTimer = 5 * 24 + 1;
inline constexpr f64 CorrosiveTouchCooldownIncrease = 0.05;
inline constexpr f64 CorrosiveTouchArmorReduction = 1 * 256;
}

namespace Zoryusthaleth {
inline constexpr s32 SwarmingOmenRadius = 10 * 32;
}

namespace Isthrathaleth {
inline constexpr s32 PlagueTimer = 5 * 3;
}

namespace Matraleth {
inline constexpr s32 AIParasiteTimer = 75;
inline constexpr s32 AIEnsnaringBroodTimer = 75;
inline constexpr s32 EnsnareTimer = 5;
}

namespace Broodspawn {
inline constexpr s32 DeathCycleEnsnareRange = 3 * 32;
inline constexpr s32 DeathCycleEnsnareTimer = 5; // Vanilla timer, approximately 1 second
inline constexpr s32 DefaultArmParasiteTimer = 2 * 24 + 1;
inline constexpr s32 LifeTimer = 720;
}

namespace Kazitrith {
inline constexpr s32 AuraRadius = 6 * 32;
}

namespace Zerg {
inline constexpr s32 LarvaSpawnRate = (14 * 24) / 8; // 14 seconds
inline constexpr s32 AmitosisSearchRange = 3 * 32;
}