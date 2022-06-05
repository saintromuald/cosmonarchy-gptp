// This header file contains the binary layout of the Unit structure.
// For the member functions, see Unit.h
// Based on BWAPI's BW/Unit.h

#include "Target.h"
#include "List.h"
#include <vector>
#include <unordered_map>
#include "SCBW/enumerations/IconId.h"

#pragma pack(1)

#define PlayerLimit 8

struct Image;
struct Sprite;
struct Order;
struct Unit;
struct UnitAi;

namespace NewGameCommand
{
	enum
	{
		ShamanAction					= 0x01,
		CyprianAction					= 0x02,
		RelicAction						= 0x03,
		Aperture						= 0x04,
		PhaseLink						= 0x05, // also Khaydarin Eclipse, obsolete
		StarSovereignTheGriefOfAllGods	= 0x06,
		CreateUnit						= 0x07,
	//	KhaydarinControl				= 0x08, // obsolete
		CarrierPrebuild					= 0x09,

		OverlordDamageField				= 0xA,
		OverlordTurbomode				= 0xB,
		OverlordCustomStim				= 0xC,
		GenericSwitchAbility			= 0x0D,
		Observance						= 0xE,
		IssueCTFCop2					= 0xF,
		OverlordAltitudeChange			= 0x10,
	};
}

namespace GptpId
{
	enum
	{
		NoOpcode					= 0,
		AddCyprianSwitch			= 1,
		SetCyprianStatus			= 2,
		SetEscalation				= 3,
		EmpowerHero					= 4,
		BlindJudgeTimedEffect		= 5,
		SetLastOrders				= 6,
		PrepareToExplode			= 7,
		ErasePersistingUnit			= 8,
		SetGenericSwitch			= 9,
		SetGenericValue				= 10,
		AddGenericValue				= 11,
		LinkWarpRelayToStructure	= 12,
		SetApertureStatus			= 13,
		PlaceScreamerPool			= 14,
		SetPhaseLinkStatus			= 15,
	//	PlaceClarionLinksForUnit	= 16,
	//	SpreadClarionDamage			= 17,
		SetGenericTimer				= 18,
	//	FreeMindControlled			= 19,
	//	SetMindControlLink			= 20,
		SetMaelstromOffset			= 21,
		SetMatrixCaster				= 22,
		SetSublimeShepherd			= 23,
		SetGenericUnit				= 24,
		DirgeImpale					= 25,
	//	ClearClarionLinks			= 26,
		SetGenericVariable			= 27,
		SetSpriteData				= 28,

		// TODO
		SetGuard					= 29,
		SetBaseLayout				= 30,

		Void						= 10005,
	};
}

namespace AiseId
{
	enum
	{
		NoOpcodeAise				= 0,
		SendCyprianValue			= 1,
		RecvEscalationValue			= 2,
		GetRelicByIndex				= 3,
		HasRelic					= 4,
		UpgradeLength				= 5,
		HasMalice					= 6,
		IsSuicidingScout			= 7,
		IsBlowingUp					= 8,
		GenericSwitch				= 9,
		GenericValue				= 10,
		SendWarpRelayValue			= 11,
		SendClarionValue			= 12,
		SendStarSovereignValue		= 13,
		LinkedToWarpRelay			= 14,
	//	HasClarionLink				= 15,
		GenericTimer				= 16,
	//	SumMindControlCost			= 17,
		GetLegionnaireMarks			= 18,
		GetMatrixCasterPlayer		= 19,
		GetShepherds				= 20,
		GetFlingyData				= 21,
		GetSpriteData				= 22,
		GetImageData				= 23,
		ReadSamaseFile				= 24,
		ReadSamaseFile_Ptr			= 25,
		GenericUnit					= 26,
		GenericVariable				= 27,
		GetUnitData					= 28,
		GetFlingyPtr				= 29, // for extender
	};
}

namespace OverlayOffset
{
	enum
	{
		Orange	= 0,
		Green	= 25,
		Red		= 50,
		Blue	= 75,
	};
}

