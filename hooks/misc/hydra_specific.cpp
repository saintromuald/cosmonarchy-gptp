//
#include "hooks/misc/hydra_specific.h"
#include "globals.h"
#include "hooks/orders/unit_making/unit_train.h"
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
		IsNotTeching=0xff09,
		IsNotUpgrading=0xff0a,
		IsNotConstructingBuilding=0xff0b,
		HasNoAddon=0xff0c,
		HasNoNydusExit=0xff0d,
		HasHangarSpace=0xff0e,
		TechIsResearched=0xff0f,
		HasNoNuke=0xff10,
		NotBurrowedOrAIControlled=0xff11,
		IsNotLandedBuilding=0xff12,
		IsLandedBuidling=0xff13,
		CanMove=0xff14,
		CanAttack=0xff15,
		IsWorker=0xff16,
		CanLiftOff=0xff17,
		IsTransport=0xff18,
		IsPowerup=0xff19,
		IsSubunit=0xff1a,
		HasSpiderMinesOrAddons=0xff1b,
		IsHero=0xff1c,
		CanRallyOrRclick=0xff1d,
		AllowOnHallucinations=0xff1e,
		UpgradeLevelBasedJump=0xff1f,//0xff20 if level 1, 0xff20 if level >=2
		Disabled=0xff22,
		Blank=0xff23,
		BroodWar=0xff24,
		IfTechResearched=0xff25,
		IfBurrowed=0xff26,
		EndOfSublist = 0xFFFF,
	};
}

//u32 parseUnitDatReqs() {
void parseUnitDatReqs(){
//	static u32 result;
	const u32 parseUnitFunc = 0x0046e100;
	__asm {
		PUSHAD
		CALL parseUnitFunc
//		MOV result,EAX
		POPAD
	}
	//return result;
}

namespace hooks {

