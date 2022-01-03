//Source file for the Transfer Tech & Upgrades hook module.
//This controls how technologies and upgrades are transferred when units are
//passed from one player to another, either by triggers or casting Mind Control.
#include "transfer_tech_upgrades.h"
#include "apply_upgrade_flags.h"
#include <SCBW/api.h>

namespace
{
	struct TechTransferData
	{
		u16 unitId;
		u16 relatedTech[5]; //Maximum number of tech (use TechId::None to mark a premature end-of-list)
	};
	const TechTransferData techTransferData[] = {
		// Terran
		{UnitId::TerranMaverick, TechId::Stimpacks, TechId::None},
		{UnitId::TerranHarakan, TechId::Stimpacks, TechId::None},
		{UnitId::TerranEidolon, TechId::Lockdown, TechId::PersonnelCloaking, TechId::None},
		{UnitId::TerranCleric, TechId::Healing, TechId::Restoration, TechId::None},
		{UnitId::TerranVulture, TechId::SpiderMines, TechId::None},
		{UnitId::TerranPhalanxTankMode, TechId::TankSiegeMode, TechId::None},
		{UnitId::TerranPhalanxSiegeMode, TechId::TankSiegeMode, TechId::None},
		{UnitId::TerranWraith, TechId::CloakingField, TechId::None},
		{UnitId::TerranSeraph, TechId::DefensiveMatrix, TechId::Irradiate, TechId::EMPShockwave, TechId::None},
		{UnitId::TerranMinotaur, TechId::YamatoGun, TechId::None},

		// Zerg
		{UnitId::ZergMatraleth, TechId::Infestation, TechId::Parasite, TechId::SpawnBroodlings, TechId::Ensnare, TechId::None},
		{UnitId::ZergIsthrathaleth, TechId::Burrowing, TechId::DarkSwarm, TechId::Plague, TechId::Consume, TechId::None},
		{UnitId::ZergDroleth, TechId::Burrowing, TechId::None},
		{UnitId::ZergZethrokor, TechId::Burrowing, TechId::None},
		{UnitId::ZergHydralisk, TechId::Burrowing, TechId::LurkerAspect, TechId::None},
		{UnitId::ZergLakizilisk, TechId::LurkerAspect, TechId::None},

		// Protoss
		{UnitId::ProtossCantavis, TechId::ArchonWarp, TechId::PsionicStorm, TechId::Hallucination, TechId::None},
		{UnitId::ProtossDidact, TechId::Recall, TechId::StasisField, TechId::None},
		{UnitId::ProtossLanifect, TechId::DisruptionWeb, TechId::None},
		{UnitId::ProtossCabalist, TechId::DarkArchonMeld, TechId::None},
		{UnitId::ProtossMindTyrant, TechId::MindControl, TechId::Feedback, TechId::Maelstrom, TechId::None},
	};

	struct UpgradeTransferData
	{
		u16 unitId;
		u8 relatedUpgrades[4]; //Maximum number of tech (use UpgradeId::None to mark a premature end-of-list)
	};