enum ExtValueId
{
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
	ValueNathrokorBuffStacks,
	ValueEmpressPassiveStacks,
	ValueEmpressSourcePlayer,
	ValueAnticthonStacks,
	ValueAxitriliskMissilePos,
	ValueBactaliskMissilePos,
	ValueCharlatanMissilePos,
	ValueIsNeutralBunker,
	ValueIsZombified,
	ValueIsSelfReviving,
	ValueTalosAnimationTimer,
	ValueBlissfulEmbraceOriginalPlayer,
	ValueBlissfulEmbraceOriginalHealth,
	ValueSpecialSovrolethBirth,
	ValuePlayingAddonWorkingAnim,
	ValueTyrannyPlayer,

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
	TimerMaledictionSlow,
	TimerObservanceDebuff,
	TimerObservancePeriod,
	TimerLurkerETA,
	TimerLobotomySlow,
	TimerPlanetCrackerChannel,
	TimerPlanetCrackerDelay,
	TimerStarSovereignInterventionMissiles,
	TimerQuaziliskCancel,
	TimerCausticDischargeSpeed,
	TimerIonFieldSpeed,
	TimerReconstitutionCooldown,
	TimerBindingGrip,
	TimerGodsMadeFlesh,
	TimerControlledDemolition,
	TimerEmergencyDetonation,
	TimerNemeanReactors,
	TimerSwarmingOmen,
	TimerSimulacrumClone,
	TimerEnsnaringBrood,
	TimerNewMedstim,
	TimerAugurGimmick,
	TimerObservanceOverlay,
	TimerNathrokorBuff,
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
	TimerUnderDisruptionWeb,
	TimerAlliedInvFrames,
	TimerWaxWingsDebuff,
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
	UnitParasiteSource,
	UnitHallucinationPointer,
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
	u32						damage_receive_timer;
	u32						damage_dealt_timer;
	u32						nathrokor_target_stacks[PlayerLimit];
	Point16					anthelion_last_move_target;
	Vector32f				repulsion_vector;
	Point16					last_bullet_launch_position;
	Point16					ai_kiting_origin;
//	Target					rally;

	f32						building_tick_progress;
	f32						building_tick_last_progress;
	u32						building_tick_last_update;

	u32						salvage_time;
	f32						salvage_minerals;
	f32						salvage_gas;

	u32						ai_timer_avoid_targeting[PlayerLimit];

	u32						values[ValueCount];
	u32						timers[TimerCount];
	Unit*					units[UnitCount];
	std::vector<u32>		value_vectors[ValueVectorCount];
	std::vector<Unit*>		unit_vectors[UnitVectorCount];
	std::vector<u32>		timer_vectors[TimerVectorCount];
};

namespace RustId
{
	enum
	{
		CastLazarusAgent,
		CastEnsnaringBrood,
		CastGodsMadeFlesh,
		CastEnthrallingAssault,
		CastControlledDemolition,
		CastEmergencyDetonation,
		CastSalvage,

		CancelLandGrab,
		CancelAttachment,

		ToggleSavantOnhit,
		ToggleReverseThrust,
		ToggleDisruptionField,
		ToggleTempoChange,

		DisruptionWebDamage,
		DirgeAttacker,
	};
}

#include "SCBW\enumerations\TechId.h"
const u32 string_toggle_on[] = {1265, 1232, 1355};
const u32 string_toggle_off[] = {1270, 1233, 1351};
const u32 button_toggle_on[] = {IconId::DisruptionWeb2, IconId::AfterburnersOnUnused, IconId::RechargeShields};
const u32 button_toggle_off[] = {IconId::Cancel, IconId::AfterburnersOffUnused, IconId::Feedback};
const u32 toggle_values[] = {RustId::ToggleDisruptionField, RustId::ToggleTempoChange, RustId::ToggleSavantOnhit};
const u32 toggle_abilities[] = {TechId::DisruptionWeb, TechId::MadrigalTempoChange, TechId::SavantOnhitToggle};
const u32 generictoggles = 3;

namespace CosmonarchyFlags1
{
	enum
	{
		BasiliskBurrowed = 1 << 0,
		VorvalingProc = 1 << 1,
		VorvalingEgg = 1 << 2,
		ShamanDeployment = 1 << 3,
		HierophantSignifyUpdateSpeed = 1 << 4,
		UpdateConsoleNewParasite = 1 << 5,
		WarpFlash = 1 << 6,
		Infestation = 1 << 7,
	};
}

