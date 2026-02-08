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
constexpr u32 AirToGroundDistance = 42;

namespace CommonBehavior {
inline constexpr s32 MeleeWeaponDistance = 30;
inline constexpr s32 MissileSplitProcDistance = 1 * 32;
}

// TERRAN \\

namespace TerranOgun {
inline constexpr s32 NaniteDronesRange = 4 * 32;
}

namespace UpstartCannibal {
inline constexpr s32 ReintegrationTimer = 10 * 24;
}

namespace Cleric {
inline constexpr s32 HealAmount = 96;  // 9 HP per second
inline constexpr s32 NanoshieldTimer = 25;
inline constexpr s32 HealRange = 30;
}

namespace Shaman {
inline constexpr s32 NaniteFieldHealAmount = 8;
inline constexpr s32 NaniteFieldDuration = 1 * 24;
inline constexpr s32 NaniteFieldSupportRange = 4 * 32;
inline constexpr s32 NaniteFieldVisionaryRange = 6 * 32;
}

namespace TerranMadcap {
constexpr s32 PassiveDecayTime = 0.5 * 24;
constexpr s32 PassiveMaxStacks = 10;
}

namespace TerranApostle {
inline constexpr u32 BloodTitheRange = 3 * 32;
inline constexpr u32 BloodTitheHeal = 5 * 256;
}

namespace Reclamation {
inline constexpr u32 ReviveStunTimer = 3 * 24;
inline constexpr u32 ReviveHealTimer = 10 * 24;
}

namespace HecatonAuriel {
inline constexpr s32 IncinerateTimer = 2 * 24 + 1;
}

namespace ThroughTheLookingGlass {
inline constexpr u32 CastRange = 6 * 32;
inline constexpr u32 TargetSearchRange = 6 * 32;
}

namespace TerranMine {
inline constexpr s32 BurrowTime = 72;
inline constexpr s32 SeekRange = 576;
inline constexpr s32 DetonateRange = 30;
inline constexpr s32 LobotomyTimerVanilla = 9; // 3 seconds
inline constexpr s32 LobotomyTimer = 3 * 24 + 1;
inline constexpr s32 IncinerateTimer = 5 * 24 + 1;
inline constexpr s32 DetonationRange = 2 * 32;
}

namespace Madrigal {
inline constexpr s32 DirgeTimer = 48 + 1;
inline constexpr s32 DirgeDamage = 5;
}

namespace TerranTriton {
inline constexpr s32 LobotomyWarheadRange = 7 * 32;
inline constexpr s32 LobotomyWarheadStunTimer = 4 * 24;
inline constexpr s32 LobotomyWarheadSlowTimer = 8 * 24;
}

namespace Cuirass {
inline constexpr u32 SelfReviveHP = 50 * 256;
inline constexpr s32 SelfHeal = 160; // (15 * 256) / 24 = 160
}

