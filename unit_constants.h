#pragma once
#pragma warning(disable: 4455)

#include "types.h"

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
constexpr u32 AirToGroundDistance = 42;

namespace CommonBehavior {
inline constexpr s32 MeleeWeaponDistance = 30;
inline constexpr s32 MissileSplitProcDistance = 1tiles;
}

// TERRAN \\

namespace TerranOgun {
inline constexpr s32 NaniteDronesRange = 4tiles;
}

namespace UpstartCannibal {
inline constexpr s32 ReintegrationTimer = 10secs;
}

namespace Cleric {
inline constexpr s32 HealAmount = 96;  // 9 HP per second
inline constexpr s32 NanoshieldTimer = 25;
inline constexpr s32 HealRange = 30;
}

namespace Shaman {
inline constexpr s32 NaniteFieldHealAmount = 8;
inline constexpr s32 NaniteFieldDuration = 1secs;
inline constexpr s32 NaniteFieldSupportRange = 4tiles;
inline constexpr s32 NaniteFieldVisionaryRange = 6tiles;
}

namespace TerranMadcap {
constexpr s32 PassiveDecayTime = 0.5secs;
constexpr s32 PassiveMaxStacks = 10;
}

namespace TerranApostle {
inline constexpr u32 BloodTitheRange = 3tiles;
inline constexpr u32 BloodTitheHeal = 5 * 256;
}

namespace Reclamation {
inline constexpr u32 ReviveStunTimer = 3secs;
inline constexpr u32 ReviveHealTimer = 10secs;
}

namespace HecatonAuriel {
inline constexpr s32 IncinerateTimer = 2secs + 1;
}

namespace ThroughTheLookingGlass {
inline constexpr u32 CastRange = 6tiles;
inline constexpr u32 TargetSearchRange = 6tiles;
}

namespace TerranMine {
inline constexpr s32 BurrowTime = 72;
inline constexpr s32 SeekRange = 576;
inline constexpr s32 DetonateRange = 30;
inline constexpr s32 LobotomyTimerVanilla = 9; // 3 seconds
inline constexpr s32 LobotomyTimer = 3secs + 1;
inline constexpr s32 GoremastTimer = 3secs + 1;
inline constexpr s32 TartarusTimer = 5secs + 1;
inline constexpr s32 DetonationRange = 2tiles;
}

namespace Madrigal {
inline constexpr s32 DirgeTimer = 2secs + 1;
inline constexpr s32 DirgeDamage = 5;
}

namespace TerranTriton {
inline constexpr s32 LobotomyWarheadRange = 7tiles;
inline constexpr s32 LobotomyWarheadStunTimer = 4secs;
inline constexpr s32 LobotomyWarheadSlowTimer = 8secs;
}

namespace Cuirass {
inline constexpr u32 SelfReviveHP = 50 * 256;
inline constexpr s32 SelfHeal = 160; // (15 * 256) / 24 = 160
}

namespace UpstartWendigo {
inline constexpr s32 ShatteredWindsDuration = 2secs + 1;
}

namespace Wyvern {
inline constexpr s32 BaseAttackCooldown = 30;
}

namespace Seraph {
inline constexpr s32 ObservanceDebuffTimer = 5secs + 1;
inline constexpr s32 ObservanceSearchRange = 6tiles;
inline constexpr s32 IrradiateTimer = 10;
}

namespace Azazel {
inline constexpr s32 DefensiveMatrixTimer = 30; // roughly 10 seconds
inline constexpr s32 DefensiveMatrixHeal = 107; // ~10 HP per second | 256 * 5 / 24
}

namespace HecatonSerathiel {
inline constexpr s32 DivineDisciplineRange = 6tiles;
inline constexpr s32 RejoinderOfServiceRange = 6tiles;
inline constexpr s32 RejoinderOfServiceTimer = 5secs;
}

namespace Centaur {
inline constexpr s32 MissileDistance = 4tiles;
}

namespace TerranMinistry {
inline constexpr s32 AmbientSFXDelay = 40;
}

namespace TerranTreasury {
inline constexpr s32 AmbientSFXDelay = 25;
}

namespace TerranStockade {
inline constexpr s32 AmbientSFXDelay = 34;
}

namespace TerranFulcrum {
inline constexpr s32 AmbientSFXDelay = 36;
}

namespace TerranStarpad {
inline constexpr s32 AmbientSFXDelay = 23;
}

namespace TerranFacade {
inline constexpr s32 PryingEyesDuration = 10secs + 1;
inline constexpr s32 PryingEyesRange = 8tiles;
}

namespace ApollyoidAdhesive {
inline constexpr s32 Timer = 2secs + 1;
inline constexpr s32 StackCap = 3;
}

namespace Terran {
inline constexpr s32 ControlledDemolitionTimer = 10secs + 1;
inline constexpr s32 SublimeShepherdRange = 6tiles;
inline constexpr s32 StimpackMinimumHP = 5 * 256;
}

