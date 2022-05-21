#include "api.h"
#include "scbwdata.h"
#include <SCBW/UnitFinder.h>
#include <hooks/cosmonarchy_finder.h>
#include <algorithm>
#include <cassert>
#include <utility>
#include <hooks/weapons/weapon_damage.h>
#include <globals.h>
#include "hooks/target_related.h"
#include "hooks/unit_ai.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <hooks\unit_movement.h>

namespace scbw
{
	void artificalBulletFrom(u8 weaponId, u8 playerId, Unit* source, Unit* target, Point16 target_pt)
	{
		auto target_unit = source->orderTarget.unit;
		auto target_pos = source->orderTarget.pt;
		source->orderTarget.unit = target;
		source->orderTarget.pt = target_pt;
		Point16 pick;
		if (target != nullptr && target_unit != nullptr)
		{
			source->orderTarget.pt = target_unit->sprite->position;
		}
		else
		{
			source->orderTarget.pt = target_pt;
		}
		helper::createBullet(weaponId, source, source->sprite->position.x, source->sprite->position.y, playerId,
			getAngle(source->position.x, source->position.y, source->orderTarget.pt.x, source->orderTarget.pt.y));
		source->orderTarget.unit = target_unit;
		source->orderTarget.pt = target_pos;
	}
	void artificalBulletFrom(u8 weaponId, u8 playerId, Unit* source, Unit* target, u16 x, u16 y)
	{
		Point16 tgt;
		tgt.x = x;
		tgt.y = y;
		artificalBulletFrom(weaponId, playerId, source, target, tgt);
	}

	bool teleport(Unit* u, Point16 target, u16 spriteAnimation, bool preserveOrder)
	{
		Point16 previous_pos, new_pos;
		bool result = false;
		previous_pos.x = u->sprite->position.x;
		previous_pos.y = u->sprite->position.y;

		// identical code to setUnitPosition() but for some reason, is using another function to do exactly the same thing
		helper::moveUnit_Duplicate(u, target.x, target.y);

		if (!scbw::checkUnitCollisionPos(u, &(u->sprite->position), &new_pos, nullptr, false, 0))
		{
			//collision detected, cancel the movement
			scbw::setUnitPosition(u, previous_pos.x, previous_pos.y);
		}
		else
		{
			result = true;
			Thingy* recall_effect_sprite;

			// finish the teleporting (and cut the recalled unit from a possible unit targeted by an order)
			scbw::prepareUnitMove(u, false);
			if (!preserveOrder)
			{
				u->orderTarget.unit = nullptr;
			}
			scbw::setUnitPosition(u, new_pos.x, new_pos.y);
			scbw::refreshUnitAfterMove(u);

			// stop some currently active orders
			if (u->id != UnitId::ZergCocoon && !preserveOrder)
			{
				u->orderComputer(units_dat::ReturnToIdleOrder[u->id]);
			}

			// create the Recall animation over the unit
			if (spriteAnimation)
			{
				recall_effect_sprite = helper::createThingy(spriteAnimation, new_pos.x, new_pos.y, 0);
				if (recall_effect_sprite != nullptr)
				{
					recall_effect_sprite->sprite->elevationLevel = u->sprite->elevationLevel + 1;
					helper::setThingyVisibilityFlags(recall_effect_sprite);
				}

			}

			// disconnect a nuke from a unit using it
			if (scbw::unitCanNuke(u))
			{
				if (u->connectedUnit != nullptr
					&& u->connectedUnit->id == UnitId::TerranNuclearMissile)
				{
					u->connectedUnit->connectedUnit = nullptr;
					u->connectedUnit = nullptr;
				}
			}
		}
		return result;
	}

	Thingy* createAndInitThingy(u32 spriteId, s16 x, s16 y, u32 playerId, s32 elevationLevel)
	{
		Thingy* thingy = helper::createThingy(spriteId, x, y, playerId);
		if (thingy)
		{
			thingy->sprite->elevationLevel = elevationLevel;
			helper::setThingyVisibilityFlags(thingy);
		}
		return thingy;
	}

	void print_string(std::string text, u32 color)
	{
		print(text.c_str(), color);
	}

	void print(const char* text, u32 color)
	{
		const u32 Func_PrintText = 0x0048CD30;
		if (!text)
		{
			return;
		}

		DWORD gtc = GetTickCount() + 7000;
		__asm
		{
			PUSHAD
			PUSH 0;//unknown
			MOV eax, text
			PUSH gtc
			PUSH color
			CALL Func_PrintText
			POPAD
		}
	}

	void printf(const char* format, ...)
	{
		char buf[256];

		va_list _ArgList;
		va_start(_ArgList, format);
		vsnprintf(buf, sizeof(buf), format, _ArgList);
		__crt_va_end(_ArgList);

		print(buf);
	}

	void printf_color(const char* format, u32 color, ...)
	{
		char buf[256];

		va_list _ArgList;
		va_start(_ArgList, format);
		vsnprintf(buf, sizeof(buf), format, _ArgList);
		va_end(_ArgList);

		print(buf, color);
	}

	void printf_console(const char* format, ...)
	{
		char buf[1025];

		va_list _ArgList;
		va_start(_ArgList, format);
		vsnprintf(buf, sizeof(buf), format, _ArgList);
		va_end(_ArgList);

		OutputDebugStringA(buf);
		OutputDebugStringA("\n");
	}

	//-------- Unit checks --------//

	// Identical to function @ 0x00475CE0
	bool canWeaponTargetUnit(u8 weaponId, Unit* target, Unit* attacker)
	{
		if (weaponId >= WEAPON_TYPE_COUNT)
		{
			return false;
		}
		if (!target)
		{
			return weapons_dat::TargetFlags[weaponId].terrain;
		}
		if (target->status & UnitStatus::Invincible)
		{
			return false;
		}

		const TargetFlag tf = weapons_dat::TargetFlags[weaponId];
		const u32 targetProps = units_dat::BaseProperty[target->id];
		if ((target->status & UnitStatus::InAir) ? !tf.air : !tf.ground)
		{
			return false;
		}
		if (tf.mechanical && !(targetProps & UnitProperty::Mechanical))
		{
			return false;
		}
		if (tf.organic && !(targetProps & UnitProperty::Organic))
		{
			return false;
		}
		if (tf.nonBuilding && (targetProps & UnitProperty::Building))
		{
			return false;
		}
		if (tf.nonRobotic && (targetProps & UnitProperty::RoboticUnit))
		{
			return false;
		}
		if (tf.orgOrMech && !(targetProps & (UnitProperty::Organic | UnitProperty::Mechanical)))
		{
			return false;
		}
		if (tf.playerOwned && target->playerId != attacker->playerId)
		{
			return false;
		}
		return true;
	}

	bool isUnderDarkSwarm(Unit* unit)
	{
		return !(unit->status & UnitStatus::InAir) && getDarkSwarmFromUnit(unit) != nullptr;
	}

	bool IsDarkSwarm(Unit* unit)
	{
		return unit->id == UnitId::Spell_DarkSwarm;
	}

	Unit* getDarkSwarmFromUnit(Unit* unit)
	{
		static UnitFinder darkSwarmFinder;
		darkSwarmFinder.search(unit->getLeft(), unit->getTop(), unit->getRight(), unit->getBottom());

		Unit* darkSwarm = darkSwarmFinder.getFirst(IsDarkSwarm);
		return darkSwarm;
	}

	Unit* getDarkSwarmFromPosition(u16 x, u16 y, u16 radius)
	{
		UnitFinder darkSwarmFinder;
		darkSwarmFinder.search(x - radius, y - radius, x + radius, y + radius);
		Unit* darkSwarm = darkSwarmFinder.getFirst(IsDarkSwarm);
		return darkSwarm;
	}

	//-------- Graphics and geometry --------//

	// Improved code from BWAPI's include/BWAPI/Position.h: getApproxDistance()
	// Logically same as function @ 0x0040C360
	// x1 = ECX;x2 = EDX;y1 = [EBP+08];y2 = [EBP+0C];
	u32 getDistanceFast(s32 x1, s32 y1, s32 x2, s32 y2)
	{
		s32 dMax = abs(x1 - x2), dMin = abs(y1 - y2);
		if (dMax < dMin)
		{
			std::swap(dMax, dMin);
		}
		if (dMin <= (dMax >> 2))
		{
			return dMax;
		}
		return (dMin * 3 >> 3) + (dMin * 3 >> 8) + dMax - (dMax >> 4) - (dMax >> 6);
	}

	u32 getDistanceSquare(s32 x1, s32 y1, s32 x2, s32 y2)
	{
		return std::max(abs(x1 - x2), abs(y1 - y2));
	}

