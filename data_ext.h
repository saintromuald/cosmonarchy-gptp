#pragma once
#include "SCBW/api.h"

// TODO: move unit ext handling here (from iquare_globals)

enum ExtValueId {
	ValueUnitInit,
	ValueUnitIdName,
	ValueWyvernLandGrabOn,
	ValueAmitosis,
	ValueHasFiredReturnAttack,
	ValueHasSublimeShepherd,
	ValueWyvernLandGrabX,
	ValueWyvernLandGrabY,
	ValueObservanceRend,
	ValueMissileReturnX,
	ValueMissileReturnY,
	ValueAdrenalStacks,
	ValueLastOrdersInit,
	ValueTempoChange,
	ValueTreasuryStacks,
	ValueSupplyProvidedSet,
	ValueSupplyProvidedId,
	ValueSupplyProvided,
	ValueSkyfallStacks,
	ValueSkyfallX,
	ValueSkyfallY,
	ValueCorrosiveTouchStacks,
	ValueKalkaliskLaunchPos,
	ValueTerminalSurge,
	ValueEmpyreanLaunchPos,
	ValueProtossSwitchOrderState,
	ValueMadcapStacks,
	ValuePhobosDischarge,
	ValueSalamanderLaunchPos,
	ValueNaniteField,
	ValuePhobosLaunchPos,
	ValueHippogriffLaunchPos,
	ValueHippogriffLaunchPos2,
	ValueIncubationCount,
	ValueIncubationUnitId,
	ValueIncubationPlayer,
	ValueCentaurLaunchPos,
	ValueRepententChorus,
	ValueMagnetarLaunchPos,
	ValueAlkajeliskSpores,
	ValueAlkajeliskSporesPlayer,
	ValueSovereignLaunchPos,
	ValueAICloakGimmick,
	ValueKeskathalorLaunchPos,
	ValueAugurSafetyCheck,
	ValuePenumbraLaunchPos,
	ValueClarionSFX,
	ValueWraithLaunchPos,
	ValueHierophantOverlay,
	ValueEnsnaringBroodAbility,
	ValuePetrificationStacks,
	ValueIonCannonLaunchPos,
	ValueReconstitution,
	ValueSimulacrumClone,
	ValueProtectiveInstinctsStatus,
	ValueEntropyGauntlets,
	ValueDeconstruction,
	ValueDeconstructionPlayer,
	ValueMindControlOriginalPlayer,
	ValuePreMorphEnergy,
	ValueWendigoShatteredWindsStacks,
//	ValueGodsMadeFleshActive,
	ValueMissileReturnTarget, // unit
//	ValueKelvinMunitionsSlow,
	ValueRaceSupplyType,
	ValueExemplarLaunchPos,
	ValueClaymoreLaunchPos,
	ValueDirgeSourcePlayer,
	ValueDirgeStacks,
	ValueSignifyStacks,
	ValueSignifyPlayer,
	ValueReviveStarted,
	ValuePreMorphHP,
	ValueSkithrokorLaunchPos,
	ValueDisruptionFieldOn,
	ValueLiiraliskLaunchPos,
	ValueAlliedInvPlayer,
	ValueAutocratControlledOppositionStacks,
	ValueAutocratControlledOppositionPlayer,
	ValueEmpressAnodyneAnnexStacks,
	ValueEmpressAnodyneAnnexSourcePlayer,
	ValueAxitriliskLaunchPos,
	ValueBactaliskLaunchPos,
	ValueDracadinLaunchPos,
	ValueCharlatanLaunchPos,
	ValueIsNeutralBunker,
	ValueIsZombified,
	ValueCuirassIsSelfReviving,
	ValueTalosAnimationTimer,
	ValueBlissfulEmbraceOriginalPlayer,
	ValueBlissfulEmbraceOriginalHealth,
	ValueSpecialSovrolethBirth,
	ValuePlayingAddonWorkingAnim,
	ValueMindControlPlayer,
	ValueNewRepairTimer,
	ValueSpraithColonyLaunchPos,
	ValueMissileSiloLoaded,
	ValueSovrolethLinkCount,
	ValueDisableAutoBurrow,
	ValueSentinelLaunchPos,
	ValueWardenLaunchPos,
	ValueCowardiceOverlay,
	ValueUnitResponseOrder,
	ValuePaladinLaunchPos,
	ValueGoliathLaunchPos,
	ValueSundogLaunchPos,
	ValueEpigraphLaunchPos,
	ValueMatrixLaunchPos,
	ValueCataphractLaunchPos,
	ValueDilettanteLuck,
	ValuePanoptusLaunchPos,
	ValueThroughTheLookingGlassX,
	ValueThroughTheLookingGlassY,
	ValueGladiusBindingPlasmaStacks,
	ValueGladiusBindingPlasmaPlayer,
	ValueCentaurSecondaryWeaponCount,
	ValueAccantorLaunchPos,
	ValuePatriarchAstralRulePlayer,
	ValueSwarmhold,
	ValueOptectonLaunchPos1,
	ValueOptectonLaunchPos2,
	ValueApollyoidAdhesiveStacks,
	ValueRamessesLaunchPos,
	ValuePositronLaunchPos,
	ValueCustomImage,
	ValueAionLaunchPos,
	ValueMortothrokorStacks,
	ValueMadrigalTempoDown,
	ValueAssignedHarvesters,
	ValueAssignedAiGasHarvesters,
	ValueActiveHarvesters,
	ValueIsActiveHarvester,
	ValueNoCorpse,
	ValueDidactChannelIndex,
	ValueDidactRecalling,
	ValueIonCannonTargeting,
	ValueIonCannonTargetPosX,
	ValueIonCannonTargetPosY,
	ValueWorshipStack,
	ValueCyprianLaunchPos,
	ValueSavantEntropyGauntlets,
	ValueSavantPowerSiphonBuff,
	ValueSavantPowerSiphonDebuff,
	ValuePhalanxSiegeModeLaunchPos,
	ValueOriginatedFromOcsalLarva,
	ValueOriginatedFromGosvileth,
	ValueCustomEggActive,
	ValueCustomEggHatchMax,
	ValueCustomEggHpMax,
	ValueProjectilePattern,
	ValueProjectileAngle,
	ValueGhitorokorRelentlessAggressionStacks,
	ValueDecaying,
	ValuePariahHitPlayer,
	ValueIsNydusExit,
	ValueIsNeutralNydus,
	ValueNeedsUnfreezing,
	ValueTerranConstruction,
	ValueMorphConnectedUnitId,
	ValueEnsnareFactor,
	ValueVultureLaunchPos,
	ValueStaggerWeaponId,
	ValueStaggerCount,

