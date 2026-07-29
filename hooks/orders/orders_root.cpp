#include "orders_root.h"
#include <BW/api.h>
#include <hooks/limits/sfx_extender.h>

#ifndef NOTVALID
#define NOTVALID 0
#endif

namespace Func_Type {
	enum Enum : u8 {
		EAX,
		EBX,
		ECX,
		EDI,
		ESI,
		PUSH
	};
}

static void set_order_queue_timer_cooldown(Unit* unit) {
	unit->order_queue_timer = unit->id == UnitId::TerranDiadem ? 4 : 0;
}

static u32 main_order_func_offsets[][3] {
	{ 0x00479480,	/*OrderId::Die*/					Func_Type::PUSH,0},
	{ 0x0047BBA0,	/*OrderId::Stop*/					Func_Type::EAX,	2},
	{ 0x00475B90,	/*OrderId::Guard*/					Func_Type::ESI,	2},
	{ 0x004774A0,	/*OrderId::PlayerGuard*/			Func_Type::EAX,	2},
	{ 0x004777F0,	/*OrderId::TurretGuard*/			Func_Type::EAX,	1},
	{ NOTVALID,		/*OrderId::BunkerGuard*/			Func_Type::EAX,	0},
	{ 0x0047C950,	/*OrderId::Move*/					Func_Type::EAX,	2},
	{ 0x004654B0,	/*OrderId::ReaverStop*/				Func_Type::PUSH,2},
	{ NOTVALID,		/*OrderId::Attack1*/				Func_Type::EAX,	0},
	{ 0x004F6FA0,	/*OrderId::Attack2*/				Func_Type::EAX,	2},
	{ 0x00479BD0,	/*OrderId::AttackUnit*/				Func_Type::ESI,	2},
	{ 0x00477D00,	/*OrderId::AttackFixedRange*/		Func_Type::EAX,	2},
	{ NOTVALID,		/*OrderId::AttackTile*/				Func_Type::EAX,	0},
	{ NOTVALID,		/*OrderId::Hover*/					Func_Type::EAX,	0},
	{ 0x00479040,	/*OrderId::AttackMove*/				Func_Type::ESI,	2},
	{ 0x004EA4C0,	/*OrderId::InfestMine1*/			Func_Type::EAX,	2},
	{ NOTVALID,		/*OrderId::Nothing1*/				Func_Type::EAX,	0},
	{ 0x004F3EA0,	/*OrderId::Powerup1*/				Func_Type::EAX,	2},
	{ 0x00476F50,	/*OrderId::TowerGuard*/				Func_Type::EAX,	2},
	{ 0x00479150,	/*OrderId::TowerAttack*/			Func_Type::EAX,	2},
	{ 0x00463DF0,	/*OrderId::VultureMine*/			Func_Type::EAX,	2},
	{ 0x0047C4F0,	/*OrderId::StayInRange*/			Func_Type::EAX,	2},
	{ 0x00477980,	/*OrderId::TurretAttack*/			Func_Type::EAX,	1},
	{ 0x00475000,	/*OrderId::Nothing2*/				Func_Type::ECX,	2},
	{ NOTVALID,		/*OrderId::Nothing3*/				Func_Type::EAX,	0},
	{ 0x0045CF80,	/*OrderId::DroneStartBuild*/		Func_Type::EAX,	2},
	{ 0x0045E090,	/*OrderId::DroneBuild*/				Func_Type::PUSH,1},
	{ 0x004E98E0,	/*OrderId::InfestMine2*/			Func_Type::EAX,	2},
	{ 0x004F6FA0,	/*OrderId::InfestMine3*/			Func_Type::EAX,	2},
	{ 0x004EA290,	/*OrderId::InfestMine4*/			Func_Type::EAX,	2},
	{ 0x00467FD0,	/*OrderId::BuildTerran*/			Func_Type::PUSH,1},
	{ 0x004E4D00,	/*OrderId::BuildProtoss1*/			Func_Type::EDI,	1},
	{ 0x004E4F20,	/*OrderId::BuildProtoss2*/			Func_Type::EAX,	2},
	{ 0x00467A70,	/*OrderId::ConstructingBuilding*/	Func_Type::EAX,	1},
	{ 0x004673D0,	/*OrderId::Repair1*/				Func_Type::EAX,	1},
	{ 0x004F6FA0,	/*OrderId::Repair2*/				Func_Type::EAX,	2},
	{ 0x004E6880,	/*OrderId::PlaceAddon*/				Func_Type::PUSH,2},
	{ NOTVALID,		/*OrderId::BuildAddon*/				Func_Type::EAX,	0},
	{ NOTVALID,		/*OrderId::TrainUnit*/					Func_Type::EAX,	0},
	{ NOTVALID,		/*OrderId::RallyPoint1*/			Func_Type::EAX,	0},
	{ NOTVALID,		/*OrderId::RallyPoint2*/			Func_Type::EAX,	0},
	{ 0x0045DD60,	/*OrderId::ZergBirth*/				Func_Type::EAX,	1},
	{ 0x0045DEA0,	/*OrderId::MorphUnit*/					Func_Type::EAX,	1},
	{ 0x0045D0D0,	/*OrderId::MorphStructure*/					Func_Type::PUSH,2},
	{ 0x00467760,	/*OrderId::BuildSelf1*/				Func_Type::EAX,	1},
	{ 0x0045D500,	/*OrderId::ZergBuildSelf*/			Func_Type::EAX,	1},
	{ 0x0045DC20,	/*OrderId::BuildNydusExit*/					Func_Type::EBX,	2},
	{ 0x004EA3E0,	/*OrderId::Enternyduscanal*/		Func_Type::EAX,	2},
	{ 0x004E4F40,	/*OrderId::BuildSelf2*/				Func_Type::EAX,	0},
	{ 0x0047C7B0,	/*OrderId::Follow*/					Func_Type::EAX,	2},
	{ 0x004666A0,	/*OrderId::Carrier*/				Func_Type::EDI,	2},
	{ 0x0047C950,	/*OrderId::CarrierIgnore1*/			Func_Type::EAX,	2},
	{ 0x00465910,	/*OrderId::CarrierStop*/			Func_Type::EAX,	2},
	{ 0x00465950,	/*OrderId::CarrierAttack1*/			Func_Type::EAX,	2},
	{ 0x004F6FA0,	/*OrderId::CarrierAttack2*/			Func_Type::EAX,	2},
	{ 0x00466720,	/*OrderId::CarrierIgnore2*/			Func_Type::EAX,	2},
	{ 0x004666A0,	/*OrderId::CarrierFight*/			Func_Type::EDI,	2},
	{ 0x004666A0,	/*OrderId::HoldPositionSolarion*/	Func_Type::EDI,	2},
	{ 0x004665D0,	/*OrderId::Reaver*/					Func_Type::EDI,	2},
	{ 0x00465690,	/*OrderId::ReaverAttack1*/			Func_Type::EAX,	2},
	{ 0x004F6FA0,	/*OrderId::ReaverAttack2*/			Func_Type::EAX,	2},
	{ 0x004665D0,	/*OrderId::ReaverFight*/			Func_Type::EDI,	2},
	{ 0x004665D0,	/*OrderId::HoldPositionReaver*/				Func_Type::EDI,	2},
	{ NOTVALID,		/*OrderId::TrainFighter*/			Func_Type::EAX,	0},
	{ 0x00465F60,	/*OrderId::StrafeUnit1*/			Func_Type::EAX,	2},
	{ 0x00465E00,	/*OrderId::StrafeUnit2*/			Func_Type::EAX,	1},
	{ 0x00493DD0,	/*OrderId::RechargeShields1*/		Func_Type::EDI,	1},
	{ 0x00493990,	/*OrderId::Rechargeshields2*/		Func_Type::EAX,	2},
	{ NOTVALID,		/*OrderId::ShieldBattery*/			Func_Type::EAX,	0},
	{ 0x00466350,	/*OrderId::Return*/					Func_Type::EAX,	2},
	{ 0x004E9AA0,	/*OrderId::DroneLand*/				Func_Type::EAX,	2},
	{ 0x00464360,	/*OrderId::BuildingLand*/			Func_Type::EAX,	1},
	{ 0x004649B0,	/*OrderId::BuildingLiftoff*/		Func_Type::ESI,	1},
	{ NOTVALID,		/*OrderId::DroneLiftoff*/			Func_Type::EAX,	0},
	{ 0x00463AC0,	/*OrderId::Liftoff*/				Func_Type::EAX,	2},
	{ 0x004548B0,	/*OrderId::ResearchTech*/			Func_Type::EAX,	1},
	{ 0x004546A0,	/*OrderId::Upgrade*/				Func_Type::EAX,	1},
	{ 0x004E9F90,	/*OrderId::Larva*/					Func_Type::EAX,	2},
	{ NOTVALID,		/*OrderId::SpawningLarva*/			Func_Type::EAX,	0},
	{ 0x00469500,	/*OrderId::Harvest1*/				Func_Type::EAX,	2},
	{ 0x004F6FA0,	/*OrderId::Harvest2*/				Func_Type::EAX,	2},
	{ 0x00469500,	/*OrderId::HarvestGas1*/			Func_Type::EAX,	2},
	{ 0x00469000,	/*OrderId::HarvestGas2*/			Func_Type::EAX,	2},
	{ 0x00469980,	/*OrderId::HarvestGas3*/			Func_Type::EAX,	2},
	{ 0x004690C0,	/*OrderId::ReturnGas*/				Func_Type::EAX,	2},
	{ 0x00469240,	/*OrderId::MoveToMinerals*/			Func_Type::EAX,	2},
	{ 0x00468F60,	/*OrderId::HarvestMinerals2*/		Func_Type::EAX,	2},
	{ 0x004697C0,	/*OrderId::MiningMinerals*/			Func_Type::EAX,	2},
	{ 0x00468E80,	/*OrderId::Harvest3*/				Func_Type::ESI,	1},
	{ 0x00468ED0,	/*OrderId::Harvest4*/				Func_Type::EAX,	1},
	{ 0x004690C0,	/*OrderId::ReturnMinerals*/			Func_Type::EAX,	2},
	{ 0x00493920,	/*OrderId::Harvest5*/				Func_Type::EDI,	1},
	{ 0x004E7CF0,	/*OrderId::EnterTransport*/			Func_Type::EAX,	2},
	{ 0x004E7300,	/*OrderId::Pickup1*/				Func_Type::EAX,	2},
	{ 0x004E75D0,	/*OrderId::Pickup2*/				Func_Type::EDI,	2},
	{ 0x004E73B0,	/*OrderId::BunkerLoad*/				Func_Type::EDI,	2},
	{ 0x004E7B70,	/*OrderId::TransportLoad*/			Func_Type::EAX,	2},
	{ 0x004F3E10,	/*OrderId::Powerup2*/				Func_Type::EAX,	2},
	{ 0x00464BD0,	/*OrderId::SiegeMode*/				Func_Type::EDI,	1},
	{ 0x00464AE0,	/*OrderId::TankMode*/				Func_Type::EAX,	1},
	{ 0x0047BAB0,	/*OrderId::WatchTarget*/			Func_Type::EAX,	2},
	{ 0x004E96D0,	/*OrderId::InitCreepGrowth*/		Func_Type::EAX,	2},
	{ NOTVALID,		/*OrderId::SpreadCreep*/			Func_Type::EAX,	0},
	{ 0x004E95E0,	/*OrderId::StoppingCreepGrowth*/	Func_Type::EAX,	2},
	{ NOTVALID,		/*OrderId::GuardianAspect*/			Func_Type::EAX,	0},
	{ 0x004948B0,	/*OrderId::WarpingArchon*/			Func_Type::EAX,	1},
	{ 0x00493B10,	/*OrderId::CompletingArchonSummon*/	Func_Type::ESI,	1},
	{ 0x00478D10,	/*OrderId::HoldPositionNormal*/		Func_Type::EAX,	2},
	{ 0x004EB5B0,	/*OrderId::HoldPositionMatraleth*/	Func_Type::ESI,	2},
	{ NOTVALID,		/*OrderId::Cloak*/					Func_Type::EAX,	0},
	{ NOTVALID,		/*OrderId::Decloak*/				Func_Type::EAX,	0},
	{ 0x004E80D0,	/*OrderId::Unload*/					Func_Type::EAX,	2},
	{ 0x004E7700,	/*OrderId::MoveUnload*/				Func_Type::PUSH,2},
	{ 0x00492850,	/*OrderId::FireYamatoGun1*/			Func_Type::EAX,	2},
	{ 0x004F6FA0,	/*OrderId::FireYamatoGun2*/			Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::LobotomyGrenade*/		Func_Type::EAX,	2},
	{ 0x004E9E60,	/*OrderId::Burrow*/					Func_Type::EAX,	2},
	{ 0x004E9860,	/*OrderId::Burrowed*/				Func_Type::EAX,	2},
	{ 0x004EA670,	/*OrderId::Unburrow*/				Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::CarapaceSwarm*/			Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::CastParasite*/			Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::SummonBroodlings*/		Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::EMPShockwave*/			Func_Type::EAX,	2},
	{ NOTVALID,		/*OrderId::NukeWait*/				Func_Type::EAX,	0},
	{ 0x004E6700,	/*OrderId::NukeTrain*/				Func_Type::EAX,	1},
	{ 0x00464730,	/*OrderId::NukeLaunch*/				Func_Type::EAX,	2},
	{ 0x00463610,	/*OrderId::NukePaint*/				Func_Type::EAX,	2},
	{ 0x00479410,	/*OrderId::NukeUnit*/				Func_Type::EAX,	2},
	{ 0x00479200,	/*OrderId::NukeGround*/				Func_Type::PUSH,2},
	{ 0x00464D10,	/*OrderId::NukeTrack*/				Func_Type::EBX,	0},
	{ 0x00493A80,	/*OrderId::InitArbiter*/			Func_Type::ESI,	1},
	{ NOTVALID,		/*OrderId::CloakNearbyUnits*/		Func_Type::EAX,	0},
	{ 0x00464FD0,	/*OrderId::PlaceMine*/				Func_Type::EAX,	2},
	{ 0x004F6EF0,	/*OrderId::Rightclickaction*/		Func_Type::EAX,	2},
	{ 0x004788E0,	/*OrderId::SapUnit*/				Func_Type::EAX,	2},
	{ 0x00478A40,	/*OrderId::SapLocation*/			Func_Type::EAX,	2},
	{ 0x004EB5B0,	/*OrderId::HoldPositionSuicide*/	Func_Type::ESI,	2},
	{ 0x00494470,	/*OrderId::Teleport*/				Func_Type::PUSH,2},
	{ NOTVALID,		/*OrderId::TeleporttoLocation*/		Func_Type::EAX,	0},
	{ 0x00464E40,	/*OrderId::PlaceScanner*/			Func_Type::EAX,	2},
	{ 0x00463D30,	/*OrderId::Scanner*/				Func_Type::EAX,	2},
	{ 0x004550A0,	/*OrderId::DefensiveMatrix*/		Func_Type::PUSH,2},
	{ 0x00492850,	/*OrderId::PsiStorm*/				Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::Irradiate*/				Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::Plague*/					Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::Consume*/				Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::Ensnare*/				Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::StasisField*/			Func_Type::EAX,	2},
	{ 0x004F6C40,	/*OrderId::Hallucianation1*/		Func_Type::EDI,	2},
	{ NOTVALID,		/*OrderId::Hallucination2*/			Func_Type::EAX,	0},
	{ 0x004671B0,	/*OrderId::ResetCollision1*/		Func_Type::EAX,	1},
	{ 0x0042E320,	/*OrderId::ResetCollision2*/		Func_Type::EAX,	1},
	{ 0x004780F0,	/*OrderId::Patrol*/					Func_Type::EAX,	2},
	{ 0x004E4210,	/*OrderId::CTFCOPInit*/				Func_Type::EAX,	2},
	{ 0x004E41A0,	/*OrderId::CTFCOP1*/				Func_Type::ESI,	2},
	{ 0x004E3FB0,	/*OrderId::CTFCOP2*/				Func_Type::EAX,	1},
	{ 0x004A28B0,	/*OrderId::ComputerAI*/				Func_Type::EAX,	2},
	{ 0x00478DE0,	/*OrderId::AtkMoveEP*/				Func_Type::EAX,	2},
	{ 0x00478EC0,	/*OrderId::HarassMove*/				Func_Type::EAX,	2},
	{ 0x004A1D80,	/*OrderId::AIPatrol*/				Func_Type::ESI,	2},
	{ 0x004778E0,	/*OrderId::GuardPost*/				Func_Type::EAX,	2},
	{ 0x004A1EF0,	/*OrderId::RescuePassive*/			Func_Type::EAX,	2},
	{ 0x004A1C20,	/*OrderId::Neutral*/				Func_Type::EDI,	2},
	{ 0x00478490,	/*OrderId::ComputerReturn*/			Func_Type::EAX,	2},
	{ 0x00493F70,	/*OrderId::InitPsiProvider*/		Func_Type::EAX,	2},
	{ 0x00475710,	/*OrderId::SelfDestructing*/		Func_Type::EAX,	1},
	{ 0x0047C3C0,	/*OrderId::Critter*/				Func_Type::EAX,	1},
	{ 0x0047BF80,	/*OrderId::HiddenGun*/				Func_Type::EAX,	2},
	{ 0x0047BCD0,	/*OrderId::OpenDoor*/				Func_Type::ESI,	2},
	{ 0x0047BC50,	/*OrderId::CloseDoor*/				Func_Type::ESI,	2},
	{ 0x0047C0A0,	/*OrderId::HideTrap*/				Func_Type::EAX,	2},
	{ 0x0047C1B0,	/*OrderId::RevealTrap*/				Func_Type::EAX,	2},
	{ 0x0047BE80,	/*OrderId::Enabledoodad*/			Func_Type::EAX,	2},
	{ 0x0047BD60,	/*OrderId::Disabledoodad*/			Func_Type::EAX,	2},
	{ 0x004E4C70,	/*OrderId::Warpin*/					Func_Type::EDI,	0},
	{ 0x00463900,	/*OrderId::Cleric*/					Func_Type::EAX,	2},
	{ 0x00464180,	/*OrderId::MedicHeal1*/				Func_Type::EAX,	1},
	{ 0x004637B0,	/*OrderId::HealMove*/				Func_Type::PUSH,1},
	{ 0x00464050,	/*OrderId::HoldPositionCleric*/		Func_Type::ESI,	1},
	{ 0x00463740,	/*OrderId::MedicHeal2*/				Func_Type::EDI,	1},
	{ 0x00492850,	/*OrderId::Restoration*/			Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::CastDisruptionWeb*/		Func_Type::EAX,	2},
	{ 0x004F6950,	/*OrderId::CastTyranny*/			Func_Type::PUSH,2},
	{ 0x00494690,	/*OrderId::WarpingDarkArchon*/		Func_Type::EAX,	1},
	{ 0x004F6D40,	/*OrderId::CastFeedback*/			Func_Type::PUSH,2},
	{ 0x00492850,	/*OrderId::CastOpticalFlare*/		Func_Type::EAX,	2},
	{ 0x00492850,	/*OrderId::CastMaelstrom*/			Func_Type::EAX,	2},
	{ 0x0047C210,	/*OrderId::JunkYardDog*/			Func_Type::EAX,	2},
	{ NOTVALID,		/*OrderId::Fatal*/					Func_Type::EAX,	0}
};