	// Identical to function @ 0x00494C10
	s32 arctangent(s32 slope)
	{
		const u32 tangentTable[] = {
				 7,	 13,	 19,	 26,	 32,	 38,	 45,	 51,
				58,	 65,	 71,	 78,	 85,	 92,	 99,	107,
			 114,	122,	129,	137,	146,	154,	163,	172,
			 181,	190,	200,	211,	221,	233,	244,	256,
			 269,	283,	297,	312,	329,	346,	364,	384,
			 405,	428,	452,	479,	509,	542,	578,	619,
			 664,	716,	775,	844,	926, 1023, 1141, 1287,
			1476, 1726, 2076, 2600, 3471, 5211, 10429, -1
		};

		bool isNegative = false;
		if (slope < 0)
		{
			isNegative = true;
			slope = -slope;
		}

		s32 min = 0, max = 64, angle = 32;
		do
		{
			if ((u32)slope <= tangentTable[angle])
			{
				max = angle;
			}
			else
			{
				min = angle + 1;
			}
			angle = (min + max) / 2;
		} while (min != max);
		return (isNegative ? -angle : angle);
	}

	// Identical to function @ 0x00495300
	//					EDX		[EBP+08]	ECX			EAX
	s32 getAngle(s32 xStart, s32 yStart, s32 xEnd, s32 yEnd)
	{

		s32 dx = xEnd - xStart, dy = yEnd - yStart;

		if (dx == 0)
			return dy > 0 ? 128 : 0;

		s32 angle = arctangent((dy << 8) / dx);

		if (dx < 0)
		{
			angle += 192;
			return angle == 256 ? 0 : angle;
		}
		else
			return angle + 64;

	}

	Point16 rotate_point(Point16 anchor, Point16 point, f64 angle_degrees) {
		f64 x1 = point.x - anchor.x;
		f64 y1 = point.y - anchor.y;
		auto radians = angle_degrees / 57.296;
		f64 x2 = x1 * cos(radians) - y1 * sin(radians);
		f64 y2 = x1 * sin(radians) + y1 * cos(radians);
		Point16 product = {};
		product.x = x2 + anchor.x;
		product.y = y2 + anchor.y;
		return product;
	}

	Vector32 rotate_point(Vector32 anchor, Vector32 point, f64 angle_degrees) {
		f64 x1 = point.x - anchor.x;
		f64 y1 = point.y - anchor.y;
		auto radians = angle_degrees / 57.296;
		f64 x2 = x1 * cos(radians) - y1 * sin(radians);
		f64 y2 = x1 * sin(radians) + y1 * cos(radians);
		Vector32 product = {};
		product.x = x2 + anchor.x;
		product.y = y2 + anchor.y;
		return product;
	}

	f64 bw_angle_to_normal(s32 bw_angle)
	{
		f64 angle = (f64)bw_angle * 1.40625;
		return angle;
	}

	s32 getPolarX(s32 distance, u8 angle)
	{
		return distance * angleDistance[angle].x >> 8;
	}

	s32 getPolarY(s32 distance, u8 angle)
	{
		return distance * angleDistance[angle].y >> 8;
	}

	//-------- Player information --------//

	s32 getSupplyRemaining(u8 playerId, u8 raceId)
	{
		assert(raceId <= 2);
		assert(playerId < PLAYER_COUNT);

		s32 supplyProvided;
		if (isCheatEnabled(CheatFlags::FoodForThought))
		{
			supplyProvided = raceSupply[RaceId::Terran].max[playerId];
		}
		else
		{
			supplyProvided = std::min(raceSupply[RaceId::Terran].max[playerId], raceSupply[RaceId::Terran].provided[playerId]);
		}
		return supplyProvided - raceSupply[RaceId::Terran].used[playerId];

	}

	bool hasTechResearched(u8 playerId, u16 techId)
	{
		assert(playerId < PLAYER_COUNT);
		assert(techId < TechId::None);
		if (techId < TechId::Restoration)
		{
			return TechSc->isResearched[playerId][techId] != 0;
		}
		else
		{
			return TechBw->isResearched[playerId][techId - TechId::Restoration] != 0;
		}
	}

	bool hasTechAvailable(u8 playerId, u16 techId)
	{
		assert(playerId < PLAYER_COUNT);
		assert(techId < TechId::None);
		if (techId < TechId::Restoration)
		{
			return TechSc->isEnabled[playerId][techId] != 0;
		}
		else
		{
			return TechBw->isEnabled[playerId][techId - TechId::Restoration] != 0;
		}
	}

	void setTechResearchState(u8 playerId, u16 techId, bool isResearched)
	{
		assert(playerId < PLAYER_COUNT);
		assert(techId < TechId::None);

		if (techId < TechId::Restoration)
		{
			TechSc->isResearched[playerId][techId] = isResearched;
		}
		else
		{
			TechBw->isResearched[playerId][techId - TechId::Restoration] = isResearched;
		}
	}

	const u32 Func_DoWeaponDamage = 0x00479930;
	void sendGPTP_aise_cmd(Unit* unit1, Unit* unit2, u32 command, u32 v1, u32 v2, u32 v3)
	{
		_asm
		{
			PUSHAD
			MOV EAX, v3
			MOV EDI, unit2
			PUSH v2
			PUSH unit1
			PUSH 50000
			PUSH command
			PUSH v1
			CALL Func_DoWeaponDamage
			POPAD
		}
	}

	u32 getFlingyData(u32 entry, u32 index)
	{
		return get_aise_value(nullptr, nullptr, AiseId::GetFlingyData, entry, index);
	}
	u32 getSpriteData(u32 entry, u32 index)
	{
		return get_aise_value(nullptr, nullptr, AiseId::GetSpriteData, entry, index);
	}

	u32 getImageData(u32 entry, u32 index)
	{
		return get_aise_value(nullptr, nullptr, AiseId::GetImageData, entry, index);
	}
	u32 getUnitData(u32 entry, u32 index)
	{
		return get_aise_value(nullptr, nullptr, AiseId::GetUnitData, entry, index);
	}

	bool isLarvalEgg(Unit* unit)
	{
		if (unit->id == UnitId::ZergEgg)
		{
			u8 origin = unit->eggOrigin;
			if (origin == UnitId::ZergLarva)
			{
				return true;
			}
		}
		return false;
	}

	bool isZergEggNew(Unit* unit, bool allow_larval_eggs, bool allow_larvae, u16 explicit_unit_id)
	{
		u16 id = UnitId::None;
		if (id != NULL)
		{
			id = unit->id;
		}
		else
		{
			id = explicit_unit_id;
		}
		switch (id)
		{
			case UnitId::ZergEgg:
			{
				if (unit && !allow_larval_eggs && unit->eggOrigin == UnitId::ZergLarva)
				{
					return false;
				}
				return true;
			}
			case UnitId::ZergCocoon:
			{
				return true;
			}
		}

		if (allow_larvae && id == UnitId::ZergLarva)
		{
			return true;
		}
		return false;
	}

	bool isZergEgg(u16 id, bool allow_normal_eggs, bool allow_larvae)
	{
		switch (id)
		{
			case UnitId::ZergCocoon:
			{
				return true; 
			}
			default:
			{
			} break;
		}
		if (id == UnitId::ZergLarva && allow_larvae)
		{
			return true;
		}
		if (id == UnitId::ZergEgg && allow_normal_eggs)
		{
			return true;
		}
		return false;
	}

	bool isConventionalUnitMorphResult(u16 id)
	{
		switch (id)
		{
			case UnitId::ZergIroleth:
			case UnitId::ZergAlaszileth:
			case UnitId::ZergVorvrokor:
			case UnitId::ZergLiiralisk:
			case UnitId::ZergBactalisk:
			case UnitId::ZergGorgrokor:
			case UnitId::ZergVithrilisk:
			case UnitId::ZergKalkalisk:
			case UnitId::ZergAkistrokor:
			case UnitId::ZergAlmaksalisk:
			case UnitId::ZergUltrokor:
			case UnitId::ZergAlkajelisk:
			case UnitId::ProtossAugur:
			{
				return true;
			}
			default:
			{
			} break;
		}
		return false;
	}

	bool isAnyMorphMorphResult(u16 id)
	{
		if (isConventionalUnitMorphResult(id))
		{
			return true;
		}
		switch (id)
		{
			case UnitId::ProtossArchon:
			case UnitId::ProtossMindTyrant:
			{
				return true;
			}
			default:
			{
			} break;
		}
		return false;
	}

	bool isConventionalUnitMorphSource(u16 id, bool allow_larvae, bool allow_high_templar)
	{
		switch (id)
		{
			case UnitId::ZergOvileth:
			case UnitId::ZergOthstoleth:
			case UnitId::ZergZethrokor:
			case UnitId::ZergQuazilisk:
			case UnitId::ZergHydralisk:
			case UnitId::ZergSkithrokor:
			case UnitId::ZergMutalisk:
			case UnitId::ZergNathrokor:
			case UnitId::ZergMatraleth:
			case UnitId::ZergKonvilisk:
			case UnitId::ZergZoryusthaleth:
			case UnitId::ZergKeskathalor:
			{
				return true; 
			}
			default:
			{
			} break;
		}
		if (id == UnitId::ProtossCantavis && allow_high_templar)
		{
			return true;
		}
		if (id == UnitId::ZergLarva && allow_larvae)
		{
			return true;
		}
		return false;
	}

