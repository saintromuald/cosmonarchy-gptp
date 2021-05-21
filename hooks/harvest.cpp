#include "harvest.h"
#include "../SCBW/api.h"


//-------- Actual hooks --------//

namespace hooks
{

	const s32 RESOURCE_HARVESTED = 8;

	// Harvests minerals/gas from the @p resource and returns the amount that a worker should carry.
	// Identical to sub_469650

	bool isGasContainer(u16 unitId)
	{
		if (unitId == UnitId::ProtossAquifer)
		{
			return true;
		}
		if (unitId == UnitId::TerranReservoir)
		{
			return true;
		}
		if (unitId == UnitId::ZergExtractor)
		{
			return true;
		}
		return false;
	}

	u8 harvestResourceFrom(Unit* resource, bool isMineral)
	{

		u8 return_value;

		if (resource->building.resource.resourceAmount < RESOURCE_HARVESTED)
		{

			if (isMineral)
			{
//				resource->remove();
				resource->building.resource.resourceAmount = 2;
				return_value = (u8)(resource->building.resource.resourceAmount);
			}
			else
			{
				resource->building.resource.resourceAmount = 0;
				return_value = 2;
			}

		}
		else
		{
			resource->building.resource.resourceAmount -= RESOURCE_HARVESTED;
			if (isMineral)
			{
				if (resource->building.resource.resourceAmount > 0)
					helper::updateMineralPatchImage(resource);	// equivalent to setResourceCount @ 00468830 ?
				else
				{
					resource->building.resource.resourceAmount = 2;
//					resource->remove();
				}
			}
			else if (resource->building.resource.resourceAmount < RESOURCE_HARVESTED
					&& isGasContainer(resource->id))
			{
				scbw::showErrorMessageWithSfx(resource->playerId, 875, 20); // Gas depleted message and sound
			}
			return_value = RESOURCE_HARVESTED;
		}
		return return_value;
	}

	// Transfers a set amount of resources from a resource patch to a worker.
	void transferResourceToWorkerHook(Unit* worker, Unit* resource)
	{

		if (resource->id >= UnitId::TerranReservoir && resource->id <= UnitId::ResourceMineralFieldType3)
		{
			u32 chunkImageId;
			bool isMineral = false;
			bool bEndThere = false;

			// TODO: Add depleted mineral chunk
			if (resource->id >= UnitId::ResourceMineralField &&
				resource->id <= UnitId::ResourceMineralFieldType3)
			{	// Is a mineral patch
				chunkImageId = ImageId::MineralChunkType1;
				isMineral = true;
			}
			else if (resource->id == UnitId::ProtossAquifer)
			{
				chunkImageId = ImageId::ProtossGasOrbType1;
			}
			else if (resource->id == UnitId::ZergExtractor)
			{
				chunkImageId = ImageId::ZergGasSacType1;
			}
			else if (resource->id == UnitId::TerranReservoir)
			{
				chunkImageId = ImageId::TerranGasTankType1;
			}
			else
			{
				bEndThere = true;
			}

			if (!bEndThere)
			{
				u8 resourceAmount = harvestResourceFrom(resource, isMineral);
				if (resourceAmount < RESOURCE_HARVESTED)
				{
					chunkImageId += 1;	// Use depleted (smaller) chunk image
				}
				if (resourceAmount > 0)
				{

					if (worker->resourceType & 3)
					{ // Is carrying a mineral / gas

					// Identical to function @ 0x004F3900
					// Remove powerup shadows (mineral chunk shadow, psi emitter shadow, etc.)
						worker->sprite->removeOverlay(ImageId::MineralChunkShadow, ImageId::PsiEmitterShadow_Carried);
						//Remove powerup graphics
						worker->sprite->removeOverlay(ImageId::Flag, ImageId::TerranGasTankType2);
						//Remove Uraj / Khalis
						worker->sprite->removeOverlay(ImageId::Uraj, ImageId::Khalis);
						worker->resourceType = 0;
					}
					// AI cheat
					if (scbw::getUpgradeLevel(worker->playerId, UpgradeId::MiningEfficiency) == 1)
					{
						//+125%					
						resourceAmount += (resourceAmount / 4);
					}
					if (scbw::getUpgradeLevel(worker->playerId, UpgradeId::MiningEfficiency) == 2)
					{
						//+133%					
						resourceAmount += (resourceAmount / 3);
					}
					if (scbw::getUpgradeLevel(worker->playerId, UpgradeId::MiningEfficiency) == 3)
					{
						//+150%					
						resourceAmount += (resourceAmount / 2);
					}
					if (scbw::getUpgradeLevel(worker->playerId, UpgradeId::MiningEfficiency) == 4)
					{
						//+175%, mapcrap only				
						resourceAmount += ((resourceAmount / 4) + (resourceAmount / 4) + (resourceAmount / 4));
					}
					if (scbw::getUpgradeLevel(worker->playerId, UpgradeId::MiningEfficiency) == 5)
					{
						//+200%
						resourceAmount *= 2;
					}
					if (scbw::getUpgradeLevel(worker->playerId, UpgradeId::MiningEfficiency) == 6)
					{
						//+225%
						resourceAmount *= (2 + (1 / 4));
					}
					if (scbw::getUpgradeLevel(worker->playerId, UpgradeId::MiningEfficiency) == 7)
					{
						//+250%
						resourceAmount *= (2 + (1 / 2));
					}

					helper::setResourceAmountCarried(worker, resourceAmount, chunkImageId, isMineral);
				}

			}

		}

	} //void transferResourceToWorkerHook(Unit* worker, Unit* resource)

	;

} //hooks