	const UpgradeTransferData upgradeTransferData[] = {
		// Terran
		{UnitId::TerranMaverick, UpgradeId::U238Shells, UpgradeId::None},
		{UnitId::TerranEidolon, UpgradeId::OcularImplants, UpgradeId::MoebiusReactor, UpgradeId::None},
		{UnitId::TerranCleric, UpgradeId::CaduceusReactor, UpgradeId::None},
		{UnitId::TerranVulture, UpgradeId::IonThrusters, UpgradeId::None},
		{UnitId::TerranGoliath, UpgradeId::CharonBooster, UpgradeId::None},
		{UnitId::TerranSeraph, UpgradeId::TitanReactor, UpgradeId::None},
		{UnitId::TerranWraith, UpgradeId::ApolloReactor, UpgradeId::None},
		{UnitId::TerranMinotaur, UpgradeId::ColossusReactor, UpgradeId::None},

		// Zerg
		{UnitId::ZergOvileth, UpgradeId::VentralSacs, UpgradeId::Antennae, UpgradeId::None},
		{UnitId::ZergZethrokor, UpgradeId::MetabolicBoost, UpgradeId::AdrenalGlands, UpgradeId::None},
		{UnitId::ZergHydralisk, UpgradeId::MuscularAugments, UpgradeId::GroovedSpines, UpgradeId::None},
		{UnitId::ZergMatraleth, UpgradeId::GameteMeiosis, UpgradeId::None},
		{UnitId::ZergIsthrathaleth, UpgradeId::MetasynapticNode, UpgradeId::None},
		{UnitId::ZergUltrokor, UpgradeId::AnabolicSynthesis, UpgradeId::ChitinousPlating, UpgradeId::None},

		// Protoss
		{UnitId::ProtossZealot, UpgradeId::LegEnhancements, UpgradeId::None},
		{UnitId::ProtossDracadin, UpgradeId::SingularityCharge, UpgradeId::None},
		{UnitId::ProtossCantavis, UpgradeId::KhaydarinAmulet, UpgradeId::None},
		{UnitId::ProtossMindTyrant, UpgradeId::ArgusTalisman, UpgradeId::None},
		{UnitId::ProtossAccantor, UpgradeId::ReinforcedScarabs, UpgradeId::ReaverCapacity, UpgradeId::None},
		{UnitId::ProtossEnvoy, UpgradeId::GraviticDrive, UpgradeId::None},
		{UnitId::ProtossWitness, UpgradeId::SensorArray, UpgradeId::GraviticBoosters, UpgradeId::None},
		{UnitId::ProtossPanoptus, UpgradeId::ApialSensors, UpgradeId::GraviticThrusters, UpgradeId::None},
		{UnitId::ProtossSolarion, UpgradeId::AuxilliaryHangars, UpgradeId::None},
		{UnitId::ProtossDidact, UpgradeId::KhaydarinCore, UpgradeId::None},
		{UnitId::ProtossLanifect, UpgradeId::ArgusJewel, UpgradeId::None},
	};

} //Unnamed namespace

namespace hooks
{

	//summon a function passed as parameter, having the same variables layout for both upgrade and tech related functions.
	bool researchHelper(u32 researchTechFunc, Unit* sourceUnit, u32 techId, u32 relativeVariable, u32 destPlayerId)
	{

		static u32 bPreResult;

		__asm {
			PUSHAD
			PUSH destPlayerId		//[EBP+0x14]
			PUSH relativeVariable	//[EBP+0x10] //this variable has different meaning depending on the caller (boolean or number) 
			PUSH techId				//[EBP+0x0C]
			PUSH sourceUnit			//[EBP+0x08]
			CALL researchTechFunc
			MOV bPreResult, EAX
			POPAD
		}

		return (bPreResult != 0);

	} //bool researchHelper(u32 researchTechFunc, Unit* sourceUnit, u32 techId, u32 relativeVariable, u32 destPlayerId)

	;

	//Transfers all tech related to the @p source unit to @p targetPlayerId.
	//Originally researchAllTechsFromUnit @ 004E2C00
	void transferUnitTechToPlayerHook(Unit* sourceUnit, u8 targetPlayerId, u32 researchTechFunc)
	{

		const TechTransferData* techDataArrayPosBegin = &techTransferData[0];								//original was (TechTransferData*) 0x005011CA;
		const TechTransferData* techDataArrayPosEnd = &techTransferData[ARRAY_SIZE(techTransferData)];	//original was (TechTransferData*) 0x005012DE;
		const s32 relatedTechArraySize = 5;

		//Stop if the source unit does not exist
		if (researchTechFunc == NULL || sourceUnit == nullptr)
			helper::SErrSetLastError(87);
		else
		{

			bool bBreakMainLoop = false;
			bool bBreakSubLoop;

			//Browse through TechTransferData structures
			for (TechTransferData* data = (TechTransferData*)techDataArrayPosBegin; !bBreakMainLoop && (data != techDataArrayPosEnd); data++)
			{

				if (sourceUnit->id == data->unitId)
				{

					bBreakSubLoop = false;

					//Browse through techs for current unit
					for (s32 techIndex = 0; !bBreakSubLoop && techIndex < relatedTechArraySize; techIndex++)
					{

						u16 techId = data->relatedTech[techIndex];

						//forced end of array, stop checking techs, but still check others TechTransferData
						//(in case the unit is represented several times?)
						if (techId == TechId::None)
							bBreakSubLoop = true;
						else
						{

							b8 bIsResearched;

							//check if tech is researched by unit for transfer
							if (techId >= TechId::Restoration)
								bIsResearched = TechBw->isResearched[sourceUnit->playerId][techId - TechId::Restoration];
							else
								bIsResearched = TechSc->isResearched[sourceUnit->playerId][techId];

							//probably research the tech of unit if not already researched by player
							//if return false, stop checking techs and TechTransferData
							if (!researchHelper(researchTechFunc, sourceUnit, techId, bIsResearched, targetPlayerId))
							{
								bBreakSubLoop = true;
								bBreakMainLoop = true;
							}

						}

					}

				} //if (sourceUnit->id == data->unitId)

			} //for (TechTransferData* data = (TechTransferData*)techDataArrayPosBegin; !bBreakMainLoop && (data != techDataArrayPosEnd); data++)

		} //if (researchTechFunc != nullptr && sourceUnit != nullptr)

	} //void transferUnitTechToPlayerHook(Unit* sourceUnit, u8 targetPlayerId, u32 researchTechFunc)