	bool isZergBuildingMorphResult(u16 id)
	{
		switch (id)
		{
			case UnitId::ZergSire:
			case UnitId::ZergHive:
			case UnitId::ZergLair:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergAlaszilArbor:
			case UnitId::ZergLarvalColony:
			case UnitId::ZergSporeColony:
			case UnitId::ZergSunkenColony:
			case UnitId::ZergLiivralPond:
			case UnitId::ZergVithrathHaunt:
			case UnitId::ZergGorgralSwamp:
			case UnitId::ZergAlmakisAntre:
			case UnitId::ZergAlkajChasm:
			case UnitId::ZergUltrokCavern:
			{
				return true;
			}
			default:
			{
			} break;
		}
		return false;
	}

	bool isZergBuildingMorphSource(u16 id)
	{
		switch (id)
		{
			case UnitId::ZergHive:
			case UnitId::ZergLair:
			case UnitId::ZergHatchery:
			case UnitId::ZergIrolIris:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergCreepColony:
			case UnitId::ZergQuazrokPool:
			case UnitId::ZergHydrokDen:
			case UnitId::ZergMuthrokSpire:
			case UnitId::ZergMatravilNest:
			case UnitId::ZergZorkizShroud:
			case UnitId::ZergGeszkathGrotto:
			case UnitId::ZergSovthrathMound:
			{
				return true;
			}
			default:
			{
			} break;
		}
		return false;
	}

	std::string readSamaseFile(char* name)
	{
		const char* data = nullptr;
		u32 len = 0;
		auto data_ptr = &data;
		auto len_ptr = &len;
		u32 val = reinterpret_cast<u32>(name);
		auto length = scbw::get_aise_value(nullptr, reinterpret_cast<Unit*>(name), AiseId::ReadSamaseFile, reinterpret_cast<s32>(data_ptr), reinterpret_cast<s32>(len_ptr));
		char* buf = (char*)scbw::get_aise_value(nullptr, nullptr, AiseId::ReadSamaseFile_Ptr, 0, 0);
		std::string str;
		if (length == 0)
		{
			return str;
		}
		else
		{
			str = std::string(buf, length);
		}
		return str;
	}

	s32 get_generic_variable(u32 id)
	{
		return get_aise_value(nullptr, nullptr, AiseId::GenericVariable, id, 0);
	}

	Unit* get_generic_rust_unit(Unit* unit, u32 id)
	{
		u32 val = get_aise_value(unit, nullptr, AiseId::GenericUnit, id, 0);
		return reinterpret_cast<Unit*>(val);
	}

	void set_generic_rust_unit(Unit* unit, u32 id, Unit* targ)
	{
		scbw::sendGPTP_aise_cmd(unit, targ, GptpId::SetGenericUnit, id, 0, 0);
	}

	void set_generic_variable(u32 id, s32 val)
	{
		scbw::sendGPTP_aise_cmd(nullptr, nullptr, GptpId::SetGenericVariable, id, val, 0);
	}

	u32 get_generic_rust_value(Unit* u, u32 val)
	{
		return get_aise_value(u, nullptr, AiseId::GenericValue, val, 0);
	}

	void set_generic_rust_value(Unit* u, u32 id, u32 val)
	{
		scbw::sendGPTP_aise_cmd(u, nullptr, GptpId::SetGenericValue, id, val, 0);
	}

	void setSpriteData(u32 entry, u32 index, u32 value)
	{
		return sendGPTP_aise_cmd(nullptr, nullptr, GptpId::SetSpriteData, entry, index, value);
	}

