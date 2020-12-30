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
bool isAttemptingProtossBuild(const CUnit* probe) {
	return probe->mainOrderId == OrderId::BuildProtoss1
		&& probe->status & UnitStatus::GroundedBuilding
		&& probe->orderTarget.unit != NULL
		&& !(probe->orderTarget.unit->status & UnitStatus::Completed);
}
bool isConstructingAddon(const CUnit* building) {
	return building->secondaryOrderId == OrderId::BuildAddon
		&& building->status & UnitStatus::GroundedBuilding
		&& building->currentBuildUnit != NULL
		&& !(building->currentBuildUnit->status & UnitStatus::Completed);
}

const u32 Func_getHangerTrainCount = 0x00465270;
u8 getHangerTrainCount(CUnit* unit) {
	static u8 result = 0;
	__asm {
		PUSHAD
		MOV EDX,unit
		CALL Func_getHangerTrainCount
		MOV result,AL
		POPAD
	}
}
const u32 Func_unitIsReaver = 0x00401490;
bool unitIsReaver(CUnit* a1) {
	static s32 result;
	__asm {
		PUSHAD
		MOV EAX, a1
		CALL Func_unitIsReaver
		MOV result, EAX
		POPAD
	}
	return (bool)result;
}
const u32 Func_unitIsCarrier = 0x00401470;
bool unitIsCarrier(CUnit* a1) {
	static s32 result;
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
	const u32 Func_unitIsTrainingOrMorphing = 0x00401500;
	static u32 result;
	__asm {
		PUSHAD
		MOV ECX,unit
		CALL Func_unitIsTrainingOrMorphing
		MOV result,EAX
		POPAD
	}
}


namespace hooks {

	CImage* createDamageOverlay(CSprite* sprite) {
	  CImage *result; // eax
	  CImage *v2; // ecx
	  int v3; // edi
	  CImage *v4; // esi
	  short v5; // ax
	  u32 v6; // eax
	  int v7; // eax
	  int v8; // ebx
	  int v9; // edi
	  GrpHead *v10; // eax
	  int v11; // edx
	  char v12; // ST1C_1
	  unsigned int v13; // eax
	  CImage *v14; // ecx
	  short v15; // si
	  char v16; // al
	  char *v17; // eax
	  int v18; // ecx
	  int v19; // eax
	  char v20; // dl
	  char v21; // bl
	  u32 v22; // esi
	  u32 v23; // ecx
	  short a4; // [esp+Ch] [ebp-14h]
	  int v25; // [esp+10h] [ebp-10h]
	  int v26; // [esp+14h] [ebp-Ch]
	  GrpHead *v27; // [esp+18h] [ebp-8h]
	  int v28; // [esp+1Ch] [ebp-4h]
	  v2 = sprite->mainGraphic;
	  u16 id = v2->id;
	  v3 = 0;
	  auto damageOverlay = scbw::getImageData(ImagesDatEntries::OverlayDamage, id);
	  if ( damageOverlay )
	  {
		v4 = sprite->images.head;
		if ( !v4 )
		{
	LABEL_6:
		  if ( *IS_IN_GAME_LOOP)
		  {
			++randomCounts[8];
			v13 = (0x15A4E35 * (*lastRandomNumber) + 1) >> 16;
			(*lastRandomNumber)++;
			*lastRandomNumber = 0x15A4E35 * (*lastRandomNumber) + 1;
			v6 = v13 & 0x7FFF;
		  }
		  else
		  {
			v6 = 0;
		  }
		  v14 = sprite->mainGraphic;
		  v28 = 0;
		  v25 = v6 % (scbw::getImageData(ImagesDatEntries::BuildingOverlayStateMax, v14->id) >> 1) + 1;
		  v27 = (GrpHead*)scbw::getImageData(ImagesDatEntries::OverlayDamagePtr, v14->id);
		  v16 = LOBYTE(v14->flags) >> 1;
		  v26 = 4 * v14->frameIndex + 8;
		  v15 = v16 & 1;
		  a4 = v16 & 1;
		  while ( true )
		  {
			if ( v3 >= 22 )
			  v3 = 0;
			v17 = (char *)&v27->frameCount + 2 * v3 + *(u32*)((char *)&v27->frameCount + v26);
			v18 = *v17;//damageOverlay crash happens here
			if ( v15 )
			  v18 = -v18;
			v19 = v17[1];
			v20 = v18;
			v21 = v19;
			if ( v18 != 127 || v19 != 127 )
			{
			  result = sprite->images.head;
			  ++v28;
			  v22 = v3 + 472;
			  if ( result )
			  {
				while ( 1 )
				{
				  v23 = (unsigned __int16)result->id;
				  if ( v23 >= v22 && v23 <= v22 )
					break;
				  result = result->link.next;
				  if ( !result )
					goto LABEL_23;
				}
			  }
			  else
			  {
	LABEL_23:
				if ( v28 >= v25 )
				{
				  if ( v3 != -1 )
					result = sprite->createTopOverlay(v3 + 450, v20, v21, 0);
				  return result;
				}
			  }
			  v15 = a4;
			}
			++v3;
		  }
		}
		while ( 1 )
		{
		  v5 = v4->id;
		  if ( (unsigned __int16)v5 >= 450u && (unsigned __int16)v5 <= 471u )
			break;
		  v4 = v4->link.next;
		  if ( !v4 )
			goto LABEL_6;
		}
		v10 = (GrpHead*)scbw::getImageData(ImagesDatEntries::OverlayDamagePtr, v2->id);
		v9 = (unsigned __int16)v4->id - 450;
		v11 = *((u32*)&v10->frames[0].width + (unsigned __int16)v2->frameIndex) + 2 * v9;
		v8 = *((char *)&v10->frameCount + v11);
		v7 = (int)v10 + v11;
		if ( v2->flags & 2 )
		  v8 = -v8;
		v12 = *(u8*)(v7 + 1);
		v4->free();
		result = sprite->createTopOverlay(v9 + 472, v8, v12, 0);
	  }
	  return result;
	}

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
		auto v14 = 0;
		auto v17 = 0;
		auto v18 = 0;
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
					if (v12 != NULL || (++v7, v12->id != reqTable[v7]))
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
	
