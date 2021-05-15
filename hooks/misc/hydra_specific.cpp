//
#include "hooks/misc/hydra_specific.h"
#include "globals.h"
#include <cassert>
#include "hooks/orders/unit_making/unit_train.h"
#include "stopwatch/Stopwatch.h"
#include "logger.h"
#include "SCBW/scbwdata.h"

namespace ReqOpcode
{
	enum
	{

		Or = 0xff01,
		CurrentUnitIs = 0xff02,
		HasUnit = 0xff03,
		HasAddonAttached = 0xff04,
		IsNotLifted = 0xff05,
		IsLifted = 0xff06,
		IsNotBusyBuilding = 0xff07,
		IsNotConstructingAddon = 0xff08,
		IsNotTeching = 0xff09,
		IsNotUpgrading = 0xff0a,
		IsNotConstructingBuilding = 0xff0b,
		HasNoAddon = 0xff0c,
		HasNoNydusExit = 0xff0d,
		HasHangarSpace = 0xff0e,
		TechIsResearched = 0xff0f,
		HasNoNuke = 0xff10,
		NotBurrowedOrAIControlled = 0xff11,
		IsNotLandedBuilding = 0xff12,
		IsLandedBuidling = 0xff13,
		CanMove = 0xff14,
		CanAttack = 0xff15,
		IsWorker = 0xff16,
		CanLiftOff = 0xff17,
		IsTransport = 0xff18,
		IsPowerup = 0xff19,
		IsSubunit = 0xff1a,
		HasSpiderMinesOrAddons = 0xff1b,
		IsHero = 0xff1c,
		CanRallyOrRclick = 0xff1d,
		AllowOnHallucinations = 0xff1e,
		UpgradeLevelBasedJump = 0xff1f,//0xff20 if level 1, 0xff20 if level >=2
		Disabled = 0xff22,
		Blank = 0xff23,
		BroodWar = 0xff24,
		IfTechResearched = 0xff25,
		IfBurrowed = 0xff26,
		EndOfSublist = 0xFFFF,
	};
}

//u32 parseUnitDatReqs() {
void parseUnitDatReqs()
{
	//	static u32 result;
	const u32 parseUnitFunc = 0x0046e100;
	__asm {
		PUSHAD
		CALL parseUnitFunc
//		MOV result,EAX
		POPAD
	}
//	return result;
}

bool isAttemptingProtossBuild(const Unit* probe)
{
	return probe->mainOrderId == OrderId::BuildProtoss1
		&& probe->status & UnitStatus::GroundedBuilding
		&& probe->orderTarget.unit != nullptr
		&& !(probe->orderTarget.unit->status & UnitStatus::Completed);
}

