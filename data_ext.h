#pragma once
#include "BW/api.h"
#include <GameModes/Uprising.h>

using EffectRef = u64;

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
    ValuePlayerColor,
    ValueMissileReturnTarget, // unit
    ValueIncineratePlayer,
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
    ValueAnnexSourcePlayer,
    ValueAxitriliskLaunchPos,
    ValueBactaliskLaunchPos,
    ValueDracadinLaunchPos,
    ValueCharlatanLaunchPos,
    ValueIsNeutralBunker,
    ValueIsZombified,
    ValueCuirassIsSelfReviving,
    ValueTalosAnimationTimer,
    ValueSpecialSovrolethBirth,
    ValuePlayingAddonWorkingAnim,
    ValueMindControlPlayer,
    ValueMindControllerUnitId,
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
    ValueCherbrathalorLaunchPos,
    ValueAssignedHarvesters,
    ValueAssignedAiGasHarvesters,
    ValueActiveHarvesters,
    ValueHarvesterActivity,
    ValueNoCorpse,
    ValueDidactChannelIndex,
    ValueDidactRecalling,
    ValueDiademTargeting,
    ValueDiademTargetPosX,
    ValueDiademTargetPosY,
    ValueCyprianLaunchPos,
    ValueSavantEntropyGauntlets,
    ValueSavantPowerSiphonBuff,
    ValueSavantPowerSiphonDebuff,
    ValuePhalanxSiegeModeLaunchPos,
    ValueRegressionUnstable,
    ValueOriginatedFromAgrileth,
    ValueCustomEggActive,
    ValueCustomEggHatchMax,
    ValueCustomEggHpMax,
    ValueWeaponFireIndex,        // Index of fire event
    ValueWeaponBulletLocalIndex, // Index of bullet in one fire event
    ValueWeaponBulletTotalIndex, // Index of bullet in the whole fire sequence
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
    ValueTriggerArmorBaseSet,
    ValueTriggerArmorBaseAmount,
    ValueTriggerArmorPenBaseSet,
    ValueTriggerArmorPenBaseAmount,
    ValueTriggerDamageBaseSet,
    ValueTriggerDamageBaseAmount,
    ValueTakingDeconstructionDamage,
    ValueTriggerWeaponSplashInnerSet,
    ValueTriggerWeaponSplashInnerAmount,
    ValueTriggerWeaponSplashMediumSet,
    ValueTriggerWeaponSplashMediumAmount,
    ValueTriggerWeaponSplashOuterSet,
    ValueTriggerWeaponSplashOuterAmount,
    ValueTriggerWeaponRangeBaseSet,
    ValueTriggerWeaponRangeBaseAmount,
    ValueTriggerBehaviorProximityCloak,
    ValueTriggerBehaviorSteadyShroud,
    ValueAddonInactive,
    ValueNoMassacreResource,
    ValueJustCastedHallucination,
    ValueInfested,
    ValueInfestedInit,
    ValueInfestingPlayer,
    ValueRepentantChorus,
    ValueDemiurgePlayer,
    ValueMadcapFullChargeRead,
    ValueHasStimpack,
    ValueControlledByDemiurge,
    ValueVorvalHidebreaker,
    ValueTryCastCustomSpell,
    ValueIsMalefactor,
    ValueReclaimed,
    ValueReclamationRegen,
    ValueDisablePassive,
    ValueRefreshSpeedNextFrame,
    ValueWyvernWeaponCharging,
    ValueTurningAggressiveMode,
    ValueUprisingInAttackWave,
    ValueUprisingSpawner,
    ValueCancellingZergStructure,
    ValueIsShade,
    ValuePlayingHatchSound,
    ValueWorkerContext,
    ValueWearAndTear,
    ValueIdolLaunchPos,
    ValueStormDivider,
    ValueCabalistSilentStridesDamage,
    ValueCohortServileVigor,
    ValueOscilliantBindingPlasmaStacks,
    ValueOscilliantBindingPlasmaPlayer,
    ValueSavantLaunchPos1,
    ValueSavantLaunchPos2,
    ValueTiangouLaunchPos,
    ValueArchitectLaunchPos,
    ValueMiasma,
    ValueNathrokHidebreaker,
    ValueIsCloning,
    ValueTicksDecolliding,
    ValueCollisionBandage,
    ValueCritter,
    ValueBlackguardLaunchPos,
    ValueArhaunLaunchPos,
    ValueDelayedRallyMoveX,
    ValueDelayedRallyMoveY,
    ValueOriginatedFromGosvileth,
    ValueNoCollision,
    ValueAirTransport,
    ValueIzirokorLaunchPos,
    ValueSwarmingOmenPlayer,
    ValueKethrostalorLaunchPos,
    ValueBloodyRain,
    ValueDontProcOnDeathEffects,
    ValueTritonAbilityLaunchPos,
    ValueGoldRushPlayer,
    ValueGarvrolHidebreaker,
    ValueUnarLaunchPos,
    ValueNoEnergyOverlay,
    ValueHasWireframeOverride,
    ValueWireframeOverride,
    ValueAmaranthHealAmount,
    ValueIsNydusEntry,
    ValueHasButtonsetOverride,
    ValueButtonsetOverride,
    ValueVicarLaunchPos,
    ValueIntendentLaunchPos,
    ValueHelionLaunchPos,
    ValueTriggerHealRangeSet,
    ValueTriggerHealRangeAmount,
    ValueReassemblyActive,
    ValueSegmentedExpansionStacks,
    ValueDyadAssisting,
    ValueHijackOriginalPlayer,
    ValueAskosiBuildProc,
    ValueBoomingBlastPlayer,
    ValueAshHeap,
    ValueAshHeapPlayer,
    ValueWarringWindsStacks,

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
    TimerAutovitalityClone,
    TimerEnsnaringBrood,
    TimerGoremastSlow,
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
    TimerIncinerateDamagePeriod,
    TimerSignify,
    TimerReviveFlagClearMin,
    TimerReviveFlagClearMax,
    TimerUnderDisruptionField,
    TimerAlliedInvFrames,
    TimerIncubationArm,
    TimerAuralAssaultBlind,
    TimerPullTowards,
    TimerAnthelionEndlessEclipse,
    TimerAnthelionAttackCooldown,
    TimerStrigaRepulsion,
    TimerAxitriliskShieldLossSFX,
    TimerOwnerChangedRecently,
    TimerMagnetarAttraction,
    TimerSalvageStun,
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
    TimerGladiusBindingPlasma,
    TimerCentaurSecondaryWeaponStagger,
    TimerPatriarchAstralRule,
    TimerTerminalSurge,
    TimerTriumphantRiseDebuff,
    TimerMatadorHeal,
    TimerMortothrokorPassive,
    TimerLiftLandDelay,
    TimerSwarmingOmenVisuals,
    TimerDidactStasisFieldOverlay,
    TimerStellarEnforcement,
    TimerDeconstruction,
    TimerKelvinMunitions,
    TimerSavantPowerSiphonBuff,
    TimerSavantPowerSiphonDebuff,
    TimerStun,
    TimerAutocratControlledOpposition,
    TimerSilvertonguePlotTwist,
    TimerClaymoreKelvinMunitionsZap,
    TimerMindTyrantMaelstromSlow,
    TimerGhitorokorRelentlessAggression,
    TimerDemiurgeWorship,
    TimerAxitriliskPerpetualWind,
    TimerReclaimStun,
    TimerAutophagySpawn,
    TimerVilgolethTimedLife,
    TimerAvenge,
    TimerAscend,
    TimerPariahHit,
    TimerHeraldAvenge,
    TimerZergAdrenalFrenzy,
    TimerAnnex,
    TimerZarcavrokorSecondaryFire,
    TimerClearStaggerBlacklist,
    TimerUseLastRepulsion,
    TimerSpawnLarva,
    TimerNathreliskMitosalFog,
    TimerStimpackDelay,
    TimerRecallEffect,
    TimerArtisanReturn,
    TimerLiftoffDelay,
    TimerReconstitutionDelay,
    TimerInfesting,
    TimerRepentantChorus,
    TimerSwarmingOmenAudio,
    TimerTurningRoam,
    TimerSavantFormSwitchCooldown,
    TimerSevenstepMindMatterCarrier,
    TimerOlympianMythicalMightSFX,
    TimerBindingPlasmaChargeOverlay,
    TimerUnitBanter,
    TimerDemiurgeWorshipOverlayDelay,
    TimerAddonDisconnectDelay,
    TimerMindTyrantTyranny,
    TimerHidebreaker,
    TimerLucilleriPheremones,
    TimerTryCastCustomSpell,
    TimerGenericAbilityAnimation,
    TimerAnthelionTurn,
    TimerManifoldConcurringChorus,
    TimerSkryspawn,
    TimerWyvernWeaponCharge,
    TimerConstructionSFXDelay,
    TimerTurningAggressiveModeThrottle,
    TimerClericHeal,
    TimerReclaimHeal,
    TimerTranquilityArray,
    TimerCriticality,
    TimerOscilliantBindingPlasma,
    TimerCorpseConsumptionDelay,
    TimerVengefulMarch,
    TimerMalefactorTyrannize,
    TimerMalefactorStun,
    TimerDiscordantChorus,
    TimerBulwarkAstralSplendor,
    TimerAstralSplendorCooldown,
    TimerFacadePryingEyesChannel,
    TimerFacadePryingEyesCloak,
    TimerRecallInterval,
    TimerAdaptiveAmitosisGrowth,
    TimerWanderDelay,
    TimerDelayedRally,
    TimerOgunNaniteDrones,
    TimerBurnDownDelay,
    TimerCannibalHealing,
    TimerSermonSalve,
    TimerSermonSalveOverlay,
    TimerIncinerateDamage,
    TimerIncinerateArmor,
    TimerSwarmingOmenSpeed,
    TimerAmbientSFX,
    TimerMimicryCheck,
    TimerTerranWindup,
    TimerGoldRush,
    TimerDilettanteLuck,
    TimerIsBeingHijacked,
    TimerAdrenalFrenzyVisuals,
    TimerImpactDissipation,
    TimerImpactDissipationVisuals,
    TimerDivineDisciplineBuff,
    TimerDivineDisciplineDebuff,
    TimerRejoinderOfService,
    TimerBoomingBlast,
    TimerAskosiPowerBreach,
    TimerSegmentedExpansion,
    TimerNaniteFieldLockout,
    TimerPowerOverload,
    TimerAshHeapArmorRend,
    TimerAshHeap,
    TimerRetaliationAssembly,
    TimerNewDominion,
    TimerWarringWindsCooldown,
    TimerScrapworldImplosion,
    TimerVitalRemainsArmorPen,
    TimerMechanicalRage,
    TimerCrushingDelivery,

    // probably convert the following to use TimerAI_GenericAbilityCooldown
    TimerAI_Lazarus,
    TimerAI_Siphon,
    TimerAI_Clarion,
    TimerAI_Skyfall,
    TimerAI_GriefOfAllGods,
    TimerAI_DisruptionField,
    TimerAI_MadrigalTempoChangeCooldown,

    TimerAI_GenericAbilityCooldown, // reusable for multiple unit ids, probably add second one if unit has two seperate abilities
    TimerAI_KitingCooldown,
    TimerAI_KitingLeash,
    TimerAI_KitingSync,
    TimerAI_KitingWorkerCombat,
    TimerAI_KitingActive,
    TimerAI_Patience,

    TimerCount,
};

