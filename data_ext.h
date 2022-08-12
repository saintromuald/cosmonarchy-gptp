#pragma once
#include "SCBW/api.h"

// TODO: move unit ext handling here (from globals)

enum ExtValueId
{
	ValueUnitInit,
	ValueWyvernLandGrabOn,
	ValueAmitosis,
	ValueSomaticImplants,
	ValueSublimeShepherd,
	ValueHasSublimeShepherd,
	ValueWyvernLandGrabX,
	ValueWyvernLandGrabY,
	ValueObservanceRend,
	ValueSpineX,
	ValueSpineY,
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
	ValueKalkaliskMissilePos,
	ValueTerminalSurge,
	ValueEmpyreanMissilePos,
	ValueProtossSwitchOrderState,
	ValueMadcapStacks,
	ValuePhobosDischarge,
	ValueSalamanderAtkPos,
	ValueNaniteField,
	ValuePhobosLaserPos,
	ValuePhobosMissilePos,
	ValueCentaurMissilePos,
	ValueLazarusAgent,
	ValueRepententChorus,
	ValueStellarEnforcement,
	ValueAlkajeliskParasite,
	ValueAlkajeliskParasitePlayer,
	ValueSovereignCannonPos,
	ValueAICloakGimmick,
	ValueKeskathalorMissilePos,
	ValueAugurSafetyCheck,
	ValuePenumbraLaserPos,
	ValueClarionSFX,
	ValueWraithLaserPos,
	ValueHierophantOverlay,
	ValueEnsnaringBroodAbility,
	ValuePetrificationStacks,
	ValueIonCannonProjectilePos,
	ValueSwarmingOmenAbility,
	ValueReconstitution,
	ValueSimulacrumClone,
	ValueSwarmingOmenStatus,
	ValueProtectiveInstinctsStatus,
	ValueNemeanReactorsStack,
	ValueEntropyGauntlets,
	ValueDeconstruction,
	ValueDeconstructionPlayer,
	ValueWorshipStack,
	ValueMindControlOriginalPlayer,
	ValuePreMorphEnergy,
	ValueGodsMadeFleshCast,
	ValueGodsMadeFleshActive,
	ValueLurkerPickTarget,
	ValueKelvinMunitionsSlow,
	ValueRaceSupplyType,
	ValueExemplarMissilePos,
	ValueClaymoreMissilePos,
	ValueDirgeSourcePlayer,
	ValueDirgeStacks,
	ValueSignifyStacks,
	ValueReviveStarted,
	ValueReviveTownPointer,
	ValuePreMorphHP,
	ValueSkithrokorMissilePos,
	ValueDisruptionFieldOn,
	ValueLiiraliskMissilePos,
	ValueAlliedInvPlayer,
	ValueControlledOpposition,
	ValueEmpressPassiveStacks,
	ValueEmpressSourcePlayer,
	ValueAnticthonStacks,
	ValueAxitriliskMissilePos,
	ValueBactaliskMissilePos,
	ValueCharlatanMissilePos,
	ValueIsNeutralBunker,
	ValueIsZombified,
	ValueCuirassIsSelfReviving,
	ValueTalosAnimationTimer,
	ValueBlissfulEmbraceOriginalPlayer,
	ValueBlissfulEmbraceOriginalHealth,
	ValueSpecialSovrolethBirth,
	ValuePlayingAddonWorkingAnim,
	ValueTyrannyPlayer,
	ValueNewRepairTimer,
	ValueSporeColonyMissilePos,
	ValueMissileSiloLoaded,
	ValueSovrolethLinkCount,
	ValueParasiteArmPlayer,
	ValueDisableAutoBurrow,
	ValueSentinelMissilePos,
	ValuePhotonCannonMissilePos,
	ValueCowardiceOverlay,
	ValueHasCustomSprite,

	ValueAI_WorkerChosenForExpansion,
	ValueAI_WorkerChosenX,
	ValueAI_WorkerChosenY,
	ValueAI_KitingSignal,
	ValueAI_PreventTargeting,
	ValueAI_BunkerCustom,

	ValueCount,
};