	ValueAI_WorkerChosenForExpansion,
	ValueAI_WorkerChosenX,
	ValueAI_WorkerChosenY,
	ValueAI_KitingSignal,
	ValueAI_PreventTargeting,
	ValueAI_BunkerCustom,

	ValueCount,
};

enum ExtTimerId {
	TimerKaiserRampage,
	TimerKaiserRampageTimeout,
	TimerApollyoidAdhesive,
	TimerApollyoidAdhesiveDamagePeriod,
	TimerObservanceDebuff,
	TimerObservancePeriod,
	TimerMissileReturnETA,
	TimerLobotomySlow,							
	TimerStarSovereignGriefOfAllGodsChannel,
	TimerStarSovereignGriefOfAllGodsDelay,
	TimerStarSovereignInterventionMissiles,
	TimerQuaziliskCancel,
	TimerCausticDischargeSpeed,
	TimerIonFieldAttackSpeed,
	TimerReconstitutionCooldown,
	TimerSalamanderFire,
	TimerWendigoShatteredWinds,
	TimerControlledDemolition,
	TimerEmergencyDetonation,
	TimerNemeanReactors,
	TimerSimulacrumClone,
	TimerEnsnaringBrood,
	TimerNewMedstim,							// unused
	TimerAugurGimmick,
	TimerObservanceOverlay,
	TimerNaniteField,
	TimerCorrosiveTouch,
	TimerSkyfall,
	TimerSireEggBoost,
	TimerHallucinating,
	TimerPetrification,
	TimerDecloak,
	TimerDecloakBurrow,
	TimerResourceBoostedRegen,
	TimerAdrenal,
	TimerSwitchModeGimmick,
	TimerCaptivatingClaws,
	TimerAlaszilethRegenBoost,
	TimerWyvernAttackCooldown,
	TimerDirgeFuse,								// unused
	TimerSignify,
	TimerReviveFlagClearMin,
	TimerReviveFlagClearMax,
	TimerUnderDisruptionField,
	TimerAlliedInvFrames,
	TimerIncubationArm,
	TimerAuralAssaultBlind,
	TimerAnthelionGravityWell,
	TimerAnthelionEndlessEclipse,
	TimerAnthelionAttackCooldown,
	TimerStrigaRepulsion,
	TimerAxitriliskShieldLossSFX,
	TimerOwnerChangedRecently,
	TimerMagnetarAttraction,
	TimerSalvageStunTimer,
	TimerCreepMorphOverlay,
	TimerCustomEggHatch,
	TimerMindControlRecede,
	TimerTalosAnimationSubtimer,
	TimerTalosSoundLoopTimer,
	TimerAddonAnimation,
	TimerSwarmingOmenDebuff,
	TimerBladeVortex,
	TimerStellarEnforcementOverlay,
	TimerSwarmingOmenSearch,
	TimerHypnagogueChannel,
	TimerSecondaryWeaponCooldown,
	TimerReclaimInterval,
	TimerBindingPlasma,
	TimerCentaurSecondaryWeaponStagger,
	TimerPatriarchAstralRule,
	TimerTerminalSurge,
	TimerTriumphantRiseDebuff,
	TimerMatadorHeal,
	TimerMortothrokorPassive,
	TimerLiftLandDelay,
	TimerMadrigalTempoChange,
	TimerDidactStasisFieldOverlay,
	TimerStellarEnforcement,
	TimerDeconstruction,
	TimerKelvinMunitions,
	TimerSavantPowerSiphonBuff,
	TimerSavantPowerSiphonDebuff,
	TimerStun,
	TimerAutocratControlledOpposition,
	TimerSevenstepMindMatter,
	TimerSilvertonguePlotTwist,
	TimerClaymoreKelvinMunitionsZap,
	TimerMindTyrantMaelstromSlow,
	TimerGhitorokorRelentlessAggression,
	TimerDemiurgeWorship,
	TimerAxitriliskPerpetualWind,
	TimerReclaiming,
	TimerDecaySafe,
	TimerVilgolethSpawn,
	TimerVilgolethTimedLife,
	TimerAvenge,
	TimerAscend,
	TimerPariahHit,
	TimerHeraldAvenge,
	TimerZergAdrenalFrenzy,
	TimerEmpressAnodyneAnnex,
	TimerZarcavrokorSecondaryFire,
	TimerClearStaggerBlacklist,
	TimerUseLastRepulsion,
	TimerCustomLarva,