	;

	//Transfers all upgrades related to the @p source unit to @p targetPlayerId.
	//Originally upgradeAllUpgradesFromUnit @ 004E2B50
	void transferUnitUpgradesToPlayerHook(Unit* sourceUnit, u8 targetPlayerId, u32 researchUpgradesFunc)
	{

		const UpgradeTransferData* upgradeDataArrayPosBegin = &upgradeTransferData[0];									//original was (UpgradeTransferData*) 0x005012E2;
		const UpgradeTransferData* upgradeDataArrayPosEnd = &upgradeTransferData[ARRAY_SIZE(upgradeTransferData)];	//original was (UpgradeTransferData*) 0x005013DC;
		const s32 relatedUpgradeArraySize = 4;

		//Stop if the source unit does not exist
		if (researchUpgradesFunc == NULL || sourceUnit == nullptr)
			helper::SErrSetLastError(87);	//0x57
		else
		{

			bool bBreakMainLoop = false;
			bool bBreakSubLoop;

			//Browse through UpgradeTransferData structures
			for (UpgradeTransferData* data = (UpgradeTransferData*)upgradeDataArrayPosBegin; !bBreakMainLoop && (data != upgradeDataArrayPosEnd); data++)
			{

				if (sourceUnit->id == data->unitId)
				{

					bBreakSubLoop = false;

					//Browse through upgrades for current unit
					for (s32 upgradeIndex = 0; !bBreakSubLoop && upgradeIndex < relatedUpgradeArraySize; upgradeIndex++)
					{

						u16 upgradeId = data->relatedUpgrades[upgradeIndex];

						//forced end of array, stop checking upgrades, but still check others UpgradeTransferData
						//(in case the unit is represented several times?)
						if (upgradeId == UpgradeId::None)
							bBreakSubLoop = true;
						else
						{

							u8 sourceUpgradeLevel;

							//check level of upgrade of by unit for transfer
							if (upgradeId >= UpgradeId::PrismaticShield)
								sourceUpgradeLevel = UpgradesBw->currentLevel[sourceUnit->playerId][upgradeId - UpgradeId::PrismaticShield];
							else
								sourceUpgradeLevel = UpgradesSc->currentLevel[sourceUnit->playerId][upgradeId];

							//probably increase the upgrade level of player until reaching same point as sourceUnit
							//if return false, stop checking upgrades and UpgradeTransferData
							if (!researchHelper(researchUpgradesFunc, sourceUnit, upgradeId, sourceUpgradeLevel, targetPlayerId))
							{
								bBreakSubLoop = true;
								bBreakMainLoop = true;
							}

						}

					}

				} //if (sourceUnit->id == data->unitId)

			} //for (upgradeTransferData* data = (upgradeTransferData*)upgradeDataArrayPosBegin; !bBreakMainLoop && (data != upgradeDataArrayPosEnd); data++)

		} //if (researchupgradeFunc != nullptr && sourceUnit != nullptr)

	}

	;

} //hooks