enum ExtTimerId
{
	TimerKaiserRampage,
	TimerKaiserRampageTimeout,
	TimerMaledictionSlow,						// unused
	TimerObservanceDebuff,
	TimerObservancePeriod,
	TimerLurkerETA,
	TimerLobotomySlow,							// unused
	TimerPlanetCrackerChannel,
	TimerPlanetCrackerDelay,
	TimerStarSovereignInterventionMissiles,
	TimerQuaziliskCancel,
	TimerCausticDischargeSpeed,
	TimerIonFieldAttackSpeed,
	TimerReconstitutionCooldown,
	TimerBindingGrip,							// unused
	TimerGodsMadeFlesh,
	TimerControlledDemolition,
	TimerEmergencyDetonation,
	TimerNemeanReactors,
	TimerSwarmingOmen,
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
	TimerResourceBoostedRegen,
	TimerAdrenal,
	TimerParasiteArm,
	TimerSwitchModeGimmick,
	TimerCaptivatingClaws,
	TimerAlaszilethRegenBoost,
	TimerWyvernAttackCooldown,
	TimerDirgeFuse,
	TimerSignify,
	TimerReviveFlagClearMin,
	TimerReviveFlagClearMax,
	TimerUnderDisruptionField,
	TimerAlliedInvFrames,
	TimerWaxWingsDebuff,						// unused
	TimerAuralAssaultBlind,
	TimerTarasqueSecondaryWeaponCooldown,
	TimerAnticthonRevive,
	TimerAnthelionGravityWell,
	TimerAnthelionEndlessEclipse,
	TimerAnthelionAttackCooldown,
	TimerStrigaRepulsion,
	TimerAxitriliskShieldLossSFX,
	TimerOwnerChangedRecently,
	TimerMagnetarAttraction,
	TimerSalvageStunTimer,
	TimerCreepMorphOverlay,
	TimerCorpseCocoonHatch,
	TimerTyrannyRecede,
	TimerTalosAnimationSubtimer,
	TimerTalosSoundLoopTimer,
	TimerAddonAnimation,
	TimerSwarmingOmenSFX,
	TimerBladeVortex,
	TimerStellarEnforcementOverlay,

	// probably convert the following to use TimerAI_GenericAbilityCooldown
	TimerAI_Lazarus,
	TimerAI_Siphon,
	TimerAI_Clarion,
	TimerAI_Parasite,
	TimerAI_Skyfall,
	TimerAI_GriefOfAllGods,
	TimerAI_DisruptionField,
	TimerAI_MadrigalTempoChangeTimer,
	TimerAI_EnsnaringBrood,	

	TimerAI_GenericAbilityCooldown, // reusable for multiple unit ids, probably add second one if unit has two seperate abilities
	TimerAI_KitingCooldown,
	TimerAI_KitingLeash,
	TimerAI_KitingSync,
	TimerAI_KitingCombat,

	TimerCount,
};

enum ExtUnitId
{
	UnitBroodlingMother,			// is this still really used?
	UnitDecaySavant,
	UnitMindController,
	UnitWyvernTargetLock,
	UnitDirgeSource,
	UnitBunkerLockOn,
	UnitEmpressSource,
	UnitAnthelionGravityWellSource,
	UnitTurretParentLink,
	UnitBlissfulEmbrace,
	UnitMorphConnectedUnit,

	UnitCount,
};

enum ExtValueVectorId
{
	ValueVectorSignifySourcePlayer,

	ValueVectorCount,
};

enum ExtUnitVectorId
{
	UnitVectorCaptivatingClaws,
	UnitVectorPhaseLink,
	UnitVectorPhaseLinkClarions,
	UnitVectorWorshipersDemiurge,
	UnitVectorCrucibles,
	UnitVectorMindControlling,

	UnitVectorCount,
};

enum ExtTimerVectorId
{
	TimerVectorCyclopsFuse,
	TimerVectorCorrupted, // last slot is always corrupted on some builds, fuck you visual studio

	TimerVectorCount,
};

struct UnitExt
{
	u32						shield_regen_timer;
	u32						damage_receive_timer;
	u32						damage_dealt_timer;
	Point16					anthelion_last_move_target;
	Vector32f				repulsion_vector;
	Point16					last_bullet_launch_position;
	Point16					ai_kiting_origin;
	//	Target					rally;

	u8						seen_by_flags;
	Point16					last_seen_position[PlayerLimit];
	u32						last_seen_timer[PlayerLimit];

	f32						building_tick_progress;
	f32						building_tick_last_progress;
	u32						building_tick_last_update;

	u32						salvage_time;
	f32						salvage_minerals;
	f32						salvage_gas;

	u32						ai_timer_avoid_targeting[PlayerLimit];
	u32						parasite_spawn_count[PlayerLimit];

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

struct SpriteExt {
	bool is_fow_sprite;
	Unit* owner;
};

extern UnitExt unit_ext_array[UNIT_ARRAY_LENGTH];
extern TileExt tile_ext_array[256][256];
extern SpriteExt sprite_ext_array[SPRITE_ARRAY_LENGTH];

void init_ext_variables();
void update_ext_variables();

TileExt* get_tile_ext_at_tile(u32 tile_x, u32 tile_y);
TileExt* get_tile_ext_at_coords(u32 x, u32 y);

UnitExt* get_unit_ext(Unit* unit);
void reset_unit_ext(Unit* unit);

SpriteExt* get_sprite_ext(Sprite* sprite);
void reset_sprite_ext(Sprite* sprite);

bool can_place_on_creep_tile(TileExt* tile_ext, u32 player);