namespace hooks {
	// Called from a global unitUpdate function. Deals with some automatic orders like being in a bunker. -GPTP
	void orders_id_cases(Unit* unit) {
		switch (unit->main_order_id) {
			case OrderId::Die: {
				helper::PUSH_order(unit, main_order_func_offsets[unit->main_order_id][0]);
			} break;	
			case OrderId::TurretAttack:
			case OrderId::Nothing2:
			case OrderId::Neutral:
			case OrderId::Medic:
			case OrderId::MedicHeal1: {
			} break;
			case OrderId::PlayerGuard:
			case OrderId::TurretGuard:
			case OrderId::StayInRange:
			case OrderId::EnterTransport:
			{
				if (unit->status & UnitStatus::InBuilding) {
					unit->order_computer(OrderId::BunkerGuard);
				} else {
					unit->order_computer(OrderId::Nothing2);
				}
			} break;
			case OrderId::HarvestGas3:
			case OrderId::NukeLaunch:
			case OrderId::InfestMine4:
			case OrderId::ResetCollision1:
			case OrderId::ResetCollision2:
			case OrderId::Powerup1:
			case OrderId::Powerup2: {
				helper::EAX_order(unit, main_order_func_offsets[unit->main_order_id][0]);
			} break;
			default: {
				if (unit->order_queue_timer != 0) {
					unit->order_queue_timer--;
				} else {
					set_order_queue_timer_cooldown(unit);
					if (unit->main_order_id == OrderId::BunkerGuard) {
						helper::EAX_order(unit, 0x004790A0);
					}	 else if (unit->main_order_id == OrderId::TransportLoad) {
						helper::EAX_order(unit, main_order_func_offsets[OrderId::TransportLoad][0]);
					} else if (unit->main_order_id == OrderId::ComputerAI) {
						if (unit->status & UnitStatus::InBuilding) {
							helper::EAX_order(unit, 0x004790A0);	// OrderId::BunkerGuard
						}
					} else if (unit->main_order_id == OrderId::RescuePassive) {
						helper::EAX_order(unit, main_order_func_offsets[OrderId::RescuePassive][0]);
					}
				}
			} break;
		}	
	}