// From BWAPI/Source/BW/MovementFlags.h
// Definition of Flags specifying movement type - used in BW#Unit#movement_flags

namespace MovementFlags
{
	enum Enum
	{
		OrderedAtLeastOnce = 0x01,		// It get's to 0 after the first move-involved command - sometimes get's to 1 again
		Accelerating = 0x02,			// Unit is accelerating
		Braking = 0x04,					// Not when unit reaches destination, just when you hit stop, or building is finished 
		StartingAttack = 0x08,			// Always 1 for a moment when the unit is starting new attack cycle
		Moving = 0x10,					// Still moving, but not accelerating
		Lifted = 0x20,					// Lifted structures have this on (just before it moves)
		unknown1 = 0x40,				// The value is unknown, but it has something to do with changing directio n& accelerating
		_alwaysZero1 = 0x80,
		HoverUnit = 0xC1				// OrderedAtLeastOnce | unknown1 | _alwaysZero1
	};
}

// Layout of the Unit structure
// Comments marked with ++ added by RavenWolf
struct UnitLayout
{
	/*0x000*/ Link<Unit> link;
	/*0x008*/ s32       hitpoints;						// Hit points of unit, note that the displayed value in broodwar is ceil(healthPoints/256)
														// Officially called "xLife"
	/*0x00C*/ Sprite*	sprite;
	/*0x010*/ Target    move_target;					// The position or unit to move to. It is NOT an order target.
	/*0x018*/ Point16   next_movement_waypoint;			// The next way point in the path the unit is following to get to its destination.
														// Equal to moveToPos for air units since they don't need to navigate around buildings.
	/*0x01C*/ Point16   next_target_waypoint;			// The desired position
	/*0x020*/ u8        movement_flags;					// Flags specifying movement type, MovementFlags::Enum contains possible explanations
	/*0x021*/ u8        current_direction_1;			// The current direction the unit is facing
	/*0x022*/ u8        flingy_turn_speed;				// AKA flingy turn radius
	/*0x023*/ u8        velocity_direction_1;			/*
															This usually only differs from the currentDirection field for units that can accelerate
															and travel in a different direction than they are facing. For example Mutalisks can change
															the direction they are facing faster than then can change the direction they are moving.
														*/
	/*0x024*/ u16       flingy_id;
	/*0x026*/ u8        new_flags_1;					// Enum NewFlags1 | was unused
	/*0x027*/ u8        flingy_movement_type;
	/*0x028*/ Point16   position;						// Current position of the unit
	/*0x02C*/ Vector32  halt;
	/*0x034*/ u32       flingy_top_speed;				// top speed
	/*0x038*/ u32       current_speed1;					// ++current speed
	/*0x03C*/ u32       current_speed2;					// ++not sure if there are any differences with these 2 variables, they always have the same value
	/*0x040*/ Vector32  current_velocity;
	/*0x048*/ u16       flingy_acceleration;
	/*0x04A*/ u8        current_direction_2;			// ?
	/*0x04B*/ u8        velocity_direction_2;			// pathing related, BWAPI has more info

	/*0x04C*/ u8        player;							// Id of player controlling the unit
	/*0x04D*/ u8        main_order_id;
	/*0x04E*/ u8        main_order_state;				// Used by various orders
														// < Additional order info (mostly unknown, wander property investigated so far) // officially "ubActionState"
														/* 0x01 Moving/Following Order
														0x02 No collide (Larva)?
														0x04 Harvesting? Working?
														0x08 Constructing Stationary
														Note: BWAPI code writer don't actually think these are flags
														*/
	/*0x04F*/ u8        order_signal;					/*
															0x01  Update building graphic/state
															0x02  Casting spell
															0x04  Reset collision? Always enabled for hallucination...
															0x10  Lift/Land state
															0x20 - Bactalisk attack (unused)
														*/
	/*0x050*/ u16       order_unit_id;
	/*0x052*/ u8		egg_origin;						// was unused
	/*0x053*/ u8		new_flags_2;					// Enum NewFlags2 | was unused
	/*0x054*/ u8        main_order_timer;				// A timer for orders, example: time left before minerals are harvested
	/*0x055*/ u8        ground_weapon_cooldown;
	/*0x056*/ u8        air_weapon_cooldown;
	/*0x057*/ u8        spell_cooldown;
	/*0x058*/ Target    order_target;					// officially called ActionFocus

