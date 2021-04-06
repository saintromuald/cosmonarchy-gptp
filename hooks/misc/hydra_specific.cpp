//
#include "hooks/misc/hydra_specific.h"
#include "globals.h"
#include <cassert>
#include "hooks/orders/unit_making/unit_train.h"
#include "stopwatch/Stopwatch.h"
#include "logger.h"
namespace ReqOpcode {
	enum {

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

//uint32 parseUnitDatReqs() {
void parseUnitDatReqs() {
	//	static uint32 result;
	const uint32 parseUnitFunc = 0x0046e100;
	__asm {
		PUSHAD
		CALL parseUnitFunc
		//		MOV result,EAX
		POPAD
	}
	//return result;
}
bool isAttemptingProtossBuild(const CUnit* probe) {
	return probe->mainOrderId == OrderId::BuildProtoss1
		&& probe->status & UnitStatus::GroundedBuilding
		&& probe->orderTarget.unit != nullptr
		&& !(probe->orderTarget.unit->status & UnitStatus::Completed);
}
bool isConstructingAddon(const CUnit* building) {
	return building->secondaryOrderId == OrderId::BuildAddon
		&& building->status & UnitStatus::GroundedBuilding
		&& building->currentBuildUnit != nullptr
		&& !(building->currentBuildUnit->status & UnitStatus::Completed);
}

const uint32 Func_getHangerTrainCount = 0x00465270;
uint8 getHangerTrainCount(CUnit* unit) {
	static uint8 result = 0;
	__asm {
		PUSHAD
		MOV EDX, unit
		CALL Func_getHangerTrainCount
		MOV result, AL
		POPAD
	}
}
const uint32 Func_unitIsReaver = 0x00401490;
bool unitIsReaver(CUnit* a1) {
	static int32 result;
	__asm {
		PUSHAD
		MOV EAX, a1
		CALL Func_unitIsReaver
		MOV result, EAX
		POPAD
	}
	return (bool)result;
}
const uint32 Func_unitIsCarrier = 0x00401470;
bool unitIsCarrier(CUnit* a1) {
	static int32 result;
	__asm {
		PUSHAD
		MOV EAX, a1
		CALL Func_unitIsCarrier
		MOV result, EAX
		POPAD
	}
	return (bool)result;
}
bool unitIsTrainingOrMorphing(CUnit* unit) {
	const uint32 Func_unitIsTrainingOrMorphing = 0x00401500;
	static uint32 result;
	__asm {
		PUSHAD
		MOV ECX, unit
		CALL Func_unitIsTrainingOrMorphing
		MOV result, EAX
		POPAD
	}
}
const uint32 Func_initializeRandomizerInfo = 0x0047E410;
uint8 initializeRandomizerInfo() {
	static uint32 result;
	__asm {
		PUSHAD
		CALL Func_initializeRandomizerInfo
		MOV result, EAX
		POPAD
	}
	return (uint8)result;
}

const uint32 Func_AI_Loop = 0x004A2B00;
void AI_Loop() {
	__asm {
		PUSHAD
		CALL Func_AI_Loop
		POPAD
	}
}

const uint32 Func_updateActiveTileInfo = 0x0049C4C0;
int32 updateActiveTileInfo() {
	static int32 result;
	__asm {
		PUSHAD
		CALL Func_updateActiveTileInfo
		MOV result, EAX
		POPAD
	}
	return result;
}

const uint32 Func_removeFOWCheat = 0x00480880;
int32 removeFOWCheat() {
	static int32 result;
	__asm {
		PUSHAD
		CALL Func_removeFOWCheat
		MOV result, EAX
		POPAD
	}
	return result;
}

const uint32 Func_updateUnits = 0x004ED000;
void updateUnits() {
	__asm {
		PUSHAD
		CALL Func_updateUnits
		POPAD
	}
}

const uint32 Func_imageDrawingBulletDrawing = 0x0048C3D0;
int32 imageDrawingBulletDrawing() {
	static int32 result;
	__asm {
		PUSHAD
		CALL Func_imageDrawingBulletDrawing
		MOV result, EAX
		POPAD
	}
	return result;
}

const uint32 Func_updateImages = 0x00496330;
void updateImages() {
	__asm {
		PUSHAD
		CALL Func_updateImages
		POPAD
	}
}

const uint32 Func_updateThingys = 0x00488510;
void updateThingys() {
	__asm {
		PUSHAD
		CALL Func_updateThingys
		POPAD
	}
}

#include <array>
namespace hooks {