namespace UpstartWendigo {
inline constexpr s32 ShatteredWindsDuration = 2 * 24 + 1;
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

namespace HecatonSerathiel {
inline constexpr s32 DivineDisciplineRange = 6 * 32;
inline constexpr s32 RejoinderOfServiceRange = 6 * 32;
inline constexpr s32 RejoinderOfServiceTimer = 5 * 24;
}

namespace Centaur {
inline constexpr s32 MissileDistance = 4 * 32;
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
inline constexpr s32 PryingEyesDuration = 10 * 24 + 1;
inline constexpr s32 PryingEyesRange = 8 * 32;
}

namespace ApollyoidAdhesive {
inline constexpr s32 Timer = 2 * 24 + 1;
inline constexpr s32 StackCap = 3;
}

namespace Terran {
inline constexpr s32 ControlledDemolitionTimer = 10 * 24 + 1;
inline constexpr s32 SublimeShepherdRange = 6 * 32;
}

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

namespace ProtossSycophant {
inline constexpr s32 IdolatryDeathRange = 3 * 32;
inline constexpr s32 IdolatryDeathShieldHeal = 50 * 256;
}

namespace ProtossMindTyrant {
inline constexpr s32 MaledictusCastTime = 5 * 24;
inline constexpr s32 MaledictusRange = 10 * 32;
inline constexpr s32 MalefactorTyrannizeDuration = 1 * 24;
inline constexpr s32 MaelstromTimer = 5 * 24 + 1;
}

namespace ProtossClarion {
inline constexpr s32 PhaseLinkRange = 3 * 32;
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

namespace ProtossSolarion {
inline constexpr s32 FighterRange = 6 * 32;
}

namespace ProtossStarSovereign {
inline constexpr s32 AIAbilSeekRange = 6 * 32;
inline constexpr s32 AIAbilMinEnemyHealth = 2000;
inline constexpr s32 AIAbilMaxAllyHealth = 2000;
inline constexpr f64 AIAbilLifePercentage = 0.5;
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
inline constexpr s32 AstralSplendorRange = 8 * 32;
inline constexpr s32 AstralSplendorCooldown = 24 * 2;
}

namespace ProtossCrucible {
inline constexpr s32 AmbientSFXDelay = 184;
}

namespace Protoss {
inline constexpr s32 TeleportRange = 3 * 32;
inline constexpr s32 AstralChargeRadius = 2 * 32;
inline constexpr s32 VengefulMarchDuration = 1 * 24 + 1;
inline constexpr s32 BindingPlasmaTimer = 24 * 0.5;
inline constexpr s32 BindingPlasmaRadius = 3 * 32;
inline constexpr s32 BindingPlasmaSearchRadius = 8 * 32;
inline constexpr s32 DimensionalRecallInterval = 12;
inline constexpr s32 DimensionalRecallRange = 2 * 32;
inline constexpr s32 TyrannyEnergyCost = 10 * 256;
}

// ZERG \\

namespace Nathrokor {
inline constexpr s32 AdrenalFrenzyTimer = 2 * 24 + 1;
}

namespace Vorvrokor {
inline constexpr s32 CaptivatingClawsTimer = 24 + 1;
}

namespace Axitrilisk {
inline constexpr s32 PerpetualWindTimer = 8 * 24 + 1;
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

namespace Vilgoleth {
inline constexpr s32 TimedLife = 10 * 24;
inline constexpr s32 SpawnDelay = 1 * 24;
inline constexpr s32 Range = 5 * 24;
}

namespace Yblyleth {
inline constexpr s32 SpawnDelay = 2 * 24;
}

namespace Malkothrokor {
inline constexpr s32 SegmentedExpansionTimer = 15 * 24 + 1;
}

namespace Broodspawn {
inline constexpr s32 DeathCycleCorrosiveTouchRange = 3 * 32;
inline constexpr s32 DeathCycleCorrosiveTouchTimer = 2 * 24 + 1;
inline constexpr s32 DefaultArmParasiteTimer = 2 * 24 + 1;
inline constexpr s32 LifeTimer = 119; // Oddly-specific because 120 (previous value) listed timed life as 12/11 in UI. -Pr0nogo
}

namespace AdaptiveAmitosis {
inline constexpr s32 GrowthTimer = 3 * 24 + 1;
}

namespace CorrosiveTouch {
inline constexpr s32 Timer = 3 * 24 + 1;
inline constexpr s32 CooldownIncrease = 3;
inline constexpr s32 ArmorReduction = 1;
}

namespace Plague {
inline constexpr s32 MiasmaTimer = 4;
}

namespace SwarmingSpite {
inline constexpr s32 Range = 6 * 32;
inline constexpr s32 Duration = 2 * 24;
}

namespace ZergEgg {
inline constexpr s32 AmbientSFXDelayMin = 36;
inline constexpr s32 AmbientSFXDelayMax = 120;
}

namespace Zerg {
inline constexpr s32 LarvaSpawnRate = (14 * 24) / 8; // 14 seconds
inline constexpr s32 HidebreakerDuration = 2 * 24 + 1;
inline constexpr s32 AmitosisSearchRange = 3 * 32;
inline constexpr s32 CorrosivePathogenRange = 4 * 32;
}

// ASKOSI \\

namespace AskosiChantry {
inline constexpr s32 SermonSalveRange = 8 * 32;
}

namespace AskosiCardinal {
inline constexpr s32 SustainingSermonRange = 3 * 32;
inline constexpr s32 SustainingSermonHeal = 5 * 256;
}

namespace AskosiMimic {
inline constexpr s32 MimicryRange = 5 * 32;
}

namespace AskosiScepter {
inline constexpr s32 PrincipalityRange = 8 * 32;
}

namespace AskosiThaumalogue {
inline constexpr s32 RetaliationAssemblyDuration = 3 * 24;
}

namespace Askosi {
inline constexpr s32 ScrapworldGoldRushTimer = 2 * 24;

inline constexpr s32 BoomingBlastTimer = 2 * 24 + 1;
inline constexpr s32 PowerOverloadTimer = 24;

inline constexpr s32 HijackCastRange = 2 * 32;

inline constexpr s32 ImpactDissipationDuration = 2 * 24 + 1;

inline constexpr s32 RematerializeCastRange = 6 * 32;

inline constexpr s32 RapidRendezvousCastRange = 5 * 32;

inline constexpr s32 VitalRemainsHeal = 10 * 256;
inline constexpr s32 VitalRemainsRange = 5 * 32;
inline constexpr s32 VitalRemainsArmorPenDuration = 4 * 24;

inline constexpr s32 ScrapworldAshHeapDuration = 2 * 24 + 1;

inline constexpr s32 PreylockCastRange = 6 * 32;

inline constexpr s32 NewDominionDuration = 10 * 24 + 1;

inline constexpr s32 WarringWindsCooldown = 3;
inline constexpr s32 WarringWindsHealAmount = 5 * 256;
inline constexpr s32 WarringWindsRange = 8 * 32;

inline constexpr s32 ScrapworldImplosionRange = 6 * 32;
inline constexpr s32 ScrapworldImplosionSlowDuration = 5 * 24 + 1;
}