	u32 checkUnitDatRequirements(u8 playerId, u16 unit_id, CUnit* unit) {
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


		if (units_dat::unitsDat43[unit->id] == ReqOpcode::EndOfSublist){
			parseUnitDatReqs();
		}
		int v7 = (u16)units_dat::unitsDat43[unit->id];

		if (units_dat::unitsDat43[unit->id] == 0)
		{
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
		CUnit* v12 = 0;
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
				v24 = completed_unit_counts[requiredId][unit->playerId]+all_unit_counts[requiredId][unit->playerId];
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
					if (v12!=NULL || (++v7, v12->id != reqTable[v7]))
					{
						if (!v27)
							*lastError = 4;
						return -1;
					}
					v11 = v9 + 1;
				}
				goto LABEL_72;
			case ReqOpcode::IsNotLifted:
				if (!(parent->statusFlags & 2))
				{
					lastInternalErr = 7;
					return 0;
				}
				v11 = v9 + 1;
				goto LABEL_72;
			case reqOpcode_Is_not_training_or_morphing:
				if (unitIsTrainingOrMorphing(parent) || LOBYTE(parent->field_C8) != 44 || BYTE1(parent->field_C8) != 61)
					goto LABEL_87;
				++v28;
				goto LABEL_73;
			case reqOpcode_Is_not_constructing_addon:
				LOBYTE(v13) = isConstructingAddon(parent);
				if (v13)
					goto LABEL_87;
				LOBYTE(v15) = isAttemptingProtossBuild(parent);
				if (v15)
					goto LABEL_87;
				v11 = v14 + 1;
				goto LABEL_72;
			case reqOpcode_Is_not_researching:
				if (LOBYTE(parent->field_C8) != 44)
					goto LABEL_87;
				v11 = v9 + 1;
				goto LABEL_72;
			case reqOpcode_Is_not_upgrading:
				if (BYTE1(parent->field_C8) != 61)
					goto LABEL_87;
				v11 = v9 + 1;
				goto LABEL_72;
			case reqOpcode_Is_not_constructing:
				if (parent->type == 7 && parent->mainOrderType == 33)
					goto LABEL_87;
				v11 = v9 + 1;
				goto LABEL_72;
			case reqOpcode_Does_not_have_addon_attached:
				if (parent->field_C0)
				{
					if (!v27)
						lastInternalErr = 4;
					return 0;
				}
				v11 = v9 + 1;
			LABEL_72:
				v28 = v11;
			LABEL_73:
				if (sgTree[++v7] == -255)
				{
					LOWORD(v8) = sgTree[v7];
					v9 = v28;
					++v7;
					continue;
				}
				if (!v28 && v26)
				{
					lastInternalErr = 8;
					return -1;
				}
				LOWORD(v8) = sgTree[v7];
				v26 = 1;
				if ((_WORD)v8 != -1)
					goto LABEL_18;
				if (v27)
				{
					lastInternalErr = 25;
					return -1;
				}
				return 1;
			case reqOpcode_Has_hangar_space:
				v17 = -1;
				LOBYTE(v18) = unitIsCarrier(parent);
				if (v18)
				{
					v19 = BYTE1(parent->field_C8) + getHangerTrainCount(v16, (int)parent);
				}
				else
				{
					LOBYTE(v21) = unitIsReaver(parent);
					if (!v21)
						goto LABEL_51;
					v19 = getHangerTrainCount(v20, (int)parent);
				}
				v17 = LOBYTE(parent->field_C8) + v19;
			LABEL_51:
				if (v17 >= (unsigned __int8)getMaxHangerSpace(parent))
					goto LABEL_86;
				++v28;
				goto LABEL_73;
			case reqOpcode_Does_not_have_loaded_nuke:
				if (*(_DWORD*)&parent->field_D0)
					goto LABEL_86;
				v11 = v9 + 1;
				goto LABEL_72;
			case reqOpcode_Is_not_burrowed:
				if (parent->statusFlags & 0x10)
				{
				LABEL_87:
					lastInternalErr = 5;
					return 0;
				}
				v11 = v9 + 1;
				goto LABEL_72;
			case reqOpcode_Grey:
				lastInternalErr = 21;
				return -1;
			case reqOpcode_Blank:
				goto LABEL_93;
			case reqOpcode_Must_be_Brood_War:
				if (!isExpansion)
				{
					lastInternalErr = 26;
					return 0;
				}
				v11 = v9 + 1;
				goto LABEL_72;
			case reqOpcode_Is_researched:
				v10 = sgTree[v7++];
				if (!isTechResearched(playerID, v10) && !(gameCheatFlags & 0x1000))
				{
					lastInternalErr = 9;
					return -1;
				}
				++v28;
				goto LABEL_73;
			case reqOpcode_Is_burrowed:
				if (!(parent->statusFlags & 0x10))
				{
					lastInternalErr = 5;
					return -1;
				}
				v11 = v9 + 1;
				goto LABEL_72;
			default:
				if (gameCheatFlags & 0x2000)
					++v9;
				v24 = completedUnitCount[0][playerID + 12 * v8];
			LABEL_71:
				v11 = v24 + v9;
				goto LABEL_72;
			}
		}*/
	return 0;
	}

	u32 realTopSpeed(CFlingy* flingy, u32 flingyId) {
		u32 speed = scbw::getFlingyData(FlingyDatEntries::TopSpeed, flingyId);
		return speed;
	}

	u16 realAcceleration(CFlingy* flingy, u32 flingyId) {
		u16 acceleration = scbw::getFlingyData(FlingyDatEntries::Acceleration, flingyId);
		return acceleration;
	}

	bool is_high_templar_conduit(u16 unitId) {
		switch (unitId) {
		case UnitId::ProtossArchon:
		case UnitId::ProtossAugur:
			return true;
		default:
			break;
		}
		return false;
	}

	u32 weapon_flingy(u32 weaponId, u8 player_id) {
		Globals.flingy_init_bullet_id = weaponId;
		Globals.flingy_init_player_id = player_id;
		return weapons_dat::FlingyId[weaponId];
	}

	u32 getFlingyHaltDistance(CFlingy* flingy) {
		auto speed = flingy->current_speed2;
		if (speed == 0 || flingy->flingyMovementType != 0) {
			return 0;
		}
		auto flingy_id = flingy->flingyId;
		if (speed != realTopSpeed(flingy, flingy_id) || flingy->flingyAcceleration != realAcceleration(flingy, flingy_id)) {
			return speed * speed / (2 * (u16)flingy->flingyAcceleration);
		}
		else {
			return scbw::getFlingyData(FlingyDatEntries::HaltDistance, flingy_id);
		}
	}

	bool is_unused_order(u8 orderId) {
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
		}
		else if (unit->id == UnitId::TerranGhost) {
			unit->vulture.spiderMineCount = 2;
		}
	}

	u32 get_mine_id(CUnit* unit) {
		if (unit->id == UnitId::TerranGhost) {
			return UnitId::TerranLobotomyMine;
		}
		return UnitId::TerranSpiderMine;
	}
	u32 extend_tech_target_check(u32 orig_result, CUnit* caster, CUnit* target, u8 techId) {
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
		u32 status = 0;
		IscriptAnimation::Enum anim = IscriptAnimation::CastSpell; //might be  changed later if required
		if (unit->id == UnitId::ProtossClarion) {
			if (scbw::get_aise_value(unit, NULL, AiseId::SendClarionValue, 0, 0) == 1) {
				status = 1;//disable clarion
			}
			else {
				status = 0;//enable clarion
			}
		}
		else if (unit->id == UnitId::ProtossExemplar) {
			status = scbw::get_generic_value(unit, ValueId::ExemplarSwitch);
		}
		else if (unit->id == UnitId::ProtossStarSovereign) {
			auto order_state = scbw::get_generic_value(unit, ValueId::ProtossSwitchOrderState);
			if (order_state == 0) {
				if (unit->movementFlags & MovementFlags::Accelerating) {
					scbw::makeToHoldPosition(unit);
				}
				scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 1);
			}
			else if (order_state == 1) {
				if (!(unit->movementFlags & MovementFlags::Accelerating)) {
					scbw::set_generic_timer(unit, ValueId::PlanetCrackerDelay, 24);
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->status |= UnitStatus::CanNotAttack;
					scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 2);
				}
			}
			else if (order_state == 2) {
				if (scbw::get_generic_timer(unit, ValueId::PlanetCrackerDelay) == 0) {
					unit->playIscriptAnim(anim);
					scbw::set_generic_timer(unit, ValueId::PlanetCrackerChannel, 121);
					scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 3);
				}
			}
			else if (order_state == 3) {
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
		}
		else if (order_state==1) {
			if (!(unit->movementFlags & MovementFlags::Accelerating)) {
				unit->playIscriptAnim(anim);
				scbw::set_generic_timer(unit, ValueId::SwitchModeGimmickTimer, 5);
				scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 2);
			}
		}
		else if (order_state == 2) {
			if (scbw::get_generic_timer(unit,ValueId::SwitchModeGimmickTimer)==0) {
				unit->orderSignal &= ~0x4;
				scbw::set_generic_value(unit, ValueId::ProtossSwitchOrderState, 0);
				if (unit->id == UnitId::ProtossClarion) {
					if (status == 0) {
						scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetPhaseLinkStatus, 1, 0, 0);
					}
					else {
						scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetPhaseLinkStatus, 0, 0, 0);
						scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::ClearClarionLinks, 0, 0, 0);
					}
				}
				else if (unit->id == UnitId::ProtossExemplar) {
					if (status == 0) {
						scbw::set_generic_value(unit, ValueId::ExemplarSwitch, 1);
					}
					else {
						scbw::set_generic_value(unit, ValueId::ExemplarSwitch, 0);
					}
				}
				unit->updateSpeed();
				unit->orderToIdle();//do next queued order
			}
		}
	}
	bool canAttackTarget_EndChunk(CUnit* source, u32 target_flags) {
		if (source->id == UnitId::ProtossClarion && scbw::get_aise_value(source, NULL, AiseId::SendClarionValue, 0, 0) == 1) {
			return false;
		}
		if (target_flags & UnitStatus::InAir) {
			if (source->getAirWeapon() != WeaponId::None) {
				return true;
			}
		}
		else {
			if (source->getGroundWeapon() != WeaponId::None) {
				return true;
			}
		}


		return false;
	}	
}
	