	void createDamageOverlay(CSprite* sprite) {
		if (sprite->mainGraphic == nullptr)
			return;
		const LO_Header* overlay = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlayDamagePtr, sprite->mainGraphic->id));
		if (overlay == nullptr)
			return;//return if there's no overlay defined
		for (CImage* image = sprite->images.head; image; image = image->link.next) {
			if (image->id >= ImageId::FirstMinorDamageOverlay &&
				image->id <= ImageId::LastMinorDamageOverlay) {
				int variation = image->id - ImageId::FirstMinorDamageOverlay;
				image->free();
				if (sprite->mainGraphic->frameIndex >= overlay->frameCount) {
					scbw::printFormattedText("CreateDamageOverlay Bug: incorrect frame %d, image id %d",
						sprite->mainGraphic->frameIndex, sprite->mainGraphic->id);
					return;
				}
				Point8 offset = overlay->getOffset(sprite->mainGraphic->frameIndex, variation);//not sure about first arg
				sprite->createTopOverlay(ImageId::FirstMajorDamageOverlay + variation, offset.x, offset.y, 0);
			}
		}
		std::array<int, 22> results;
		auto result_pos = results.begin();
		if (sprite->mainGraphic->frameIndex >= overlay->frameCount) {
			scbw::printFormattedText("CreateDamageOverlay Bug: incorrect frame %d, image id %d",
				sprite->mainGraphic->frameIndex, sprite->mainGraphic->id);
			return;
		}
		for (int i = 0; i < 22; i++) {
			Point8 offset = overlay->getOffset(sprite->mainGraphic->frameIndex, i);
			if (offset.x == 127 && offset.y == 127)
				continue;
			uint32 overlay_id = ImageId::FirstMajorDamageOverlay + i;
			bool found = false;
			for (CImage* image = sprite->images.head; image; image = image->link.next) {
				if (image->id == overlay_id) {
					found = true;
					break;
				}
			}
			if (!found) {
				*result_pos++ = i;
			}
		}
		if (result_pos != results.begin()) {
			int variation = results[scbw::randBetween(0, result_pos - results.begin() - 1)];
			Point8 offset = overlay->getOffset(sprite->mainGraphic->frameIndex, variation);
			sprite->createTopOverlay(ImageId::FirstMinorDamageOverlay + variation, offset.x, offset.y, 0);
		}
	}