enum ExtUnitId {
    UnitBroodlingMother,			// is this still really used?
    UnitDecaySavant,
    UnitWyvernTargetLock,
    UnitDirgeSource,
    UnitBunkerLockOn,
    UnitAnnexSource,
    UnitPullTowardsSource,
    UnitTurretParentLink,
    UnitMorphConnectedUnit,
    UnitAddonParent,
    UnitResponseTarget,
    UnitCentaurSecondaryWeaponTarget,
    UnitDiademTarget,
    UnitPennant,
    UnitMindTyrantLink,
    UnitMindTyrantMalefactor,
    UnitInfestingMatraleth,
    UnitPlagueSource,
    UnitHijacker,
    UnitHijackTarget,
    UnitHiveOrgan,
    UnitHiveTissue,
    UnitPowerOverloadSource,

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
    UnitVectorStaggerBlacklist,
    UnitVectorBarghests,
    UnitVectorManifolds,
    UnitVectorAdaptiveAmitosis,
    UnitVectorAskosiBuilders,
    UnitVectorScepters,

    UnitVectorCount,
};

enum ExtTimerVectorId {
    TimerVectorCorrupted, // last slot is always corrupted on some builds, fuck you visual studio

    TimerVectorCount,
};

struct UnitExt {
    bool					pause_timers;
    u32						energy_regen_timer;