	const u32 Func_CanTargetSpell = 0x00492020;
	u32 get_aise_value(Unit* unit1, Unit* unit2, u32 command, u32 v1, u32 v2)
	{
		u32 result = 0;
		u32 v3 = 0; // pointer
		_asm
		{
			PUSHAD
			MOV ESI, v2 // fow_unit_id
			MOV EDI, unit2 // target
			MOV EDX, 50000 // tech
			PUSH v3 // outerror
			PUSH v1 // y
			PUSH command // x
			PUSH unit1 // unit
			CALL Func_CanTargetSpell
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_PsiPlacement = 0x00473920;
	u8 getUpgradeLevel(u8 playerId, u8 upgradeId)
	{
		assert(playerId < PLAYER_COUNT);
//		assert(upgradeId < UpgradeId::None);
		assert(upgradeId < 256);

		if (upgradeId < UpgradeId::PrismaticShield)
		{
			return UpgradesSc->currentLevel[playerId][upgradeId];
		}
		/*
		else if (upgradeId<UpgradeId::None)
		{
			return UpgradesBw->currentLevel[playerId][upgradeId - UpgradeId::PStructurePlating];
		}
		*/
		else
		{
			u32 result = 0;
			u32 player_u32 = playerId;
			u32 upgrade_u32 = upgradeId;
			_asm {
				PUSHAD
				MOV ESI, upgrade_u32
				MOV EAX, player_u32
				PUSH 0
				PUSH 50000
				CALL Func_PsiPlacement
				MOV result, EAX
				POPAD
			}
			return result;
		}
	}
	//EBX = max/current upgrade read, ECX - player, EDI - id

	u8 getUpgradeMaxLevel(u8 playerId, u8 upgradeId)
	{
		assert(playerId < PLAYER_COUNT);
//		assert(upgradeId < UpgradeId::None);
		assert(upgradeId < 256);
		if (upgradeId < UpgradeId::PrismaticShield)
		{
			return UpgradesSc->maxLevel[playerId][upgradeId];
		}
		/*
		else if (upgradeId<UpgradeId::None)
		{
			return UpgradesBw->maxLevel[playerId][upgradeId - UpgradeId::PStructurePlating];
		}
		*/
		else
		{
			u32 result = 0;
			u32 player_u32 = playerId;
			u32 upgrade_u32 = upgradeId;
			_asm
			{
				PUSHAD
				MOV ESI, upgrade_u32
				MOV EAX, player_u32
				PUSH 1
				PUSH 50000
				CALL Func_PsiPlacement
				MOV result, EAX
				POPAD
			}
			return result;
		}
	}

	void setUpgradeLevel(u8 playerId, u8 upgradeId, u8 level)
	{
		assert(playerId < PLAYER_COUNT);
		assert(upgradeId < UpgradeId::None);
		if (upgradeId < UpgradeId::PrismaticShield)
			UpgradesSc->currentLevel[playerId][upgradeId] = level;
		else
			UpgradesBw->currentLevel[playerId][upgradeId - UpgradeId::PrismaticShield] = level;
	}

	void adjustAddonX(u16 addonId, u16 sourceId, u16* x)
	{
		if (addonId == UnitId::TerranQuarry)
		{
			*x = 128;
		}
	}

	void adjustAddonY(u16 addonId, u16 sourceId, u16* y)
	{
		if (addonId == UnitId::TerranQuarry)
		{
			*y = 32;
		}
	}

	void adjustAddonCoordinates(u16 addonId, u16 sourceId, u16* x, u16* y)
	{
		adjustAddonX(addonId, sourceId, x);
		adjustAddonY(addonId, sourceId, y);
	}

	//-------- Moving Units --------//

	bool moveUnit(Unit* unit, s16 x, s16 y)
	{
		Point16 targetPos = {x, y};
		Point16 actualPos = {};
		Point16 prevPos = unit->sprite->position;

		// Based on the code for Recall/Nydus Canal/Move Unit trigger
		setUnitPosition(unit, x, y);

		if (checkUnitCollisionPos(unit, &targetPos, &actualPos))
		{
			prepareUnitMove(unit);
			setUnitPosition(unit, actualPos.x, actualPos.y);
			refreshUnitAfterMove(unit);
			return true;
		}
		else
		{
			setUnitPosition(unit, prevPos.x, prevPos.y);
			return false;
		}
	}

	bool moveUnitForce(Unit* unit, s16 x, s16 y)
	{
		prepareUnitMove(unit);
		hooks::MoveUnit(unit, x, y, true);
		return true;
	}

	bool moveUnitImproved(Unit* unit, s16 x, s16 y)
	{
		if (!(unit->status & UnitStatus::InAir))
		{
			ActiveTile originalTile = scbw::getActiveTileAt(unit->position.x, unit->position.y);
			ActiveTile finalTile = scbw::getActiveTileAt(x, y);

			if (finalTile.isUnwalkable || originalTile.groundHeight != finalTile.groundHeight)
			{
				return false;
			}

			Unit* collision = scbw::UnitFinder(
				unit->getLeft(),
				unit->getTop(),
				unit->getRight(),
				unit->getBottom()
			).getFirst(cosmonarchyFinderMatch(unit, Flags::NotSelf | Flags::NoAir, {}));

			if (collision)
			{
				return false;
			}
		}

		prepare_unit_move_custom(unit);
		setUnitPosition(unit, x, y);
		refreshUnitAfterMove(unit);
		return true;
	}

	void prepare_unit_move_custom(Unit* unit)
	{
		helper::sort_unit(unit);
		helper::refresh_grp_image(unit->sprite);
		if (unit->subunit)
		{
			helper::refresh_grp_image(unit->subunit->sprite);
		}

		helper::toggleUnitPath(unit);
		char pathing_flags = unit->pathingFlags;
		unit->movementState = 0;
		unit->pathingFlags = (pathing_flags ^ (unit->sprite->elevationLevel < 12)) & 1 ^ pathing_flags;
	}

	//-------- Utility functions --------//

	Unit* createUnitInGame(u16 unitId, u8 playerId, u32 x, u32 y, Unit* creator, bool initAI)
	{
		if (*unitCount >= 1700)
		{
			return nullptr; // Prevent crashes, move to create unit later. - Veeq7
		}
		Point16 pos = {x, y};
		helper::fixTargetLocation(&pos, unitId);
		Unit* unit = helper::createUnit(unitId, pos.x, pos.y, playerId);
		if (unit == nullptr)
		{
			helper::displayLastNetErrorForPlayer(playerId);
		}
		else
		{
			helper::updateUnitStatsFinishBuilding(unit);
			helper::updateUnitStrength(unit);
			if (initAI)
			{
				helper::AI_training_unit(unit, creator);
			}
		}
		return unit;
	}

	u32 getUnitOverlayAdjustment(Unit* const unit)
	{
		if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
		{
			return 1;
		}
		else if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
		{
			return 2;
		}
		else
		{
			return 0;
		}
	}

	void refreshScreen(s32 left, s32 top, s32 right, s32 bottom)
	{

		left >>= 4; right = (right + 15) >> 4;
		top >>= 4; bottom = (bottom + 15) >> 4;

		if (left > right)
		{
			std::swap(left, right);
		}
		if (top > bottom)
		{
			std::swap(top, bottom);
		}

		// Rect out of bounds
		if (left >= 40 || right < 0 || top >= 30 || bottom < 0)
		{
			return;
		}

		left = std::max(left, 0); right = std::min(right, 40 - 1);
		top = std::max(top, 0); bottom = std::min(bottom, 30 - 1);

		for (s32 y = top; y <= bottom; ++y)
		{
			memset(&refreshRegions[40 * y + left], 1, right - left + 1);
		}
	}

	void refreshScreen()
	{
		memset(refreshRegions, 1, 1200);
	}

	/* /!\ Not working like 004DC550 RandBetween /!\ */
	u32 randBetween(u32 min, u32 max)
	{
		assert(min <= max);
		return min + ((max - min + 1) * random() >> 15);
	}

	// Similar to 004DC4A0 RandomizeShort, but doesn't update the RNG and doesn't use a parameter among possible differences.
	u16 random()
	{
		if (*IS_IN_GAME_LOOP)
		{
			*lastRandomNumber = 22695477 * (*lastRandomNumber) + 1;
			return (*lastRandomNumber >> 16) % 32768; // Make a number between 0 and 32767
		}
		else
		{
			return 0;
		}
	}

	void refreshConsole() // 004C36C0
	{
		static u32* const bCanUpdateCurrentButtonSet = (u32*)0x0068C1B0;
		static u8* const bCanUpdateSelectedUnitPortrait = (u8*)0x0068AC74;
		static u8* const bCanUpdateStatDataDialog = (u8*)0x0068C1F8;
		static BinDlg** const someDialogUnknown = (BinDlg**)0x0068C1E8;
		static BinDlg** const someDialogUnknownUser = (BinDlg**)0x0068C1EC;

		*bCanUpdateCurrentButtonSet = 1;
		*bCanUpdateSelectedUnitPortrait = 1;
		*bCanUpdateStatDataDialog = 1;
		*someDialogUnknown = nullptr;
		*someDialogUnknownUser = nullptr;
	}

	void createUnitFromAbility(s32 unitId, s32 count, u16 playerId, s32 x, s32 y, u32 removeTimer, Unit* creator)
	{
		for (s32 i = 0; i < count; i++)
		{
			Unit* unit = scbw::createUnitInGame(unitId, playerId, x, y, creator, false);
			if (unit == nullptr)
			{
				return;
			}
			hooks::AI_training_unit(unit, creator, true);
			if (creator != nullptr)
			{
				if (creator->status & UnitStatus::IsHallucination)
				{
					helper::apply_hallucination_stats(unit);
					unit->status |= UnitStatus::IsHallucination;
					unit->removeTimer = 1350;
					unit->sprite->createOverlay(ImageId::HallucinationDeath3);
				}
			}
			switch (unitId)
			{
				case UnitId::ZergRilirokor:
				case UnitId::ZergRililisk:
				{
					helper::playSoundFromSource(SoundId::Zerg_BROODLING_ZBrRdy00_WAV, unit, 1, 0);
					if (unit->removeTimer == 0 || unit->removeTimer > removeTimer)
					{
						unit->removeTimer = removeTimer;
					}
					u32 range = 6 * 32;
					scbw::UnitFinder(
						unit->getX() - range,
						unit->getY() - range,
						unit->getX() + range,
						unit->getY() + range
					).forEach([&](Unit* target)
					{
						if (target->id != UnitId::ZergMatraleth)
						{
							return;
						}
						if (!scbw::isAlliedTo(unit->playerId, target->playerId))
						{
							return;
						}
						if (target->energy < 200 * 256)
						{
							target->energy = std::min((u16)(target->energy + (u16)5 * (u16)256), target->getMaxEnergy());
							target->sprite->createUnderlay(425);
						}
					});
					break;
				}
				case UnitId::ProtossSimulacrum:
				{
					if (creator != nullptr)
					{
						unit->hitPoints = std::max(256, creator->hitPoints);
						unit->shields = std::max(256, creator->shields);
						scbw::inheritDebuffs(creator, unit);
					}
					helper::playSoundFromSource(SoundId::Simulacrum_Clone_01, unit, 1, 0);
				}
			}
		}
	}

	void rangeModifiedFire(u8 weaponId, Unit* unit, s16 x, s16 y, u32 offset)
	{
		auto target_unit = unit->orderTarget.unit;
		auto target_pos = unit->orderTarget.pt;
		auto distance = scbw::getDistanceFast(unit->position.x, unit->position.y, unit->orderTarget.unit->position.x, unit->orderTarget.unit->position.y);
		distance += offset;
		if (distance < 0)
		{
			distance = 0;
		}
		auto polar_x = unit->getX() + scbw::getPolarX(distance, unit->currentDirection1);
		auto polar_y = unit->getY() + scbw::getPolarY(distance, unit->currentDirection1);
		unit->orderTarget.unit = nullptr;
		unit->orderTarget.pt.x = polar_x;
		unit->orderTarget.pt.y = polar_y;
		helper::createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
		unit->orderTarget.unit = target_unit;
		unit->orderTarget.pt = target_pos;
	}

	bool canSpawnBroodling(Unit* target)
	{
		if (units_dat::GroupFlags[target->id].isBuilding) return false;
		return true;
	}

	s32 getBroodlingCountFromSupply(Unit* target)
	{
		if (!canSpawnBroodling(target)) return 0;
		bool ground = !(target->status & UnitStatus::InAir);
		bool onCreep = scbw::getActiveTileAt(target->getX(), target->getY()).hasCreep;
		f64 multiplier = (ground && onCreep) ? 1.5 : 1;
		return std::ceil(std::max((s32)units_dat::SupplyRequired[target->id], 1) * multiplier);
	}

	s32 dir_saturate(s32 direction, s32 value)
	{
		s32 d = direction + value;
		if (d > 255)
		{
			return d - 255;
		}
		if (d < 0)
		{
			return d + 255;
		}
		return d;
	}

	u32 get_sized_overlay(Unit* unit, u32 initial)
	{
		u32 overlayImageId;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
		{
			overlayImageId = initial + 1;
		}
		else if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
		{
			overlayImageId = initial + 2;
		}
		else
		{
			overlayImageId = initial;
		}
		return overlayImageId;
	}

	bool can_use_cliff_advantage(Unit* unit, u8 weapon)
	{
		return !(unit->status & UnitStatus::InAir)
			&& unit->id != UnitId::ProtossPlasmaShell
			&& (!weapon || weapons_dat::MaxRange[weapon] > 32)
			&& unit->id != UnitId::TerranHarakan;
	}

	s32 get_cliff_advantage(Unit* unit, Unit* target_unit, u8 weapon)
	{
		if (!can_use_cliff_advantage(unit, weapon))
		{
			return 0;
		}
		s32 source_height = scbw::getActiveTileAt(unit->position.x, unit->position.y).groundHeight / 2;
		if (!target_unit)
		{
			return source_height * 2;
		}
		s32 target_height = scbw::getActiveTileAt(target_unit->position.x, target_unit->position.y).groundHeight / 2;
		return std::max(source_height - target_height, 0) * 2;
	}

	void applyEnsnare(Unit* unit)
	{ // 004F45E0
		if (!(unit->ensnareTimer)
			&& !(unit->status & UnitStatus::Burrowed))
		{
			Unit* victim = unit;
			if (unit->subunit != nullptr)
			{
				victim = unit->subunit;
			}

			// Create overlay
			victim->sprite->createOverlay(ImageId::EnsnareOverlay_Small + scbw::getOverlayOffset(unit->id));
		}

		// Apply effect
		unit->ensnareTimer = 37;
		unit->updateSpeed();
	}

	bool isEnabled(Unit* unit)
	{
		if (unit->lockdownTimer
			|| unit->maelstromTimer
			|| unit->stasisTimer
			|| unit->status & UnitStatus::Disabled)
		{
			return false;
		}
		return true;
	}

	s32 getUpgradeType(Unit* unit)
	{
		s32 race = unit->getRace();
		f64 upgType;
		switch (race)
		{
			case RaceId::Zerg:
			{
				upgType = UpgradeId::ZergMeleeAttacks;
			} break;
			case RaceId::Protoss:
			{
				upgType = UpgradeId::ProtossGroundWeapons;
			} break;
			default:
			{
				upgType = UpgradeId::TerranInfantryWeapons;
			}
		}
		return upgType;
	}

	s32 getOverlayOffset(u16 unitId, bool xlargeAllowed)
	{
		u32 overlayOffset = 0;
		if (xlargeAllowed && scbw::unitIdUsesXlargeOverlays(unitId))
		{
			overlayOffset = 3;
		}
		else if (units_dat::BaseProperty[unitId] & UnitProperty::MediumOverlay)
		{
			overlayOffset = 1;
		}
		else if (units_dat::BaseProperty[unitId] & UnitProperty::LargeOverlay)
		{
			overlayOffset = 2;
		}
		return overlayOffset;
	}

	s32 getFinalDamage(Unit* unit, u16 weaponId)
	{
		s32 upgType = getUpgradeType(unit);
		s32 baseDamage = weapons_dat::DamageAmount[weaponId];
		f64 bonusDamage = (f64)weapons_dat::DamageBonus[weaponId] * (f64)scbw::getUpgradeLevel(unit->playerId, upgType);
		f64 finalDamage = baseDamage;
		if (bonusDamage)
		{
			finalDamage = baseDamage + bonusDamage;
		}
		return finalDamage;
	}

	// I may need to make this function more flexible later -- Pr0nogo
	bool unitIsPacifist(Unit* unit)
	{
		return !((unit->hasWeapon())
			|| unit->id == UnitId::ZergLakizilisk); // LAME
	}

	// Identical to function OrderIsSpell @ 00436930 -- Pr0nogo
	// We should update the lardcode for Cosmonarchy
	bool orderIsSpell(s32 orderId)
	{
		return orderId == OrderId::Plague
			|| orderId == OrderId::DarkSwarm
//			|| orderId == OrderId::Ensnare				// Land Grab
			|| orderId == OrderId::Irradiate
			|| orderId == OrderId::StasisField
			|| orderId == OrderId::EmpShockwave			// Observance
			|| orderId == OrderId::MagnaPulse			// Lobotomy Mine
			|| orderId == OrderId::PsiStorm
//			|| orderId == OrderId::SummonBroodlings		// unused
			|| orderId == OrderId::FireYamatoGun1;
	}

	bool canBeParasited(Unit* unit)
	{
		return !(units_dat::BaseProperty[unit->id] & UnitProperty::Building);
	}

	// Creates the Plasma Shield flickering effect.
	// Identical to function @ 0x004E6140
	void createShieldOverlay(Unit* unit, u32 attackDirection)
	{
		//const LO_Header* shield_lo = shieldOverlays[unit->sprite->mainGraphic->id];
		const LO_Header* shield_lo = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlayShieldPtr, unit->sprite->mainGraphic->id));
		u32 frameAngle = (attackDirection - 124) / 8 % 32;

		Point8 offset = shield_lo->getOffset(unit->sprite->mainGraphic->direction, frameAngle);

		unit->sprite->createOverlay(ImageId::ShieldOverlay, offset.x, offset.y, frameAngle);
	}

