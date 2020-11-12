//
#include "hooks/misc/hydra_specific.h"
#include "globals.h"
namespace hooks {
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
			return UnitId::LobotomyMine;
		}
		return UnitId::TerranVultureSpiderMine;
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
	