    // Every unit starts counted as out of combat
    u32						shield_regen_timer = UINT_MAX;
    u32						damage_taken_timer = UINT_MAX;
    u32						damage_dealt_timer = UINT_MAX;

    u32                     total_damage_taken;
    u32                     total_damage_dealt;
    Point16					anthelion_last_move_target;
    Vec32f					repulsion_vector;
    Point16					ai_kiting_origin;
    Target                  projectile_pattern_target;
    Vec32f					last_repulsion;
//	Target					rally;
    const char*				custom_name;

    u8						seen_by_flags;
    Point16					last_seen_position[PlayerLimit];
    u32						last_seen_timer[PlayerLimit]{
        UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX
    };

    fp32                    build_progress_accumulator;

    u32						salvage_time;
    fp32                    salvage_minerals;
    fp32                    salvage_gas;

    Point16					terran_build_pos;
    Point16					recall_pos;

    u32						ai_timer_avoid_targeting[FullPlayerLimit];

    UnitRef					loaded_unit[TransportCap];

    std::optional<Target>   last_unit_target;
    std::optional<Point16>  target_pos_override;

    EffectRef               current_storm;
    
    Sprite*					power_aura;
    Link<Unit>				psi_link;

    Point16                 last_pos;

    u32                     unar_solitary_focus_stacks;
    u32                     unar_distance_traveled;
    u32                     indomitable_annex_stacks;
    u32                     last_malefactor_player;