	// Also known as 004EC170 perform_secondary_orders
	// Less complex than the one about main orders, so not using any array. Still using a switch though. -GPTP
	void secondary_orders_root(Unit* unit) {
		if (unit->secondary_order_id == OrderId::Hallucination2) {
			helper::EAX_order(unit, 0x004F63A0);
		} else if (!unit->is_frozen()) {
			switch (unit->secondary_order_id) {
				case OrderId::BuildAddon: {
					helper::EAX_order(unit, 0x004E6790);
				} break;
				case OrderId::Cloak: {
					helper::EAX_order(unit, 0x00491790);
				} break;
				case OrderId::CloakNearbyUnits: {
					helper::EDI_order(unit, 0x00491C20);
				} break;
				case OrderId::Decloak: {
					helper::EAX_order(unit, 0x004633E0);
				} break;
				case OrderId::ShieldBattery: {
					helper::EDI_order(unit, 0x004932D0);
				} break;
				case OrderId::SpawningLarva: {
					helper::ESI_order(unit, 0x004EA780);
				} break;
				case OrderId::SpreadCreep: {
					helper::EAX_order(unit, 0x004EA880);
				} break;
				case OrderId::TrainFighter: {
					helper::EAX_order(unit, 0x00466790);
				} break;
				case OrderId::TrainUnit: {
					helper::EAX_order(unit, 0x00468420);
				} break;
				default: {
				} break;
			}
		}
	}