// PROTOSS \\

namespace ProtossEcclesiast {
inline constexpr s32 AstralBlessingTimer = 5secs + 1;
}

namespace KhalendricAshguard {
inline constexpr s32 IndomitableAnnexCap = 5;
inline constexpr s32 IndomitableAnnexDamageMultiplier = 5;
inline constexpr s32 IndomitableAnnexTimedLife = 10;
}

namespace ProtossCantavis {
inline constexpr s32 HallucinatingFieldTimer = 10secs + 1;
}

namespace ProtossSycophant {
inline constexpr s32 IdolatryDeathRange = 3tiles;
inline constexpr s32 IdolatryDeathShieldHeal = 50 * 256;
}

namespace ProtossMindTyrant {
inline constexpr s32 MaledictusCastTime = 5secs;
inline constexpr s32 MaledictusRange = 10tiles;
inline constexpr s32 MalefactorTyrannizeDuration = 1secs;
inline constexpr s32 MaelstromTimer = 5secs + 1;
}

namespace ProtossClarion {
inline constexpr s32 PhaseLinkRange = 3tiles;
}

namespace ProtossUnar {
inline constexpr s32 SolitaryFocusTravelThreshold = 1tiles;
inline constexpr s32 SolitaryFocusCap = 4;
}

namespace ProtossVibrance {
inline constexpr s32 WeaponRadiusInner = 64;
inline constexpr s32 WeaponRadiusMedium = 128;
inline constexpr s32 WeaponRadiusOuter = 192;
}

namespace ProtossReactivePayload {
inline constexpr s32 SplitPeriod = 4tiles;
inline constexpr s32 SplitSearchRadius = 3tiles;
}

namespace ProtossEmpathyCore {
inline constexpr s32 EnergyAmount = 5 * 256;
}

namespace ProtossLanifect {
inline constexpr s32 AICheckRange = 3tiles;
inline constexpr s32 AISeekRange = 8tiles;
inline constexpr s32 AIWebTimer = 1secs;
}

namespace ProtossDidact {
inline constexpr s32 ShroudOfAdunRange = 4tiles;
inline constexpr s32 StasisFieldTimer = 23; // roughly 8 seconds
}

namespace ProtossSolarion {
inline constexpr s32 FighterRange = 6tiles;
}

namespace ProtossStarSovereign {
inline constexpr s32 AIAbilSeekRange = 6tiles;
inline constexpr s32 AIAbilMinEnemyHealth = 2000;
inline constexpr s32 AIAbilMaxAllyHealth = 2000;
inline constexpr fp32 AIAbilLifePercentage = 0.5fp;
inline constexpr s32 AIAbilTimer = 612;
}

namespace ProtossNexus {
inline constexpr s32 AmbientSFXDelay = 37;
}

namespace ProtossEmbassy {
inline constexpr s32 AmbientSFXDelay = 32;
}

namespace ProtossGateway {
inline constexpr s32 AmbientSFXDelay = 41;
}

namespace ProtossGrandLibrary {
inline constexpr s32 AmbientSFXDelay = 32;
}

namespace ProtossRogueGallery {
inline constexpr s32 AmbientSFXDelay = 32;
}

namespace ProtossProstrationStage {
inline constexpr s32 AmbientSFXDelay = 38;
}

namespace ProtossAncestralArchives {
inline constexpr s32 AmbientSFXDelay = 24;
}

namespace ProtossLattice {
inline constexpr s32 AmbientSFXDelay = 24;
}

namespace ProtossArdentAuthority {
inline constexpr s32 AmbientSFXDelay = 27;
}

namespace ProtossUnitaryUnion {
inline constexpr s32 AmbientSFXDelay = 18;
}

namespace ProtossSyntheticSynod {
inline constexpr s32 AmbientSFXDelay = 51;
}

namespace ProtossStargate {
inline constexpr s32 AmbientSFXDelay = 33;
}

namespace ProtossArgosy {
inline constexpr s32 AmbientSFXDelay = 25;
}

namespace ProtossMonumentOfSin {
inline constexpr s32 AmbientSFXDelay = 40;
}

namespace ProtossBulwark {
inline constexpr s32 AstralSplendorShieldAmount = 10 * 256;
inline constexpr s32 AstralSplendorRange = 8tiles;
inline constexpr s32 AstralSplendorCooldown = 2secs;
}

namespace ProtossCrucible {
inline constexpr s32 AmbientSFXDelay = 184;
}

namespace Protoss {
inline constexpr s32 TeleportRange = 3tiles;
inline constexpr s32 AstralChargeRadius = 2tiles;
inline constexpr s32 VengefulMarchDuration = 1secs + 1;
inline constexpr s32 BindingPlasmaTimer = 0.5secs;
inline constexpr s32 BindingPlasmaRadius = 3tiles;
inline constexpr s32 BindingPlasmaSearchRadius = 8tiles;
inline constexpr s32 DimensionalRecallInterval = 12;
inline constexpr s32 DimensionalRecallRange = 2tiles;
inline constexpr s32 TyrannyEnergyCost = 10 * 256;
}

