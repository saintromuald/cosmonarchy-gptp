#include "buttonsets.h"
#include "SCBW/api.h"
#include "globals.h"

// Main names from Firegraft, comments names from vgce
namespace ButtonRequirements
{
	enum Enum : u32
	{
		Slow_Down_Replay = 0x004DEB40,																	// BTNSCOND_ReplaySlowDown
		Speed_Up_Replay = 0x004DEAF0,																	// BTNSCOND_ReplaySpeedUp
		PlayorPause_Replay = 0x004DEAC0,																// BTNSCOND_ReplayPlayPause
		Always = 0x004282D0,																			// BTNSCOND_Always
		Can_Create_UnitorBuilding = 0x00428E60,															// BTNSCOND_CanBuildUnit
		Can_Build_Subunit = 0x00428E00,																	// BTNSCOND_TrainingFighter
		Mixed_Group_MoveorPatrolorHold_Position = 0x00428DA0,											// BTNSCOND_CanMove
		Mixed_Group_Stop = 0x00428D40,																	// BTNSCOND_CanMoveSpecialCase
		Unit_Has_A_Weapon = 0x00428F30,																	// BTNSCOND_CanAttack
		Attacking_Building = 0x00428ED0,																// BTNSCOND_Stationary
		Cancel_Last = 0x00428530,																		// BTNSCOND_IsTraining
		Cancel_AddOn = 0x00428920,																		// BTNSCOND_IsBuildingAddon
		Larva_Exists_Upgrading_Only = 0x00428500,														// BTNSCOND_HatcheryLairHive
		Rally_Point = 0x00429520,																		// BTNSCOND_Rally
		Rally_Point_While_Upgrading_Only = 0x00429740,													// BTNSCOND_HatcheryLairHiveRally
		ConstructionorMutation_Underway = 0x004284E0,													// BTNSCOND_IsConstructing
		Upgrade_Underway = 0x00428900,																	// BTNSCOND_isUpgrading
		Research_Underway = 0x004288E0,																	// BTNSCOND_IsResearching
		Unit_Is_Carrying_Nothing__Gather_Probe = 0x004284B0,											// BTNSCOND_NoCargo
		Unit_Is_Carrying_Something_Return_Cargo_Probe = 0x00428480,										// BTNSCOND_HasCargo
		Unit_Capacity_Has_Not_Been_Met = 0x00428FF0,													// BTNSCOND_HasRoom
		Carrying_Some_Units = 0x00428EA0,																// BTNSCOND_HasUnit
		Tech_Spell_Not_Researched = 0x00429500,															// BTNSCOND_CanResearch
		Spell_Researched = 0x004294E0,																	// BTNSCOND_HasTech
		Has_Spidermine_and_Is_Researched = 0x00429470,													// BTNSCOND_HasSpidermines
		Upgrade_Not_At_Max_Level = 0x00429450,															// BTNSCOND_CanUpgrade
		Can_Cloak = 0x004293E0,																			// BTNSCOND_CanCloak_0
		Can_Decloak = 0x00429370,																		// BTNSCOND_IsCloaked_0
		Can_Cloak_Mixed = 0x004292C0,																	// BTNSCOND_CanCloak
		Can_Decloak_Mixed = 0x00429210,																	// BTNSCOND_IsCloaked
		Tank_Is_In_Tank_Mode = 0x004291C0,																// BTNSCOND_IsUnsieged
		Tank_Is_In_Siege_Mode = 0x00429170,																// BTNSCOND_IsSieged
		Tank_Is_In_Tank_Mode_Move = 0x00428860,															// BTNSCOND_TankMove
		Nuke_Available = 0x00428810,																	// BTNSCOND_HasNuke
		Recharge_Shields = 0x00428440,																	// BTNSCOND_CanRechargeShields
		Building_Can_Move_And_Has_Lifted_Off = 0x00428420,												// BTNSCOND_IsLiftedCanMove
		Building_Has_Lifted_Off = 0x004283F0,															// BTNSCOND_IsLifted
		Building_Has_Landed = 0x004287D0,																// BTNSCOND_IsLanded
		Ability_Is_Researched = 0x004290F0,																// BTNSCOND_CanBurrow
		Is_Using_Ability = 0x00429070,																	// BTNSCOND_IsBurrowed
		Morph_To_Lurker = 0x00429720,																	// BTNSCOND_CanMorphLurker
		Not_Burrowed_MoveorStoporPatrolorHold_Position = 0x004283C0,									// BTNSCOND_Movement
		Not_Burrowed__Attack = 0x00428FA0,																// BTNSCOND_BattleOrders
		Has_No_Cargo__Gather_Drone = 0x00428780,														// BTNSCOND_ZergNoCargo
		Has_Cargo__Return_Cargo_Drone = 0x00428730,														// BTNSCOND_ZergHasCargo
		Burrowed__Stop_Lurker = 0x00428360,																// BTNSCOND_LurkerStop
		Has_No_Exit = 0x00428340,																		// BTNSCOND_NoNydusExit
		Zerg_Basic_Buildings = 0x00428CB0,																// BTNSCOND_ZergBasic
		Zerg_Advanced_Buildings = 0x00428C30,															// BTNSCOND_ZergAdvanced
		Has_Interceptors = 0x004286E0,																	// BTNSCOND_HasInterceptors
		Has_Scarabs = 0x004286A0,																		// BTNSCOND_HasScarabs
		Two_Units_Selected_and_Not_Researched_Archon_Merge = 0x004296F0,								// BTNSCOND_CanMergeArchonOneSelected
		Two_Units_Selected__Archon_Warp = 0x004296B0,													// BTNSCOND_CanMergeArchonTwoSelected
		Two_Units_Selected_and_Not_Researched_Dark_Archon_Meld = 0x00429680,							// BTNSCOND_CanMergeDarkArchonOneSelected
		Two_Units_Selected__Dark_Archon_Meld = 0x00429640,												// BTNSCOND_CanMergeDarkArchonTwoSelected
		Protoss_Basic_Buildings = 0x00428B80,															// BTNSCOND_ProtossBasic
		Protoss_Advanced_Buildings = 0x00428AD0,														// BTNSCOND_ProtossAdvanced
		Is_Constructing__Halt_SCV = 0x00428310,															// BTNSCOND_SCVisBuilding
		Move__SCV = 0x00428670,																			// BTNSCOND_SCVCanMove
		Stop__SCV = 0x00428640,																			// BTNSCOND_SCVCanStop
		Attack__SCV = 0x00428610,																		// BTNSCOND_SCVCanAttack
		Repair__SCV = 0x004285E0,																		// BTNSCOND_CanRepair
		Gather__SCV = 0x004285A0,																		// BTNSCOND_NoCargoTerran
		Return_Cargo__SCV = 0x00428560,																	// BTNSCOND_HasCargoTerran
		Terran_Basic_Buildings = 0x00428A10,															// BTNSCOND_TerranBasic
		Terran_Advanced_Buildings = 0x00428990,															// BTNSCOND_TerranAdvanced
		Nuke_Train = 0x00428960																			// BTNSCOND_SiloHasNoNuke
	};
}