    Point16                 pull_pos;

    u32						values[ValueCount];
    u32						timers[TimerCount];
    Unit*					units[UnitCount];
    std::vector<u32>		value_vectors[ValueVectorCount];
    std::vector<Unit*>		unit_vectors[UnitVectorCount];
    std::vector<u32>		timer_vectors[TimerVectorCount];

    inline bool try_reset_timer(ExtTimerId timer, u32 time) {
        if (timers[timer] != 0) {
            return false;
        }
        timers[timer] = time;
        return true;
    }
};

struct TileExt {
    u8 creep_owners;
};

struct BulletExt {
    Point16 bullet_source;
    u32 distance_traveled;
    u32 ticks_traveled;
    u32 divisor_multiplier = 1;
    u32 damage_override = 0;
    u32 hit_count;

    fp32 t;
    Vec32f displacement;

    u32 fire_index;
    u32 bullet_local_index;
    u32 bullet_total_index;

    bool bounced;
    bool is_phantom;
    bool did_pierce;

    Unit* cargo;
    u32 source_id;
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
    u32 create_origin;
    u32 mine_count;
    u32 is_regression;
};

#define GasBuildingsCap 16
struct AiTownExt {
    u32 gas_buildings_count;
    Unit* gas_buildings[GasBuildingsCap];
};

void init_ext_variables();
void update_ext_variables();

UnitExt* get_unit_ext(Unit* unit);
void reset_unit_ext(UnitExt* unit_ext);
void unit_ext_remove_references_to(Unit* unit);

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