	/*0x060*/ s32       shields;						// Although BWAPI says this is unsigned, StarCraft.exe uses signed comparisons for this value
														// BW shows this value/256

	/*0x064*/ u16       id;								// Unit ID in units.dat
	/*0x066*/ u8		cmbw_resource_regen_cooldown;	// Cosmonarchy BW - mineral regen cooldown
			  u8		unused_67;						// unused
	/*0x068*/ Link<Unit> player_link;
	/*0x070*/ Unit*		subunit;
	/*0x074*/ Order*	order_queue_head;
	/*0x078*/ Order*	order_queue_tail;
	/*0x07C*/ Unit*		auto_target_unit;				// The auto-acquired target (Note: This field is actually used for different targets internally, especially by the AI)
	/*0x080*/ Unit*		connected_unit;					// larva store a reference to the structure who created it
														// units inside transports store a reference to the transport
														// addons store a reference to its parent building (but not the other way around)
	/*0x084*/ u8        order_queue_count;
	/*0x085*/ u8        order_queue_timer;				// Cycles down from from 8 to 0 (inclusive). See also 0x122.
	/*0x086*/ u8        unused_86;
	/*0x087*/ u8        attack_notify_timer;			// Prevent "Your forces are under attack." on every attack
	/*0x088*/ u16       previous_unit_type;				// Also used by zerg buildings while morphing
	/*0x08A*/ u8        last_event_timer;				// Countdown that stops being recent when it hits 0
	/*0x08B*/ u8        last_event_color;				// 17 = was completed (train, morph), 174 = was attacked
	/*0x08C*/ u16       unused_8c;						// Cosmonarchy BW
														// 0x1 - Hierophant Detect Add Maelstrom Overlay
														// 0x2 - Hierophant Detect Remove Maelstrom Overlay

	/*0x08E*/ u8        rank_increase;					// Adds this value to the unit's base rank
	/*0x08F*/ u8        kill_count;