// Main names from Firegraft, comments names from vgce
namespace ButtonActions
{
	enum Enum : u32
	{
		Cancel_Infestation = 0x00423180,																// BTNSACT_DoNothing
		Rally_Point = 0x004244A0,																		// BTNSACT_RallyPoint
		Select_Larva = 0x00423930,																		// BTNSACT_SelectLarva
		Create_Unit = 0x004234B0,																		// CMDACT_Train
		Cancel_Last = 0x00423490,																		// CMDACT_CancelTrain
		Tank_Mode = 0x00423470,																			// CMDACT_Unsiege
		Siege_Mode = 0x00423450,																		// CMDACT_Unsiege
		Cancel_Construction = 0x00423430,																// CMDACT_CancelConstruction
		Cancel_Morph = 0x00423410,																		// CMDACT_CancelUnitMorph
		Move = 0x00424440,																				// BTNSACT_Move
		Stop = 0x004233F0,																				// CMDACT_Stop
		Attack = 0x00424380,																			// BTNSACT_AttackMove
		Attack_Suicide = 0x00424320,																	// BTNSACT_AttackUnit
		Attack_Building = 0x004242C0,																	// BTNSACT_TowerAttack
		Move__CarrierorReaver = 0x00424260,																// BTNSACT_CarrierMove
		Stop_Carrier = 0x004233D0,																		// CMDACT_CarrierStop
		Stop_Reaver = 0x004233B0,																		// CMDACT_ReaverStop
		Attack_Carrier = 0x00424200,																	// BTNSACT_CarrierAttack
		Attack_Reaver = 0x004241A0,																		// BTNSACT_ReaverAttack
		Build_Subunit = 0x00423390,																		// CMDACT_TrainFighter
		Patrol = 0x00424140,																			// BTNSACT_Patrol
		Hold_Position = 0x00423370,																		// CMDACT_HoldPosition
		Research_Technology = 0x00423350,																// CMDACT_Research
		Cancel_Technology_Research = 0x00423330,														// CMDACT_CancelResearch
		Use_Technology = 0x00423F70,																	// BTNSACT_UseTech
		StimPack = 0x004234D0,																			// CMDACT_Stimpack
		Research_Upgrade = 0x00423310,																	// CMDACT_Upgrade
		Cancel_Upgrade_Research = 0x004232F0,															// CMDACT_CancelUpgrade
		Cancel_AddOn = 0x004232D0,																		// CMDACT_CancelAddon
		Create_Building_Terran = 0x00423EB0,															// BTNSACT_BuildTerran
		Place_COP = 0x00423E90,																			// BTNSACT_PlaceCOP
		Create_Building_Protoss = 0x00423DD0,															// BTNSACT_BuildProtoss
		Create_AddOn = 0x00423D10,																		// BTNSACT_PlaceAddon
		Create_Building_Zerg = 0x00423C50,																// BTNSACT_ZergBuild
		Place_Nydus_Canal_Exit = 0x00423C40,															// BTNSACT_PlaceNydusExit
		Building_Morph = 0x00423860,																	// CMDACT_BuildingMorph
		Land = 0x00423C30,																				// BTNSACT_BuildingLand
		Repair = 0x00423BD0,																			// BTNSACT_Repair
		Unit_Morph = 0x00423790,																		// CMDACT_UnitMorph
		Gather = 0x00423B70,																			// BTNSACT_Harvest
		Return_Cargo = 0x00423760,																		// CMDACT_ReturnCargo
		Burrow = 0x004232B0,																			// CMDACT_Burrow
		Unburrow = 0x00423290,																			// CMDACT_Unburrow
		Cloak = 0x00423730,																				// CMDACT_Cloak
		Decloak = 0x00423270,																			// CMDACT_Decloak
		Lift_Off = 0x00423230,																			// CMDACT_Lift
		Load = 0x00423B40,																				// BTNSACT_LoadUnit
		Unload = 0x00423B00,																			// CMDACT_UnloadAll
		Archon_Warp = 0x00423210,																		// CMDACT_MergeArchon
		Dark_Archon_Meld = 0x004231F0,																	// CMDACT_MergeDarkArchon
		Recharge_Shields = 0x00423AA0,																	// BTNSACT_RechargeShields
		Nuclear_Strike = 0x00423A40,																	// BTNSACT_UseNuke
		Cancel_Nuclear_Strike = 0x004231D0,																// CMDACT_CancelNuke
		Heal = 0x004239E0,																				// BTNSACT_Heal
		Slow_Down_Replay = 0x004DF300,																	// CMDACT_DecreaseReplaySpeed
		PlayorPause_Replay = 0x004DF2C0,																// CMDACT_SetReplaySpeed
		Speed_Up_Replay = 0x004DF260,																	// CMDACT_IncreaseReplaySpeed
		Cancel = 0x00429770,																			// BTNSACT_TargetOrderCancel
		Cancel_Place_Building = 0x00429780,																// BTNSACT_CancelPlacement
		Change_Displayed_Buttons = 0x00459AF0															// BTNSACT_ChangeButtons
	};
}

// Example on how to create a custom button set.
// This makes a copy of the Marine buttonset.
// Note that the function would have to be duplicated for each custom buttonset
BUTTON_SET* getCustomButtonSet()
{
	// Use static variables to avoid memory allocation trouble
	static BUTTON_SET customButtonSet;
	static BUTTON customButtonsArray[50];

	// since it's a copy of the marine set, it would have the same amount of buttons
	customButtonSet.buttonsInSet = buttonSetTable[UnitId::TerranMaverick].buttonsInSet;

	// connectedUnit should usually be init at 0xFFFF.Almost unused (maybe not fully implemented).
	customButtonSet.connectedUnit = 0xFFFF;

	// this should always be like this since the array is static
	customButtonSet.firstButton = &customButtonsArray[0];

	// For each button in the array until reaching the amount of buttonsInSet, copy the data for
	// each member of the button structure from the marine to the custom set.
	for (u32 i = 0; i < customButtonSet.buttonsInSet; i++)
	{
		customButtonsArray[i].actFunc = buttonSetTable[UnitId::TerranMaverick].firstButton[i].actFunc;
		customButtonsArray[i].actStringID = buttonSetTable[UnitId::TerranMaverick].firstButton[i].actStringID;
		customButtonsArray[i].actVar = buttonSetTable[UnitId::TerranMaverick].firstButton[i].actVar;
		customButtonsArray[i].iconID = buttonSetTable[UnitId::TerranMaverick].firstButton[i].iconID;
		customButtonsArray[i].position = buttonSetTable[UnitId::TerranMaverick].firstButton[i].position;
		customButtonsArray[i].reqFunc = buttonSetTable[UnitId::TerranMaverick].firstButton[i].reqFunc;
		customButtonsArray[i].reqStringID = buttonSetTable[UnitId::TerranMaverick].firstButton[i].reqStringID;
		customButtonsArray[i].reqVar = buttonSetTable[UnitId::TerranMaverick].firstButton[i].reqVar;
	}
	return &customButtonSet;
}

