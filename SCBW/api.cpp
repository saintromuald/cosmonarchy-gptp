#include "api.h"
#include <SCBW/UnitFinder.h>
#include <algorithm>
#include <cassert>
#include <utility>
#include <hooks/weapons/weapon_damage.h>
#include <globals.h>
#include "hooks/target_related.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace scbw {

	const uint32 Func_ApplySpores = 0x004F4480;
	void ApplySpores(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EBX, unit
			CALL Func_ApplySpores
			POPAD
		}
	}

	const uint32 Func_SetThingyVisibilityFlags = 0x004878F0;
	bool setThingyVisibilityFlags(CThingy* thingy) {

		static bool32 bPreResult;

		__asm {
			PUSHAD
			MOV ESI, thingy
			CALL Func_SetThingyVisibilityFlags
			MOV bPreResult, EAX
			POPAD
		}

		return (bPreResult != 0);

	}
	;

	const uint32 Helper_CreateBullet = 0x0048C260;
	void createBullet(uint8 weaponId, CUnit* source, int16 x, int16 y, uint8 attackingPlayer, uint8 direction) {
		static uint32 attackingPlayer_;
		static uint32 direction_;
		static int32 x_;
		static int32 y_;

		attackingPlayer_ = attackingPlayer;
		direction_ = direction;
		x_ = x; y_ = y;
		__asm {
			PUSHAD
			PUSH direction_
			PUSH attackingPlayer_
			PUSH y_
			PUSH x_
			MOV EAX, source
			MOVZX ECX, weaponId
			CALL Helper_CreateBullet
			POPAD
		}
	}

	const uint32 Helper_WeaponBulletShot = 0x00479AE0;
	void weaponBulletShot(CUnit* target, CBullet* bullet, uint32 dmgDivisor) {
		__asm {
			PUSHAD
			PUSH dmgDivisor
			MOV EAX, target
			MOV EDX, bullet
			CALL Helper_WeaponBulletShot
			POPAD
		}
	}

	CBullet* createBulletRet(uint8 weaponId, CUnit* source, int16 x, int16 y, uint8 attackingPlayer, uint8 direction) {

		static uint32 attackingPlayer_;
		static uint32 direction_;
		static int32 x_;
		static int32 y_;

		attackingPlayer_ = attackingPlayer;
		direction_ = direction;
		x_ = x; y_ = y;
		static CBullet* bullet;
		__asm {
			PUSHAD
			PUSH direction_
			PUSH attackingPlayer_
			PUSH y_
			PUSH x_
			MOV EAX, source
			MOVZX ECX, weaponId
			CALL Helper_CreateBullet
			MOV bullet, EAX
			POPAD
		}
		return bullet;

	}
	void artificalBulletFrom(uint8 weaponId, uint8 playerId, CUnit* source, CUnit* target, Point16 target_pt) {
		auto target_unit = source->orderTarget.unit;
		auto target_pos = source->orderTarget.pt;
		source->orderTarget.unit = target;
		source->orderTarget.pt = target_pt;
		Point16 pick;
		if (target != nullptr && target_unit != nullptr) {
			source->orderTarget.pt = target_unit->sprite->position;
		} else {
			source->orderTarget.pt = target_pt;
		}
		createBullet(weaponId, source, source->sprite->position.x, source->sprite->position.y, playerId,
			getAngle(source->position.x, source->position.y, source->orderTarget.pt.x, source->orderTarget.pt.y));
		source->orderTarget.unit = target_unit;
		source->orderTarget.pt = target_pos;
	}
	void artificalBulletFrom(uint8 weaponId, uint8 playerId, CUnit* source, CUnit* target, uint16 x, uint16 y) {
		Point16 tgt;
		tgt.x = x;
		tgt.y = y;
		artificalBulletFrom(weaponId, playerId, source, target, tgt);
	}

	const uint32 Func_Sub4EBAE0 = 0x004EBAE0;
	void function_004EBAE0(CUnit* unit, int32 x, int32 y) {
		__asm {
			PUSHAD
			MOV EDX, unit
			MOV EAX, x
			MOV ECX, y
			CALL Func_Sub4EBAE0
			POPAD
		}
	}
	;

	bool teleport(CUnit* u, Point16 target, uint16 spriteAnimation, bool preserveOrder) {
		Point16 previous_pos, new_pos;
		bool result = false;
		previous_pos.x = u->sprite->position.x;
		previous_pos.y = u->sprite->position.y;

		//identical code to setUnitPosition() but for some reason, is
		//using another function to do exactly the same thing
		function_004EBAE0(u, target.x, target.y);

		if (!scbw::checkUnitCollisionPos(u, &(u->sprite->position), &new_pos, nullptr, false, 0))
			//collision detected, cancel the movement
			scbw::setUnitPosition(u, previous_pos.x, previous_pos.y);
		else {
			result = true;
			CThingy* recall_effect_sprite;

			//finish the teleporting (and cut the recalled unit from a possible unit targeted by an order)
			scbw::prepareUnitMove(u, false);
			if (!preserveOrder) {
				u->orderTarget.unit = nullptr;
			}
			scbw::setUnitPosition(u, new_pos.x, new_pos.y);
			scbw::refreshUnitAfterMove(u);

			//stop some currently active orders
			if (u->id != UnitId::ZergCocoon && !preserveOrder)
				u->orderComputerCL(units_dat::ReturnToIdleOrder[u->id]);

			//create the Recall animation over the unit
			if (spriteAnimation) {
				recall_effect_sprite = createThingy(spriteAnimation, new_pos.x, new_pos.y, 0);
				if (recall_effect_sprite != nullptr) {
					recall_effect_sprite->sprite->elevationLevel = u->sprite->elevationLevel + 1;
					scbw::setThingyVisibilityFlags(recall_effect_sprite);
				}

			}

			//disconnect a nuke from a ghost using it
			if (
				u->id == UnitId::TerranEidolon ||
				u->id == UnitId::Hero_SarahKerrigan ||
				u->id == UnitId::Hero_AlexeiStukov ||
				u->id == UnitId::Hero_SamirDuran ||
				u->id == UnitId::Hero_InfestedDuran
				) {
				if (
					u->connectedUnit != nullptr &&
					u->connectedUnit->id == UnitId::TerranNuclearMissile
					) {
					(u->connectedUnit)->connectedUnit = nullptr;
					u->connectedUnit = nullptr;
				}
			}
		}
		return result;
	}
	const uint32 Func_CreateFlingy = 0x00496460;
	CFlingy* createFlingy(uint32 flingyId, uint32 playerId, uint32 x, uint32 y) {
		static CFlingy* flingy;
		__asm {
			PUSHAD
			MOV EAX, playerId
			MOV ECX, x
			MOV EDX, y
			PUSH flingyId
			CALL Func_CreateFlingy
			MOV flingy, EAX
			POPAD
		}
		return flingy;
	}


	const uint32 Func_CreateThingy = 0x00488210;
	CThingy* createThingy(uint32 spriteId, int16 x, int16 y, uint32 playerId) {

		static CThingy* thingy;
		int32 x_ = x;
		__asm {
			PUSHAD
			PUSH playerId
			MOVSX EDI, y
			PUSH x_
			PUSH spriteId
			CALL Func_CreateThingy
			MOV thingy, EAX
			POPAD
		}
		return thingy;
	}

	CThingy* createAndInitThingy(uint32 spriteId, int16 x, int16 y, uint32 playerId, int elevationLevel) {
		CThingy* thingy = createThingy(spriteId, x, y, playerId);
		if (thingy != nullptr) {
			thingy->sprite->elevationLevel = elevationLevel;
			scbw::setThingyVisibilityFlags(thingy);
		}
		return thingy;
	}

	;

	const uint32 Func_SendCommand = 0x00485BD0;
	void SendCommand(uint8* params, uint32 param_length) {

		__asm {
			PUSHAD
			MOV EDX, param_length
			MOV ECX, params
			CALL Func_SendCommand
			POPAD
		}

	}
	//-------- Output functions --------//

	const uint32 Func_PlaySound = 0x0048ED50;
	void playSound(uint32 sfxId, CUnit* sourceUnit) {

		__asm {
			PUSHAD
			PUSH 0
			PUSH 1
			MOV ESI, sourceUnit
			MOV EBX, sfxId
			CALL Func_PlaySound
			POPAD
		}
	}

	void printString(std::string text, uint32 color) {
		print(text.c_str(), color);
	}

	void printf(const char* format, ...) {
		char buf[256];

		va_list _ArgList;
		__crt_va_start(_ArgList, format);
		vsnprintf(buf, sizeof(buf), format, _ArgList);
		__crt_va_end(_ArgList);

		print(buf);
	}

	void printfColor(const char* format, uint32 color, ...) {
		char buf[256];

		va_list _ArgList;
		__crt_va_start(_ArgList, format);
		vsnprintf(buf, sizeof(buf), format, _ArgList);
		__crt_va_end(_ArgList);

		print(buf, color);
	}

	const uint32 Func_PrintText = 0x0048CD30;
	void print(const char* text, uint32 color) {

		if (!text)
			return;

		DWORD gtc = GetTickCount() + 7000;

		__asm {
			PUSHAD
			PUSH 0;//unknown
			MOV eax, text
				PUSH gtc
				PUSH color
				CALL Func_PrintText
				POPAD
		}

	}

	const uint32 Func_ShowErrorMessageWithSfx = 0x0048EE30;
	void showErrorMessageWithSfx(uint32 playerId, uint32 statTxtId, uint32 sfxId) {

		__asm {
			PUSHAD
			MOV ESI, sfxId
			MOV EDI, statTxtId
			MOV EBX, playerId
			CALL Func_ShowErrorMessageWithSfx
			POPAD
		}

	}

	//-------- Unit checks --------//

	const uint32 Func_CanBeEnteredBy = 0x004E6E00; //AKA CanEnterTransport()
	bool canBeEnteredBy(CUnit* transport, CUnit* unit) {

		static uint32 result;

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

	//Identical to function @ 0x00475CE0
	bool canWeaponTargetUnit(uint8 weaponId, CUnit* target, CUnit* attacker) {

		if (weaponId >= WEAPON_TYPE_COUNT)
			return false;

		if (!target)
			return weapons_dat::TargetFlags[weaponId].terrain;


		if (target->status & UnitStatus::Invincible)
			return false;

		const TargetFlag tf = weapons_dat::TargetFlags[weaponId];
		const uint32 targetProps = units_dat::BaseProperty[target->id];

		if ((target->status & UnitStatus::InAir) ? !tf.air : !tf.ground)
			return false;

		if (tf.mechanical && !(targetProps & UnitProperty::Mechanical))
			return false;

		if (tf.organic && !(targetProps & UnitProperty::Organic))
			return false;

		if (tf.nonBuilding && (targetProps & UnitProperty::Building))
			return false;

		if (tf.nonRobotic && (targetProps & UnitProperty::RoboticUnit))
			return false;

		if (tf.orgOrMech && !(targetProps & (UnitProperty::Organic | UnitProperty::Mechanical)))
			return false;

		if (tf.playerOwned && target->playerId != attacker->playerId)
			return false;

		return true;

	}

	bool isUnderDarkSwarm(CUnit* unit) {
		return getDarkSwarm(unit) != nullptr;
	}

	bool IsDarkswarm(CUnit* unit) {
		return unit->id == UnitId::Spell_DarkSwarm;
	}

	CUnit* getDarkSwarm(CUnit* unit) {
		static UnitFinder darkSwarmFinder;
		darkSwarmFinder.search(unit->getLeft(), unit->getTop(), unit->getRight(), unit->getBottom());

		CUnit* darkSwarm = darkSwarmFinder.getFirst(IsDarkswarm);
		return darkSwarm;
	}

	CUnit* getDarkSwarm(uint16 x, uint16 y, uint16 radius) {
		UnitFinder darkSwarmFinder;
		darkSwarmFinder.search(x - radius, y - radius, x + radius, y + radius);
		CUnit* darkSwarm = darkSwarmFinder.getFirst(IsDarkswarm);
		return darkSwarm;
	}

	uint16 getTechUseErrorMessage(CUnit* target, uint8 castingPlayer, uint16 techId) {

		static uint32 techId_;
		static uint16 errorMessage;

		const uint32 Func_GetTechUseErrorMessage = 0x00491E80;

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

	//-------- Graphics and geometry --------//

	//Improved code from BWAPI's include/BWAPI/Position.h: getApproxDistance()
	//Logically same as function @ 0x0040C360
	//x1 = ECX;x2 = EDX;y1 = [EBP+08];y2 = [EBP+0C];
	uint32 getDistanceFast(int32 x1, int32 y1, int32 x2, int32 y2) {

		int dMax = abs(x1 - x2), dMin = abs(y1 - y2);

		if (dMax < dMin)
			std::swap(dMax, dMin);

		if (dMin <= (dMax >> 2))
			return dMax;

		return (dMin * 3 >> 3) + (dMin * 3 >> 8) + dMax - (dMax >> 4) - (dMax >> 6);

	}

	uint32 getDistanceSquare(int32 x1, int32 y1, int32 x2, int32 y2) {
		return std::max(abs(x1 - x2), abs(y1 - y2));
	}

	//Identical to function @ 0x00494C10
	int arctangent(int slope) {

		const unsigned int tangentTable[] = {
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

		if (slope < 0) {
			isNegative = true;
			slope = -slope;
		}

		int min = 0, max = 64, angle = 32;

		do {

			if ((unsigned int)slope <= tangentTable[angle])
				max = angle;
			else
				min = angle + 1;

			angle = (min + max) / 2;

		} while (min != max);

		return (isNegative ? -angle : angle);
	}

	//Identical to function @ 0x00495300
				//    EDX		[EBP+08]	ECX			EAX
	int32 getAngle(int32 xStart, int32 yStart, int32 xEnd, int32 yEnd) {

		int32 dx = xEnd - xStart, dy = yEnd - yStart;

		if (dx == 0)
			return dy > 0 ? 128 : 0;

		int32 angle = arctangent((dy << 8) / dx);

		if (dx < 0) {
			angle += 192;
			return angle == 256 ? 0 : angle;
		} else
			return angle + 64;

	}

	Point32 rotate_point(Point32 anchor, Point32 point, double angle_degrees) {
		double x1 = point.x - anchor.x;
		double y1 = point.y - anchor.y;
		auto radians = angle_degrees / 57.296;
		double x2 = x1 * cos(radians) - y1 * sin(radians);
		double y2 = x1 * sin(radians) + y1 * cos(radians);
		Point32 product;
		product.x = x2 + anchor.x;
		product.y = y2 + anchor.y;
		return product;
	}


	double bw_angle_to_normal(int32 bw_angle) {
		double angle = (double)bw_angle * 1.40625;
		return angle;
	}
	int32 getPolarX(int32 distance, uint8 angle) {
		return distance * angleDistance[angle].x >> 8;
	}

	int32 getPolarY(int32 distance, uint8 angle) {
		return distance * angleDistance[angle].y >> 8;
	}

	//-------- Player information --------//

	int32 getSupplyRemaining(uint8 playerId, uint8 raceId) {
		assert(raceId <= 2);
		assert(playerId < PLAYER_COUNT);

		int32 supplyProvided;

		if (isCheatEnabled(CheatFlags::FoodForThought))
			supplyProvided = raceSupply[raceId].max[playerId];
		else
			supplyProvided = std::min(raceSupply[raceId].max[playerId], raceSupply[raceId].provided[playerId]);

		return supplyProvided - raceSupply[raceId].used[playerId];

	}

	bool hasTechResearched(uint8 playerId, uint16 techId) {
		assert(playerId < PLAYER_COUNT);
		assert(techId < TechId::None);
		if (techId < TechId::Restoration)
			return TechSc->isResearched[playerId][techId] != 0;
		else
			return TechBw->isResearched[playerId][techId - TechId::Restoration] != 0;

	}

	bool hasTechAvailable(uint8 playerId, uint16 techId) {
		assert(playerId < PLAYER_COUNT);
		assert(techId < TechId::None);
		if (techId < TechId::Restoration)
			return TechSc->isEnabled[playerId][techId] != 0;
		else
			return TechBw->isEnabled[playerId][techId - TechId::Restoration] != 0;
	}


	void setTechResearchState(uint8 playerId, uint16 techId, bool isResearched) {
		assert(playerId < PLAYER_COUNT);
		assert(techId < TechId::None);

		if (techId < TechId::Restoration)
			TechSc->isResearched[playerId][techId] = isResearched;
		else
			TechBw->isResearched[playerId][techId - TechId::Restoration] = isResearched;

	}


	const uint32 Func_DoWeaponDamage = 0x00479930;
	void sendGPTP_aise_cmd(CUnit* unit1, CUnit* unit2, uint32 command, uint32 v1, uint32 v2, uint32 v3) {
		_asm {
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

	uint32 getFlingyData(uint32 entry, uint32 index) {
		return get_aise_value(nullptr, nullptr, AiseId::GetFlingyData, entry, index);
	}
	uint32 getSpriteData(uint32 entry, uint32 index) {
		return get_aise_value(nullptr, nullptr, AiseId::GetSpriteData, entry, index);
	}

	uint32 getImageData(uint32 entry, uint32 index) {
		return get_aise_value(nullptr, nullptr, AiseId::GetImageData, entry, index);
	}
	uint32 getUnitData(uint32 entry, uint32 index) {
		return get_aise_value(nullptr, nullptr, AiseId::GetUnitData, entry, index);
	}

	bool isLarvalEgg(CUnit* unit) {
		if (unit->id == UnitId::ZergEgg) {
			auto origin = unit->eggOrigin;
			if (origin == UnitId::ZergLarva) {
				return true;
			}
		}
		return false;
	}
	bool isZergEggNew(CUnit* unit, bool allow_larval_eggs, bool allow_larvae, uint16 explicit_unit_id) {

		uint16 id = UnitId::None;

		if (id != NULL) {
			id = unit->id;
		}

		else {
			id = explicit_unit_id;
		}

		switch (id) {
			case UnitId::ZergEgg:
				if (unit
					&& !allow_larval_eggs
					&& unit->eggOrigin == UnitId::ZergLarva) {
					return false;
				}
				return true;
			case UnitId::ZergCocoon:
				return true;
			default:
				break;
		}

		if (allow_larvae
			&& id == UnitId::ZergLarva) {
			return true;
		}
		return false;
	}

	bool isZergEgg(uint16 id, bool allow_normal_eggs, bool allow_larvae) {
		switch (id) {
			case UnitId::ZergCocoon:
				return true;
			default:
				break;
		}
		if (id == UnitId::ZergLarva && allow_larvae) {
			return true;
		}
		if (id == UnitId::ZergEgg && allow_normal_eggs) {
			return true;
		}
		return false;
	}
	bool isConventionalUnitMorphResult(uint16 id) {
		switch (id) {
			case UnitId::ZergIroleth:
			case UnitId::ZergOthstoleth:
			case UnitId::ZergVorvaling:
			case UnitId::ZergNathrokor:
			case UnitId::ZergBactalisk:
			case UnitId::ZergLakizilisk:
			case UnitId::ZergVithrilisk:
			case UnitId::ZergGorgrokor:
			case UnitId::ZergGeszithalor:
			case UnitId::ZergAkistrokor:
			case UnitId::ZergKonvilisk:
			case UnitId::ZergAlmaksalisk:
			case UnitId::ZergKalkalisk:
			case UnitId::ZergAlkajelisk:
			case UnitId::ZergKeskathalor:
			case UnitId::ZergIsthrathaleth:
			case UnitId::ProtossAugur:
				return true;
			default:
				break;
		}
		return false;
	}

	bool isAnyMorphMorphResult(uint16 id) {
		if (isConventionalUnitMorphResult(id)) {
			return true;
		}
		switch (id) {
			case UnitId::ProtossArchon:
			case UnitId::ProtossMindTyrant:
				return true;
			default:
				break;
		}
		return false;
	}
	bool isConventionalUnitMorphSource(uint16 id, bool allow_larvae, bool allow_high_templar) {
		switch (id) {
			case UnitId::ZergOvileth:
			case UnitId::ZergIroleth:
			case UnitId::ZergZergling:
			case UnitId::ZergHydralisk:
			case UnitId::ZergMutalisk:
			case UnitId::ZergGorgrokor:
			case UnitId::ZergQuazilisk:
			case UnitId::ZergMatraleth:
			case UnitId::ZergUltrakor:
			case UnitId::ZergZoryusthaleth:
				return true;
			default:
				break;
		}
		if (id == UnitId::ProtossCantavis && allow_high_templar) {
			return true;
		}
		if (id == UnitId::ZergLarva && allow_larvae) {
			return true;
		}
		return false;
	}
	bool isZergBuildingMorphResult(uint16 id) {
		switch (id) {
			case UnitId::ZergSire:
			case UnitId::ZergHive:
			case UnitId::ZergLair:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergAlazilArbor:
			case UnitId::ZergLarvalColony:
			case UnitId::ZergSporeColony:
			case UnitId::ZergSunkenColony:
			case UnitId::ZergVorvalPond:
			case UnitId::ZergNathrokLake:
			case UnitId::ZergVithrilHaunt:
			case UnitId::ZergGorgrokApiary:
			case UnitId::ZergGeszithRoost:
			case UnitId::ZergBactalDen:
			case UnitId::ZergLakizDen:
			case UnitId::ZergAlmakAntre:
			case UnitId::ZergKalkathBloom:
			case UnitId::ZergAkisNest:
			case UnitId::ZergKonvilNest:
			case UnitId::ZergKeskathGrotto:
			case UnitId::ZergAlkajChasm:
			case UnitId::ZergIsthrathMound:
				return true;
			default:
				break;
		}
		return false;
	}
	bool isZergBuildingMorphSource(uint16 id) {
		switch (id) {
			case UnitId::ZergHive:
			case UnitId::ZergLair:
			case UnitId::ZergHatchery:
			case UnitId::ZergIrolIris:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergCreepColony:
			case UnitId::ZergSpawningPool:
			case UnitId::ZergMutalSpire:
			case UnitId::ZergGorgrokApiary:
			case UnitId::ZergHydralDen:
			case UnitId::ZergBactalDen:
			case UnitId::ZergQuazilQuay:
			case UnitId::ZergMatralNest:
			case UnitId::ZergUltrakCavern:
			case UnitId::ZergZoryusShroud:
				return true;
			default:
				break;
		}
		return false;
	}

	std::string readSamaseFile(char* name) {
		const char* data = nullptr;
		uint32 len = 0;
		auto data_ptr = &data;
		auto len_ptr = &len;
		uint32 val = reinterpret_cast<uint32>(name);
		auto length = scbw::get_aise_value(nullptr, reinterpret_cast<CUnit*>(name), AiseId::ReadSamaseFile, reinterpret_cast<int>(data_ptr), reinterpret_cast<int>(len_ptr));
		char* buf = (char*)scbw::get_aise_value(nullptr, nullptr, AiseId::ReadSamaseFile_Ptr, 0, 0);
		std::string str;
		if (length == 0) {
			return str;
		} else {
			str = std::string(buf, length);
		}
		return str;
	}

	int get_generic_variable(uint32 id) {
		return get_aise_value(nullptr, nullptr, AiseId::GenericVariable, id, 0);
	}
	CUnit* get_generic_rust_unit(CUnit* unit, uint32 id) {
		uint32 val = get_aise_value(unit, nullptr, AiseId::GenericUnit, id, 0);
		return reinterpret_cast<CUnit*>(val);
	}
	void set_generic_rust_unit(CUnit* unit, uint32 id, CUnit* targ) {
		scbw::sendGPTP_aise_cmd(unit, targ, GptpId::SetGenericUnit, id, 0, 0);
	}
	void set_generic_variable(uint32 id, int val) {
		scbw::sendGPTP_aise_cmd(nullptr, nullptr, GptpId::SetGenericVariable, id, val, 0);
	}

	uint32 get_generic_rust_value(CUnit* u, uint32 val) {
		return get_aise_value(u, nullptr, AiseId::GenericValue, val, 0);
	}
	void set_generic_rust_value(CUnit* u, uint32 id, uint32 val) {
		scbw::sendGPTP_aise_cmd(u, nullptr, GptpId::SetGenericValue, id, val, 0);
	}
	void setSpriteData(uint32 entry, uint32 index, uint32 value) {
		return sendGPTP_aise_cmd(nullptr, nullptr, GptpId::SetSpriteData, entry, index, value);
	}

	const uint32 Func_CanTargetSpell = 0x00492020;
	uint32 get_aise_value(CUnit* unit1, CUnit* unit2, uint32 command, uint32 v1, uint32 v2) {
		uint32 result = 0;
		uint32 v3 = 0;//pointer
		_asm {
			PUSHAD
			MOV ESI, v2//fow_unit_id
			MOV EDI, unit2//target
			MOV EDX, 50000//tech
			PUSH v3//outerror
			PUSH v1//y
			PUSH command//x
			PUSH unit1//unit
			CALL Func_CanTargetSpell
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_PsiPlacement = 0x00473920;
	uint8 getUpgradeLevel(uint8 playerId, uint8 upgradeId) {
		assert(playerId < PLAYER_COUNT);
		//assert(upgradeId < UpgradeId::None);
		assert(upgradeId < 256);


		if (upgradeId < UpgradeId::PrismaticShield)
			return UpgradesSc->currentLevel[playerId][upgradeId];
		/*else if (upgradeId<UpgradeId::None)
			return UpgradesBw->currentLevel[playerId][upgradeId - UpgradeId::PStructurePlating];*/
		else {
			uint32 result = 0;
			uint32 player_u32 = playerId;
			uint32 upgrade_u32 = upgradeId;
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


	uint8 getUpgradeMaxLevel(uint8 playerId, uint8 upgradeId) {
		assert(playerId < PLAYER_COUNT);
		//	assert(upgradeId < UpgradeId::None);
		assert(upgradeId < 256);
		if (upgradeId < UpgradeId::PrismaticShield)
			return UpgradesSc->maxLevel[playerId][upgradeId];
		/*else if (upgradeId<UpgradeId::None)
			return UpgradesBw->maxLevel[playerId][upgradeId - UpgradeId::PStructurePlating];*/
		else {
			uint32 result = 0;
			uint32 player_u32 = playerId;
			uint32 upgrade_u32 = upgradeId;
			_asm {
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


	void setUpgradeLevel(uint8 playerId, uint8 upgradeId, uint8 level) {
		assert(playerId < PLAYER_COUNT);
		assert(upgradeId < UpgradeId::None);
		if (upgradeId < UpgradeId::PrismaticShield)
			UpgradesSc->currentLevel[playerId][upgradeId] = level;
		else
			UpgradesBw->currentLevel[playerId][upgradeId - UpgradeId::PrismaticShield] = level;
	}
	const uint32 Func_getLastQueueSlotType = 0x0047B270;
	uint16 getLastQueueSlotType(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EDX, a1
			CALL Func_getLastQueueSlotType
			MOV result, EAX
			POPAD
		}
		return (uint16)result;
	}
	void setOffsetToParentSpecialOverlay(CImage* image) {

		const uint32 Func_updateImagePositionOffset = 0x004D5A00;

		__asm {
			PUSHAD
			MOV ECX, image
			CALL Func_updateImagePositionOffset
			POPAD
		}
	}

	bool isStunned(CUnit* unit) {
		if (unit->maelstromTimer || unit->stasisTimer || unit->lockdownTimer || unit->status & UnitStatus::DoodadStatesThing) {
			return true;
		}
		return false;
	}

	void adjustAddonX(uint16 addonId, uint16 sourceId, uint16& x) {
		if (addonId == UnitId::TerranQuarry) {
			x = 128;
		}
	}
	void adjustAddonY(uint16 addonId, uint16 sourceId, uint16& y) {
		if (addonId == UnitId::TerranQuarry) {
			y = 32;
		}
	}

	void adjustAddonCoordinates(uint16 addonId, uint16 sourceId, uint16& x, uint16& y) {
		adjustAddonX(addonId, sourceId, x);
		adjustAddonY(addonId, sourceId, y);
	}


	//-------- Map information --------//

	const uint32 Func_GetGroundHeightAtPos = 0x004BD0F0;
	uint32 getGroundHeightAtPos(int32 x, int32 y) {

		static uint32 height;

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

	//-------- Moving Units --------//

	bool moveUnit(CUnit* unit, int16 x, int16 y) {
		Point16 targetPos = {x, y};
		Point16 actualPos;
		Point16 prevPos = unit->sprite->position;

		//Based on the code for Recall/Nydus Canal/Move Unit trigger
		setUnitPosition(unit, x, y);

		if (checkUnitCollisionPos(unit, &targetPos, &actualPos)) {
			prepareUnitMove(unit);
			setUnitPosition(unit, actualPos.x, actualPos.y);
			refreshUnitAfterMove(unit);
			return true;
		} else {
			setUnitPosition(unit, prevPos.x, prevPos.y);
			return false;
		}

	}

	const uint32 Func_DisableDialog = 0x00418640;
	void disableDialog(BinDlg* dialog) {

		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_DisableDialog
			POPAD
		}

	}

	;

	const uint32 Func_ShowDialog = 0x004186A0;
	void showDialog(BinDlg* dialog) {

		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_ShowDialog
			POPAD
		}

	}

	;

	const uint32 Func_HideDialog = 0x00418700;
	void hideDialog(BinDlg* dialog) {

		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_HideDialog
			POPAD
		}

	}

	;

	const uint32 Func_Sub418E00 = 0x00418E00;
	void function_418E00(BinDlg* dialog) {

		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_Sub418E00
			POPAD
		}

	}

	;

	const uint32 Func_UpdateDialog = 0x0041C400;
	void updateDialog(BinDlg* dialog) {
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL Func_UpdateDialog
			POPAD
		}
	}

	;

	const uint32 Func_PrepareUnitMoveClearRefs = 0x00493CA0;
	void prepareUnitMove(CUnit* unit, bool hideUnit) {

		assert(unit);

		static bool32 _hideUnit;

		_hideUnit = hideUnit ? 1 : 0;

		__asm {
			PUSHAD
			PUSH _hideUnit
			MOV EDI, unit
			CALL Func_PrepareUnitMoveClearRefs
			POPAD
		}

	}

	const uint32 Func_CheckUnitCollisionPos = 0x0049D3E0;
	bool checkUnitCollisionPos(CUnit* unit, const Point16* inPos, Point16* outPos, Box16* moveArea, bool hideErrorMsg, uint32 someFlag) {
		assert(unit);
		assert(inPos);
		assert(outPos);

		static uint32 result;
		static uint32 _hideErrorMsg;

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

	const uint32 Func_SetUnitPosition = 0x004EB9F0;
	void setUnitPosition(CUnit* unit, uint16 x, uint16 y) {
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

	const uint32 Func_RefreshRevealUnitAfterMove = 0x00494160;
	void refreshUnitAfterMove(CUnit* unit) {
		assert(unit);

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RefreshRevealUnitAfterMove
			POPAD
		}

	}

	//-------- Utility functions --------//

	const uint32 Func_CreateUnitAtPos = 0x004CD360; //AKA createUnitXY()
	CUnit* createUnitAtPos(uint16 unitType, uint16 playerId, uint32 x, uint32 y) {

		if (unitType >= UNIT_TYPE_COUNT)
			return nullptr;

		static CUnit* unit;

		__asm {
			PUSHAD
			MOV CX, unitType
			MOV AX, playerId
			PUSH y
			PUSH x
			CALL Func_CreateUnitAtPos
			MOV unit, EAX
			POPAD
		}

		return unit;
	}

	const uint32 Func_fixTargetLocation = 0x00401FA0;
	void fixTargetLocation(Point16* coords, uint32 unitId) {

		__asm {
			PUSHAD
			MOV EAX, unitId
			MOV EDX, coords
			CALL Func_fixTargetLocation
			POPAD
		}
	}

	const uint32 Func_CreateUnit = 0x004A09D0;
	CUnit* CreateUnit(uint32 unitId, int x, int y, uint32 playerId) {
		static CUnit* unit_created;
		__asm {
			PUSHAD
			PUSH playerId
			PUSH y
			MOV ECX, unitId
			MOV EAX, x
			CALL Func_CreateUnit
			MOV unit_created, EAX
			POPAD
		}
		return unit_created;
	}
	const uint32 Func_Sub4A01F0 = 0x004A01F0;
	void FinishUnit_Pre(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4A01F0
			POPAD
		}
	}
	const uint32 Func_UpdateUnitStrength = 0x0049FA40;
	void updateUnitStrength(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_UpdateUnitStrength
			POPAD
		}
	}
	const uint32 Func_AI_TrainingUnit = 0x004A2830;
	void InheritAI2(CUnit* result, CUnit* parent) {
		__asm {
			PUSHAD
			MOV ECX, parent
			MOV EAX, result
			CALL Func_AI_TrainingUnit
			POPAD
		}
	};
	CUnit* createUnitIngame(uint16 unitId, uint8 playerId, uint32 x, uint32 y, CUnit* creator) {
		if (*unitCount >= 1700) return nullptr; // prevent crashes, move to create unit later
		Point16 pos = {x, y};
		fixTargetLocation(&pos, unitId);
		CUnit* unit = CreateUnit(unitId, pos.x, pos.y, playerId);
		if (unit == nullptr) {
			displayLastNetErrForPlayer(playerId);
		} else {
			FinishUnit_Pre(unit);
			updateUnitStrength(unit);
			if (creator != nullptr && creator->pAI != nullptr) {
				InheritAI2(unit, creator);
			}
		}
		return unit;
	}
	const uint32 Func_displayLastNetErrForPlayer = 0x0049E530;
	void displayLastNetErrForPlayer(uint32 playerId) {
		__asm {
			PUSHAD
			PUSH playerId
			CALL Func_displayLastNetErrForPlayer
			POPAD
		}
	}
	uint32 getUnitOverlayAdjustment(CUnit* const unit) {
		if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
			return 1;
		else if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
			return 2;
		else
			return 0;
	}

	const uint32 Func_OrdersHoldPositionSuicidal = 0x004EB5B0;
	void makeToHoldPosition(CUnit* unit) {

		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_OrdersHoldPositionSuicidal
			POPAD
		}
	}
	void refreshScreen(int left, int top, int right, int bottom) {

		left >>= 4; right = (right + 15) >> 4;
		top >>= 4; bottom = (bottom + 15) >> 4;

		if (left > right)
			std::swap(left, right);
		if (top > bottom)
			std::swap(top, bottom);

		//Rect out of bounds
		if (left >= 40 || right < 0 || top >= 30 || bottom < 0)
			return;

		left = std::max(left, 0); right = std::min(right, 40 - 1);
		top = std::max(top, 0); bottom = std::min(bottom, 30 - 1);

		for (int y = top; y <= bottom; ++y)
			memset(&refreshRegions[40 * y + left], 1, right - left + 1);

	}

	void refreshScreen() {
		memset(refreshRegions, 1, 1200);
	}

	/* /!\ Not working like 004DC550	RandBetween /!\ */
	uint32 randBetween(uint32 min, uint32 max) {
		assert(min <= max);
		return min + ((max - min + 1) * random() >> 15);
	}

	//similar to 004DC4A0 RandomizeShort, but don't update
	//the RNG and don't use a parameter among possible
	//differences
	uint16 random() {
		if (*IS_IN_GAME_LOOP) {
			*lastRandomNumber = 22695477 * (*lastRandomNumber) + 1;
			return (*lastRandomNumber >> 16) % 32768;	//Make a number between 0 and 32767
		} else
			return 0;
	}

	//-------- Needs research --------//

		//Logically equivalent to function @ 0x004C36C0
	void refreshConsole() {

		static uint32* const bCanUpdateCurrentButtonSet = (uint32*)0x0068C1B0;
		static uint8* const bCanUpdateSelectedUnitPortrait = (uint8*)0x0068AC74;
		static uint8* const bCanUpdateStatDataDialog = (uint8*)0x0068C1F8;
		static BinDlg** const someDialogUnknown = (BinDlg**)0x0068C1E8;
		static BinDlg** const someDialogUnknownUser = (BinDlg**)0x0068C1EC;

		*bCanUpdateCurrentButtonSet = 1;
		*bCanUpdateSelectedUnitPortrait = 1;
		*bCanUpdateStatDataDialog = 1;
		*someDialogUnknown = nullptr;
		*someDialogUnknownUser = nullptr;

	}

	const uint32 Func_MoveScreen = 0x0049C440;
	//Player check added based on how others functions test it
	//Values above or equal to 8 means all players
	//Equivalent to moveScreenToUnit @ 0x004E6020
	void MoveScreenToUnit(CUnit* unit, uint32 playerId) {

		if (playerId >= 8 || playerId == *LOCAL_NATION_ID) {

			int x, y;

			x = (int16)unit->sprite->position.x;
			y = (int16)unit->sprite->position.y;

			if (x < 0)
				x += 31;
			if (y < 0)
				y += 31;

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

	const uint32 Func_minimapPing = 0x004A34C0;
	//Player check added based on how others functions test it
	//Values above or equal to 8 means all players
	//x and y are coords as gotten from unit->sprite->position
	void minimapPing(uint32 x, uint32 y, int32 color, uint32 playerId) {

		if (playerId >= 8 || playerId == *LOCAL_NATION_ID)
			__asm {
			PUSHAD
			PUSH color
			PUSH y
			PUSH x
			CALL Func_minimapPing
			POPAD
		}

	}

	const uint32 Func_hasOverlay = 0x0047B720;
	uint8 hasOverlay(CUnit* unit) {
		static uint8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_hasOverlay
			MOV result, AL
			POPAD
		}
		return result;
	}

	const uint32 Func_ApplyHallucinationStats = 0x004F6180;
	void ApplyHallucinationStats(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_ApplyHallucinationStats
			POPAD
		}
	}

	void createUnitFromAbility(int unitId, int count, uint16 playerId, int32 x, int32 y, uint32 removeTimer, CUnit* creator) {
		for (int i = 0; i < count; i++) {
			CUnit* unit = scbw::createUnitIngame(unitId, playerId, x, y, creator);
			if (unit == nullptr) {
				return;
			}
			if (creator != nullptr) {
				if (creator->status & UnitStatus::IsHallucination) {
					ApplyHallucinationStats(unit);
					unit->status |= UnitStatus::IsHallucination;
					unit->removeTimer = 1350;
					unit->sprite->createOverlay(ImageId::HallucinationDeath3);
				}
			}
			switch (unitId) {
				case UnitId::ZergBroodling:
				case UnitId::ZergBroodlisk:
				{
					scbw::playSound(SoundId::Zerg_BROODLING_ZBrRdy00_WAV, unit);
					if (unit->removeTimer == 0 || unit->removeTimer > removeTimer) {
						unit->removeTimer = removeTimer;
					}
					uint32 range = 6 * 32;
					scbw::UnitFinder(
						unit->getX() - range,
						unit->getY() - range,
						unit->getX() + range,
						unit->getY() + range
					).forEach([&](CUnit* target) {
						if (target->id != UnitId::ZergMatraleth) {
							return;
						}
						if (!scbw::isAlliedTo(unit->playerId, target->playerId)) {
							return;
						}
						if (target->energy < 200 * 256) {
							target->energy = std::min((uint16)(target->energy + (uint16)5 * (uint16)256), target->getMaxEnergy());
							target->sprite->createUnderlay(425);
						}
					});
					break;
				}
				case UnitId::ProtossSimulacrum:
				{
					if (creator != nullptr) {
						unit->hitPoints = std::max(256, creator->hitPoints);
						unit->shields = std::max(256, creator->shields);
						scbw::inheritDebuffs(creator, unit);
					}
					scbw::playSound(SoundId::Simulacrum_Clone_01, unit);
				}
			}
		}
	}

	void rangeModifiedFire(uint8 weaponId, CUnit* unit, int16 x, int16 y, uint32 offset) {
		auto target_unit = unit->orderTarget.unit;
		auto target_pos = unit->orderTarget.pt;
		auto distance = scbw::getDistanceFast(unit->position.x, unit->position.y,
			unit->orderTarget.unit->position.x, unit->orderTarget.unit->position.y);
		distance += offset;
		if (distance < 0) {
			distance = 0;
		}
		auto polar_x = unit->getX() + scbw::getPolarX(distance, unit->currentDirection1);
		auto polar_y = unit->getY() + scbw::getPolarY(distance, unit->currentDirection1);
		unit->orderTarget.unit = nullptr;
		unit->orderTarget.pt.x = polar_x;
		unit->orderTarget.pt.y = polar_y;
		createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
		unit->orderTarget.unit = target_unit;
		unit->orderTarget.pt = target_pos;
	}

	bool canSpawnBroodling(CUnit* target) {
		if (units_dat::GroupFlags[target->id].isBuilding) return false;
		return true;
	}

	int getBroodlingCountFromSupply(CUnit* target) {
		if (!canSpawnBroodling(target)) return 0;
		bool ground = !(target->status & UnitStatus::InAir);
		bool onCreep = scbw::getActiveTileAt(target->getX(), target->getY()).hasCreep;
		double multiplier = (ground && onCreep) ? 1.5 : 1;
		return ceil(std::max(((int)units_dat::SupplyRequired[target->id]), 1) * multiplier);
	}

	const uint32 Func_unitDeathSomething_0 = 0x004E6340;
	void hideAndDisableUnit(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitDeathSomething_0
			POPAD
		}

	}

	int dir_saturate(int direction, int value) {
		int d = direction + value;
		if (d > 255) {
			return d - 255;
		}
		if (d < 0) {
			return d + 255;
		}
		return d;
	}

	uint32 get_sized_overlay(CUnit* unit, uint32 initial) {
		uint32 overlayImageId;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
			overlayImageId = initial + 1;
		else
			if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
				overlayImageId = initial + 2;
			else
				overlayImageId = initial;
		return overlayImageId;
	}

	bool canUseCliffAdvantage(CUnit* unit, uint8 weapon) {
		return unit != nullptr
			&& !(unit->status & UnitStatus::InAir)
			&& unit->id != UnitId::ProtossPlasmaShell
			&& (weapon == NULL || weapons_dat::MaxRange[weapon] > 32)
			&& !(unit->status & UnitStatus::InBuilding)
			&& unit->id != UnitId::TerranHarakan;
	}

	int32 getCliffAdvantage(CUnit* unit, uint8 weapon) {
		if (unit == nullptr)
			return 0;
		if (!canUseCliffAdvantage(unit, weapon))
			return 0;
		CUnit* targetUnit = unit->orderTarget.unit;
		if (targetUnit == nullptr)
			return 0;
		//if (targetUnit->status & UnitStatus::InAir)
		//	return 0;
		int32 sourceHeight = scbw::getActiveTileAt(unit->position.x, unit->position.y).groundHeight;
		int32 targetHeight = scbw::getActiveTileAt(targetUnit->position.x, targetUnit->position.y).groundHeight;
		return (sourceHeight - targetHeight) / 2;
	}

	int32 getCreepAdvantage(CUnit* unit, uint8 weapon) {
		if (unit == nullptr)
			return 0;
		if (!canUseCliffAdvantage(unit, weapon) || unit->getRace() != RaceId::Zerg)
			return 0;
		CUnit* targetUnit = unit->orderTarget.unit;
		if (targetUnit == nullptr)
			return 0;
		//if (targetUnit->status & UnitStatus::InAir)
		//	return 0;
		bool sourceCreep = scbw::getActiveTileAt(unit->position.x, unit->position.y).hasCreep;
		bool targetCreep = scbw::getActiveTileAt(targetUnit->position.x, targetUnit->position.y).hasCreep;
		return (sourceCreep && !targetCreep) ? 1 : 0;
	}

	void applyEnsnare(CUnit* unit) { // 004F45E0
		int32 baseProperty; // eax
		uint8 overlayPadding; // al
		CUnit* victim; // ecx

		if (!(unit->ensnareTimer)
			&& !(unit->status & UnitStatus::Burrowed)) {

			victim = unit;
			if (unit->subunit != nullptr) {
				victim = unit->subunit;
			}

			// Create overlay
			victim->sprite->createOverlay(ImageId::EnsnareOverlay_Small + scbw::getOverlayOffset(unit->id));
		}

		// Apply effect
		unit->ensnareTimer = 37;
		unit->updateSpeed();
	}

	bool isEnabled(CUnit* unit) {
		if (unit->lockdownTimer
			|| unit->maelstromTimer
			|| unit->stasisTimer
			|| unit->status & UnitStatus::Disabled) {
			return false;
		}
		return true;
	}

	int32 getUpgradeType(CUnit* unit) {
		int32 race = unit->getRace();
		double upgType;
		switch (race) {
			case RaceId::Zerg:
				upgType = UpgradeId::ZergMeleeAttacks;
			case RaceId::Protoss:
				upgType = UpgradeId::ProtossGroundWeapons;
			default:
				upgType = UpgradeId::TerranInfantryWeapons;
		}
		return upgType;
	}

	int32 getOverlayOffset(uint16 unitId) {
		uint32 overlayOffset = 0;
		if (units_dat::BaseProperty[unitId] & UnitProperty::MediumOverlay) {
			overlayOffset = 1;
		} else if (units_dat::BaseProperty[unitId] & UnitProperty::LargeOverlay) {
			overlayOffset = 2;
		}
		return overlayOffset;
	}

	int32 getFinalDamage(CUnit* unit, uint16 weaponId) {
		int32 upgType = getUpgradeType(unit);
		int32 baseDamage = weapons_dat::DamageAmount[weaponId];
		double bonusDamage = (double)weapons_dat::DamageBonus[weaponId] * (double)scbw::getUpgradeLevel(unit->playerId, upgType);
		double finalDamage = baseDamage;
		if (bonusDamage) {
			finalDamage = baseDamage + bonusDamage;
		}
		return finalDamage;
	}

	// I may need to make this function more flexible later -- Pr0nogo
	bool unitIsPacifist(CUnit* unit) {
		return !((unit->hasWeapon())
			|| unit->id == UnitId::ZergLakizilisk); // LAME
	}

	// Identical to function OrderIsSpell @ 00436930 -- Pr0nogo
	// We should update the lardcode for HYDRA
	bool orderIsSpell(int32 orderId) {
		return orderId == OrderId::Plague
			|| orderId == OrderId::DarkSwarm
			//	|| orderId == OrderId::Ensnare				// Land Grab
			|| orderId == OrderId::Irradiate
			|| orderId == OrderId::StasisField
			|| orderId == OrderId::EmpShockwave				// Observance
			|| orderId == OrderId::MagnaPulse				// Lobotomy Mine
			|| orderId == OrderId::PsiStorm
			//	|| orderId == OrderId::SummonBroodlings		// unused
			|| orderId == OrderId::FireYamatoGun1;
	}

	bool canBeParasited(CUnit* unit) {
		return !(units_dat::BaseProperty[unit->id] & UnitProperty::Building)
			&& !(unit->status & UnitStatus::Burrowed);
	}

	//Creates the Plasma Shield flickering effect.
	//Identical to function @ 0x004E6140
	void createShieldOverlay(CUnit* unit, uint32 attackDirection) {
		//const LO_Header* shield_lo = shieldOverlays[unit->sprite->mainGraphic->id];
		const LO_Header* shield_lo = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlayShieldPtr, unit->sprite->mainGraphic->id));
		uint32 frameAngle = (attackDirection - 124) / 8 % 32;

		Point8 offset = shield_lo->getOffset(unit->sprite->mainGraphic->direction, frameAngle);

		unit->sprite->createOverlay(ImageId::ShieldOverlay, offset.x, offset.y, frameAngle);
	}

	void decloakProc(CUnit* unit) {
		if (isDecloakable(unit)) {
			scbw::set_generic_timer(unit, ValueId::DecloakTimer, 48 + 1);
			unit->visibilityStatus = 0xff;
			hooks::updateUnitVisibility(unit);
		}
	}

	bool isDecloakable(CUnit* unit) {
		return (units_dat::BaseProperty[unit->id] & UnitProperty::PermanentCloak || unit->status & UnitStatus::CloakingForFree)
			&& !(unit->status & UnitStatus::Burrowed);
	}

} //scbw