	/*0x090*/ u8        last_attacking_player;			// the player that last attacked this unit
	/*0x091*/ u8        secondary_order_timer;
	/*0x092*/ u8        ai_action_flags;				// Used internally by the AI
	/*0x093*/ u8        user_action_flags;				// some flags that change when the user interacts with the unit
														// 2 = issued an order, 3 = interrupted an order, 4 = hide self before death (self-destruct?)
	/*0x094*/ u16       current_button_set;				// The u16 is a guess, used to be u8
	/*0x096*/ u8        is_cloaked;
	/*0x097*/ u8        movement_state;					// A value based on conditions related to pathing, see BWAPI Path.h or UnitMovement.cs for info
	/*0x098*/ u16       build_queue[5];					// Also train queue for buildings and unit_with_hangar/reaver (stores the unitid) / zerg eggs stores the id of the morphing unit in queue[0]
														// Note that it doesn't begin with index 0, but with #build_queue_slot index.
	/*0x0A2*/ u16       energy;							// Energy Points // officially "xwMagic"
	/*0x0A4*/ u8        build_queue_slot;				// Index of active unit in build_queue.
	/*0x0A5*/ u8        target_order_special;			// A byte used to determine the target ID for the unit (found in units in transports)
	/*0x0A6*/ u8        secondary_order_id;				// (Build addon verified) @todo verify (Cloak, Build, ExpandCreep suggested by EUDDB)
	/*0x0A7*/ u8        damage_overlay_state;			// 0 means the building has the largest amount of fire/blood
	/*0x0A8*/ u16       build_repair_hp_gain;			// Also HP amount increased per frame when a unit/building its constructed/repaired
	/*0x0AA*/ u16       shield_gain;					// Shield gain on construction
	/*0x0AC*/ u16       remaining_build_time;			// Remaining bulding time; also used by powerups (flags) as the timer for returning to their original location.
	/*0x0AE*/ u16       previous_hitpoints;				// The HP of the unit before it changed (example Drone->Hatchery, the Drone's HP will be stored here)
	/*0x0B0*/ struct
	{
		u16 index   : 11;								// 1-based index in the unit table.
		u16 unit_id : 5;
	} loaded_unit[8];									// officially called "uwTransport[8]"
	/*0x0C0*/ union
	{
		struct
		{
			/*0x0C0*/     u8 mine_count;				// Used for Eidolon Lobotomy Mines
		} mine_carrier;
		struct
		{
			/*0x0C0*/     Unit* in_hangar_child;		// First child inside the hangar
			/*0x0C4*/     Unit* out_hangar_child;		// First child outside the hangar
			/*0x0C8*/     u8    in_hangar_count;		// Number inside the hangar
			/*0x0C9*/     u8    out_hangar_count;		// Number outside the hangar
		} unit_with_hangar;		// Accantor, Solarion
		struct
		{
			/*0x0C0*/     Unit*			parent;
			/*0x0C4*/     Link<Unit>	hangar_link;
			/*0x0CC*/     bool			is_outside_hangar;
		} hangar_unit;			// Plasma Shell, Steward

		struct
		{
			/*0x0C0*/     u32 _unknown_00;
			/*0x0C4*/     u32 _unknown_04;
			/*0x0C8*/     u32 flag_spawn_frame;			// flag beacons, the frame that the flag will spawn
		} beacon;

		struct
		{
			/*0x0C0*/     Unit*		addon;
			/*0x0C4*/     u16		addon_build_type;
			/*0x0C6*/     u16		upgrade_research_time;
			/*0x0C8*/     u8		tech_id;
			/*0x0C9*/     u8		upgrade_id;
			/*0x0CA*/     u8		larva_timer;
			/*0x0CB*/     u8		landing_timer;
			/*0x0CC*/     u8		creep_timer;
			/*0x0CD*/     u8		upgrade_level;
			/*0x0CE*/     u16		_padding_0E;
			/*0x0D0*/     union
			{
				struct
				{
					/*0x0D0*/		u16     resource_amount;
					/*0x0D2*/		u8      resource_iscript;
					/*0x0D3*/		u8      gather_queue_count;
					/*0x0D4*/		Unit*	next_gatherer;			// pointer to the next worker unit waiting in line to gather
					/*0x0D8*/		u8      resource_group;
					/*0x0D9*/		u8      resource_belongs_to_ai;
				} resource; // When the unit is resource container
				/*0x0D0*/			Unit*	nydus_exit;				// connected nydus canal
				/*0x0D0*/			Unit*	nuke_missile;			// confirmed to be Unit* and not Sprite*
				/*0x0D0*/			Sprite*	pylon_aura;
				struct
				{
					/*0x0D0*/		Unit*	nuke;					// attached nuke
					/*0x0D4*/		bool	is_ready;
				} silo;
				/*0x0D0*/			Box16	larva_spawn_zone;		// Not game coordinates, but "activity values".
																	// Increased by 25 every time worker returns resources to the side unless over 100, and all
																	// sides are decreased by 1 at the same time. Larvas spawn at the side with smallest value.
			};
		} building;
		struct
		{
			/*0x0C0*/				Unit*		powerup;
			/*0x0C4*/				Target		target_resource;	// Position, and when moving to harvest
			/*0x0CC*/				u16			repair_resource_loss_timer;
			/*0x0CE*/				bool		is_harvesting;
			/*0x0CF*/				u8			resource_carry_amount;
			/*0x0D0*/				Unit*		harvest_target;		// When waiting or harvesting
			/*0x0D4*/				Link<Unit>	harvest_link;		// When there is a gather conflict
		} worker;
		/*0x0D0*/			Point16	powerup_origin;
		/*0x0D4*/			Unit*	powerup_carrier;
	};

