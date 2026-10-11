#pragma once
#include <BW/api.h>

// Taken from hooks/interface/buttonsets.cpp
// Main names from Firegraft, comments names from vgce
namespace ButtonRequirements {
    enum Enum : u32 {
        Slow_Down_Replay = 0x004DEB40,										//BTNSCOND_ReplaySlowDown
        Speed_Up_Replay = 0x004DEAF0,										//BTNSCOND_ReplaySpeedUp
        PlayorPause_Replay = 0x004DEAC0,									//BTNSCOND_ReplayPlayPause
        Always = 0x004282D0,												//BTNSCOND_Always
        Can_Create_UnitorBuilding = 0x00428E60,								//BTNSCOND_CanBuildUnit
        Can_Build_Subunit = 0x00428E00,										//BTNSCOND_TrainingFighter
        Mixed_Group_MoveorPatrolorHold_Position = 0x00428DA0,				//BTNSCOND_CanMove
        Mixed_Group_Stop = 0x00428D40,										//BTNSCOND_CanMoveSpecialCase
        Unit_Has_A_Weapon = 0x00428F30,										//BTNSCOND_CanAttack
        Attacking_Building = 0x00428ED0,									//BTNSCOND_Stationary
        Cancel_Last = 0x00428530,											//BTNSCOND_IsTraining
        Cancel_AddOn = 0x00428920,											//BTNSCOND_IsBuildingAddon
        Larva_Exists_Upgrading_Only = 0x00428500,							//BTNSCOND_HatcheryLairHive
        Rally_Point = 0x00429520,											//BTNSCOND_Rally
        Rally_Point_While_Upgrading_Only = 0x00429740,						//BTNSCOND_HatcheryLairHiveRally
        ConstructionorMutation_Underway = 0x004284E0,						//BTNSCOND_IsConstructing
        Upgrade_Underway = 0x00428900,										//BTNSCOND_isUpgrading
        Research_Underway = 0x004288E0,										//BTNSCOND_IsResearching
        Unit_Is_Carrying_Nothing__Gather_Probe = 0x004284B0,				//BTNSCOND_NoCargo
        Unit_Is_Carrying_Something_Return_Cargo_Probe = 0x00428480,			//BTNSCOND_HasCargo
        Unit_Capacity_Has_Not_Been_Met = 0x00428FF0,						//BTNSCOND_HasRoom
        Carrying_Some_Units = 0x00428EA0,									//BTNSCOND_HasUnit
        Tech_Spell_Not_Researched = 0x00429500,								//BTNSCOND_CanResearch
        Spell_Researched = 0x004294E0,										//BTNSCOND_HasTech
        Has_Spidermine_and_Is_Researched = 0x00429470,						//BTNSCOND_HasSpidermines
        Upgrade_Not_At_Max_Level = 0x00429450,								//BTNSCOND_CanUpgrade
        Can_Cloak = 0x004293E0,												//BTNSCOND_CanCloak_0
        Can_Decloak = 0x00429370,											//BTNSCOND_IsCloaked_0
        Can_Cloak_Mixed = 0x004292C0,										//btnscond_can_cloak
        Can_Decloak_Mixed = 0x00429210,										//btnscond_is_cloaked
        Tank_Is_In_Tank_Mode = 0x004291C0,									//BTNSCOND_IsUnsieged
        Tank_Is_In_Siege_Mode = 0x00429170,									//BTNSCOND_IsSieged
        Tank_Is_In_Tank_Mode_Move = 0x00428860,								//BTNSCOND_TankMove
        Nuke_Available = 0x00428810,										//BTNSCOND_HasNuke
        Recharge_Shields = 0x00428440,										//BTNSCOND_CanRechargeShields
        Building_Can_Move_And_Has_Lifted_Off = 0x00428420,					//BTNSCOND_IsLiftedCanMove
        Building_Has_Lifted_Off = 0x004283F0,								//BTNSCOND_IsLifted
        Building_Has_Landed = 0x004287D0,									//BTNSCOND_IsLanded
        Ability_Is_Researched = 0x004290F0,									//BTNSCOND_CanBurrow
        Is_Using_Ability = 0x00429070,										//BTNSCOND_IsBurrowed
        Morph_To_Lurker = 0x00429720,										//BTNSCOND_CanMorphLurker
        Not_Burrowed_MoveorStoporPatrolorHold_Position = 0x004283C0,		//BTNSCOND_Movement
        Not_Burrowed__Attack = 0x00428FA0,									//BTNSCOND_BattleOrders
        Has_No_Cargo__Gather_Drone = 0x00428780,							//BTNSCOND_ZergNoCargo
        Has_Cargo__Return_Cargo_Drone = 0x00428730,							//BTNSCOND_ZergHasCargo
        Burrowed__Stop_Lurker = 0x00428360,									//BTNSCOND_LurkerStop
        Has_No_Exit = 0x00428340,											//BTNSCOND_NoNydusExit
        Zerg_Basic_Buildings = 0x00428CB0,									//BTNSCOND_ZergBasic
        Zerg_Advanced_Buildings = 0x00428C30,								//BTNSCOND_ZergAdvanced
        Has_Interceptors = 0x004286E0,										//BTNSCOND_HasInterceptors
        Has_Scarabs = 0x004286A0,											//BTNSCOND_HasScarabs
        Two_Units_Selected_and_Not_Researched_Archon_Merge = 0x004296F0,	//BTNSCOND_CanMergeArchonOneSelected
        Two_Units_Selected__Archon_Warp = 0x004296B0,						//BTNSCOND_CanMergeArchonTwoSelected
        Two_Units_Selected_and_Not_Researched_Dark_Archon_Meld = 0x00429680,//BTNSCOND_CanMergeDarkArchonOneSelected
        Two_Units_Selected__Dark_Archon_Meld = 0x00429640,					//BTNSCOND_CanMergeDarkArchonTwoSelected
        Protoss_Basic_Buildings = 0x00428B80,								//BTNSCOND_ProtossBasic
        Protoss_Advanced_Buildings = 0x00428AD0,							//BTNSCOND_ProtossAdvanced
        Is_Constructing__Halt_SCV = 0x00428310,								//BTNSCOND_SCVisBuilding
        Move__SCV = 0x00428670,												//BTNSCOND_SCVCanMove
        Stop__SCV = 0x00428640,												//BTNSCOND_SCVCanStop
        Attack__SCV = 0x00428610,											//BTNSCOND_SCVCanAttack
        Repair__SCV = 0x004285E0,											//BTNSCOND_CanRepair
        Gather__SCV = 0x004285A0,											//BTNSCOND_NoCargoTerran
        Return_Cargo__SCV = 0x00428560,										//BTNSCOND_HasCargoTerran
        Terran_Basic_Buildings = 0x00428A10,								//BTNSCOND_TerranBasic
        Terran_Advanced_Buildings = 0x00428990,								//BTNSCOND_TerranAdvanced
        Nuke_Train = 0x00428960												//BTNSCOND_SiloHasNoNuke
    };
}