	// probably convert the following to use TimerAI_GenericAbilityCooldown
	TimerAI_Lazarus,
	TimerAI_Siphon,
	TimerAI_Clarion,
	TimerAI_Skyfall,
	TimerAI_GriefOfAllGods,
	TimerAI_DisruptionField,
	TimerAI_MadrigalTempoChangeTimer,
	TimerAI_MadrigalTempoChangeCooldown,

	TimerAI_GenericAbilityCooldown, // reusable for multiple unit ids, probably add second one if unit has two seperate abilities
	TimerAI_KitingCooldown,
	TimerAI_KitingLeash,
	TimerAI_KitingSync,
	TimerAI_KitingCombat,
	TimerAI_Patience,

	TimerCount,
};

enum ExtUnitId {
	UnitBroodlingMother,			// is this still really used?
	UnitDecaySavant,
	UnitMindController,
	UnitWyvernTargetLock,
	UnitDirgeSource,
	UnitBunkerLockOn,
	UnitEmpressAnodyneAnnexSource,
	UnitAnthelionGravityWellSource,
	UnitTurretParentLink,
	UnitBlissfulEmbrace,
	UnitMorphConnectedUnit,
	UnitAddonParent,
	UnitResponseTarget,
	UnitCentaurSecondaryWeaponTarget,
	UnitIonCannonTarget,
	UnitSevenstepMindMatterSource,
	UnitPennant,
	UnitDemiurge,