	// Hooked to enable unit mover when placing buildings
	int32 isTileBlockedBy(CUnit** unitArray, CUnit* builder, int32 x_tile, int32 y_tile, int8 dontIgnoreReacting, int8 alsoInvisible) { // 00473300
		CUnit* unitArrayPtr; // edx
		CUnit* builderVar; // edi
		CUnit** unitArrayVar; // esi
		DWORD unitStatus; // eax
		BOOL needsDetection; // eax

		unitArrayVar = unitArray;
		unitArrayPtr = *unitArray;
		builderVar = builder;

		if (!*unitArray) {
			return 0;
		}

		while (1) {
			needsDetection = 0;
			if (builderVar) {
				unitStatus = unitArrayPtr->status;
				if (unitStatus & 0x300) { // Requires Detection + Cloaked
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
				|| unitArrayPtr->sprite->flags & 0x20) {
				unitArrayPtr = unitArrayVar[1];
				++unitArrayVar;
				if (!unitArrayPtr) {
					return 0;
				}
			}
			if (unitOrderingX[unitArrayPtr->finderIndex.left].position < 32 * x_tile + 32
				&& unitOrderingX[unitArrayPtr->finderIndex.right].position > 32 * x_tile
				&& unitOrderingY[unitArrayPtr->finderIndex.top].position < 32 * y_tile + 32
				&& unitOrderingY[unitArrayPtr->finderIndex.bottom].position > 32 * y_tile) {
				return needsDetection != 0 ? 1 : 4;
			}
			builderVar = builder;
			unitArrayPtr = unitArrayVar[1];
			++unitArrayVar;
			if (!unitArrayPtr) {
				return 0;
			}
		}
	}

	uint32 checkUnitDatRequirements(uint8 playerId, uint16 unit_id, CUnit* unit) {
		//
		//
		// Warning: This hook don't points to firegraft offsets. Firegraft offsets must be taken into account and added to hook
		// before using it.
		//
		//
		auto v3 = unit_id;
		static uint32* lastError = (uint32*)0x0066FF60;
		static uint16* reqTable = (uint16*)0x00514178;
		*lastError = 0;
		if (unit_id >= UnitId::None) {
			*lastError = 16;
			return 0;
		}
		if (playerId != unit->playerId) {
			*lastError = 1;
			return 0;
		}
		if (!(unit->status & UnitStatus::Completed)) {
			*lastError = 20;
			return 0;
		}
		if (unit->isFrozen()) {
			*lastError = 10;
			return 0;
		}
		if (!UnitAvailability->available[unit->playerId][unit->id]) {
			*lastError = 2;
			return 0;
		}
		if (hooks::isQueueSlotActive(unit, 4)) {//if build queue is full
		LABEL_86:
			*lastError = 3;
			return 0;
		}
		if (unit->status & UnitStatus::IsHallucination) {
			*lastError = 22;
			return 0;
		}
		// 57


		if (units_dat::unitsDat43[unit->id] == ReqOpcode::EndOfSublist) {
			parseUnitDatReqs();
		}
		int v7 = (uint16)units_dat::unitsDat43[unit->id];

		if (units_dat::unitsDat43[unit->id] == 0) {
		LABEL_93:
			*lastError = 23;
			return 0;
		}
		auto v26 = 1;
		auto v25 = 1;
		int v8 = reqTable[v7];
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
		CUnit* v12 = 0;
		auto requiredId = 0;
		/*		while (true)
				{
					v8 = (uint16)v8;
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

	uint32 realTopSpeed(CFlingy* flingy, uint32 flingyId) {
		uint32 speed = scbw::getFlingyData(FlingyDatEntries::TopSpeed, flingyId);
		return speed;
	}

	uint16 realAcceleration(CFlingy* flingy, uint32 flingyId) {
		uint16 acceleration = scbw::getFlingyData(FlingyDatEntries::Acceleration, flingyId);
		return acceleration;
	}

	bool is_high_templar_conduit(uint16 unitId) {
		switch (unitId) {
			case UnitId::ProtossArchon:
			case UnitId::ProtossAugur:
				return true;
			default:
				break;
		}
		return false;
	}

	uint32 weapon_flingy(uint32 weaponId, uint8 player_id) {
		Globals.flingy_init_bullet_id = weaponId;
		Globals.flingy_init_player_id = player_id;
		return weapons_dat::FlingyId[weaponId];
	}

	uint32 getFlingyHaltDistance(CFlingy* flingy) {
		auto speed = flingy->current_speed2;
		if (speed == 0 || flingy->flingyMovementType != 0) {
			return 0;
		}
		auto flingy_id = flingy->flingyId;
		if (speed != realTopSpeed(flingy, flingy_id) || flingy->flingyAcceleration != realAcceleration(flingy, flingy_id)) {
			return speed * speed / (2 * (uint16)flingy->flingyAcceleration);
		} else {
			return scbw::getFlingyData(FlingyDatEntries::HaltDistance, flingy_id);
		}
	}

	bool is_unused_order(uint8 orderId) {
		switch (orderId) {
			case OrderId::GuardianAspect:
			case OrderId::Skyfall://OrderId::AttackTile
			case OrderId::Hover:
			case OrderId::Warpin:
			case OrderId::CTFCOP2:
				return true;
			default:
				break;
		}
		return false;
	}

	void setupSpiderMines(CUnit* unit) {
		if (unit->id == UnitId::TerranVulture) {
			unit->vulture.spiderMineCount = 3;
		} else if (unit->id == UnitId::TerranEidolon) {
			unit->vulture.spiderMineCount = 2;
		}
	}

	uint32 get_mine_id(CUnit* unit) {
		if (unit->id == UnitId::TerranEidolon) {
			return UnitId::TerranLobotomyMine;
		}
		return UnitId::TerranSpiderMine;
	}
	uint32 extend_tech_target_check(uint32 orig_result, CUnit* caster, CUnit* target, uint8 techId) {
		if (orig_result == 0) {//Valid Target
			scbw::printText("Can cast");
			if (techId == TechId::Lockdown) {
				scbw::printFormattedText("Lockdown %d %d", caster->orderTarget.pt.x, caster->orderTarget.pt.y);
			}
		}
		return orig_result;
	}
	//replaces CTFCop2 order
	void order_PhaseLink(CUnit* unit) {
		uint32 status = 0;
		IscriptAnimation::Enum anim = IscriptAnimation::CastSpell; //might be  changed later if required
		if (unit->id == UnitId::ProtossClarion) {
			if (scbw::get_aise_value(unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1) {
				status = 1;//disable clarion
			} else {
				status = 0;//enable clarion
			}
		} else if (unit->id == UnitId::ProtossExemplar) {
			status = scbw::get_generic_rust_value(unit, ValueId::ExemplarSwitch);
		} else if (unit->id == UnitId::ProtossStarSovereign) {
			auto order_state = scbw::get_generic_value(unit, ValueId::ProtossSwitchOrderState);
			if (order_state == 0) {
				if (unit->movementFlags & MovementFlags::Accelerating) {
					scbw::makeToHoldPosition(unit);
				}
				scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 1);
			} else if (order_state == 1) {
				if (!(unit->movementFlags & MovementFlags::Accelerating)) {
					scbw::set_generic_timer(unit, ValueId::PlanetCrackerDelay, 24);
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->status |= UnitStatus::CanNotAttack;
					scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 2);
				}
			} else if (order_state == 2) {
				if (scbw::get_generic_timer(unit, ValueId::PlanetCrackerDelay) == 0) {
					unit->playIscriptAnim(anim);
					scbw::set_generic_timer(unit, ValueId::PlanetCrackerChannel, 121);
					scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 3);
				}
			} else if (order_state == 3) {
				if (unit->orderSignal & 4) {
					unit->orderSignal &= ~0x4;
					unit->status &= ~UnitStatus::CanNotReceiveOrders;
					unit->status &= ~UnitStatus::CanNotAttack;
					scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 0);
					unit->orderToIdle();
				}
			}

			/*			else if (order_state == 1) {
							if (!(unit->movementFlags & MovementFlags::Accelerating)) {
								unit->playIscriptAnim(anim);
								scbw::set_generic_timer(unit, ValueId::PlanetCrackerChannel, 121);
								scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 2);
							}
						}
						else if (order_state == 2) {
							if (unit->orderSignal & 4) {
								unit->orderSignal &= ~0x4;
								scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 0);
								unit->orderToIdle();
							}
						}*/
			return;
		}

		auto order_state = scbw::get_generic_value(unit, ValueId::ProtossSwitchOrderState);
		if (order_state == 0) {
			if (unit->movementFlags & MovementFlags::Accelerating) {
				scbw::makeToHoldPosition(unit);
			}
			scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 1);
		} else if (order_state == 1) {
			if (!(unit->movementFlags & MovementFlags::Accelerating)) {
				unit->playIscriptAnim(anim);
				scbw::set_generic_timer(unit, ValueId::SwitchModeGimmickTimer, 5);
				scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 2);
			}
		} else if (order_state == 2) {
			if (scbw::get_generic_timer(unit, ValueId::SwitchModeGimmickTimer) == 0) {
				unit->orderSignal &= ~0x4;
				scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 0);
				if (unit->id == UnitId::ProtossClarion) {
					if (status == 0) {
						scbw::placeClarionLinks(unit);
					} else {
						// TODO: is placing needed?
						scbw::placeClarionLinks(unit);
						scbw::clearClarionLinks(unit);
					}
				} else if (unit->id == UnitId::ProtossExemplar) {
					if (status == 0) {
						scbw::set_generic_rust_value(unit, ValueId::ExemplarSwitch, 1);
					} else {
						scbw::set_generic_rust_value(unit, ValueId::ExemplarSwitch, 0);
					}
				}
				unit->updateSpeed();
				unit->orderToIdle();//do next queued order
			}
		}
	}
	bool canAttackTarget_EndChunk(CUnit* source, uint32 target_flags) {
		if (source->id == UnitId::ProtossClarion && scbw::get_aise_value(source, nullptr, AiseId::SendClarionValue, 0, 0) == 1) {
			return false;
		}
		if (target_flags & UnitStatus::InAir) {
			if (source->getAirWeapon() != WeaponId::None) {
				return true;
			}
		} else {
			if (source->getGroundWeapon() != WeaponId::None) {
				return true;
			}
		}


		return false;
	}

	int32 gameLoop() { // 004D94B0 - hooked for lag testing - Pr0nogo
		bool v0;
		win32::Stopwatch sw;
		sw.Start();
		double aiLoop, unitLoop, imageThingyLoop, tileLoop;

		scbw::setInGameLoopState(1);
		initializeRandomizerInfo();
		AI_Loop();
		aiLoop = sw.ElapsedMilliseconds();
		sw.Stop();

		if (!visionUpdateCount)
			*visionUpdateCount = 100;
		v0 = *visionUpdateCount == 1;
		*visionUpdateCount -= 1;
		*visionUpdated = v0;
		sw.Start();
		if (v0) {
			updateActiveTileInfo();
			removeFOWCheat();
		}
		sw.Stop();
		tileLoop = sw.ElapsedMilliseconds();
		sw.Start();
		updateUnits();
		sw.Stop();
		sw.Start();
		unitLoop = sw.ElapsedMilliseconds();
		imageDrawingBulletDrawing();
		updateImages();
		updateThingys();
		sw.Stop();
		imageThingyLoop = sw.ElapsedMilliseconds();
		//		if (*elapsedTimeFrames % 24 == 0) {
		GPTP::logger << "aiLoop: " << aiLoop << ", unitLoop: " << unitLoop << ", " << "tile loop: " << tileLoop << ", thingy/bullet/imageLoop: " << imageThingyLoop << '\n';
		//}
		return scbw::setInGameLoopState(0);
	}
}