namespace hooks
{
	// 00458BC0 
	// Called (with a jump) as last instruction of updateButtonSet
	// Will update the buttonset for the currently selected unit if alone
	// Handle special case of group orders if multiple units are selected
	void updateButtonSetEx()
	{
		u16* const BUTTONSET_CURRENT_BUTTONSETID = (u16*)0x0068C1C4;
		u8 countWorkers = 0;
		u8 countCloakables = 0;
		u8 countNotInAirAndZergs = 0;
		u8 countBurrowables = 0;
		u8 countLarva = 0;
		u16 activeButtonSetId;
		u16 selectedButtonSetId;
		u32 flagsHolder;
		s32 unitIndexFromSelection = 0;
		Unit* unitFromSelection;
		activeButtonSetId = (*activePortraitUnit)->currentButtonSet;
		do
		{ // count units to know if will be using special group buttonset (peons, cloakables...)
			unitFromSelection = clientSelectionGroup->unit[unitIndexFromSelection];
			if (unitFromSelection != nullptr)
			{
				flagsHolder = units_dat::BaseProperty[unitFromSelection->id];
				selectedButtonSetId = unitFromSelection->currentButtonSet;

				if (flagsHolder & UnitProperty::Worker)
				{
					countWorkers++;
				}

				if (flagsHolder & UnitProperty::Cloakable)
				{
					countCloakables++;
				}

				// count flying zerg units (to disable "burrow group" if there's one)
				if (!(unitFromSelection->status & UnitStatus::InAir)
					&& units_dat::GroupFlags[unitFromSelection->id].isZerg)
				{
					countNotInAirAndZergs++;
				}

				if (flagsHolder & UnitProperty::Burrowable)
				{
					countBurrowables++;
				}

				if (unitFromSelection->id == UnitId::ZergLarva)
				{
					countLarva++;
				}

				// if you use a custom buttonset, and are reusing the currentButtonSet property of a unit differently, change these conditions to take it in account
				if (selectedButtonSetId != activeButtonSetId)
				{
					BUTTON_SET* selectedUnitButtonSet = hooks::getButtonSet(selectedButtonSetId);
					BUTTON_SET* activeUnitButtonSet = hooks::getButtonSet(activeButtonSetId);
					if (activeUnitButtonSet->firstButton != selectedUnitButtonSet->firstButton)
					{
						if (selectedUnitButtonSet->connectedUnit != activeButtonSetId)
						{
							// Kerrigan (Ghost) and Raynor (Vulture) have the corresponding normal unit as "connected unit"
							if (activeUnitButtonSet->connectedUnit == selectedButtonSetId)
							{
								activeButtonSetId = selectedButtonSetId;
								*BUTTONSET_CURRENT_BUTTONSETID = selectedButtonSetId;
							}
							else
							{
								*BUTTONSET_CURRENT_BUTTONSETID = UnitId::Buttons_GroupMixed;
							}
						}
					}
				}
			}
			// 00458C88
			unitIndexFromSelection++;
		}
		while (unitIndexFromSelection < SELECTION_ARRAY_LENGTH);
		if (*BUTTONSET_CURRENT_BUTTONSETID != UnitId::Buttons_Blank)
		{
			if (countWorkers == *clientSelectionCount)
			{
				*BUTTONSET_CURRENT_BUTTONSETID = UnitId::Buttons_GroupPeons;
			}
			else if (countCloakables == *clientSelectionCount)
			{
				*BUTTONSET_CURRENT_BUTTONSETID = UnitId::Buttons_GroupCloaker;
			}
			else if ((countBurrowables != 0) && (countNotInAirAndZergs == *clientSelectionCount))
			{
				*BUTTONSET_CURRENT_BUTTONSETID = UnitId::Buttons_GroupBurrower; // aerial units and non-zergs units prevent this (but not Ultralisk)
			}
			else if (countLarva == *clientSelectionCount)
			{
				*BUTTONSET_CURRENT_BUTTONSETID = UnitId::ZergLarva;
			}
		}
	}

	// 00458D50
	// Not calling others converted to C functions
	// Probably making it so the button corresponding to current order has light on
	// Last function called within updateCurrentButtonSet
	void updateButtonSet_Sub458D50()
	{
		BinDlg** const BUTTONSET_DIALOG = (BinDlg**)0x0068C148;
		u8* const BUTTONSET_SUBUNIT_MAINORDERID = (u8*)0x0068C1B8;
		u16* const BUTTONSET_MAINBUTTONGRAPHIC = (u16*)0x0068C1BC;
		u8* const BUTTONSET_UNIT_MAINORDERID = (u8*)0x0068C1E4;

		u8 currentUnitMainOrderId;
		u8 currentSubUnitMainOrderId;
		u16 currentUnitButtonIcon;
		u16 currentSubUnitButtonIcon;
		u16 usedButtonIcon;
		BinDlg* currentButtonDlg;

		currentUnitMainOrderId = *BUTTONSET_UNIT_MAINORDERID;
		currentUnitButtonIcon = orders_dat::ButtonIcon[currentUnitMainOrderId];
		currentButtonDlg = (*BUTTONSET_DIALOG)->childrenDlg;
		currentSubUnitMainOrderId = *BUTTONSET_SUBUNIT_MAINORDERID;

		// if the subunit holds the weapon, get the icon from it
		if (currentSubUnitMainOrderId != 0xFF)
		{
			currentSubUnitButtonIcon = orders_dat::ButtonIcon[currentSubUnitMainOrderId];
		}
		else
		{
			currentSubUnitButtonIcon = 0xFFFF;
		}

		if (currentUnitButtonIcon != 0xFFFF)
		{
			if (currentSubUnitButtonIcon == 0xFFFF || currentUnitButtonIcon != IconId::Stop)
			{
				usedButtonIcon = currentUnitButtonIcon;
			}
			else
			{
				usedButtonIcon = currentSubUnitButtonIcon;
			}
		}
		else
		{
			usedButtonIcon = currentSubUnitButtonIcon;
		}

		if (currentButtonDlg != nullptr)
		{
			bool bStopLooping = false;
			do
			{
				// look for the button corresponding to the one used for the current mainOrder
				if (currentButtonDlg->flags & BinDlgFlags::Visible)
				{
					if (currentButtonDlg->graphic != usedButtonIcon)
					{
						currentButtonDlg = currentButtonDlg->next;
					}
					else
					{
						bStopLooping = true;
					}
				}
				else
				{
					currentButtonDlg = currentButtonDlg->next;
				}
			} while (!bStopLooping && currentButtonDlg != nullptr);
		}
		if (currentButtonDlg == nullptr)
		{
			*BUTTONSET_MAINBUTTONGRAPHIC = 0xFFFF;
		}
		else
		{
			if (!(currentButtonDlg->flags & BinDlgFlags::Unknown0))
			{
				currentButtonDlg->flags |= BinDlgFlags::Unknown0;
				scbw::updateDialog(currentButtonDlg);
			}
			*BUTTONSET_MAINBUTTONGRAPHIC = usedButtonIcon;
		}
	}

	// 00458DE0
	// Case when buttonset would use BUTTONSET_SPECIAL_BUTTONSETID
	// Use updateButtonSetEx() for individual units and groups
	// Called by updateCurrentButtonset and updateSelectedUnitData (004C38B0)
	void updateButtonSet()
	{
		u16* const BUTTONSET_PORTRAIT_BUTTONSETID = (u16*)0x0068C14C;
		u16* const BUTTONSET_CURRENT_BUTTONSETID = (u16*)0x0068C1C4;
		u16* const BUTTONSET_SPECIAL_BUTTONSETID = (u16*)0x0068C1C8; // replay, cancel building...
		b8* const bDoingCancellableTargetOrder = (b8*)0x00641694;
		*BUTTONSET_SPECIAL_BUTTONSETID = UnitId::Buttons_Blank;
		*BUTTONSET_PORTRAIT_BUTTONSETID = UnitId::Buttons_Blank;
		if (!scbw::isInReplay())
		{
			if (*clientSelectionCount != 0)
			{
				if (*bDoingCancellableTargetOrder == 0)
				{
					if (*IS_PLACING_BUILDING)
					{
						*BUTTONSET_SPECIAL_BUTTONSETID = UnitId::Buttons_CancelPlaceBuilding;
					}
				}
				else
				{
					*BUTTONSET_SPECIAL_BUTTONSETID = UnitId::Buttons_Cancel;
				}
			}
		}
		else
		{
			if (*IS_REPLAY_PAUSED != 0)
			{
				*BUTTONSET_SPECIAL_BUTTONSETID = UnitId::Buttons_ReplayPaused;
			}
			else
			{
				*BUTTONSET_SPECIAL_BUTTONSETID = UnitId::Buttons_ReplayPlaying;
			}
		}
		*BUTTONSET_CURRENT_BUTTONSETID = UnitId::Buttons_Blank;
		if ((*clientSelectionCount > 1) && (!scbw::isInReplay()))
		{
			updateButtonSetEx();
		}
	}