	// Also known as 004EC4D0 ordersEntries | perform_orders
	// Called between updateUnitState() (see hooks\update_unit_state) and performSecondaryOrders() (see secondary_orders_root)
	// Implementation quite different from original because hard to manipulate.
	void main_orders_root(Unit* unit) {
		// 1st part is functions that ignore disabled/stunned/other statuses.
		// Not implemented through an array like original, since there are only 4 valid cases. -GPTP
		switch (unit->main_order_id) {
			case OrderId::Die: {
				helper::PUSH_order(unit, main_order_func_offsets[OrderId::Die][0]);
			} break;
			case OrderId::BuildSelf2: {
				helper::EAX_order(unit, main_order_func_offsets[OrderId::BuildSelf2][0]);
			} break;
			case OrderId::NukeTrack: {
				helper::EBX_order(unit, main_order_func_offsets[OrderId::NukeTrack][0]);
			} break;
			case OrderId::Warpin: {
				helper::EDI_order(unit, main_order_func_offsets[OrderId::Warpin][0]);
			} break;
			case OrderId::BlissfulEmbrace: {
				auto& unit_ext = unit->ext();
				Unit* target = unit->order_target.unit;
				if (target && can_infest(unit->player, target)) {
					if (helper::get_distance_to_unit(unit, target) > 16) {
						helper::spell_order_move_towards_target(unit, 12);
					} else if (!(unit->movement_flags & MovementFlags_Moving) && !unit_ext.timers[TimerGenericAbilityAnimation]) {
						mark_infesting(unit->player, target);
						bw::decloak_passive_proc(unit);
						UnitExt* target_ext = get_unit_ext(target);
						target_ext->units[UnitInfestingMatraleth] = unit;

						unit->play_iscript_animation(IscriptAnimation::CastSpell);
						unit_ext.timers[TimerGenericAbilityAnimation] = 12;
					}
				} else {
					unit->order_target.unit = 0;
					unit->order_target.pt = unit->position;
					unit->order_to_idle();
				}
			} break;
			case OrderId::GriefOfAllGods: {
				hooks::orders_grief_of_all_gods(unit);
			} break;
			case OrderId::TargetGroundNew: {
				if (unit->id == UnitId::TerranWyvern) {
					auto& unit_ext = unit->ext();
					unit_ext.values[ValueWyvernLandGrabOn] = 1;
					unit_ext.values[ValueWyvernLandGrabX] = unit->order_target.pt.x;
					unit_ext.values[ValueWyvernLandGrabY] = unit->order_target.pt.y;
				}
			} break;
			case OrderId::CastThroughTheLookingGlass: {
				auto& unit_ext = unit->ext();
				if (!unit_ext.timers[TimerHypnagogueChannel] && !unit_ext.values[ValueTryCastCustomSpell]) {
					unit_ext.values[ValueTryCastCustomSpell] = true;
				}
			} break;
			case OrderId::LobotomyWarhead:
			case OrderId::DivineDiscipline:
			case OrderId::RejoinderOfService:
			case OrderId::AskosiRematerialize:
			case OrderId::AskosiRapidRendezvous:
			case OrderId::AskosiHijack:
			case OrderId::AskosiSalvage:
			case OrderId::AskosiRemoteHijack:
			case OrderId::AskosiPreylock: {
				auto& unit_ext = unit->ext();
				if (!unit_ext.values[ValueTryCastCustomSpell]) {
					unit_ext.values[ValueTryCastCustomSpell] = true;
				}
			} break;
			case OrderId::Follow: {
				Unit* target = unit->order_target.unit;
				if (!target) {
					if (unit->order_queue_head) {
						unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
						helper::do_next_queued_order(unit);
					} else if (unit->unit_ai) {
						helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI);
					} else {
						helper::issue_order_targeting_nothing(unit, units_dat::return_to_idle_order[unit->id]);
					}
					return;
				}
				if (unit->unit_ai) {
					u32 hp = units_dat::max_hitpoints[unit->id] * 256;
					if (!hp) {
						hp = (unit->hitpoints + 255) * 256;
						if (!hp) {
							hp = 1;
						}
					}
					if ((unit->hitpoints + 255) * 256 == hp) {
						helper::to_idle(unit);
						return;
					}
				}
				if (!unit->main_order_state) {
					if (!helper::order_to_move_to_target(unit, target)) {
						return;
					}
					if (target->is_valid_nydus()) {
						if (helper::can_enter_nydus(unit, target)) {
							helper::assign_order_with_target(unit, target, OrderId::EnterNydusCanal);
							return;
						}
					} else if (target->recharges_shields()) {
						if (helper::can_recharge_shields(unit, target)) {
							helper::assign_order_with_target(unit, target, OrderId::RechargeShields1);
							return;
						}
					}
					unit->main_order_state = 1;
				}
				if (!unit->main_order_timer) {
					unit->main_order_timer = 7;
					if (helper::is_unit_enemy(unit, target)) {
						if (helper::unit_can_attack_target(target, unit, 1)) {
							helper::issue_order_targeting_nothing(unit, OrderId::Guard);
							return;
						}
					}
					if (!unit->is_in_air()) {
						if (!helper::unit_has_path_to_target(unit, target)) {
							helper::make_to_hold_position(unit);
							return;
						}
					}
					if (target->is_rescuable() && !helper::is_target_within_min_range(unit, 1, target)) {
						helper::order_to_move_to_target(unit, target);
						return;
					}
					if (!helper::is_target_within_min_range(unit, 96, target)) {
						helper::move_towards(unit, target);
						return;
					}
					if (!target->is_grounded_building() && !unit->order_queue_head) {
						helper::make_to_hold_position(unit);
						return;
					}
					helper::to_idle(unit);
					return;
				}
			} break;
			default: {
				if (unit->is_frozen() || (!(unit->status & UnitStatus::Reacts) && unit->status & UnitStatus::CanNotAttack)) {
					helper::EAX_order(unit, 0x0043FF00); // known as "get_maelstrom_target", though don't make much sense in this context
				} else {
					// 2nd part is functions that are used if not one of those of the 1st part and not disabled unit.
					// Ignore order_queue_timer, and if they change the main_order_id to something that can be used
					// in the 3rd part, then both the 2nd and 3rd part functions would indeed be used. -GPTP
					if (main_order_func_offsets[unit->main_order_id][2] == 1 && main_order_func_offsets[unit->main_order_id][0] != NOTVALID) {
						switch (main_order_func_offsets[unit->main_order_id][1]) {
							case Func_Type::EAX: {
								helper::EAX_order(unit, main_order_func_offsets[unit->main_order_id][0]);
							} break;
							case Func_Type::EBX: {
								helper::EBX_order(unit, main_order_func_offsets[unit->main_order_id][0]);
							} break;
							case Func_Type::ECX: {
								helper::ECX_order(unit, main_order_func_offsets[unit->main_order_id][0]);
							} break;
							case Func_Type::EDI: {
								helper::EDI_order(unit, main_order_func_offsets[unit->main_order_id][0]);
							} break;
							case Func_Type::ESI: {
								helper::ESI_order(unit, main_order_func_offsets[unit->main_order_id][0]);
							} break;
							case Func_Type::PUSH: {
								helper::PUSH_order(unit, main_order_func_offsets[unit->main_order_id][0]);
							} break;
						}
					}
	
					if (unit->order_queue_timer != 0) {
						unit->order_queue_timer--;
					} else if ((unit->movement_state != UnitMovement_CheckIllegal && unit->movement_state != UnitMovement_MoveToLegal) || order_id_ignores_collision(unit->main_order_id)) {
						// the above checks are new, to prevent units from procesesing orders when they should be decolliding. -Pr0nogo
						
						// 3rd part, executed after the 2nd part (whether it did something or not) only if unit->order_queue_timer was 0.
						// Execute "get_target_something" before, and has some special cases, otherwise work pretty much like 2nd part. -GPTP
						set_order_queue_timer_cooldown(unit);
						helper::EAX_order(unit, 0x0043FF90); // known as "get_target_something"
						unit->ai_action_flags = 0;
	
						if (unit->main_order_id == OrderId::HoldPositionMatraleth || unit->main_order_id == OrderId::HoldPositionSuicide) {
							if (unit->main_order_state == 0) {
								helper::ESI_order(unit, main_order_func_offsets[unit->main_order_id][0]);
								unit->main_order_state = 1;
							}
							if (unit->order_queue_head) {
								helper::ECX_order(unit, main_order_func_offsets[OrderId::Nothing2][0]);
							}
						} else if (unit->main_order_id == OrderId::Nothing2) {
							if (unit->order_queue_head) {
								helper::ECX_order(unit, main_order_func_offsets[OrderId::Nothing2][0]);
							}
						} else if (main_order_func_offsets[unit->main_order_id][2] == 2) {
							switch (main_order_func_offsets[unit->main_order_id][1]) {
								case Func_Type::EAX: {
									helper::EAX_order(unit, main_order_func_offsets[unit->main_order_id][0]);
								} break;
								case Func_Type::EBX: {
									helper::EBX_order(unit, main_order_func_offsets[unit->main_order_id][0]);
								} break;
								case Func_Type::ECX: {
									helper::ECX_order(unit, main_order_func_offsets[unit->main_order_id][0]);
								} break;
								case Func_Type::EDI: {
									helper::EDI_order(unit, main_order_func_offsets[unit->main_order_id][0]);
								} break;
								case Func_Type::ESI: {
									helper::ESI_order(unit, main_order_func_offsets[unit->main_order_id][0]);
								} break;
								case Func_Type::PUSH: {
									helper::PUSH_order(unit, main_order_func_offsets[unit->main_order_id][0]);
								} break;
							}
						}
					}
				}
			}
		}
	}

	// Hooked to refund queued structures. -Pr0nogo
	void do_next_queued_order(Unit* unit) { // 00475000
		Order* queue_head = unit->order_queue_head;
		if (unit->unit_ai && queue_head && unit->is_burrowed()) {
			u32 order_queue_id = queue_head->order_id;
			if (order_queue_id) {
				if (order_queue_id != OrderId::Unburrow && order_queue_id != OrderId::Burrowed && order_queue_id != OrderId::ComputerAI) {
					if (!unit_id_is_trap(unit->id)
						&& (!unit_id_is_burrowed_attacker(unit->id) || order_queue_id != OrderId::Guard && order_queue_id != OrderId::AttackFixedRange))
					{
						helper::insert_order_targeting_ground_before(OrderId::Unburrow, queue_head, unit, unit->sprite->position.x, unit->sprite->position.y);
						unit->status &= ~UnitStatus::CanNotReceiveOrders;
						helper::to_idle(unit);
						return;
					}
				}
			}
		}

		u32 status = unit->status;
		if (!(status & (UnitStatus::NoBrkCodeStart | UnitStatus::CanNotReceiveOrders)) || queue_head && !queue_head->order_id) {
			if (queue_head) {
				Order* order = helper::remove_order_from_unit_queue(queue_head, unit);
				u32 order_unit_id = order->unit_id;
				u32 order_id = order->order_id;
				v2u16 order_position = order->target.pt;
				Unit* order_target = order->target.unit;
				bool cannot_be_interrupted = orders_dat::can_be_interrupted[order_id] == 0;

				status = unit->status & ~(UnitStatus::AttackMove | UnitStatus::CanNotReceiveOrders | UnitStatus::Disabled);
				unit->order_queue_timer = 0;
				unit->recent_order_timer = 0;
				unit->user_action_flags &= ~UserActionFlags_IscriptGoToIdle;
				unit->status = status | (cannot_be_interrupted << 12);
				if (order_target) {
					helper::unit_order_move_to_target_reset_order_state(unit, order_id, order_target);
				} else {
					unit->main_order_state = 0;
					unit->order_target.pt = order_position;
					unit->order_target.unit = 0;
					unit->main_order_id = order_id;
					if (order_unit_id == UnitId::None) {
						unit->order_target.pt.y = order_position.y;
						unit->order_unit_id = UnitId::None;
					} else {
						unit->order_unit_id = order_unit_id;
						unit->order_target.pt.y = order_position.y;
					}
				}
				if (!unit->unit_ai) {
					unit->auto_target_unit = 0;
				}
				helper::ignore_iscript_go_to_idle(unit);
				if (!(units_dat::base_property[unit->id] & UnitProperty::Subunit)) {
					Unit* subunit = unit->subunit;
					if (!subunit || !subunit->is_subunit()) {
						return;
					}
					u32 final_order_id = order_id;
					if (order_id == units_dat::return_to_idle_order[unit->id]) {
						final_order_id = units_dat::return_to_idle_order[subunit->id];
					} else if (order_id == units_dat::attack_unit_order[unit->id]) {
						final_order_id = units_dat::attack_unit_order[subunit->id];
					} else if (order_id == units_dat::attack_move_order[unit->id]) {
						final_order_id = units_dat::attack_move_order[subunit->id];
					} else if (!orders_dat::Unknown4[order_id]) {
						return;
					}
					if (order_target) {
						helper::order_interrupt(subunit, order_target, order_position.x, order_position.y, final_order_id);
					} else {
						helper::issue_order_targeting_fow(subunit, final_order_id, order_unit_id, order_position.x, order_position.y);
					}
				}
			}
		}
	}

	// Hooked to nullcheck target sprite. -Pr0nogo
	void orders_stay_in_range(Unit* unit) { // 0047C4F0
		Unit* target = unit->order_target.unit;
		if (target && target->sprite) { // sprite is new
			unit->order_target.pt = target->position;
			if (!helper::unit_can_attack_target(target, unit, true)) {
				if (!(unit->status & UnitStatus::AttackMove) && !unit->unit_ai) {
					helper::prepend_order_targeting_ground(OrderId::Move, unit->order_target.pt.x, unit->order_target.pt.y, unit);
				}
				helper::to_idle(unit);
				return;
			}

			if (unit->main_order_state) {
				if (!target) {
					helper::make_to_hold_position(unit);
					goto turn_to_face_target;
				}

				if (!helper::is_target_visible(unit, target)) {
					if (!helper::move_towards(unit, unit->order_target.unit)) {
						return;
					}
					goto turn_to_face_target;
				}
				Unit* final_attacker = unit->subunit;
				if (!final_attacker || !final_attacker->is_subunit()) {
					final_attacker = unit;
				}

				u32 weapon_id = bw::get_unit_weapon_based_on_target(final_attacker, target);
				u32 min_range = weapons_dat::min_range[weapon_id];
				u32 max_range = helper::get_unit_max_weapon_range(unit, weapon_id);
				if (weapon_id == WeaponId::None
				||  min_range && helper::is_target_within_min_range(final_attacker, min_range, target)
				|| !helper::is_target_within_min_movement_range(final_attacker, target, max_range))
				{
					if (!helper::move_towards(unit, unit->order_target.unit)) {
						return;
					}
				} else {
					helper::make_to_hold_position(unit);
				}
				goto turn_to_face_target;
			}

			if (!target) {
				unit->main_order_state = 1;
				goto turn_to_face_target;
			}

			if (helper::is_target_visible(unit, target)) {
				Unit* final_attacker_2 = unit->subunit;
				if (!final_attacker_2 || !final_attacker_2->is_subunit()) {
					final_attacker_2 = unit;
				}

				u32 weapon_id = bw::get_unit_weapon_based_on_target(final_attacker_2, target);
				if (weapon_id != WeaponId::None) {
					u32 min_range_2 = weapons_dat::min_range[weapon_id];
					if (!min_range_2 || !helper::is_target_within_min_range(final_attacker_2, min_range_2, target)) {
						u32 max_range_2 = helper::get_unit_max_weapon_range(unit, weapon_id);
						if (helper::is_target_within_min_movement_range(final_attacker_2, target, max_range_2)) {
							unit->main_order_state = 1;
							goto turn_to_face_target;
						}
					}
				}
			}

			if (helper::move_to_target(unit->order_target.unit, unit)) {
				unit->main_order_state = 1;
			turn_to_face_target:
				if (UnitId::turns_to_face_target((UnitId::Enum)unit->id) && !(unit->movement_flags & UnitMovement::UnitMovement_Lump)) {
					u32 pos_x = unit->order_target.pt.x;
					u32 pos_y = unit->order_target.pt.y;
					s32 direction = helper::get_direction_from_points(pos_y, unit->sprite->position.x, pos_x, unit->sprite->position.y) - unit->current_direction_1;
					if (direction >= 128 || direction <= -128) {
						direction = -128;
					}
					if (abs(direction) >= 32) {
						helper::set_secondary_waypoint(unit, pos_x, pos_y);
					}
				}
				return;
			}
		} else {
			helper::set_resource_target(unit);
			u32 move_x = unit->move_target.pt.x;
			u32 move_y = unit->move_target.pt.y;
			if (move_x != unit->next_target_waypoint.x || move_y != unit->next_target_waypoint.y) {
				unit->next_target_waypoint.x = move_x;
				unit->next_target_waypoint.y = move_y;
			}

			if (unit->order_queue_head) {
				unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
				helper::do_next_queued_order(unit);
			} else if (unit->unit_ai) {
				helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI);
			} else {
				helper::issue_order_targeting_nothing(unit, units_dat::return_to_idle_order[unit->id]);
			}
		}
	}

	// Hooked to add custom advisor handling for missile launches. -Pr0nogo
	void partial_order_handling(Unit* unit) { // 004EBED0
		switch (unit->main_order_id) {
			case OrderId::Die: {
				helper::orders_die(unit);
			} break;
			case OrderId::PlayerGuard:
			case OrderId::TurretGuard:
			case OrderId::TurretAttack:
			case OrderId::EnterTransport: {
				helper::issue_order_targeting_nothing(unit, unit->is_garrisoned() != 0 ? OrderId::BunkerGuard : OrderId::Nothing2);
			} break;
			case OrderId::Powerup1: {
				helper::orders_powerup_1(unit);
			} break;
			case OrderId::Nothing2:
			case OrderId::Nothing3:
			case OrderId::Neutral:
			case OrderId::Medic:
			case OrderId::MedicHeal1: {
				return;
			}
			case OrderId::InfestMine4: {
				helper::orders_infest_mine_4(unit);
			} break;
			case OrderId::HarvestGas3: {
				helper::orders_enter_exit_gas(unit);
			} break;
			case OrderId::Powerup2: {
				helper::orders_powerup_2(unit);
			} break;
			case OrderId::NukeLaunch: {
				helper::orders_nuke_launch(unit);
			} break;
			case OrderId::ResetCollision1: {
				helper::orders_reset_collision_1(unit);
			} break;
			case OrderId::ResetCollision2: {
				helper::orders_reset_collision_2(unit); 
			} break;
			default: {
				u32 timer = unit->order_queue_timer;
				if (timer) {
                    unit->order_queue_timer = timer - 1;
				} else {
					set_order_queue_timer_cooldown(unit);
					switch (unit->main_order_id) {
						case OrderId::BunkerGuard:
						case OrderId::ComputerAI: {
							if (unit->main_order_id == OrderId::BunkerGuard || unit->is_garrisoned()) { // This seems tautologous but it's to omit a stupid jump. -Pr0nogo
								Unit* garrisoned_unit = unit;
								u32 garrisoned_unit_id = unit->id;
								if (!bw::unit_is_ranged_attacker(garrisoned_unit)) {
									break;
								}

								garrisoned_unit->status |= UnitStatus::AttackMove;
								Unit* subunit_in_bunker = garrisoned_unit->subunit;
								if (subunit_in_bunker) {
									subunit_in_bunker->status |= UnitStatus::AttackMove;
								}
								if (helper::attack_apply_cooldown(garrisoned_unit)) {
									u32 bunker_x_pos = garrisoned_unit->order_target.pt.x;
									u32 bunker_y_pos = garrisoned_unit->order_target.pt.y;
									if (bunker_x_pos != garrisoned_unit->next_target_waypoint.x || bunker_y_pos != garrisoned_unit->next_target_waypoint.y) {
										garrisoned_unit->next_target_waypoint.x = bunker_x_pos;
										garrisoned_unit->next_target_waypoint.y = bunker_y_pos;
									}
								} else if (!garrisoned_unit->main_order_timer) {
									garrisoned_unit->main_order_timer = 15;
									Unit* target = helper::find_random_attack_target(garrisoned_unit);
									garrisoned_unit->order_target.unit = target;
									if (target) {
										garrisoned_unit->order_queue_timer = 0;
									}
								}
							}
						} break;
						case OrderId::TransportLoad: {
							helper::orders_pickup_4(unit);
						} break;
						case OrderId::RescuePassive: {
							helper::orders_rescue_passive(unit);
						} break;
						default: {
							return;
						}
					}
				}
			} break;
		}
	}

	// Hooked to do a silly imlpementation of the Artisan revision. -Pr0nogo
	s32 order_to_move_to_target(Unit* target, Unit* unit) { // 004EB980
		if (unit->ext().timers[TimerArtisanReturn]) {
			return 0;
		}
		s32 result = helper::move_to_target(target, unit);
		u32 pos_x = target->sprite->position.x;
		u32 pos_y = target->sprite->position.y;
		if (pos_x != unit->next_target_waypoint.x || pos_y != unit->next_target_waypoint.y) {
			unit->next_target_waypoint.x = pos_x;
			unit->next_target_waypoint.y = pos_y;
		}
		return result;
	}

	void orders_nuke_launch(Unit* unit) { // 00464730
		if (unit->main_order_state >= 5 || unit->connected_unit) {
			switch (unit->main_order_state) {
				case 0: {
					helper::play_sound_from_unit(SoundId::Bullet_TNsFir00_wav, unit, 1, 0);
					helper::change_movement_target_move_target_2(unit, unit->position.x, units_dat::unit_bounds[UnitId::TerranMorningstar].y);
					unit->main_order_timer = 90;
					unit->main_order_state = 1;
				} break;
				case 1: {
					if (unit->main_order_timer <= 45 || helper::is_unit_movable(unit)) {
						u32 sound_id = bw::get_advisor_sound_from_event_type(g_local_nation_id, AdvisorEvent_MissileLaunch);
						bw::play_sound(sound_id);
						if (g_stat_txt_buffer.get_string_count32() > 1046) {
							helper::show_stat_txt_to_player(g_stat_txt_buffer.get_string32(1047), g_local_nation_id);
						} else {
							helper::show_stat_txt_to_player("", g_local_nation_id);
						}
						unit->main_order_state = 2;
					}
				} break;
				case 2: {
					if (!unit->main_order_timer || helper::is_unit_movable(unit)) {
						helper::hide_unit(unit);
						unit->connected_unit->connected_unit = unit;
						helper::set_resource_target(unit);
						unit->main_order_state = 3;
					}
				} break;
				case 3: {
					if (!(unit->movement_flags & MovementFlags_Accelerating)) {
						helper::play_sprite_iscript(unit->sprite, IscriptAnimation::WarpIn, 1);
						unit->main_order_state = 4;
					}
				} break;
				case 4: {
					u32 order_signal = unit->order_signal;
					if (order_signal & 2) {
						unit->order_signal = order_signal & 0xFD;

						u32 bounds_x = unit->order_target.pt.x;
						if (units_dat::unit_bounds[UnitId::TerranMorningstar].x > bounds_x) {
							bounds_x = units_dat::unit_bounds[14].x;
						}
						u32 bounds_y = units_dat::unit_bounds[UnitId::TerranMorningstar].y;
						if (units_dat::unit_bounds[UnitId::TerranMorningstar].y <= unit->order_target.pt.y - 320) {
							bounds_y = unit->order_target.pt.y - 320;
						}

						helper::move_unit(unit, bounds_x, bounds_y);
						helper::turn_unit(unit, 128);
						helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y);
						helper::show_and_enable_unit(unit);
						unit->main_order_state = 5;

						Unit* nuker = unit->connected_unit;
						if (nuker) {
							u32 action_sound = SoundId::None;
							switch (nuker->id) {
								case UnitId::TerranEidolon:
								case UnitId::TerranEidolonNydus: {
									action_sound = SoundId::Terran_GHOST_TGhPss01_wav;
								} break;
							}

							if (action_sound) {
								if (helper::play_sound_from_unit(action_sound, nuker, g_options_list->flags & 1, 99)) {
									helper::do_unit_event_notification(nuker, 17, sfx_dat::portrait_duration[action_sound] + 1000 * (sfx_dat::sound_data[action_sound].duration_ms / 1000), 0, action_sound);
								}
							}
						}
					}
				} break;
				case 5: {
					if (helper::is_distance_greater_than_halt_distance(unit, unit->move_target.pt.x, unit->move_target.pt.y, 10)) {
						unit->order_target.unit = unit;
						for (Image* image = unit->sprite->images.head; image; image = image->link.next)
						{
							play_image_iscript(image, IscriptAnimation::SpecialState1);
						}
						unit->main_order_state = 6;
					}
				} break;
				case 6: {
					u32 order_signal_2 = unit->order_signal;
					if (order_signal_2 & 1) {
						unit->order_signal = order_signal_2 & 0xFE;
						unit->user_action_flags |= 4;
						helper::orders_self_destructing(unit);
					}
				} break;
				default: {
					return;
				}
			}
		} else {
			helper::orders_self_destructing(unit);
		}
	}
	
	// replaces CTFCop2 order
	void orders_grief_of_all_gods(Unit* unit) { // Patch for 004EC629
		if (unit->id != UnitId::ProtossStarSovereign) {
			return;
		}
		bw::decloak_passive_proc(unit);
		unit->order_target.pt = unit->position;

		auto& unit_ext = unit->ext();
		switch (unit_ext.values[ValueProtossSwitchOrderState]) {
			case 0: {
				if (unit->movement_flags & MovementFlags_Accelerating) {
					bw::make_to_hold_position(unit);
				}
				unit_ext.values[ValueProtossSwitchOrderState] = 1;
			} break;
			case 1: {
				if (!(unit->movement_flags & MovementFlags_Accelerating)
				&&    unit->sprite->main_graphic->animation != IscriptAnimation::Walking
				&&    unit_ext.timers[TimerStarSovereignGriefOfAllGodsDelay] == 0)
				{
					unit_ext.timers[TimerStarSovereignGriefOfAllGodsDelay] = 24;
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->status |= UnitStatus::CanNotAttack;
					unit_ext.values[ValueProtossSwitchOrderState] = 2;
					bw::play_sound(SoundId::StarSovereignGriefOfAllGodsChannel, unit);
					replay_witness_event(unit, SpectatorEvent_Grief);
				}
			} break;
			case 2: {
				unit->play_iscript_animation(IscriptAnimation::CastSpell);
				unit_ext.timers[TimerStarSovereignGriefOfAllGodsChannel] = 121;
				unit_ext.values[ValueProtossSwitchOrderState] = 3;
			} break;
			case 3: {
				if (unit_ext.timers[TimerStarSovereignGriefOfAllGodsChannel] != 0 && unit_ext.timers[TimerStarSovereignGriefOfAllGodsDelay] == 0 && unit->sprite->main_graphic->animation != IscriptAnimation::CastSpell) {
					unit->play_iscript_animation(IscriptAnimation::CastSpell);
					unit_ext.timers[TimerStarSovereignGriefOfAllGodsChannel] = 121;
				} else if (unit->order_signal & 4) {
					unit->order_signal &= ~0x4;
					unit->status &= ~UnitStatus::CanNotReceiveOrders;
					unit->status &= ~UnitStatus::CanNotAttack;
					unit_ext.values[ValueProtossSwitchOrderState] = 0;
					unit->order_to_idle();
				}
			} break;
		}
	}

	// Hooked to troubleshoot custom abilities. -Pr0nogo
	void targeted_order(u16 x_pos, u32 fow_unit_id, Unit* unit, u16 y_pos, u32 order_type, u32 is_queued, u8 order_id, Unit* target, v2u16 position, Unit* secondary_target) { // 004754F0
		if (is_queued && orders_dat::can_be_queued[order_id]) {
			switch (unit->main_order_id) {
				case OrderId::Guard:
				case OrderId::PlayerGuard:
				case OrderId::Nothing2:
				case OrderId::Pickup1:
				case OrderId::Patrol:
				case OrderId::Medic: {
					helper::issue_order_by_type(order_id, x_pos, y_pos, order_type, unit, target, fow_unit_id, position, secondary_target);
					return;
				} break;
				default: {
					if (unit->order_queue_count < 8) {
						if (g_order_count < 1800) {
							helper::queue_new_order(order_type, x_pos, y_pos, unit, order_id, target, fow_unit_id, position, secondary_target);
						}
						else {
							if (g_stat_txt_buffer.get_string_count() > 872) {
								helper::show_stat_txt_to_player(g_stat_txt_buffer.get_string(873), unit->player);
							} else {
								helper::show_stat_txt_to_player("", unit->player);
							}
						}
					} else {
						if (g_stat_txt_buffer.get_string_count() > 870) {
							helper::show_stat_txt_to_player(g_stat_txt_buffer.get_string(871), unit->player);
						} else {
							helper::show_stat_txt_to_player("", unit->player);
						}
					}
					break;
				}
			}
		} else {
			helper::issue_order_by_type(order_id, x_pos, y_pos, order_type, unit, target, fow_unit_id, position, secondary_target);
		}
	}

	// Hooked to investigate a crash. -Pr0nogo
	void force_order_done(Unit* unit) { // 00475420
		unit->status = unit->status & 0xFFFFEFFF;
		if (unit->order_queue_head) {
			unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
			helper::do_next_queued_order(unit);
		} else if (unit->unit_ai) {
			helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI); // Computer AI
		} else {
			helper::issue_order_targeting_nothing(unit, units_dat::return_to_idle_order[unit->id]);
		}
	}

	// Eventually, hook the following to make stat_txt.tbl a .txt:
	//		00464E40 orders_place_scanner 
	//		00474540 insert_order_before
	//		004745F0 perform_another_order
	//		004754F0 targeted_order
	// -Pr0nogo
}