	UnitCount,
};

enum ExtValueVectorId {
	ValueVectorPleaseDontUseVectors,

	ValueVectorCount,
};

enum ExtUnitVectorId {
	UnitVectorCaptivatingClaws,
	UnitVectorPhaseLink,
	UnitVectorPhaseLinkClarions,
	UnitVectorCrucibles,
	UnitVectorMindControlling,
	UnitVectorStaggerBlacklist,

	UnitVectorCount,
};

enum ExtTimerVectorId {
	TimerVectorCorrupted, // last slot is always corrupted on some builds, fuck you visual studio

	TimerVectorCount,
};

struct UnitExt {
	bool					pause_timers;
	u32						energy_regen_timer;
	u32						shield_regen_timer;
	u32						damage_taken_timer;
	u32						damage_dealt_timer;
	u32                     total_damage_taken;
	u32                     total_damage_dealt;
	Point16					anthelion_last_move_target;
	Vector32f				repulsion_vector;
	Point16					last_bullet_launch_position;
	Point16					ai_kiting_origin;
	Target                  projectile_pattern_target;
	Vector32f				last_repulsion;
//	Target					rally;
	const char*				custom_name;

	u8						seen_by_flags;
	Point16					last_seen_position[PlayerLimit];
	u32						last_seen_timer[PlayerLimit];

	f32						building_tick_progress;
	f32						building_tick_last_progress;
	u32						building_tick_last_update;

	u32						salvage_time;
	f32						salvage_minerals;
	f32						salvage_gas;

	Point16					terran_build_pos;

	u32						ai_timer_avoid_targeting[PlayerLimit];

	CargoUnit				loaded_unit[TransportCap];

	u32						values[ValueCount];
	u32						timers[TimerCount];
	Unit*					units[UnitCount];
	std::vector<u32>		value_vectors[ValueVectorCount];
	std::vector<Unit*>		unit_vectors[UnitVectorCount];
	std::vector<u32>		timer_vectors[TimerVectorCount];
};

struct TileExt {
	u8 creep_owners;
};

struct BulletExt {
	Point16 bullet_source;
	u32 distance_traveled;
	u32 time_traveled;
	u32 divisor_multiplier = 1;
	u32 current_z_displacement;
	bool bounced;
	bool is_phantom;
	bool did_pierce;
};

struct SpriteExt {
	bool is_fow_sprite;
	Unit* owner;
};

struct ImageExt {
	u32 iscript_header_offset;
	u32 iscript_offset;
	u32 return_address;
};

struct ThingyExt {
	u32 player;
	u32 unit_id;
	u32 activity_timer;
	u32 avenge_or_ascend_player;
	u32 sight_range_before_death;
};

#define GasBuildingsCap 16
struct AiTownExt {
	u32 gas_buildings_count;
	Unit* gas_buildings[GasBuildingsCap];
};

void init_ext_variables();
void update_ext_variables();

UnitExt* get_unit_ext(Unit* unit);
void reset_unit_ext(Unit* unit);

void unit_death_start(Unit* unit);
void unit_death_end(Unit* unit);

TileExt* get_tile_ext_at_tile(u32 tile_x, u32 tile_y);
TileExt* get_tile_ext_at_coords(u32 x, u32 y);

BulletExt* get_bullet_ext(Bullet* bullet);
void reset_bullet_ext(Bullet* bullet);

SpriteExt* get_sprite_ext(Sprite* sprite);
void reset_sprite_ext(Sprite* sprite); // TODO: call on new sprite creation!!!

ImageExt* get_image_ext(Image* image);
void reset_image_ext(Image* image); // TODO: call on new image creation!!!

ThingyExt* get_thingy_ext(Thingy* thingy);
void reset_thingy_ext(Thingy* thingy); // TODO: call on new thingy creation!!!

AiTownExt* get_town_ext(AiTown* town);
void reset_town_ext(AiTown* town); // TODO: call on new town creation!!!

bool can_place_on_creep_tile(TileExt* tile_ext, u32 player);