// Main names from Firegraft, comments names from vgce
namespace ButtonActions {
    enum Enum : u32  {
        Cancel_Infestation = 0x00423180,									//BTNSACT_DoNothing
        Rally_Point = 0x004244A0,											//BTNSACT_RallyPoint
        Select_Larva = 0x00423930,											//BTNSACT_SelectLarva
        Create_Unit = 0x004234B0,											//CMDACT_Train
        Cancel_Last = 0x00423490,											//CMDACT_CancelTrain
        Tank_Mode = 0x00423470,												//CMDACT_Unsiege
        Siege_Mode = 0x00423450,											//CMDACT_Siege
        Cancel_Construction = 0x00423430,									//CMDACT_CancelConstruction
        Cancel_Morph = 0x00423410,											//CMDACT_CancelUnitMorph
        Move = 0x00424440,													//BTNSACT_Move
        Stop = 0x004233F0,													//CMDACT_Stop
        Attack = 0x00424380,												//BTNSACT_AttackMove
        Attack_Suicide = 0x00424320,										//BTNSACT_AttackUnit
        Attack_Building = 0x004242C0,										//BTNSACT_TowerAttack
        Move__CarrierorReaver = 0x00424260,									//BTNSACT_CarrierMove
        Stop_Carrier = 0x004233D0,											//CMDACT_CarrierStop
        Stop_Reaver = 0x004233B0,											//CMDACT_ReaverStop
        Attack_Carrier = 0x00424200,										//BTNSACT_CarrierAttack
        Attack_Reaver = 0x004241A0,											//BTNSACT_ReaverAttack
        Build_Subunit = 0x00423390,											//CMDACT_TrainFighter
        Patrol = 0x00424140,												//BTNSACT_Patrol
        Hold_Position = 0x00423370,											//CMDACT_HoldPosition
        Research_Technology = 0x00423350,									//CMDACT_Research
        Cancel_Technology_Research = 0x00423330,							//CMDACT_CancelResearch
        Use_Technology = 0x00423F70,										//BTNSACT_UseTech
        StimPack = 0x004234D0,												//CMDACT_Stimpack
        Research_Upgrade = 0x00423310,										//CMDACT_Upgrade
        Cancel_Upgrade_Research = 0x004232F0,								//CMDACT_CancelUpgrade
        Cancel_AddOn = 0x004232D0,											//CMDACT_CancelAddon
        Create_Building_Terran = 0x00423EB0,								//BTNSACT_BuildTerran
        Place_COP = 0x00423E90,												//BTNSACT_PlaceCOP
        Create_Building_Protoss = 0x00423DD0,								//BTNSACT_BuildProtoss
        Create_AddOn = 0x00423D10,											//BTNSACT_PlaceAddon
        Create_Building_Zerg = 0x00423C50,									//BTNSACT_ZergBuild
        Place_Nydus_Canal_Exit = 0x00423C40,								//BTNSACT_PlaceNydusExit
        Building_Morph = 0x00423860,										//CMDACT_BuildingMorph
        Land = 0x00423C30,													//BTNSACT_BuildingLand
        Repair = 0x00423BD0,												//BTNSACT_Repair
        Unit_Morph = 0x00423790,											//CMDACT_UnitMorph
        Gather = 0x00423B70,												//BTNSACT_Harvest
        Return_Cargo = 0x00423760,											//CMDACT_ReturnCargo
        Burrow = 0x004232B0,												//CMDACT_Burrow
        Unburrow = 0x00423290,												//CMDACT_Unburrow
        Cloak = 0x00423730,													//CMDACT_Cloak
        Decloak = 0x00423270,												//CMDACT_Decloak
        Lift_Off = 0x00423230,												//CMDACT_Lift
        Load = 0x00423B40,													//BTNSACT_LoadUnit
        Unload = 0x00423B00,												//CMDACT_UnloadAll
        Archon_Warp = 0x00423210,											//CMDACT_MergeArchon
        Dark_Archon_Meld = 0x004231F0,										//CMDACT_MergeDarkArchon
        Recharge_Shields = 0x00423AA0,										//BTNSACT_RechargeShields
        Nuclear_Strike = 0x00423A40,										//BTNSACT_UseNuke
        Cancel_Nuclear_Strike = 0x004231D0,									//CMDACT_CancelNuke
        Heal = 0x004239E0,													//BTNSACT_Heal
        Slow_Down_Replay = 0x004DF300,										//CMDACT_DecreaseReplaySpeed
        PlayorPause_Replay = 0x004DF2C0,									//CMDACT_SetReplaySpeed
        Speed_Up_Replay = 0x004DF260,										//CMDACT_IncreaseReplaySpeed
        Cancel = 0x00429770,												//BTNSACT_TargetOrderCancel
        Cancel_Place_Building = 0x00429780,									//BTNSACT_CancelPlacement
        Change_Displayed_Buttons = 0x00459AF0								//BTNSACT_ChangeButtons
    };
}