#include <array>
namespace hooks
{
	void createDamageOverlay(Sprite* sprite)
	{
		if (sprite->mainGraphic == nullptr)
		{
			return;
		}
		const LO_Header* overlay = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlayDamagePtr, sprite->mainGraphic->id));
		if (overlay == nullptr)
		{
			return; // return if there's no overlay defined
		}
		for (Image* image = sprite->images.head; image; image = image->link.next)
		{
			if (image->id >= ImageId::FirstMinorDamageOverlay
				&& image->id <= ImageId::LastMinorDamageOverlay)
			{
				s32 variation = image->id - ImageId::FirstMinorDamageOverlay;
				image->free();
				if (sprite->mainGraphic->frameIndex >= overlay->frameCount)
				{
					scbw::printf("CreateDamageOverlay Bug: incorrect frame %d, image id %d",
						sprite->mainGraphic->frameIndex, sprite->mainGraphic->id);
					{
						return; 
					}
				}
				Point8 offset = overlay->getOffset(sprite->mainGraphic->frameIndex, variation); // not sure about first arg
				sprite->createTopOverlay(ImageId::FirstMajorDamageOverlay + variation, offset.x, offset.y, 0);
			}
		}
		std::array<s32, 22> results;
		auto result_pos = results.begin();
		if (sprite->mainGraphic->frameIndex >= overlay->frameCount)
		{
			scbw::printf("CreateDamageOverlay Bug: incorrect frame %d, image id %d",
				sprite->mainGraphic->frameIndex, sprite->mainGraphic->id);
			return;
		}
		for (s32 i = 0; i < 22; i++)
		{
			Point8 offset = overlay->getOffset(sprite->mainGraphic->frameIndex, i);
			if (offset.x == 127 && offset.y == 127)
			{
				continue;
			}
			u32 overlay_id = ImageId::FirstMajorDamageOverlay + i;
			bool found = false;
			for (Image* image = sprite->images.head; image; image = image->link.next)
			{
				if (image->id == overlay_id)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				*result_pos++ = i;
			}
		}
		if (result_pos != results.begin())
		{
			s32 variation = results[scbw::randBetween(0, result_pos - results.begin() - 1)];
			Point8 offset = overlay->getOffset(sprite->mainGraphic->frameIndex, variation);
			sprite->createTopOverlay(ImageId::FirstMinorDamageOverlay + variation, offset.x, offset.y, 0);
		}
	}

	// Hooked to enable unit mover when placing buildings
	s32 isTileBlockedBy(Unit** unitArray, Unit* builder, s32 x_tile, s32 y_tile, s8 dontIgnoreReacting, s8 alsoInvisible)
	{ // 00473300
		Unit* unitArrayPtr; // edx
		Unit* builderVar; // edi
		Unit** unitArrayVar; // esi
		DWORD unitStatus; // eax
		BOOL needsDetection; // eax

		unitArrayVar = unitArray;
		unitArrayPtr = *unitArray;
		builderVar = builder;

		if (!*unitArray)
		{
			return 0;
		}

		while (1)
		{
			needsDetection = 0;
			if (builderVar)
			{
				unitStatus = unitArrayPtr->status;
				if (unitStatus & 0x300)
				{ // Requires Detection + Cloaked
					if (!((1 << builderVar->playerId) & unitArrayPtr->visibilityStatus))
						needsDetection = 1;
				}
			}
			if (!dontIgnoreReacting && unitArrayPtr->status & 0x20000
				|| unitArrayPtr == builderVar
				|| unitArrayPtr->status & 6
				|| unitArrayPtr->id == UnitId::Spell_DarkSwarm
				|| unitArrayPtr->id == UnitId::Spell_DisruptionWeb
				|| needsDetection && !alsoInvisible
				|| unitArrayPtr->sprite->flags & 0x20)
			{
				unitArrayPtr = unitArrayVar[1];
				++unitArrayVar;
				if (!unitArrayPtr)
				{
					return 0;
				}
			}
			if (unitOrderingX[unitArrayPtr->finderIndex.left].position < 32 * x_tile + 32
				&& unitOrderingX[unitArrayPtr->finderIndex.right].position > 32 * x_tile
				&& unitOrderingY[unitArrayPtr->finderIndex.top].position < 32 * y_tile + 32
				&& unitOrderingY[unitArrayPtr->finderIndex.bottom].position > 32 * y_tile)
			{
				return needsDetection != 0 ? 1 : 4;
			}
			builderVar = builder;
			unitArrayPtr = unitArrayVar[1];
			++unitArrayVar;
			if (!unitArrayPtr)
			{
				return 0;
			}
		}
	}

	u32 checkUnitDatRequirements(u8 playerId, u16 unit_id, Unit* unit)
	{
		//
		//
		// Warning: This hook don't points to firegraft offsets. Firegraft offsets must be taken into account and added to hook
		// before using it.
		//
		//
		auto v3 = unit_id;
		static u32* lastError = (u32*)0x0066FF60;
		static u16* reqTable = (u16*)0x00514178;
		*lastError = 0;
		if (unit_id >= UnitId::None)
		{
			*lastError = 16;
			return 0;
		}
		if (playerId != unit->playerId)
		{
			*lastError = 1;
			return 0;
		}
		if (!(unit->status & UnitStatus::Completed))
		{
			*lastError = 20;
			return 0;
		}
		if (unit->isFrozen())
		{
			*lastError = 10;
			return 0;
		}
		if (!UnitAvailability->available[unit->playerId][unit->id])
		{
			*lastError = 2;
			return 0;
		}
		if (hooks::isQueueSlotActive(unit, 4))
		{//if build queue is full
LABEL_86:
			*lastError = 3;
			return 0;
		}
		if (unit->status & UnitStatus::IsHallucination)
		{
			*lastError = 22;
			return 0;
		}
		// 57


		if (units_dat::unitsDat43[unit->id] == ReqOpcode::EndOfSublist)
		{
			parseUnitDatReqs();
		}
		s32 v7 = (u16)units_dat::unitsDat43[unit->id];

		if (units_dat::unitsDat43[unit->id] == 0)
		{
LABEL_93:
			*lastError = 23;
			return 0;
		}
		auto v26 = 1;
		auto v25 = 1;
		s32 v8 = reqTable[v7];
		auto v27 = 0;
		if (v8 == -1)
			return 1;
LABEL_18:
		auto v9 = 0;
		auto v28 = 0;
		auto v11 = 0;
		auto v24 = 0;
		auto v14 = 0;
		auto v17 = 0;
		auto v18 = 0;
		Unit* v12 = 0;
		auto requiredId = 0;
		/*		while (true)
				{
					v8 = (u16)v8;
					switch (v8)
					{
					case ReqOpcode::CurrentUnitIs:
						requiredId = reqTable[v7++];
						if (completed_unit_counts[requiredId].player[unit->playerId] > 0)
							v26 = 0;
						if (requiredId == unit->id)
						{
							v27 = 0;
							v25 = 0;
							v11 = v9 + 1;
							goto LABEL_72;//end
						}
						if (v25)
							v27 = 1;
						goto LABEL_73;
					case ReqOpcode::HasUnit:
						requiredId = reqTable[v7++];
						v24 = completed_unit_counts[requiredId].player[unit->playerId] + all_unit_counts[requiredId].player[unit->playerId];
						goto LABEL_71;
					case ReqOpcode::HasAddonAttached:
						if (*CHEAT_STATE & CheatFlags::ModifyThePhaseVariance)
						{
							++v7;
							v11 = v9 + 1;
						}
						else
						{
							v12 = unit->building.addon;
							if (v12 != nullptr || (++v7, v12->id != reqTable[v7]))
							{
								if (!v27)
									*lastError = 4;
								return -1;
							}
							v11 = v9 + 1;
						}
						goto LABEL_72;
					case ReqOpcode::IsNotLifted:
						if (!(unit->status & UnitStatus::GroundedBuilding))
						{
							*lastError = 7;
							return 0;
						}
						v11 = v9 + 1;
						goto LABEL_72;
					case ReqOpcode::IsNotBusyBuilding:
						if (unitIsTrainingOrMorphing(unit) || unit->building.upgradeType != TechId::None ||
							unit->building.techType != UpgradeId::None)
							goto LABEL_87;
						++v28;
						goto LABEL_73;
					case ReqOpcode::IsNotConstructingAddon:
					{
						auto v13 = isConstructingAddon(unit);
						if (v13)
							goto LABEL_87;
						auto v15 = isAttemptingProtossBuild(unit);
						if (v15)
							goto LABEL_87;
						v11 = v14 + 1;
						goto LABEL_72;
					}
					case ReqOpcode::IsNotTeching:
						if (unit->building.techType != TechId::None)
							goto LABEL_87;
						v11 = v9 + 1;
						goto LABEL_72;
					case ReqOpcode::IsNotUpgrading:
						if (unit->building.upgradeType != UpgradeId::None)
							goto LABEL_87;
						v11 = v9 + 1;
						goto LABEL_72;
					case ReqOpcode::IsNotConstructingBuilding:
						if (unit->id == UnitId::TerranSCV && unit->mainOrderId == OrderId::ConstructingBuilding)
							goto LABEL_87;
						v11 = v9 + 1;
						goto LABEL_72;
					case ReqOpcode::HasNoAddon:
						if (unit->building.addon)
						{
							if (!v27)
								*lastError = 4;
							return 0;
						}
						v11 = v9 + 1;
					LABEL_72:
						v28 = v11;
					LABEL_73:
						if (reqTable[++v7] == -255)
						{
							v8 = reqTable[v7];
							v9 = v28;
							++v7;
							continue;
						}
						if (!v28 && v26)
						{
							*lastError = 8;
							return -1;
						}
						v8 = reqTable[v7];
						v26 = 1;
						if (v8 != -1)
							goto LABEL_18;
						if (v27)
						{
							*lastError = 25;
							return -1;
						}
						return 1;
					case ReqOpcode::HasHangarSpace:
					{
						v17 = -1;
						auto v19 = 0;
						auto v21 = 0;
						auto v20 = 0;
						v18 = unitIsCarrier(unit);
						if (v18)
						{
							v19 = unit->carrier.inHangarCount + getHangerTrainCount(unit);
						}
						else
						{
							v21 = unitIsReaver(unit);
							if (!v21)
								goto LABEL_51;
							v19 = getHangerTrainCount(unit);
						}
						v17 = unit->carrier.inHangarCount + v19;

					LABEL_51:
						if (v17 >= getMaxHangerSpace(unit))
							goto LABEL_86;
						++v28;
						goto LABEL_73;
					}
					case ReqOpcode::HasNoNuke:
						if (unit->building.silo.nuke)
							goto LABEL_86;
						v11 = v9 + 1;
						goto LABEL_72;
					case ReqOpcode::NotBurrowedOrAIControlled:
						if (unit->status & UnitStatus::Burrowed)
						{
						LABEL_87:
							*lastError = 5;
							return 0;
						}
						v11 = v9 + 1;
						goto LABEL_72;
					case ReqOpcode::Disabled:
						*lastError = 21;
						return -1;
					case ReqOpcode::Blank:
						goto LABEL_93;
					case ReqOpcode::BroodWar:
						if (!*IS_BROOD_WAR)
						{
							*lastError = 26;
							return 0;
						}
						v11 = v9 + 1;
						goto LABEL_72;
					case ReqOpcode::TechIsResearched:
						auto v10 = reqTable[v7++];
						if (scbw::hasTechResearched(unit->playerId,v10) && !(*CHEAT_STATE & CheatFlags::MedievalMan))
						{
							*lastError = 9;
							return -1;
						}
						++v28;
						goto LABEL_73;
					case ReqOpcode::IfBurrowed:
						if (!(unit->status & UnitStatus::Burrowed))
						{
							*lastError = 5;
							return -1;
						}
						v11 = v9 + 1;
						goto LABEL_72;
					default:
						if (*CHEAT_STATE & 0x2000)
							++v9;
						v24 = completed_unit_counts[requiredId][playerId];
					LABEL_71:
						v11 = v24 + v9;
						goto LABEL_72;
					}
				}*/
		return 0;
	}

	u32 realTopSpeed(Flingy* flingy, u32 flingyId)
	{
		u32 speed = scbw::getFlingyData(FlingyDatEntries::TopSpeed, flingyId);
		return speed;
	}

	u16 realAcceleration(Flingy* flingy, u32 flingyId)
	{
		u16 acceleration = scbw::getFlingyData(FlingyDatEntries::Acceleration, flingyId);
		return acceleration;
	}

	bool is_high_templar_conduit(u16 unitId)
	{
		switch (unitId)
		{
			case UnitId::ProtossArchon:
			case UnitId::ProtossAugur:
				return true;
			default:
				break;
		}
		return false;
	}

	u32 weapon_flingy(u32 weaponId, u8 player_id)
	{
		globals.flingy_init_bullet_id = weaponId;
		globals.flingy_init_player_id = player_id;
		return weapons_dat::FlingyId[weaponId];
	}

	u32 getFlingyHaltDistanceHook(Flingy* flingy)
	{
		auto speed = flingy->current_speed2;
		if (speed == 0 || flingy->flingyMovementType != 0)
		{
			return 0;
		}
		auto flingy_id = flingy->flingyId;
		if (speed != realTopSpeed(flingy, flingy_id) || flingy->flingyAcceleration != realAcceleration(flingy, flingy_id))
		{
			return speed * speed / (2 * (u16)flingy->flingyAcceleration);
		}
		else
		{
			return scbw::getFlingyData(FlingyDatEntries::HaltDistance, flingy_id);
		}
	}

	bool is_unused_order(u8 orderId)
	{
		switch (orderId)
		{
			case OrderId::GuardianAspect:
			case OrderId::Skyfall: // OrderId::AttackTile
			case OrderId::Hover:
			case OrderId::Warpin:
			case OrderId::CTFCOP2:
			{
				return true; 
			}
			default:
			{
			} break;
		}
		return false;
	}

	void setupSpiderMines(Unit* unit)
	{
		if (unit->id == UnitId::TerranVulture)
		{
			unit->vulture.spiderMineCount = 3;
		}
		else if (unit->id == UnitId::TerranEidolon)
		{
			unit->vulture.spiderMineCount = 2;
		}
	}

	u32 get_mine_id(Unit* unit)
	{
		if (unit->id == UnitId::TerranEidolon)
		{
			return UnitId::TerranLobotomyMine;
		}
		return UnitId::TerranSpiderMine;
	}
	u32 extend_tech_target_check(u32 orig_result, Unit* caster, Unit* target, u8 techId)
	{
		if (orig_result == 0)
		{ // Valid Target
			scbw::print("Can cast");
			if (techId == TechId::Lockdown)
			{
				scbw::printf("Lockdown %d %d", caster->orderTarget.pt.x, caster->orderTarget.pt.y);
			}
		}
		return orig_result;
	}

	// replaces CTFCop2 order
	void order_PhaseLink(Unit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		scbw::decloakProc(unit);

		u32 status = 0;
		IscriptAnimation::Enum anim = IscriptAnimation::CastSpell; // might be changed later if required
		if (unit->id == UnitId::ProtossClarion)
		{
			if (scbw::get_aise_value(unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
			{
				status = 1;//disable clarion
			}
			else
			{
				status = 0;//enable clarion
			}
		}
		else if (unit->id == UnitId::ProtossStarSovereign)
		{
			auto order_state = unitExt->values[ValueProtossSwitchOrderState];
			if (order_state == 0)
			{
				if (unit->movementFlags & MovementFlags::Accelerating)
				{
					scbw::makeToHoldPosition(unit);
				}
				unitExt->values[ValueProtossSwitchOrderState] = 1;
			}
			else if (order_state == 1)
			{
				if (!(unit->movementFlags & MovementFlags::Accelerating))
				{
					unitExt->timers[TimerPlanetCrackerDelay] = 24;
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->status |= UnitStatus::CanNotAttack;
					unitExt->values[ValueProtossSwitchOrderState] = 2;
				}
			}
			else if (order_state == 2)
			{
				if (unitExt->timers[TimerPlanetCrackerDelay] == 0)
				{
					unit->playIscriptAnim(anim);
					unitExt->timers[TimerPlanetCrackerChannel] = 121;
					unitExt->values[ValueProtossSwitchOrderState] = 3;
				}
			}
			else if (order_state == 3)
			{
				if (unit->orderSignal & 4)
				{
					unit->orderSignal &= ~0x4;
					unit->status &= ~UnitStatus::CanNotReceiveOrders;
					unit->status &= ~UnitStatus::CanNotAttack;
					unitExt->values[ValueProtossSwitchOrderState] = 0;
					unit->orderToIdle();
				}
			}
			return;
		}

		auto order_state = unitExt->values[ValueProtossSwitchOrderState];
		if (order_state == 0)
		{
			if (unit->movementFlags & MovementFlags::Accelerating)
			{
				scbw::makeToHoldPosition(unit);
			}
			unitExt->values[ValueProtossSwitchOrderState] = 1;
		}
		else if (order_state == 1)
		{
			if (!(unit->movementFlags & MovementFlags::Accelerating))
			{
				unit->playIscriptAnim(anim);
				unitExt->timers[TimerSwitchModeGimmick] = 5;
				unitExt->values[ValueProtossSwitchOrderState] = 2;
			}
		}
		else if (order_state == 2)
		{
			if (unitExt->timers[TimerSwitchModeGimmick] == 0)
			{
				unit->orderSignal &= ~0x4;
				unitExt->values[ValueProtossSwitchOrderState] = 0;
				if (unit->id == UnitId::ProtossClarion)
				{
					if (status == 0)
					{
						scbw::placeClarionLinks(unit);
					}
					else
					{
						scbw::placeClarionLinks(unit);
						scbw::clearClarionLinks(unit);
					}
				}
				unit->updateSpeed();
				unit->orderToIdle(); // do next queued order
			}
		}
	}

	bool canAttackTarget_EndChunk(Unit* source, u32 target_flags)
	{
		if (source->id == UnitId::ProtossClarion && scbw::get_aise_value(source, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
		{
			return false;
		}
		if (target_flags & UnitStatus::InAir)
		{
			if (source->getAirWeapon() != WeaponId::None)
			{
				return true;
			}
		}
		else
		{
			if (source->getGroundWeapon() != WeaponId::None)
			{
				return true;
			}
		}
		return false;
	}

	s32 gameLoop() // 004D94B0 - hooked for lag testing - Pr0nogo
	{
		bool v0;
		win32::Stopwatch sw;
		sw.Start();
		f64 aiLoop, unitLoop, imageThingyLoop, tileLoop;

		scbw::setInGameLoopState(1);
		helper::initializeRandomizerInfo();
		helper::AI_Loop();
		aiLoop = sw.ElapsedMilliseconds();
		sw.Stop();

		if (!visionUpdateCount)
			*visionUpdateCount = 100;
		v0 = *visionUpdateCount == 1;
		*visionUpdateCount -= 1;
		*visionUpdated = v0;
		sw.Start();
		if (v0)
		{
			helper::updateActiveTileInfo();
			helper::removeFOWCheat();
		}
		sw.Stop();
		tileLoop = sw.ElapsedMilliseconds();
		sw.Start();
		helper::updateUnits();
		sw.Stop();
		sw.Start();
		unitLoop = sw.ElapsedMilliseconds();
		helper::imageDrawingBulletDrawing();
		helper::updateImages();
		helper::updateThingys();
		sw.Stop();
		imageThingyLoop = sw.ElapsedMilliseconds();
		GPTP::logger << "aiLoop: " << aiLoop << ", unitLoop: " << unitLoop << ", " << "tile loop: " << tileLoop << ", thingy/bullet/imageLoop: " << imageThingyLoop << '\n';
		return scbw::setInGameLoopState(0);
	}

	Unit* createUnit(u16 unitType, s32 x, s32 y, s32 playerId)  // 004A09D0 - hooked to add ccmu cases (protect against garbage pointers) - Pr0nogo
	{
		Unit* result;
		s32* pointer;

		u32 randomSeed;
		if (IS_IN_GAME_LOOP)
		{
			++randomCounts[14];
			*lastRandomNumber = 0x15A4E35 * *lastRandomNumber + 1;
			++(*randomCountsTotal);
			randomSeed = (*lastRandomNumber >> 16) & 0x7FFF;
		}
		else
		{
			randomSeed = 0;
		}
		Unit* unitConstructorResult = helper::unitConstructor(x, y, unitType, playerId, randomSeed);
		Unit* unitConstructorResult2 = unitConstructorResult;
		if (!unitConstructorResult)
		{
			return 0;
		}
		if (unitConstructorResult->status & UnitStatus::GroundedBuilding)
		{
			helper::CListPushBackUsedUnitEntry(unitConstructorResult);
		}
		else
		{
			helper::CListPushBackHiddenUnitEntry(unitConstructorResult);
		}
		if (unitType > UnitId::Spell_DisruptionWeb
			|| units_dat::SubUnit[unitType] == UnitId::None)
		{
			unitConstructorResult2->subunit = 0;
			result = unitConstructorResult2;
		}
		else
		{
			Unit* unitConstructorResult3 = helper::unitConstructor(x, y, units_dat::SubUnit[unitType], playerId, randomSeed);
			Unit* unitConstructorResult4 = unitConstructorResult3;
			if (!unitConstructorResult3)
			{
				helper::unitDestructor(unitConstructorResult2);
				return 0;
			}
			unitConstructorResult2->subunit = unitConstructorResult3;
			unitConstructorResult3->subunit = unitConstructorResult2;
			helper::getImageAttackFrame(pointer, unitConstructorResult2->sprite, 0, 2u);
			helper::setSpriteMainImageOffset(unitConstructorResult4->sprite, *pointer, randomSeed);
			result = unitConstructorResult2;
		}
		return result;
	}
}