	void decloakProc(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		if (isDecloakable(unit))
		{
			if (unitExt->timers[TimerDecloak] == 0)
			{
				unit->visibilityStatus = 0xff;
				helper::animDecloak(unit, SoundId::Terran_PHOENIX_TPhClo01_WAV);
			}
			if (unitExt->timers[TimerDecloak] != 1)
			{
				unitExt->timers[TimerDecloak] = 48 + 1;
			}
		}
		if (unit->isSubunit() && unit->subunit)
		{
			decloakProc(unit->subunit);
		}
	}

	bool isDecloakable(Unit* unit)
	{
		return (units_dat::BaseProperty[unit->id] & UnitProperty::PermanentCloak || unit->status & UnitStatus::CloakingForFree)
			&& !(unit->status & UnitStatus::Burrowed);
	}

	// Replacement for isUnitGhost
	bool unitCanNuke(Unit* unit)
	{
		return (unit->id == UnitId::TerranEidolon || unit->id == UnitId::TerranSeraph);
	}

	bool unitIsTrap(Unit* unit)
	{
		return (unit->id == UnitId::TerranSpiderMine || unit->id == UnitId::TerranLobotomyMine);
	}

	bool unitIsRangedAttacker(Unit* unit)
	{
		u16 id = unit->id;
		// Would be nice to somehow avoid ID hardcode
		switch (id)
		{
			// Prohibited IDs
			case UnitId::ProtossAmaranth:
			case UnitId::ZergAkistrokor:
			case UnitId::ZergUltrokor:
			{
				return false;
			} break;
			// Allowed IDs
			case UnitId::TerranHarakan:
			case UnitId::TerranGoliathTurret:
			case UnitId::TerranPhalanxTankMode:
			case UnitId::TerranPhalanxSiegeMode:
			case UnitId::TerranPhalanxTankModeTurret:
			case UnitId::TerranPhalanxSiegeModeTurret:
			case UnitId::ProtossAccantor:
			case UnitId::ProtossPlasmaShell:
			{
				return true;
			} break;
			default:
			{
			}
		}

		u16 groundWeaponId = units_dat::GroundWeapon[id];
		if (unit->subunit)
		{
			groundWeaponId = units_dat::GroundWeapon[unit->subunit->id];
		}
		if (groundWeaponId != WeaponId::None)
		{
			if (weapons_dat::MaxRange[groundWeaponId] > 32)
			{
				return true;
			}
		}
		u16 airWeaponId = units_dat::AirWeapon[id];
		if (unit->subunit)
		{
			airWeaponId = units_dat::AirWeapon[unit->subunit->id];
		}
		if (airWeaponId != WeaponId::None)
		{
			if (weapons_dat::MaxRange[airWeaponId] > 32)
			{
				return true;
			}
		}
		return false;
	}

	void playWorkAnimation(Unit* unit)
	{
		Image* current_image = unit->sprite->images.head;
		unit->secondaryOrderState = 2;
		while (current_image != nullptr)
		{
			current_image->playIscriptAnim(IscriptAnimation::IsWorking);
			current_image = current_image->link.next;
		}
	}

	bool unit_id_is_available_without_an_addon(u16 id)
	{
		return id == UnitId::TerranMason
			|| id == UnitId::TerranTalos
			|| id == UnitId::TerranMaverick
			|| id == UnitId::TerranHarakan
			|| id == UnitId::TerranCyprian
			|| id == UnitId::TerranVulture
			|| id == UnitId::TerranCyclops
			|| id == UnitId::TerranGoliath
			|| id == UnitId::TerranGorgon
			|| id == UnitId::TerranWraith
			|| id == UnitId::TerranTrojan
			|| id == UnitId::TerranValkyrie
			|| id == UnitId::TerranBlackjack
			|| id == UnitId::SharedCohort
			|| id == UnitId::SharedAspirant
			|| id == UnitId::TerranSiren;
	}

	void applyNewHallucination(Unit* unit, u8 playerId, u32 time)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		// Sounds
		if (!unitExt->timers[TimerHallucinating])
		{
			helper::playSoundFromSource(SoundId::Protoss_TEMPLAR_PTeHal00_WAV, unit, 1, 0);
		}
		else
		{
			helper::playSoundFromSource(SoundId::Protoss_TEMPLAR_PTeHal01_WAV, unit, 1, 0);
		}

		// Overlays
		if (unit->subunit != nullptr)
		{
			unit->subunit->sprite->createOverlay(ImageId::HallucinationHit, 0, 0, 0);
		}
		else
		{
			unit->sprite->createOverlay(ImageId::HallucinationHit, 0, 0, 0);
		}
		