// ZERG \\

namespace Nathrokor {
inline constexpr s32 AdrenalFrenzyTimer = 2secs + 1;
}

namespace Vorvrokor {
inline constexpr s32 CaptivatingClawsTimer = 1secs + 1;
}

namespace Axitrilisk {
inline constexpr s32 PerpetualWindTimer = 8secs + 1;
}

namespace Zoryusthaleth {
inline constexpr s32 SwarmingOmenRadius = 10tiles;
}

namespace Isthrathaleth {
inline constexpr s32 PlagueTimer = 5 * 3;
}

namespace Matraleth {
inline constexpr s32 AIParasiteTimer = 75;
inline constexpr s32 AIEnsnaringBroodTimer = 75;
inline constexpr s32 EnsnareTimer = 5;
}

namespace Vilgoleth {
inline constexpr s32 TimedLife = 10secs;
inline constexpr s32 SpawnDelay = 1secs;
inline constexpr s32 Range = 5tiles;
}

namespace Yblyleth {
inline constexpr s32 SpawnDelay = 2secs;
}

namespace Malkothrokor {
inline constexpr s32 SegmentedExpansionTimer = 15secs + 1;
}

namespace Broodspawn {
inline constexpr s32 DeathCycleCorrosiveTouchRange = 3tiles;
inline constexpr s32 DeathCycleCorrosiveTouchTimer = 2secs + 1;
inline constexpr s32 DefaultArmParasiteTimer = 2secs + 1;
inline constexpr s32 LifeTimer = 119; // Oddly-specific because 120 (previous value) listed timed life as 12/11 in UI. -Pr0nogo
}

namespace AdaptiveAmitosis {
inline constexpr s32 GrowthTimer = 3secs + 1;
}

namespace CorrosiveTouch {
inline constexpr s32 Timer = 3secs + 1;
inline constexpr s32 CooldownIncrease = 3;
inline constexpr s32 ArmorReduction = 1;
}

namespace Plague {
inline constexpr s32 MiasmaTimer = 4;
}

namespace SwarmingSpite {
inline constexpr s32 Range = 6tiles;
inline constexpr s32 Duration = 2secs;
}

namespace ZergEgg {
inline constexpr s32 AmbientSFXDelayMin = 36;
inline constexpr s32 AmbientSFXDelayMax = 120;
}

namespace Zerg {
inline constexpr s32 LarvaSpawnRate = 14secs / 8; // 14 seconds
inline constexpr s32 HidebreakerDuration = 2secs + 1;
inline constexpr s32 AmitosisSearchRange = 3tiles;
inline constexpr s32 CorrosivePathogenRange = 4tiles;
}

// ASKOSI \\

namespace AskosiChantry {
inline constexpr s32 SermonSalveRange = 8tiles;
}

namespace AskosiCardinal {
inline constexpr s32 SustainingSermonRange = 3tiles;
inline constexpr s32 SustainingSermonHeal = 5 * 256;
}

namespace AskosiMimic {
inline constexpr s32 MimicryRange = 5tiles;
}

namespace AskosiScepter {
inline constexpr s32 PrincipalityRange = 8tiles;
}

namespace AskosiThaumalogue {
inline constexpr s32 RetaliationAssemblyDuration = 3secs;
}

namespace Askosi {
inline constexpr s32 ScrapworldGoldRushTimer = 2secs;

inline constexpr s32 BoomingBlastTimer = 2secs + 1;
inline constexpr s32 PowerOverloadTimer = 1secs;

inline constexpr s32 HijackCastRange = 2tiles;

inline constexpr s32 ImpactDissipationDuration = 2secs + 1;

inline constexpr s32 RematerializeCastRange = 6tiles;

inline constexpr s32 RapidRendezvousCastRange = 5tiles;

inline constexpr s32 VitalRemainsHeal = 10 * 256;
inline constexpr s32 VitalRemainsRange = 5tiles;
inline constexpr s32 VitalRemainsArmorPenDuration = 4secs;

inline constexpr s32 ScrapworldAshHeapDuration = 2secs + 1;

inline constexpr s32 PreylockCastRange = 6tiles;

inline constexpr s32 NewDominionDuration = 10secs + 1;

inline constexpr s32 WarringWindsCooldown = 3;
inline constexpr s32 WarringWindsHealAmount = 5 * 256;
inline constexpr s32 WarringWindsRange = 8tiles;

inline constexpr s32 ScrapworldImplosionRange = 6tiles;
inline constexpr s32 ScrapworldImplosionSlowDuration = 5secs + 1;
}