	/*0x0DC*/ u32       status;							// See UnitStatus::Enum for more info
	/*0x0E0*/ u8        resource_type;					// Type of resource chunk carried by this worker. 1 = vespene gas, 2 = mineral.
	/*0x0E1*/ u8        wireframe_randomizer;	
	/*0x0E2*/ u8        secondary_order_state;	
	/*0x0E3*/ u8        recent_order_timer;				// Counts down from 15 to 0 when most orders are given, or when the unit moves after reaching a patrol location
	/*0x0E4*/ s32       visibility_status;				// Flags specifying which players can detect this unit (cloaked/burrowed)
	/*0x0E8*/ Point16   secondary_order_pos;			// Unused according to BWAPI
	/*0x0EC*/ Unit*		current_build_unit;				// tied to secondary order according to BWAPI
	/*0x0F0*/ Link<Unit>burrow_link;
	/*0x0F8*/ union
	{
		Target		rally;
		Link<Unit>	psi_link;
	};
	/*0x100*/ void*		path;							// use BW::Path* in BWAPI
	/*0x104*/ u8        pathing_collision_interval;		// unknown
	/*0x105*/ u8        pathing_flags;					// 0x01 = uses pathing; 0x02 = ?; 0x04 = ?
	/*0x106*/ u8        unused_106;
	/*0x107*/ b8		is_being_healed;				// 1 if a cleric is healing this unit
	/*0x108*/ Box16     contour_bounds;					// A rect that specifies the closest contour (collision) points

	/*0x110*/ u16       remove_timer;					// Hallucination, Dark Swarm, Disruption Web, Broodling (but not Scanner Sweep according to BWAPI)
	/*0x112*/ u16       defensive_matrix_hp;
	/*0x114*/ u8        defensive_matrix_timer;
	/*0x115*/ u8        stim_timer;
	/*0x116*/ u8        ensnare_timer;
	/*0x117*/ u8        lockdown_timer;
	/*0x118*/ u8        irradiate_timer;
	/*0x119*/ u8        stasis_timer;
	/*0x11A*/ u8        plague_timer;
	/*0x11B*/ u8        is_under_storm;					// Used to tell if a unit is under psi storm	(is "stormTimer" in BWAPI)
	/*0x11C*/ Unit*		irradiate_caster;
	/*0x120*/ u8        irradiate_player_id;
	/*0x121*/ u8        parasite_flags;					// Each bit corrisponds to the player who has parasited this unit*/
	/*0x122*/ u8        cycle_counter;					// counts/cycles up from 0 to 7 (inclusive). See also 0x85.
	/*0x123*/ u8        is_blind;						// is bool in BWAPI
	/*0x124*/ u8        maelstrom_timer;
	/*0x125*/ u8        cmbw_resource_regen_mining_delay;	// Used on resources for regeneration delay - Cosmonarchy
	/*0x126*/ u8        corrosive_spore_count_unused;		// replaced with generic_value
	/*0x127*/ u8        corrosive_spore_timer_unused[9];	// replaced with generic_timer
	/*0x130*/ u16       offset_index_3_by_3;				// Cycles between 0-12 for each bullet fired by this unit (if it uses a "Attack 3x3 area" weapon)
	/*0x132*/ u8		new_flags_3;						// Cosmonarchy - different flags
															// 0x1 - burrowed bactalisk flag (unused)
															// 0x2 - vorvaling proc flag
															// 0x4 - vorvaling egg flag (unused)
															// 0x8 - shaman deployment flag (unused)
															// 0x10 - hierophant signify update speed flag
															// 0x20 - update console - new parasite
															// 0x40 - warp flash
	/*-133-*/ u8        unused_133;
	/*0x134*/ UnitAi*	unit_ai;
	/*0x138*/ u16       air_strength;
	/*0x13A*/ u16       ground_strength;
	/*0x13C*/ struct
	{
		u32 left, right, top, bottom;
	} finder_index;										// Ordering for unit boundries in unit finder for binary search
	/*0x14C*/ u8        repulse_unknown;
	/*0x14D*/ u8        repulse_angle;					// updated only when air unit is being pushed
	/*0x14E*/ Point8    driftPos;						// (mapsizex / 1.5 max)
};
C_ASSERT(sizeof(UnitLayout) == 336);
#pragma pack()