	// Add active/selected units checks here and you can change the buttonset to any you created here in GPTP using something based on getCustomButtonSet method or a more customized way.
	// Note: if you use updateButtonSet_Sub4591D0, you can disable the injection of this function in the _inject.cpp file.
	BUTTON_SET* getButtonSet(s32 index)
	{
		return &(buttonSetTable[index]);
	}

	// 004599A0
	// Main updateButtonSet function because it call updateButtonSet() and others
	void updateCurrentButtonset()
	{
		BinDlg** const BUTTONSET_DIALOG = (BinDlg**)0x0068C148;
		u16* const BUTTONSET_PORTRAIT_BUTTONSETID = (u16*)0x0068C14C;
		u8* const BUTTONSET_SUBUNIT_MAINORDERID = (u8*)0x0068C1B8;
		u16* const BUTTONSET_CURRENT_BUTTONSETID = (u16*)0x0068C1C4;
		u16* const BUTTONSET_SPECIAL_BUTTONSETID = (u16*)0x0068C1C8; // replay, cancel building...
		u8* const BUTTONSET_UNIT_MAINORDERID = (u8*)0x0068C1E4;
		u32* const bCanUpdateCurrentButtonSet = (u32*)0x0068C1B0;
		bool jumpTo59A38 = false;
		bool jumpTo59A9F = false;
		bool jumpTo59A83 = false;
		bool jumpTo59A86 = false;
		u8 mainOrderIdHolderUnit_EBP;
		u8 mainOrderIdHolderSubUnit_BL;
		u16 CX_Holder;
		if (!*bCanUpdateCurrentButtonSet)
		{
			// 004599B7
			bool bNeedUpdateButtonSet = false;
			if (*BUTTONSET_SPECIAL_BUTTONSETID != UnitId::Buttons_Blank)
			{
				if (*IS_IN_REPLAY)
				{
					bNeedUpdateButtonSet = true;
				}
			}
			else
			{
				bNeedUpdateButtonSet = true;
			}
			if (bNeedUpdateButtonSet)
			{
				updateButtonSet();
			}
		}
		CX_Holder = *BUTTONSET_SPECIAL_BUTTONSETID;
		// 004599D3
		if (*BUTTONSET_SPECIAL_BUTTONSETID == UnitId::Buttons_Blank)
		{
			// 004599DF
			CX_Holder = *BUTTONSET_CURRENT_BUTTONSETID;
			if (*BUTTONSET_CURRENT_BUTTONSETID == UnitId::Buttons_Blank)
			{
				if (*activePortraitUnit == nullptr)
				{
					jumpTo59A38 = true;
				}
				else
				{
					CX_Holder = (*activePortraitUnit)->currentButtonSet;
				}
			}
		}

		if (!jumpTo59A38)
		{
			// 004599F8
			if (*BUTTONSET_PORTRAIT_BUTTONSETID == CX_Holder)
			{
				jumpTo59A38 = true;
			}
			else
			{
				*BUTTONSET_PORTRAIT_BUTTONSETID = CX_Holder;
				*bCanUpdateCurrentButtonSet = 1;
			}
			if (!jumpTo59A38)
			{
				// 00459A12:
				// draw buttons
				helper::updateCommandCard();
				// refresh popup text over button
				helper::function_459770();
				jumpTo59A9F = (*activePortraitUnit == nullptr);
				if (!jumpTo59A9F)
				{
					mainOrderIdHolderUnit_EBP = (*activePortraitUnit)->mainOrderId;
					CX_Holder = mainOrderIdHolderUnit_EBP;
					if ((*activePortraitUnit)->subunit == nullptr)
					{
						jumpTo59A83 = true;
					}
					else
					{
						mainOrderIdHolderSubUnit_BL = (*activePortraitUnit)->subunit->mainOrderId;
						jumpTo59A86 = true;
					}
				}
			}
		}

		if (jumpTo59A38)
		{
			if (*bCanUpdateCurrentButtonSet)
			{
				// 00459A12:
				// draw buttons
				helper::updateCommandCard();
				// refresh popup text over button
				helper::function_459770();
				jumpTo59A9F = (*activePortraitUnit == nullptr);
				if (!jumpTo59A9F)
				{
					mainOrderIdHolderUnit_EBP = (*activePortraitUnit)->mainOrderId;
					CX_Holder = mainOrderIdHolderUnit_EBP;
					if ((*activePortraitUnit)->subunit == nullptr)
					{
						jumpTo59A83 = true;
					}
					else
					{
						mainOrderIdHolderSubUnit_BL = (*activePortraitUnit)->subunit->mainOrderId;
						jumpTo59A86 = true;
					}
				}
			}
		}

		if (!jumpTo59A83 && !jumpTo59A86 && !jumpTo59A9F)
		{
			// 00459A3C
			if (*activePortraitUnit != nullptr)
			{
				bool jumpTo59ABD = false;
				mainOrderIdHolderUnit_EBP = (*activePortraitUnit)->mainOrderId;
				if ((*activePortraitUnit)->subunit != nullptr)
				{
					mainOrderIdHolderSubUnit_BL = (*activePortraitUnit)->subunit->mainOrderId;
				}
				else
				{
					mainOrderIdHolderSubUnit_BL = 0xFF;
				}
				// 00459A55
				if (*BUTTONSET_UNIT_MAINORDERID == mainOrderIdHolderUnit_EBP)
				{
					// 00459A5D
					if (*BUTTONSET_SUBUNIT_MAINORDERID == mainOrderIdHolderSubUnit_BL)
					{
						jumpTo59ABD = true;
					}
				}
				if (!jumpTo59ABD)
				{
					// 00459A65
					// light up the button of the current order
					updateButtonSet_Sub458D50();
					*BUTTONSET_UNIT_MAINORDERID = mainOrderIdHolderUnit_EBP;
					*BUTTONSET_SUBUNIT_MAINORDERID = mainOrderIdHolderSubUnit_BL;
					// light up the button of the current order
					updateButtonSet_Sub458D50();
				}
			}
		}
		if (jumpTo59A83) // cannot be set if a jump beyond is set
		{
			mainOrderIdHolderSubUnit_BL = 0xFF;
			jumpTo59A86 = true;
		}
		if (jumpTo59A86)
		{
			// light up the button of the current order
			updateButtonSet_Sub458D50();
			*BUTTONSET_UNIT_MAINORDERID = mainOrderIdHolderUnit_EBP;
			*BUTTONSET_SUBUNIT_MAINORDERID = mainOrderIdHolderSubUnit_BL;
			// light up the button of the current order
			updateButtonSet_Sub458D50();
			jumpTo59A9F = true;
		}

		if (jumpTo59A9F)
		{
			if (*IS_IN_REPLAY)
			{
				helper::replayStatBtns(*BUTTONSET_DIALOG);
			}
			// 00459AB3
			*bCanUpdateCurrentButtonSet = 0;
		}
	}