		// Value
		unitExt->timers[TimerHallucinating] = time;
	}

	bool canBeEnteredBy(Unit* transport, Unit* unit)
	{
		const u32 Func_CanBeEnteredBy = 0x004E6E00; // AKA CanEnterTransport()
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, transport
			PUSH unit
			CALL Func_CanBeEnteredBy
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	void showErrorMessageWithSfx(u32 playerId, u32 statTxtId, u32 sfxId)
	{
		const u32 Func_ShowErrorMessageWithSfx = 0x0048EE30;
		__asm {
			PUSHAD
			MOV ESI, sfxId
			MOV EDI, statTxtId
			MOV EBX, playerId
			CALL Func_ShowErrorMessageWithSfx
			POPAD
		}
	}

	u16 getTechUseErrorMessage(Unit* target, u8 castingPlayer, u16 techId)
	{
		const u32 Func_GetTechUseErrorMessage = 0x00491E80;
		static u32 techId_;
		static u16 errorMessage;
		techId_ = techId;
		__asm {
			PUSHAD
			PUSH techId_
			MOV BL, castingPlayer
			MOV EAX, target
			CALL Func_GetTechUseErrorMessage
			MOV errorMessage, AX
			POPAD
		}
		return errorMessage;
	}

	u16 getLastQueueSlotType(Unit* a1)
	{
		const u32 Func_getLastQueueSlotType = 0x0047B270;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, a1
			CALL Func_getLastQueueSlotType
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	void setOffsetToParentSpecialOverlay(Image* image)
	{
		const u32 Func_updateImagePositionOffset = 0x004D5A00;
		__asm {
			PUSHAD
			MOV ECX, image
			CALL Func_updateImagePositionOffset
			POPAD
		}
	}

	u32 getGroundHeightAtPos(s32 x, s32 y)
	{
		const u32 Func_GetGroundHeightAtPos = 0x004BD0F0;
		static u32 height;
		__asm {
			PUSHAD
			MOV EAX, y
			MOV ECX, x
			CALL Func_GetGroundHeightAtPos
			MOV height, EAX
			POPAD
		}
		return height;
	}

	void disableDialog(BinDlg* dialog)
	{
		const u32 Func_DisableDialog = 0x00418640;
		__asm
		{
			PUSHAD
			MOV ESI, dialog
			CALL Func_DisableDialog
			POPAD
		}
	}

	void showDialog(BinDlg* dialog)
	{
		const u32 Func_ShowDialog = 0x004186A0;
		__asm
		{
			PUSHAD
			MOV ESI, dialog
			CALL Func_ShowDialog
			POPAD
		}
	}

	void hideDialog(BinDlg* dialog)
	{
		const u32 Func_HideDialog = 0x00418700;
		__asm
		{
			PUSHAD
			MOV ESI, dialog
			CALL Func_HideDialog
			POPAD
		}
	}

	void function_418E00(BinDlg* dialog)
	{
		const u32 Func_Sub418E00 = 0x00418E00;
		__asm
		{
			PUSHAD
			MOV ESI, dialog
			CALL Func_Sub418E00
			POPAD
		}
	}

	void updateDialog(BinDlg* dialog)
	{
		const u32 Func_UpdateDialog = 0x0041C400;
		__asm
		{
			PUSHAD
			MOV EAX, dialog
			CALL Func_UpdateDialog
			POPAD
		}
	}

	void prepareUnitMove(Unit* unit, bool hideUnit)
	{
		const u32 Func_PrepareUnitMoveClearRefs = 0x00493CA0;
		assert(unit);
		static b32 _hideUnit;
		_hideUnit = hideUnit ? 1 : 0;
		__asm
		{
			PUSHAD
			PUSH _hideUnit
			MOV EDI, unit
			CALL Func_PrepareUnitMoveClearRefs
			POPAD
		}
	}

	bool checkUnitCollisionPos(Unit* unit, const Point16* inPos, Point16* outPos, Box16* moveArea, bool hideErrorMsg, u32 someFlag)
	{
		const u32 Func_CheckUnitCollisionPos = 0x0049D3E0;
		assert(unit);
		assert(inPos);
		assert(outPos);
		static u32 result;
		static u32 _hideErrorMsg;
		_hideErrorMsg = hideErrorMsg;
		__asm {
			PUSHAD
			PUSH someFlag;
			PUSH _hideErrorMsg
				PUSH outPos
				PUSH inPos
				PUSH unit
				MOV EAX, moveArea
				CALL Func_CheckUnitCollisionPos
				MOV result, EAX
				POPAD
		}
		return result != 0;
	}

	void setUnitPosition(Unit* unit, u16 x, u16 y)
	{
		const u32 Func_SetUnitPosition = 0x004EB9F0;
		assert(unit);
		__asm {
			PUSHAD
			MOV CX, y
			MOV AX, x
			MOV EDX, unit
			CALL Func_SetUnitPosition
			POPAD
		}
	}

	void refreshUnitAfterMove(Unit* unit)
	{
		const u32 Func_RefreshRevealUnitAfterMove = 0x00494160;
		assert(unit);
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RefreshRevealUnitAfterMove
			POPAD
		}
	}

	void make_to_hold_position(Unit* unit)
	{
		const u32 Func_OrdersHoldPositionSuicidal = 0x004EB5B0;
		__asm
		{
			PUSHAD
			MOV ESI, unit
			CALL Func_OrdersHoldPositionSuicidal
			POPAD
		}
	}

	// Player check added based on how others functions test it. Values above or equal to 8 means all players.
	void MoveScreenToUnit(Unit* unit, u32 playerId) // 004E6020
	{
		const u32 Func_MoveScreen = 0x0049C440;
		if (playerId >= 8 || playerId == *LOCAL_NATION_ID)
		{
			s32 x, y;
			x = (s16)unit->sprite->position.x;
			y = (s16)unit->sprite->position.y;
			if (x < 0)
			{
				x += 31;
			}
			if (y < 0)
			{
				y += 31;
			}
			x = ((x / 32) - 10) * 32;
			y = ((y / 32) - 6) * 32;
			__asm {
				PUSHAD
				MOV ECX, y
				MOV EAX, x
				CALL Func_MoveScreen
				POPAD
			}
		}
	}

	// Player check added based on how others functions test it
	// Values above or equal to 8 means all players
	// x and y are coords as gotten from unit->sprite->position
	void minimapPing(u32 x, u32 y, s32 color, u32 playerId)
	{
		const u32 Func_minimapPing = 0x004A34C0;
		if (playerId >= 8 || playerId == *LOCAL_NATION_ID)
		{
			__asm
			{
				PUSHAD
				PUSH color
				PUSH y
				PUSH x
				CALL Func_minimapPing
				POPAD
			}
		}
	}

	u8 hasOverlay(Unit* unit)
	{
		const u32 Func_hasOverlay = 0x0047B720;
		static u8 result;
		__asm
		{
			PUSHAD
			MOV EAX, unit
			CALL Func_hasOverlay
			MOV result, AL
			POPAD
		}
		return result;
	}

	void hideAndDisableUnit(Unit* unit)
	{
		const u32 Func_unitDeathSomething_0 = 0x004E6340;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitDeathSomething_0
			POPAD
		}
	}

	bool weaponPiercesTargets(u16 weaponId)
	{
		switch (weaponId)
		{
			case WeaponId::KalkaliskViscousAcid:
			case WeaponId::PenumbraErebolShockCannon:
			case WeaponId::ClaymoreHarpoonRailgun:
			{
				return true;
			}
			default:
			{
				return false;
			}
		}
	}

	void dirgeFuse(Unit* unit, UnitExt* unitExt)
	{
		unit->sprite->createTopOverlay(ImageId::ArcliteShockCannon_Damageless);
		const u32 radius = 2 * 32;
		scbw::UnitFinder(
			unit->getX() - radius,
			unit->getY() - radius,
			unit->getX() + radius,
			unit->getY() + radius
		).forEachInMatch(cosmonarchyFinderMatch(unit, Flags::IncludingSelf, {}),
			[&unitExt](Unit* target)
		{
			target->damageWithCustom(unitExt->values[ValueDirgeStacks] * MadrigalDirgeDamage * 256, DamageType::Explosive, unitExt->units[UnitDirgeSource], unitExt->values[ValueDirgeSourcePlayer], 0, 1, false);
		});
		unitExt->values[ValueDirgeStacks] = 0;
	}

	void updateSignifyStatus(Unit* unit, UnitExt* unitExt)
	{
		unit->updateSpeed();

		const u32 cowardiceId = ImageId::Cowardice_Small + scbw::getOverlayOffset(unit->id);
		const u32 maliceId = ImageId::Malice_Small + scbw::getOverlayOffset(unit->id);

		if (unitExt->values[ValueSignifyStacks] == 0)
		{
			unit->removeOverlay(ImageId::Cowardice_Small, ImageId::Cowardice_Large);
			unit->removeOverlay(ImageId::Malice_Small, ImageId::Malice_Large);
		}
		else if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Building)
			|| units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding
			|| unit->hasWeapon())
		{
			if (unitExt->values[ValueSignifyStacks] < 5 && !unit->getOverlay(cowardiceId))
			{
				unit->sprite->createTopOverlay(cowardiceId);
			}
			else if (unitExt->values[ValueSignifyStacks] >= 5 && !unit->getOverlay(maliceId))
			{
				unit->removeOverlay(ImageId::Cowardice_Small, ImageId::Cowardice_Large);
				unit->sprite->createTopOverlay(maliceId);
			}
		}
	}

	bool isUnitIsolated(Unit* unit)
	{
		u32 allyCount = 0;
		u32 enemyCount = 0;

		auto match = cosmonarchyFinderMatch(unit, Flags::NotSelf, {});
		const u32 radius = 5 * 32;
		scbw::UnitFinder(
			unit->getX() - radius,
			unit->getY() - radius,
			unit->getX() + radius,
			unit->getY() + radius
		).forEachInMatch(match, [&](Unit* target)
		{
			if (scbw::isAlliedTo(target->playerId, unit->playerId))
			{
				allyCount++;
			}
			else
			{
				enemyCount++;
			}
		});

		return allyCount < enemyCount;
	}

	bool unitIdIsZergSupply(u16 unitId)
	{
		return (unitId == UnitId::ZergOvileth
			|| unitId == UnitId::ZergIroleth
			|| unitId == UnitId::ZergOthstoleth
			|| unitId == UnitId::ZergAlaszileth);
	}

	void mind_control(Unit* source, Unit* target)
	{
		if (!source || !target || source->playerId == target->playerId)
		{
			return;
		}

		UnitExt* sourceExt = globals.get_unit_ext(source);
		sourceExt->unitVectors[UnitVectorMindControlling].push_back(target);

		UnitExt* targetExt = globals.get_unit_ext(target);
		targetExt->units[UnitMindController] = source;
		if (!targetExt->values[ValueMindControlOriginalPlayer])
		{
			targetExt->values[ValueMindControlOriginalPlayer] = target->playerId + 1;
		}

		// Recurse
		for (u32 i = 0; i < 8; ++i) {
			Unit* unit = target->getLoadedUnit(i);
			mind_control(source, unit);
		}
		for (Unit* unit : targetExt->unitVectors[UnitVectorMindControlling])
		{
			mind_control(source, unit);
		}
		targetExt->unitVectors[UnitVectorMindControlling].clear();

		// Demiurge - apply behavior
		if (source->id == UnitId::ProtossDemiurge) {
			auto& vec = sourceExt->unitVectors[UnitVectorWorshipersDemiurge];
			if (std::find(vec.begin(), vec.end(), target) == vec.end())
			{
				vec.push_back(target);
			}

			targetExt->values[ValueWorshipStack] = 1;
		}

		// Mind Tyrant - apply behavior
		if (source->id == UnitId::ProtossMindTyrant) {
			targetExt->units[UnitMindTyrantSource] = source;
			targetExt->timers[TimerMindTyrantRecede] = 5 * 24;
		}

		// Obtain control
		target->giveTo(source->playerId);

		// Audio Visuals
		if (!(source->sprite->flags & SpriteFlags::Hidden)) {
			u32 offset = scbw::getOverlayOffset(target->id);
			if (source->id == UnitId::ProtossDemiurge)
			{
				target->sprite->createTopOverlay(ImageId::Worship_Small + offset, 0, 0, 0);
				helper::playSoundFromSource(SoundId::DemiurgeWorship01, target, 1, 0);
			}
			else if (source->id == UnitId::ProtossMindTyrant)
			{
				target->sprite->createTopOverlay(ImageId::MindControlHit_Small + offset, 0, 0, 0);
			}
		}
	}

	void recede_mind_control(Unit* target)
	{
		if (!target)
		{
			return;
		}

		UnitExt* targetExt = globals.get_unit_ext(target);
		Unit* mindController = targetExt->units[UnitMindController];
		targetExt->units[UnitMindController] = 0;

		if (mindController)
		{
			UnitExt* mindControllerExt = globals.get_unit_ext(mindController);
			auto& vec = mindControllerExt->unitVectors[UnitVectorMindControlling];
			vec.erase(std::remove(vec.begin(), vec.end(), target), vec.end());
		}

		target->giveTo(targetExt->values[ValueMindControlOriginalPlayer] - 1);
		targetExt->values[ValueMindControlOriginalPlayer] = 0;

		targetExt->values[ValueWorshipStack] = 0;
		targetExt->timers[TimerGodsMadeFlesh] = 0;
		targetExt->units[UnitMindTyrantSource] = 0;
	}

	bool isZergProduction(u16 unitId)
	{
		return unitId == UnitId::ZergHatchery
			|| unitId == UnitId::ZergLair
			|| unitId == UnitId::ZergHive
			|| unitId == UnitId::ZergSire
			|| unitId == UnitId::ZergLarvalColony;
	}
	
	bool unitIdIsTownCenter(u16 unitId)
	{
		return unitId == UnitId::TerranCommandCenter
			|| unitId == UnitId::ProtossNexus
			|| unitId == UnitId::ZergHatchery
			|| unitId == UnitId::ZergLair
			|| unitId == UnitId::ZergHive
			|| unitId == UnitId::ZergSire;
	}

	bool unitIdIsShared(u16 unitId)
	{
		return unitId == UnitId::SharedAnticthon
			|| unitId == UnitId::SharedAspirant
			|| unitId == UnitId::SharedAxitrilisk
			|| unitId == UnitId::SharedCharlatan
			|| unitId == UnitId::SharedCohort
			|| unitId == UnitId::SharedEmpress
			|| unitId == UnitId::SharedLuminary
			|| unitId == UnitId::SharedStriga
			|| unitId == UnitId::SharedTarasque;
	}

	bool transportIsFull(Unit* transport)
	{
		return transport->loadedUnit[0].index
			&& transport->loadedUnit[1].index
			&& transport->loadedUnit[2].index
			&& transport->loadedUnit[3].index
			&& transport->loadedUnit[4].index
			&& transport->loadedUnit[5].index
			&& transport->loadedUnit[6].index
			&& transport->loadedUnit[7].index;
	}

	bool transportIsEmpty(Unit* transport)
	{
		return (!transport->loadedUnit[0].index
			&& !transport->loadedUnit[1].index
			&& !transport->loadedUnit[2].index
			&& !transport->loadedUnit[3].index
			&& !transport->loadedUnit[4].index
			&& !transport->loadedUnit[5].index
			&& !transport->loadedUnit[6].index
			&& !transport->loadedUnit[7].index);
	}

	bool unitIdUsesXlargeOverlays(u16 unitId)
	{
		return unitId == UnitId::TerranPenumbra
			|| unitId == UnitId::TerranPhobos
			|| unitId == UnitId::ProtossStarSovereign;
	}

	bool unitIdIsResource(u16 unit_id, bool count_gas_structures)
	{
		if (count_gas_structures)
		{
			return unit_id == UnitId::ResourceMineralField
				|| unit_id == UnitId::ResourceMineralFieldType2
				|| unit_id == UnitId::ResourceMineralFieldType3
				|| unit_id == UnitId::ResourceVespeneGeyser
				|| unit_id == UnitId::TerranReservoir
				|| unit_id == UnitId::ProtossAquifer
				|| unit_id == UnitId::ZergExcisant;
		}
		else
		{
			return unit_id == UnitId::ResourceMineralField
				|| unit_id == UnitId::ResourceMineralFieldType2
				|| unit_id == UnitId::ResourceMineralFieldType3
				|| unit_id == UnitId::ResourceVespeneGeyser;
		}
	}

	void set_cmbw_ai_preference(u16 unit_id, u32 player) {
		unit_deaths[unit_id].player[player] = 1;
	}

	Box32 unit_area(Unit* unit) {
		Box32 area = {};
		area.w = units_dat::BuildingDimensions[unit->id].x;
		area.h = units_dat::BuildingDimensions[unit->id].y;
		area.x = unit->position.x - area.w / 2;
		area.y = unit->position.y - area.h / 2;
		return area;
	}

	void clear_creep(Box32 area) {
		for (u32 x = 0; x < area.w / 32; ++x) {
			for (u32 y = 0; y < area.h / 32; ++y) {
				helper::removeCreepAtUnit(
					area.x + x * 32,
					area.y + y * 32,
					UnitId::ZergEgg);
			}
		}
	}
	
	u32 get_closest_resarea(Point16 position) {
		u32 min_distance = UINT32_MAX;
		u32 best_resarea = 0;
		for (u32 i = 0; i < ResAreaLimit; ++i) {
			ResourceArea* resarea = &resource_areas->areas[i];
			u32 distance = scbw::getDistanceFast(position, resarea->pos);
			if (distance < min_distance) {
				min_distance = distance;
				best_resarea = i;
			}
		}
		return best_resarea;
	}
	
	AiTown* get_closest_town(u32 player, Point16 position) {
		u32 min_distance = UINT32_MAX;
		AiTown* best_town = 0;
		for (AiTown* town = AiActiveTownArray[player].first; town; town = town->link.prev) {
			if (!town->inited) {
				continue;
			}
			u32 distance = scbw::getDistanceFast(position, town->position);
			if (distance < min_distance) {
				min_distance = distance;
				best_town = town;
			}
		}
		return best_town;
	}

	u32 get_region_id(Point16 position) {
		return helper::get_region_id(position.x, position.y);
	}

	AiRegion* get_ai_region(u32 player, u32 index) {
		return &AiRegionCaptains[player][index];
	}

	Region* get_region(u32 index) {
		return &(*SAI_Paths)->regions[index];
	}

	void ai_defend(u32 player, u32 region_id, u32 priority, bool recurse) {
		AiRegion* ai_region = scbw::get_ai_region(player, region_id);
		switch (ai_region->state) {
			case RegionState::Nothing:
			case RegionState::EnemySighted:
			case RegionState::TownRegionLow:
			case RegionState::TownRegionHigh:
			case RegionState::Defending:
			case RegionState::ChaseAway: {
				helper::AI_AssignMilitary(RegionState::Defending, ai_region);
				if (ai_region->defensePriorityAdd < priority) {
					ai_region->defensePriorityAdd = priority;
				}
				ai_region->defendTimeout = 0;
			} break;
		}
		if (recurse) {
			Region* region = scbw::get_region(region_id);
			for (u32 i = 0; i < region->neighborCount; ++i) {
				scbw::ai_defend(player, region->neighbors[i], priority, false);
			}
		}
	}

	void ask_for_ai_help(Unit* victim, Unit* attacker) {
		if (!victim || !attacker) {
			return;
		}

		UnitArray allied_structures = UnitSearchRadial(victim->position, 40 * 32);
		UnitArrayFilter(&allied_structures, victim, FilterIncludingSelf | FilterAlly | FilterBuilding);
		if (allied_structures.size) {
			for (u32 i = 0; i < 8; ++i) {
				if (playerTable[i].type == PlayerType::Computer && victim->isVisibleTo(i) &&
					!scbw::isAlliedTo(i, attacker->playerId) && scbw::isAlliedTo(i, victim->playerId)) {
					scbw::ai_defend(i, scbw::get_region_id(attacker->position), 120, true);
				}
			}
		}
	}

	bool unit_id_is_capped_gas_structure(u16 unit_id) {
		return unit_id == UnitId::TerranReservoir || unit_id == UnitId::ProtossAquifer || unit_id == UnitId::ZergExcisant;
	}

	bool unit_id_is_gas_structure(u16 unit_id) {
		return unit_id_is_capped_gas_structure(unit_id) || unit_id == UnitId::ResourceVespeneGeyser;
	}

	bool unit_id_is_smaller_building(u16 unit_id) {
		return units_dat::BaseProperty[unit_id] & UnitProperty::Addon
			|| unit_id == UnitId::TerranSupplyDepot
			|| unit_id == UnitId::TerranWatchdog
			|| unit_id == UnitId::TerranBunker
			|| unit_id == UnitId::ProtossPylon
			|| unit_id == UnitId::ProtossPhotonCannon
			|| unit_id == UnitId::ProtossShieldBattery
			|| unit_id == UnitId::ProtossPennant
			|| unit_id == UnitId::ZergCreepColony
			|| unit_id == UnitId::ZergSunkenColony
			|| unit_id == UnitId::ZergSporeColony
			|| unit_id == UnitId::ZergLarvalColony
			|| unit_id == UnitId::ZergNydusCanal;
	}

	inline UnitArray get_writhing_web_sovroleths(Unit* unit) {
		UnitArray sovroleths = UnitSearchRadial(unit->position, 8 * 32);
		UnitArrayFilter(&sovroleths, unit, FilterAlly, {UnitId::ZergSovroleth}); 
		return sovroleths;
	}

	void spawn_custom_creep(Unit* unit) {
		u32 w = units_dat::BuildingDimensions[unit->id].x / 32;
		u32 h = units_dat::BuildingDimensions[unit->id].y / 32;
		u32 x = unit->position.x / 32 - w / 2;
		u32 y = unit->position.y / 32 - h / 2;

		helper::clear_tile_flags_from_area(0x8000000, x, y, w, h);
		helper::set_creep(x, y, w, h, false);
		helper::set_map_tile_flags(0x8000000, x, y, w, h);
	}

	static void stop_custom_creep_disappearing(Unit* unit)
	{
		Box32 rect = {};
		bool spreads_creep = helper::GetCreepSpreadArea(UnitId::ZergHive, unit->position.x, unit->position.y, true, &rect);
		if (rect.top > rect.bottom) {
			return;
		}

		for (u32 y_tile = rect.top; y_tile <= rect.bottom; ++y_tile) {
			for (u32 x_tile = rect.left; x_tile <= rect.right; ++x_tile) {
				if ((*activeTileHandle)[x_tile + y_tile * mapTileSize->width] & 0x10000000) {
					u32 x = x_tile * 32 - unit->position.x + 16;
					u32 y = y_tile * 32 - unit->position.y + 16;
					if (!spreads_creep || 100 * x * x + (y * y << 8) <= 10240000) {
						helper::stop_creep_disappearing(x_tile, y_tile);
					}
				}
			}
		}
	}

	void spread_custom_creep(Unit* unit) {
		scbw::stop_custom_creep_disappearing(unit);
		helper::spreadCreep(UnitId::ZergHive, unit->position.x, unit->position.y);
	}

	void update_custom_creep_disappearance(Unit* unit) {
		UnitExt* unitExt = globals.get_unit_ext(unit);
		if (unitExt->values[ValueBlissfulEmbraceOriginalPlayer]) {
			helper::update_creep_disappearance_unit_id(UnitId::ZergHive, unit->position.x, unit->position.y, true);
		}
	}

	void blissful_embrace(Unit* unit, u32 player) {
		// Sanity Check. Spawning creep wouldn't work properly on units. Spawning creep on units or addons could corrupt/crash the game. Addons don't transfer properly anyways
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Building) || units_dat::BaseProperty[unit->id] & UnitProperty::Addon) {
			return;
		}

		UnitExt* targetExt = globals.get_unit_ext(unit);

		// Obtain control
		if (!targetExt->values[ValueBlissfulEmbraceOriginalPlayer]) {
			targetExt->values[ValueBlissfulEmbraceOriginalPlayer] = unit->playerId + 1;
			targetExt->values[ValueBlissfulEmbraceOriginalHealth] = unit->hitPoints;
		}
		unit->giveTo(player);

		// Spread initial creep
		scbw::spawn_custom_creep(unit);

		// Remove connected larva
		UnitArray larvae = UnitSearchRadial(unit->position, 4 * 32);
		UnitArrayFilter(&larvae, unit, 0, {UnitId::ZergLarva});
		for (u32 i = 0; i < larvae.size; ++i) {
			Unit* larva = larvae.units[i];
			if (larva->connectedUnit == unit) {
				larva->connectedUnit = 0;
			}
		}

		// Spawn infested overlay
		u32 infested_image_id = ImageId::InfestedOverlay_Large;
		if (scbw::unit_id_is_smaller_building(unit->id)) {
			infested_image_id = ImageId::InfestedOverlay_Small;
		}
		if (!unit->getOverlay(infested_image_id)) {
			unit->sprite->createTopOverlay(infested_image_id);
		}

		// Spawn effect overlay  TODO: Make this a sprite
		u32 effect_image_id = ImageId::ZergAirDeathExplosion_Small;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay) {
			effect_image_id = ImageId::ZergBuildingExplosion;
		} else if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay) {
			effect_image_id = ImageId::ZergAirDeathExplosion_Large;
		}
		unit->sprite->createOverlay(effect_image_id);

		// Play Sound
		helper::playSoundFromSource(SoundId::Zerg_Bldg_ZBldgDth_WAV, unit, true, 0);
	}

	b32 is_tech_unitid(u32 unit_id) {
		if (units_dat::BaseProperty[unit_id] & UnitProperty::Addon) {
			switch (unit_id) {
				case UnitId::TerranComsatStation:
				case UnitId::TerranNuclearSilo:
				case UnitId::TerranQuarry:
					return false;
			}
			return true;
		}

		switch (unit_id) {
			// Terran
			case UnitId::TerranEngineeringBay:
			case UnitId::TerranScienceFacility:

			// Protoss
			case UnitId::ProtossCenotaph:
			case UnitId::ProtossArchonArchives:
			case UnitId::ProtossAutomatonRegister:
			case UnitId::ProtossSyntheticSynod:
			case UnitId::ProtossAstralOmen:
			case UnitId::ProtossCelestialTribune:
			case UnitId::ProtossFleetBeacon:
			case UnitId::ProtossCosmicAltar:
			case UnitId::ProtossSanctumOfSorrow:
			case UnitId::ProtossWaywardLure:
			case UnitId::ProtossMonumentOfSin:

			// Zerg
			case UnitId::ZergIrolIris:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergAlaszilArbor:
			case UnitId::ZergQuazrokPool:
			case UnitId::ZergLiivralPond:
			case UnitId::ZergHydrokDen:
			case UnitId::ZergGorgralSwamp:
			case UnitId::ZergMuthrokSpire:
			case UnitId::ZergVithrathHaunt:
			case UnitId::ZergOrbothOmlia:
			case UnitId::ZergZorkizShroud:
			case UnitId::ZergUltrokCavern:
			case UnitId::ZergMatravilNest:
			case UnitId::ZergAlmakisAntre:
			case UnitId::ZergSovthrathMound:
			case UnitId::ZergGeszkathGrotto:
			case UnitId::ZergAlkajChasm:
				return true;
		}
		return false;
	}

	AiScript* run_ai(u32 player, Box32 area, s32 name) {
		s32 is_bw = 0;
		s32 block = helper::parseAIScriptName(name, &is_bw);
		if (block) {
			AiScript* ai_script = helper::AI_RunAIScript(&area, player, block, is_bw);
			if (ai_script) {
				Point16 center = {};
				center.x = area.x + area.w / 2;
				center.y = area.y + area.h / 2;

				ai_script->flags &= 0x3;
				ai_script->flags |= scbw::get_closest_resarea(center) << 3;
			}
			return ai_script;
		} else {
			scbw::printf("scenario.cpp, run_ai: Invalid AI block!");
		}
	}

	AiScript* run_ai(u32 player, Point16 pos, s32 name) {
		Box32 area = {};
		area.left = pos.x - 2 * 32;
		area.right = pos.x + 2 * 32;
		area.top = pos.y - 1.5 * 32;
		area.bottom = pos.y + 1.5 * 32;
		return run_ai(player, area, name);
	}
}