	void updateButtonSet_Sub4591D0()
	{
		BinDlg** const BUTTONSET_DIALOG = (BinDlg**)0x0068C148;
		u16* const BUTTONSET_PORTRAIT_BUTTONSETID = (u16*)0x0068C14C;
		auto buttonsetId = *BUTTONSET_PORTRAIT_BUTTONSETID;
		BinDlg* current_dialog = *BUTTONSET_DIALOG;

		// Custom buttonset overrides
		if (buttonsetId == UnitId::TerranTreasury)
		{
			buttonsetId = UnitId::Special_StartLocation;
		}

		BUTTON_SET* const current_buttonset = getButtonSet(buttonsetId);
		u32* const unknown_0066FF60 = (u32*)0x0066FF60;
		bool bStopThere = false;
		bool jumpto59336 = false;
		bool jumpto5933C = false;
		bool jumpto5934E = false;
		s32 current_button_state;
		BUTTON* current_button;
		u16 buttons_count = 0;
		current_button = current_buttonset->firstButton;
		current_button_state = BUTTON_STATE::Invisible;

		if (current_dialog->controlType != DialogControlTypes::DialogBoxDC)
		{
			current_dialog = current_dialog->parent;
		}
		// 00459203:
		current_dialog = current_dialog->childrenDlg;
		while (current_dialog != nullptr && current_dialog->index != 1)
		{
			current_dialog = current_dialog->next;
		}
		if (current_dialog == nullptr || current_dialog->index != 1)
		{
			bStopThere = true;
		}
		while (!bStopThere)
		{
			// 00459230
			if (current_dialog->index < 1)
			{
				bStopThere = true;
			}
			else if (current_dialog->index > 9)
			{
				jumpto5934E = true;
			}
			else
			{ // loop to ignore the first unused/disabled buttons
				current_button_state = BUTTON_STATE::Invisible;
				while (current_button_state == BUTTON_STATE::Invisible && (buttons_count < current_buttonset->buttonsInSet))
				{
					current_button_state = helper::req_check((u32)current_button->reqFunc, (u8)current_button->reqVar, (u8)*LOCAL_NATION_ID, *activePortraitUnit);

					// I am currently forcing the buttons to show, which is effectively fighting lardcode that forces them not to show when group-selected.
					// This is STUPID, EXPENSIVE, and should be REPLACED as soon as possible. - Pr0nogo
					if (current_button->actFunc == (ACT_FUNC*)ButtonActions::Change_Displayed_Buttons)
					{
						if ((*activePortraitUnit)->id == UnitId::ZergLarva)
						{
							current_button_state = BUTTON_STATE::Enabled;
						}
					}

					if (current_button->reqFunc == (REQ_FUNC*)ButtonRequirements::Spell_Researched)
					{
						UnitExt* unitExt = globals.GetUnitExt((*activePortraitUnit));
						// Terran structure buttonset handling - Controlled Demolition / Emergency Detonation
						if (current_button->reqVar == TechId::ControlledDemolition)
						{
							if (unitExt->timers[TimerControlledDemolition] || unitExt->timers[TimerEmergencyDetonation])
							{
								current_button_state = BUTTON_STATE::Invisible;
							}
							else
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
						}
						else if (current_button->reqVar == TechId::EmergencyDetonation)							
						{
							if (!unitExt->timers[TimerControlledDemolition] || unitExt->timers[TimerEmergencyDetonation])
							{
								current_button_state = BUTTON_STATE::Invisible;
							}
							else
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
						}
						// Anticthon buttonset handling - Failure Mode
						else if (current_button->reqVar == TechId::FailureMode)
						{
							if (unitExt->timers[TimerAnticthonFailureMode])
							{
								current_button_state = BUTTON_STATE::Invisible;
							}
							else
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
						}
					}

					if (current_button->reqFunc == (REQ_FUNC*)ButtonRequirements::Can_Create_UnitorBuilding)
					{
						if ((*activePortraitUnit)->id == UnitId::TerranBarracks)
						{
							// Barracks buttonset handling
							if (current_button->reqVar == UnitId::TerranCleric
								|| current_button->reqVar == UnitId::TerranShaman)
							{
								if ((*activePortraitUnit)->building.addon == nullptr)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else if ((*activePortraitUnit)->building.addon->id != UnitId::TerranMedbay)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else
								{
									current_button_state = BUTTON_STATE::Enabled;
								}
							}
							else if (current_button->reqVar == UnitId::TerranEidolon
									|| current_button->reqVar == UnitId::TerranSavant)
							{
								if ((*activePortraitUnit)->building.addon == nullptr)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else if ((*activePortraitUnit)->building.addon->id != UnitId::TerranCovertOps)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else
								{
									current_button_state = BUTTON_STATE::Enabled;
								}
							}
						}

						else if ((*activePortraitUnit)->id == UnitId::TerranFactory)
						{
							// Factory buttonset handling
							if (current_button->reqVar == UnitId::TerranSouthpaw
								|| current_button->reqVar == UnitId::TerranPhalanxTankMode)
							{
								if ((*activePortraitUnit)->building.addon == nullptr)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else if ((*activePortraitUnit)->building.addon->id != UnitId::TerranMachineShop)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else
								{
									current_button_state = BUTTON_STATE::Enabled;
								}
							}
							else if (current_button->reqVar == UnitId::TerranMadrigal
									|| current_button->reqVar == UnitId::TerranPaladin)
							{
								if ((*activePortraitUnit)->building.addon == nullptr)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else if ((*activePortraitUnit)->building.addon->id != UnitId::TerranSalvoYard)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else
								{
									current_button_state = BUTTON_STATE::Enabled;
								}
							}
						}

						else if ((*activePortraitUnit)->id == UnitId::TerranStarport)
						{
							// Starport buttonset handling
							if (current_button->reqVar == UnitId::TerranSalamander
								|| current_button->reqVar == UnitId::TerranWyvern)
							{
								if ((*activePortraitUnit)->building.addon == nullptr)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else if ((*activePortraitUnit)->building.addon->id != UnitId::TerranMunitionsBay)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else
								{
									current_button_state = BUTTON_STATE::Enabled;
								}
							}
							else if (current_button->reqVar == UnitId::TerranAzazel
									|| current_button->reqVar == UnitId::TerranSeraph)
							{
								if ((*activePortraitUnit)->building.addon == nullptr)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else if ((*activePortraitUnit)->building.addon->id != UnitId::TerranControlTower)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else
								{
									current_button_state = BUTTON_STATE::Enabled;
								}
							}
						}

						// Scrapyard buttonset handling
						if ((*activePortraitUnit)->id == UnitId::TerranScrapyard)
						{
							if (current_button->reqVar == UnitId::SharedStriga
								|| current_button->reqVar == UnitId::SharedLuminary)
							{
								if ((*activePortraitUnit)->building.addon == NULL)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else if ((*activePortraitUnit)->building.addon->id != UnitId::TerranTinkerersTower)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else
								{
									current_button_state = BUTTON_STATE::Enabled;
								}
							}
							if (current_button->reqVar == UnitId::SharedTarasque
								|| current_button->reqVar == UnitId::SharedAnticthon)
							{
								if ((*activePortraitUnit)->building.addon == NULL)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else if ((*activePortraitUnit)->building.addon->id != UnitId::TerranBioticBase)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else
								{
									current_button_state = BUTTON_STATE::Enabled;
								}
							}
						}

						else if ((*activePortraitUnit)->id == UnitId::TerranCaptaincy)
						{
							// Captaincy buttonset handling
							if (current_button->reqVar == UnitId::TerranOlympian
								|| current_button->reqVar == UnitId::TerranAutocrat)
							{
								if ((*activePortraitUnit)->building.addon == nullptr)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else if ((*activePortraitUnit)->building.addon->id != UnitId::TerranGuildhall)
								{
									current_button_state = BUTTON_STATE::Invisible;
								}
								else
								{
									current_button_state = BUTTON_STATE::Enabled;
								}
							}
						}
					}

					// Passive icons (yes, it's quite ugly)
					if (current_button->reqFunc == (REQ_FUNC*)ButtonRequirements::Spell_Researched)
					{
						if (current_button->iconID == IconId::Lockdown
							&& (*activePortraitUnit)->vulture.spiderMineCount == 0)
						{
							current_button_state = BUTTON_STATE::Invisible;
						}
						if (current_button->reqVar == TechId::Healing && current_button->iconID != IconId::Heal)
						{
							current_button_state = BUTTON_STATE::Disabled;

							// SHARED \\
								
							if ((*activePortraitUnit)->id == UnitId::SharedCohort &&  // Cohort Servile Vigor passive 
								current_button->iconID == IconId::TerranInfantryArmor)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::SharedCharlatan && // Charlatan Leading the Blind passive
								current_button->iconID == IconId::DemiurgePassiveWorship)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::SharedAspirant &&  // Aspirant Criticality passive 
								current_button->iconID == IconId::ScarabDamage)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::SharedStriga &&  // Striga Fleeting Focus passive 
								current_button->iconID == IconId::PsiAssault)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::SharedStriga &&  // Striga Transient Travels passive 
								current_button->iconID == IconId::EMPShockwave)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::SharedLuminary &&  // Luminary Wax Wings passive 
								current_button->iconID == IconId::GraviticThrusters)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::SharedAnticthon &&  // Anticthon Counter-Proposal passive 
								current_button->iconID == IconId::Toxic_Spores)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::SharedAxitrilisk &&  // Axitrilisk Sating Strikes passive 
								current_button->iconID == IconId::RoarUnused)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::SharedTarasque &&  // Tarasque Fighting Flight passive 
								current_button->iconID == IconId::AcidSpore)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::SharedEmpress &&  // Empress Anodyne Annex passive 
								current_button->iconID == IconId::Particle_Beam_Harvest)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}

							// TERRAN \\

							else if ((*activePortraitUnit)->id == UnitId::TerranHarakan && // Harakan Apollyoid Adhesive passive
								current_button->iconID == IconId::Flyer_Attack)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranShaman &&  // Shaman Nanite Field passive
								current_button->iconID == IconId::Restoration)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranEidolon &&  // Eidolon Somatic Implants passive
								current_button->iconID == IconId::OcularImplants)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranSavant &&  // Savant Energy Decay passive
								current_button->iconID == IconId::Feedback)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranMadcap && // Madcap Maverick Feeders passive
								current_button->iconID == IconId::Twin_Autocannons)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranHeracles && // Heracles Titan Reactors passive
								current_button->iconID == IconId::TitanReactor)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranApostle &&  // Apostle Blood Tithe passive
								current_button->iconID == IconId::Optical_Flare)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranApostle &&  // Apostle Last Breath passive
								current_button->iconID == IconId::Suicide_Scourge)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranOlympian && // Olympian Mythical Might passive
								current_button->iconID == IconId::PsiAssault)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranAutocrat && // Autocrat Controlled Opposition passive
								current_button->iconID == IconId::OcularImplants)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranSouthpaw &&  // Southpaw Knockout Drivers passive
								current_button->iconID == IconId::BurstLasers)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranSouthpaw &&  // Southpaw Bar-Brawl Refraction passive
								current_button->iconID == IconId::Fusion_Cutter_Harvest)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranCyclops &&  // Cyclops Fragmentary Shells passive
								current_button->iconID == IconId::FragmentationGrenade)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranPhalanxTankMode &&  // Phalanx Babylon Plating passive
								current_button->iconID == IconId::TerranVehiclePlating)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranPhalanxSiegeMode &&  // Phalanx Babylon Plating passive
								current_button->iconID == IconId::TerranVehiclePlating)
							{
								current_button_state = BUTTON_STATE::Disabled; // Disable while sieged
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranMadrigal &&  // Madrigal Furia Salvo passive
								current_button->iconID == IconId::GeminiMissiles &&
								scbw::get_generic_rust_value((*activePortraitUnit), RustId::ToggleTempoChange) == 0)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranMadrigal &&  // Madrigal Shrike Payload passive
								current_button->iconID == IconId::Fusion_Cutter_Harvest &&
								scbw::get_generic_rust_value((*activePortraitUnit), RustId::ToggleTempoChange) > 0)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranPazuzu &&  // Pazuzu Deconstruction passive
								current_button->iconID == IconId::ScarabDamage)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranPenumbra &&  // Penumbra Shroud Piercer passive 
								current_button->iconID == IconId::BurstLasers)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranWraith &&  // Wraith Capacitors passive
								current_button->iconID == IconId::BurstLasers)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranTrojan &&  // Trojan Hotdrop passive
								current_button->iconID == IconId::Land)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranWendigo &&  // Wendigo Automedicus passive
								current_button->iconID == IconId::PhotonCannon_Weapon)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranGorgon &&  // Gorgon Petrification passive
								current_button->iconID == IconId::Argus_Jewel)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranWyvern &&  // Wyvern Reverse Thrust passive
								current_button->iconID == IconId::Afterburners_ON_Unused)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranSalamander &&  // Salamander Incendiary Payload passive
								current_button->iconID == IconId::Flame_Thrower)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranValkyrie &&  // Valkyrie Charon Afterburners passive
								current_button->iconID == IconId::GraviticBoosters)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranAzazel &&  // Azazel Innervating Matrices passive
								current_button->iconID == IconId::Restoration_Research)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranCentaur &&  // Centaur Sunchaser Salvo passive
								current_button->iconID == IconId::GeminiMissiles)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranMinotaur &&  // Minotaur Fore Castle passive 
								current_button->iconID == IconId::TerranShipPlating)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranMinotaur &&  // Minotaur Monoceros Targeting passive 
								current_button->iconID == IconId::SingularityCharge)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranMagnetar &&  // Magnetar Gravitational Theory passive 
								current_button->iconID == IconId::PsionicShockwave)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranPhobos &&  // Phobos Argent Capacitors passive 
								current_button->iconID == IconId::KhaydarinCore)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranBlackjack &&  // Blackjack Wear and Tear passive 
								current_button->iconID == IconId::ProtossAirPlating)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranClaymore &&  // Claymore Kelvin Munitions passive 
								current_button->iconID == IconId::Fusion_Cutter_Harvest)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranSiren &&  // Siren Discovered Attacks passive 
								current_button->iconID == IconId::PhaseDisruptorCannon)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranQuarry &&  // Quarry Meskalloid Stimulant passive 
								current_button->iconID == IconId::Argus_Jewel)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranTreasury &&  // Treasury Expanded Storage passive 
								current_button->iconID == IconId::ApolloReactor)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranIonCannon &&  // Ion Cannon Ion Field passive 
								current_button->iconID == IconId::Optical_Flare)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}

							// PROTOSS \\
								
							else if ((*activePortraitUnit)->id == UnitId::ProtossClarion &&  // Clarion Phase Link passive
								current_button->iconID == IconId::RechargeShields)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossClarion &&  // Clarion Harmony Drive passive
								current_button->iconID == IconId::GraviticDrive)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossEcclesiast &&  // Ecclesiast Astral Blessing passive 
								current_button->iconID == IconId::ProtossGroundWeapons)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossLegionnaire && // Legionnaire Unified Strikes passive
								current_button->iconID == IconId::ProtossGroundWeapons)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossLegionnaire &&  // Legionnaire Blade Vortex passive
								current_button->iconID == IconId::PsiAssault)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossHierophant && // Hierophant Cowardice passive
								current_button->iconID == IconId::PhotonCannon_Weapon)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossHierophant && // Hierophant Malice passive
								current_button->iconID == IconId::PhaseDisruptorCannon)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossCabalist &&  // Cabalist Silent Strikes passive
								current_button->iconID == IconId::ProtossGroundWeapons)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossCantavis &&  // Cantavis Ascendancy passive
								current_button->iconID == IconId::MindControlUnused)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossCantavis &&  // Cantavis Ephemeral Blades passive
								current_button->iconID == IconId::ProtossGroundWeapons)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossAmaranth &&  // Amaranth Eternal Service passive 
								current_button->iconID == IconId::PhaseDisruptorCannon)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossAtreus &&  // Atreus Stalwart Stance passive 
								current_button->iconID == IconId::PlasmaShields)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossArchon &&  // Archon Astral Aegis passive
								current_button->iconID == IconId::RechargeShields)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossAugur &&  // Augur Final Hour passive
								current_button->iconID == IconId::PsionicShockwave)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossPariah &&  // Pariah Heretical Bedlam passive 
								current_button->iconID == IconId::PhaseDisruptor)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossVassal &&  // Vassal Terminal Surge passive 
								current_button->iconID == IconId::RechargeShields)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossSimulacrum &&  // Simulacrum Autovitality passive 
								current_button->iconID == IconId::ApialSensors)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossGolem &&  // Golem Deathless Procession passive 
								current_button->iconID == IconId::LegEnhancements)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossBarghest &&  // Barghest Repentent Chorus passive 
								current_button->iconID == IconId::PhotonCannon_Weapon)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossArchitect &&  // Architect Reactive Payload passive 
								current_button->iconID == IconId::AntimatterMissiles)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossDemiurge &&  // Demiurge Worship passive 
								current_button->iconID == IconId::DemiurgePassiveWorship)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossAurora &&  // Aurora Last Orders passive
								current_button->iconID == IconId::MojoScout)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossGladius &&  // Gladius Binding Plasma passive
								current_button->iconID == IconId::PhaseDisruptor)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossMagister &&  // Magister Stellar Enforcement passive
								current_button->iconID == IconId::PhaseDisruptorCannon)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossDidact &&  // Didact Shroud of Adun passive
								current_button->iconID == IconId::DidactPassiveShroudOfAdun)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossEmpyrean &&  // Empyrean Heavenly Blaze passive
								current_button->iconID == IconId::PulseCannon)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossStarSovereign &&  // Star Sovereign Intervention passive
								current_button->iconID == IconId::AntimatterMissiles)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossAnthelion &&  // Anthelion Gravity Well passive 
							current_button->iconID == IconId::Recall)
							{
							current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossAnthelion &&  // Anthelion Gravity Well passive 
							current_button->iconID == IconId::Argus_Jewel)
							{
							current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossPylon &&  // Pylon Warp Matrix passive
								current_button->iconID == IconId::KhaydarinCore)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossCrucible &&  // Crucible Khaydarin Charge passive
								current_button->iconID == IconId::KhaydarinCore)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ProtossPennant &&  // Pennant Prismatic Surge passive
								current_button->iconID == IconId::KhaydarinCore)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}

							// ZERG \\
								
							else if ((*activePortraitUnit)->id == UnitId::ZergZethrokor &&  // Zethrokor Adrenal Glands passive
								current_button->iconID == IconId::AdrenalGlands)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergVorvrokor && // Vorvrokor Adrenal Surge passive
								current_button->iconID == IconId::MetabolicBoost)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergVorvrokor && // Vorvrokor Captivating Claws passive
								current_button->iconID == IconId::Claws)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergQuazilisk && // Quazilisk Melting Points passive
								current_button->iconID == IconId::Suicide_Infested_Terran)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergIzirokor && // Izirokor Crushing Tide passive
								current_button->iconID == IconId::AcidSpore)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergNathrokor && // Nathrokor Adrenal Frenzy passive
								current_button->iconID == IconId::Suicide_Scourge)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergKalkalisk && // Kalkalisk Lithe Organelle passive
								current_button->iconID == IconId::AcidSpore)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergBactalisk && // Bactalisk Caustic Fission passive
								current_button->iconID == IconId::AcidSpore) {
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergLiiralisk && // Liiralisk Lithe Organelle passive
								current_button->iconID == IconId::AcidSpore)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergAlmaksalisk && // Almaksalisk Flanking Spines passive
								current_button->iconID == IconId::Chitinous_Plating)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergLakizilisk && // Lakizilisk Seismic Spines passive
								current_button->iconID == IconId::SubterraneanSpines)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergMutalisk &&  // Mutalisk Lithe Organelle passive
								current_button->iconID == IconId::AcidSpore)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergGorgrokor &&  // Gorgrokor Probing Incisors passive
								current_button->iconID == IconId::KaiserBlades)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergGorgrokor &&  // Gorgrokor Ravening Mandible passive
								current_button->iconID == IconId::RoarUnused)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergVithrilisk &&  // Vithrlisk Corrosive Touch passive
								current_button->iconID == IconId::VenomUnused)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergVithrilisk &&  // Vithrlisk Critical Mass passive
								current_button->iconID == IconId::Suicide_Infested_Terran)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergGeszithalor &&  // Geszithalor Searing Acid passive
								current_button->iconID == IconId::ZergFlyerAttacks)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergMatraleth &&  // Matraleth Gamete Meiosis passive
								current_button->iconID == IconId::GameteMeiosis)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergRilirokor &&  // Broodling Incubators passive
								current_button->iconID == IconId::RoarUnused)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergKonvilisk &&  // Konvilisk Fertile Spines passive
								current_button->iconID == IconId::Subterranean_Tentacle)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergUltrokor &&  // Ultrakor Saber Siphon passive
								current_button->iconID == IconId::KaiserBlades)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergUltrokor &&  // Ultrakor Kaiser Rampage passive
								current_button->iconID == IconId::AnabolicSynthesis)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergKeskathalor &&  // Keskathalor Caustic Discharge passive
								current_button->iconID == IconId::VenomUnused)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergAlkajelisk &&  // Alkajelisk Parasitic Spores passive
								current_button->iconID == IconId::Seeker_Spores)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergZoryusthaleth &&  // Zoryusthaleth Reconstitution passive
								current_button->iconID == IconId::PneumatizedCaparace)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergZoryusthaleth &&  // Zoryusthaleth Swarming Omen passive
								current_button->iconID == IconId::MetabolicBoost)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergZoryusthaleth &&  // Zoryusthaleth Protective Instincts passive
								current_button->iconID == IconId::MuscularAugments)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergIroleth &&  // Iroleth Endomorphic Accelerant passive
								current_button->iconID == IconId::ZergBasicBuildings)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergAlaszileth &&  // Alaszileth Dermal Accelerant passive
								current_button->iconID == IconId::Consume)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if (((*activePortraitUnit)->id == UnitId::ZergLair			// Lair, Hive, Sire Larval Gestation passive
								|| (*activePortraitUnit)->id == UnitId::ZergHive
								|| (*activePortraitUnit)->id == UnitId::ZergSire) &&
								current_button->iconID == IconId::Suicide_Scourge)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergSire &&  // Sire Ectomorphic Accelerant passive
							current_button->iconID == IconId::ZergAdvancedBuildings)
							{
							current_button_state = BUTTON_STATE::Enabled;
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergLarvalColony &&  // Larval Colony Larval Gestation passive
								current_button->iconID == IconId::MetasynapticNode)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}

							// Passive icons: END \\

						}
					}

					// Attack overrides
					if (current_button->actFunc == (ACT_FUNC*)ButtonActions::Attack)
					{
						if ((*activePortraitUnit)->id == UnitId::TerranBunker)
						{
							int loaded = 0;
							for (int i = 0; i < 8; ++i)
							{								
								Unit* bunkerUnit = (*activePortraitUnit)->getLoadedUnit(i);
								if (bunkerUnit && scbw::unitIsRangedAttacker(bunkerUnit))
								{
									loaded++;
								}
							}
							if (loaded)
							{
								current_button_state = BUTTON_STATE::Enabled;
							}
							else
							{
								current_button_state = BUTTON_STATE::Disabled;
							}
						}
					}

					// Form switch/mode change?
					if (current_button->actFunc == (ACT_FUNC*)ButtonActions::Use_Technology)
					{
						if ((*activePortraitUnit)->id != UnitId::ProtossMindTyrant)
						{
							for (s32 i = 0; i < generictoggles; i++)
							{
								if (current_button->actVar == toggle_abilities[i])
								{
									if (scbw::get_generic_rust_value(*activePortraitUnit, toggle_values[i]) == 1)
									{
										current_button->iconID = button_toggle_off[i];
									}
									else
									{
										current_button->iconID = button_toggle_on[i];
									}
								}
							}
						}

						// Cancel new abilities!
						if (current_button->actVar == TechId::CancelNewAbiliity)
						{
							bool canCancel = false;
							if ((*activePortraitUnit)->id == UnitId::ProtossMindTyrant)
							{
								for (s32 i = 0; i < 12; i++)
								{
									Unit* selU = clientSelectionGroup->unit[i];
									if (selU != nullptr && selU->id == UnitId::ProtossMindTyrant)
									{
										if (scbw::get_aise_value(selU, nullptr, AiseId::SumMindControlCost, 0, 0) > 0)
										{
											canCancel = true;
										}
									}
								}
							}
							else if ((*activePortraitUnit)->id == UnitId::TerranWyvern)
							{
								UnitExt* activeExt = globals.GetUnitExt(*activePortraitUnit);
								if (activeExt->values[ValueWyvernLandGrabOn])
								{
									canCancel = true;
								}
							}
							else if ((*activePortraitUnit)->id == UnitId::ZergAkistrokor
								|| (*activePortraitUnit)->id == UnitId::ZergSovroleth)
							{
								UnitExt* activeExt = globals.GetUnitExt(*activePortraitUnit);
								if (activeExt->units[UnitAttachedTarget])
								{
									canCancel = true;
								}
							}

							if (!canCancel)
							{
								current_button_state = BUTTON_STATE::Invisible;
							}
						}
					}

					if (current_button_state == BUTTON_STATE::Invisible)
					{
						buttons_count++;
						current_button++; // select next button (advance by sizeof(BUTTON))
					}
				}
			}

			// 00459278
			if (!bStopThere && !jumpto5934E)
			{
				jumpto5933C = (!*IS_IN_REPLAY) && (*activePortraitUnit != nullptr) && ((*activePortraitUnit)->playerId != *LOCAL_NATION_ID);
				if (!jumpto5933C)
				{
					// 0045929A
					if (buttons_count >= current_buttonset->buttonsInSet
						|| current_dialog->index < current_button->position)
					{
						jumpto5933C = true;
					}
					else
					{
						// 004592B5
						current_dialog->buttonUser = current_button;
						if (current_dialog->graphic != current_button->iconID)
						{
							// 004592C2
							struct
							{
								u32 unknown_value_1;	//[EBP-1C]	[00]
								u32 unknown_value_2;	//[EBP-18]	[04]
								u32 not_allocated;		//[EBP-14]	[08]
								u16 unknown_value_3;	//[EBP-10]	[0C]
								s16 mouseX;				//[EBP-0E]	[0E]
								s16 mouseY;				//[EBP-0C]	[10]
								u16 not_allocated_2;	//[EBP-0A]	[12]
								u32 buttonState;		//[EBP-08]	[14]
								BUTTON_SET* buttonset;	//[EBP-04]	[18]
							} stack_placeholder;
							current_dialog->graphic = current_button->iconID;
							if (!(current_dialog->flags & BinDlgFlags::Unknown0))
							{
								current_dialog->flags |= BinDlgFlags::Unknown0;
								scbw::updateDialog(current_dialog);
							}
							stack_placeholder.not_allocated = 0;
							stack_placeholder.not_allocated_2 = 0;
							stack_placeholder.unknown_value_1 = 0x00000006;
							stack_placeholder.unknown_value_2 = 0x00000000;
							stack_placeholder.unknown_value_3 = 0x000E;
							stack_placeholder.mouseY = mouse->y;
							stack_placeholder.mouseX = mouse->x;
							stack_placeholder.buttonState = current_button_state;
							stack_placeholder.buttonset = current_buttonset;
							helper::fxnInteract(current_dialog, (u32)&stack_placeholder);
							current_button_state = stack_placeholder.buttonState;
						}
						// 0045930B
						scbw::showDialog(current_dialog);
						if (current_button_state == BUTTON_STATE::Disabled)
						{
							scbw::disableDialog(current_dialog);
							// 0045931C
							if (*unknown_0066FF60 == 0x15)
							{
								current_button->actStringID = 0x02FA; // Text: Not Implemented.
								current_button++;	// select next button (advance by sizeof(BUTTON)
								buttons_count++;
								jumpto5934E = true;
							}
							else
							{
								jumpto59336 = true;
							}
						}
						// 00459331
						if (!jumpto59336 && !jumpto5934E)
						{
							helper::enableControl(current_dialog);
						}
						if (!jumpto5934E)
						{
							// 00459336
							current_button++;	// select next button (advance by sizeof(BUTTON)
							buttons_count++;
							jumpto5934E = true;
						}
					}
				}
			}
			if (!bStopThere && !jumpto5934E)
			{
				// 0045933C
				scbw::hideDialog(current_dialog);
				current_dialog->graphic = 0xFFFF;
				current_dialog->buttonUser = nullptr;
			}
			// 0045934E
			current_dialog = current_dialog->next;
			if (current_dialog == nullptr)
			{
				bStopThere = true;
			}
			jumpto59336 = false;
			jumpto5933C = false;
			jumpto5934E = false;
		}
	}

	// 00459AD0
	// Identical to array at 0x0051AD30
	// Size is 40 = 4 * 10 = 0x28
	// 0x004598D0 is statbtn_Btn_Interact
	// 0x00459890 is statbtn_BIN_ReplayProgressbar
	const u32 Btn_fxnInteractFuncs[] =
	{
		0x004598D0, 0x004598D0, 0x004598D0,
		0x004598D0, 0x004598D0, 0x004598D0,
		0x004598D0, 0x004598D0, 0x004598D0,
		0x00459890
	};

	// Using a local array instead of the original may allow to go past the 9 buttons limitations
	// May need a special case using the original array adress and size when in replay if modified
	void statbtn_BIN_CustomCtrlID(BinDlg* dialog)
	{
		helper::registerUserDialogAction(dialog, (u32)&Btn_fxnInteractFuncs, sizeof(Btn_fxnInteractFuncs));
		helper::BINDLG_BlitSurface(dialog);
	}
}