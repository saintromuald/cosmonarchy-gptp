#include "api.h"
#include <SCBW/UnitFinder.h>
#include <algorithm>
#include <cassert>
#include <utility>
#include <hooks/weapons/weapon_damage.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class DarkSwarmFinderProc : public scbw::UnitFinderCallbackMatchInterface {
	public:
		bool match(CUnit* unit) {
			return unit->id == UnitId::Spell_DarkSwarm;
		}
};

class gameteMeiosisProc : public scbw::UnitFinderCallbackProcInterface {
private:
	CUnit* broodling;

public:
	gameteMeiosisProc(CUnit* broodling)
		: broodling(broodling) {}
	void proc(CUnit* unit) {
		if (unit->id != UnitId::ZergMatraleth) {
			return;
		}
		if (!scbw::isAlliedTo(broodling->playerId, unit->playerId)) {
			return;
		}
		if (unit->energy < 200 * 256) {
			unit->energy = std::min((u16)(unit->energy + (u16)5 * (u16)256), unit->getMaxEnergy());
			unit->sprite->createUnderlay(425);
		}
	}
};

namespace scbw {

	const u32 Func_ApplySpores = 0x004F4480;
	void ApplySpores(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EBX, unit
			CALL Func_ApplySpores
			POPAD
		}
	}

	const u32 Func_SetThingyVisibilityFlags = 0x004878F0;
	bool setThingyVisibilityFlags(CThingy* thingy) {

		static Bool32 bPreResult;

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

	const u32 Helper_CreateBullet = 0x0048C260;
	void createBullet(u8 weaponId, CUnit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction) {
		static u32 attackingPlayer_;
		static u32 direction_;
		static s32 x_;
		static s32 y_;

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

	const u32 Helper_WeaponBulletShot = 0x00479AE0;
	void weaponBulletShot(CUnit* target, CBullet* bullet, u32 dmgDivisor) {
		__asm {
			PUSHAD
			PUSH dmgDivisor
			MOV EAX, target
			MOV EDX, bullet
			CALL Helper_WeaponBulletShot
			POPAD
		}
	}

	CBullet* createBulletRet(u8 weaponId, CUnit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction) {

		static u32 attackingPlayer_;
		static u32 direction_;
		static s32 x_;
		static s32 y_;

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
			MOV bullet,EAX
			POPAD
		}
		return bullet;

	}
	void artificalBulletFrom(u8 weaponId, u8 playerId, CUnit* source, CUnit* target, Point16 target_pt) {
		auto target_unit = source->orderTarget.unit;
		auto target_pos = source->orderTarget.pt;
		source->orderTarget.unit = target;
		source->orderTarget.pt = target_pt;
		Point16 pick;
		if (target != NULL && target_unit!=NULL) {
			source->orderTarget.pt = target_unit->sprite->position;
		}
		else {
			source->orderTarget.pt = target_pt;
		}
		createBullet(weaponId, source, source->sprite->position.x, source->sprite->position.y, playerId,
			getAngle(source->position.x, source->position.y, source->orderTarget.pt.x, source->orderTarget.pt.y));
		source->orderTarget.unit = target_unit;
		source->orderTarget.pt = target_pos;
	}
	void artificalBulletFrom(u8 weaponId, u8 playerId, CUnit* source, CUnit* target, u16 x, u16 y) {
		Point16 tgt;
		tgt.x = x;
		tgt.y = y;
		artificalBulletFrom(weaponId, playerId, source, target, tgt);
	}

const u32 Func_Sub4EBAE0 = 0x004EBAE0;
void function_004EBAE0(CUnit* unit, s32 x, s32 y) {
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

bool teleport(CUnit* u, Point16 target, u16 spriteAnimation, bool preserveOrder) {
	Point16 previous_pos, new_pos;
	bool result = false;
	previous_pos.x = u->sprite->position.x;
	previous_pos.y = u->sprite->position.y;

	//identical code to setUnitPosition() but for some reason, is
	//using another function to do exactly the same thing
	function_004EBAE0(u, target.x,target.y);

	if (!scbw::checkUnitCollisionPos(u, &(u->sprite->position), &new_pos, NULL, false, 0))
		//collision detected, cancel the movement
		scbw::setUnitPosition(u, previous_pos.x, previous_pos.y);
	else {
		result = true;
		CThingy* recall_effect_sprite;

		//finish the teleporting (and cut the recalled unit from a possible unit targeted by an order)
		scbw::prepareUnitMove(u, false);
		if (!preserveOrder) {
			u->orderTarget.unit = NULL;
		}
		scbw::setUnitPosition(u, new_pos.x, new_pos.y);
		scbw::refreshUnitAfterMove(u);
		
		//stop some currently active orders
		if (u->id != UnitId::ZergCocoon && !preserveOrder)
			u->orderComputerCL(units_dat::ReturnToIdleOrder[u->id]);

		//create the Recall animation over the unit
		if (spriteAnimation) {
			recall_effect_sprite = createThingy(spriteAnimation, new_pos.x, new_pos.y, 0);
			if (recall_effect_sprite != NULL) {
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
			)
		{
			if (
				u->connectedUnit != NULL &&
				u->connectedUnit->id == UnitId::TerranNuclearMissile
				)
			{
				(u->connectedUnit)->connectedUnit = NULL;
				u->connectedUnit = NULL;
			}
		}
	}
	return result;
}
const u32 Func_CreateFlingy = 0x00496460;
CFlingy* createFlingy(u32 flingyId, u32 playerId, u32 x, u32 y) {
	static CFlingy* flingy;
	__asm {
		PUSHAD
		MOV EAX,playerId
		MOV ECX,x
		MOV EDX,y
		PUSH flingyId
		CALL Func_CreateFlingy
		MOV flingy,EAX
		POPAD
	}
	return flingy;
}


const u32 Func_CreateThingy = 0x00488210;
CThingy* createThingy(u32 spriteId, s16 x, s16 y, u32 playerId) {

	static CThingy* thingy;
	s32 x_ = x;
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
;

const u32 Func_SendCommand = 0x00485BD0;
void SendCommand(u8* params, u32 param_length) {

	__asm {
		PUSHAD
		MOV EDX, param_length
		MOV ECX, params
		CALL Func_SendCommand
		POPAD
	}

}
//-------- Output functions --------//

const u32 Func_PlaySound = 0x0048ED50;
void playSound(u32 sfxId, CUnit* sourceUnit) {

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


void print(std::string text, u32 color) {
	printText(text.c_str(), color);
}

void printFormattedText(const char* format, ...) {
	char buf[256];

	va_list _ArgList;
	__crt_va_start(_ArgList, format);
	vsnprintf(buf, sizeof(buf), format, _ArgList);
	__crt_va_end(_ArgList);

	printText(buf);
}

void printFormattedColoredText(const char* format, u32 color, ...) {
	char buf[256];

	va_list _ArgList;
	__crt_va_start(_ArgList, format);
	vsnprintf(buf, sizeof(buf), format, _ArgList);
	__crt_va_end(_ArgList);

	printText(buf, color);
}

const u32 Func_PrintText = 0x0048CD30;
void printText(const char* text, u32 color) {

	if (!text) 
		return;

	DWORD gtc = GetTickCount() + 7000;

	__asm	 {
		PUSHAD
		PUSH 0	;//unknown
		MOV eax, text
		PUSH gtc
		PUSH color
		CALL Func_PrintText
		POPAD
	}

}

const u32 Func_ShowErrorMessageWithSfx = 0x0048EE30;
void showErrorMessageWithSfx(u32 playerId, u32 statTxtId, u32 sfxId) {

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

const u32 Func_CanBeEnteredBy = 0x004E6E00; //AKA CanEnterTransport()
bool canBeEnteredBy(CUnit* transport, CUnit* unit) {

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

//Identical to function @ 0x00475CE0
bool canWeaponTargetUnit(u8 weaponId, CUnit* target, CUnit* attacker) {

	if (weaponId >= WEAPON_TYPE_COUNT)
		return false;

	if (!target)
		return weapons_dat::TargetFlags[weaponId].terrain;


	if (target->status & UnitStatus::Invincible)
		return false;

	const TargetFlag tf = weapons_dat::TargetFlags[weaponId];
	const u32 targetProps = units_dat::BaseProperty[target->id];

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
	return getDarkSwarm(unit) != NULL;
}

CUnit* getDarkSwarm(CUnit* unit) {
	static UnitFinder darkSwarmFinder;
	darkSwarmFinder.search(unit->getLeft(), unit->getTop(), unit->getRight(), unit->getBottom());

	CUnit* darkSwarm = darkSwarmFinder.getFirst(DarkSwarmFinderProc());
	return darkSwarm;
}

CUnit* getDarkSwarm(u16 x, u16 y, u16 radius) {
	UnitFinder darkSwarmFinder;
	darkSwarmFinder.search(x-radius,y-radius,x+radius,y+radius);
	CUnit* darkSwarm = darkSwarmFinder.getFirst(DarkSwarmFinderProc());
	return darkSwarm;

}



u16 getTechUseErrorMessage(CUnit* target, u8 castingPlayer, u16 techId) {

	static u32 techId_;
	static u16 errorMessage;

	const u32 Func_GetTechUseErrorMessage = 0x00491E80;

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
u32 getDistanceFast(s32 x1, s32 y1, s32 x2, s32 y2) {

	int dMax = abs(x1 - x2), dMin = abs(y1 - y2);

	if (dMax < dMin)
		std::swap(dMax, dMin);

	if (dMin <= (dMax >> 2))
		return dMax;

	return (dMin * 3 >> 3) + (dMin * 3 >> 8) + dMax - (dMax >> 4) - (dMax >> 6);

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

		if ((unsigned int) slope <= tangentTable[angle])
			max = angle;
		else
			min = angle + 1;

		angle = (min + max) / 2;

	} while (min != max);

	return (isNegative ? -angle : angle);
}

//Identical to function @ 0x00495300
			//    EDX		[EBP+08]	ECX			EAX
s32 getAngle(s32 xStart, s32 yStart, s32 xEnd, s32 yEnd) {

	s32 dx = xEnd - xStart, dy = yEnd - yStart;
	
	if (dx == 0)
		return dy > 0 ? 128 : 0;

	s32 angle = arctangent((dy << 8) / dx);

	if (dx < 0) {
		angle += 192;
		return angle == 256 ? 0 : angle;
	}
	else
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


double bw_angle_to_normal(s32 bw_angle) {
	double angle = (double)bw_angle*1.40625;
	return angle;
}
s32 getPolarX(s32 distance, u8 angle) {
	return distance * angleDistance[angle].x >> 8;
}

s32 getPolarY(s32 distance, u8 angle) {
	return distance * angleDistance[angle].y >> 8;
}

//-------- Player information --------//

s32 getSupplyRemaining(u8 playerId, u8 raceId) {
	assert(raceId <= 2);
	assert(playerId < PLAYER_COUNT);

	s32 supplyProvided;

	if (isCheatEnabled(CheatFlags::FoodForThought))
		supplyProvided = raceSupply[raceId].max[playerId];
	else
		supplyProvided = std::min(raceSupply[raceId].max[playerId], raceSupply[raceId].provided[playerId]);

	return supplyProvided - raceSupply[raceId].used[playerId];

}

bool hasTechResearched(u8 playerId, u16 techId) {
	assert(playerId < PLAYER_COUNT);
	assert(techId < TechId::None);
	if (techId < TechId::Restoration)
		return TechSc->isResearched[playerId][techId] != 0;
	else
		return TechBw->isResearched[playerId][techId - TechId::Restoration] != 0;

}

bool hasTechAvailable(u8 playerId, u16 techId) {
	assert(playerId < PLAYER_COUNT);
	assert(techId < TechId::None);
	if (techId < TechId::Restoration)
		return TechSc->isEnabled[playerId][techId] != 0;
	else
		return TechBw->isEnabled[playerId][techId - TechId::Restoration] != 0;
}


void setTechResearchState(u8 playerId, u16 techId, bool isResearched) {
	assert(playerId < PLAYER_COUNT);
	assert(techId < TechId::None);

	if (techId < TechId::Restoration)
		TechSc->isResearched[playerId][techId] = isResearched;
	else
		TechBw->isResearched[playerId][techId - TechId::Restoration] = isResearched;

}


const u32 Func_DoWeaponDamage = 0x00479930;
void sendGPTP_aise_cmd(CUnit* unit1, CUnit* unit2, u32 command, u32 v1, u32 v2, u32 v3){
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

u32 getFlingyData(u32 entry, u32 index) {
	return get_aise_value(NULL, NULL, AiseId::GetFlingyData, entry, index);
}
u32 getSpriteData(u32 entry, u32 index) {
	return get_aise_value(NULL, NULL, AiseId::GetSpriteData, entry, index);
}

u32 getImageData(u32 entry, u32 index) {
	return get_aise_value(NULL, NULL, AiseId::GetImageData, entry, index);
}
u32 getUnitData(u32 entry, u32 index) {
	return get_aise_value(NULL, NULL, AiseId::GetUnitData, entry, index);
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
bool isZergEggNew(CUnit* unit, bool allow_larval_eggs, bool allow_larvae, u16 explicit_unit_id) {

	u16 id = UnitId::None;

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
	&&  id == UnitId::ZergLarva) {
		return true;
	}
	return false;
}

bool isZergEgg(u16 id, bool allow_normal_eggs, bool allow_larvae) {
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
bool isConventionalUnitMorphResult(u16 id) {
	switch (id) {
	case UnitId::ZergAlkajelisk:
	case UnitId::ZergKeskathalor:
	case UnitId::ZergGeszithalor:
	case UnitId::ZergVithrilisk:
	case UnitId::ZergGorgrokor:
	case UnitId::ZergVorvaling:
	case UnitId::ZergNathrokor:
	case UnitId::ZergKalkalisk:
	case UnitId::ZergBactalisk:
	case UnitId::ZergAlmaksalisk:
	case UnitId::ZergLakizilisk:
	case UnitId::ZergIroleth:
	case UnitId::ZergOthstoleth:
	case UnitId::ZergIsthrathaleth:
	case UnitId::ProtossAugur:
		return true;
	default:
		break;
	}
	return false;
}

bool isAnyMorphMorphResult(u16 id) {
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
bool isConventionalUnitMorphSource(u16 id, bool allow_larvae, bool allow_high_templar) {
	switch (id) {
	case UnitId::ZergUltrakor:
	case UnitId::ZergMutalisk:
	case UnitId::ZergGorgrokor:
	case UnitId::ZergHydralisk:
	case UnitId::ZergZergling:
	case UnitId::ZergQuazilisk:
	case UnitId::ZergIroleth:
	case UnitId::ZergOvileth:
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
bool isZergBuildingMorphResult(u16 id){
	switch (id) {
	case UnitId::ZergSire:
	case UnitId::ZergHive:
	case UnitId::ZergLair:
	case UnitId::ZergLarvalColony:
	case UnitId::ZergSporeColony:
	case UnitId::ZergSunkenColony:
	case UnitId::ZergKonvilNest:
	case UnitId::ZergKeskathGrotto:
	case UnitId::ZergAlkajChasm:
	case UnitId::ZergGeszithRoost:
	case UnitId::ZergVithrilHaunt:
	case UnitId::ZergGorgrokApiary:
	case UnitId::ZergBactalDen:
	case UnitId::ZergAlmakAntre:
	case UnitId::ZergLakizDen:
	case UnitId::ZergVorvalPond:
	case UnitId::ZergNathrokLake:
	case UnitId::ZergKalkathBloom:
	case UnitId::ZergOthstolOviform:
	case UnitId::ZergAlazilArbor:
	case UnitId::ZergIsthrathMound:
		return true;
	default:
		break;
	}
	return false;
}
bool isZergBuildingMorphSource(u16 id){
	switch (id) {
	case UnitId::ZergHive:
	case UnitId::ZergLair:
	case UnitId::ZergHatchery:
	case UnitId::ZergCreepColony:
	case UnitId::ZergAkisNest:
	case UnitId::ZergUltrakCavern:
	case UnitId::ZergMutalSpire:
	case UnitId::ZergGorgrokApiary:
	case UnitId::ZergHydralDen:
	case UnitId::ZergBactalDen:
	case UnitId::ZergSpawningPool:
	case UnitId::ZergQuazilQuay:
	case UnitId::ZergIrolIris:
	case UnitId::ZergOthstolOviform:
	case UnitId::ZergZoryusShroud:
		return true;
	default:
		break;
	}
	return false;
}

std::string readSamaseFile(char* name) {
	const char* data = NULL;
	u32 len = 0;
	auto data_ptr = &data;
	auto len_ptr = &len;
	u32 val = reinterpret_cast<u32>(name);
	auto length = scbw::get_aise_value(NULL, reinterpret_cast<CUnit*>(name), AiseId::ReadSamaseFile, reinterpret_cast<int>(data_ptr), reinterpret_cast<int>(len_ptr));
	char* buf = (char*)scbw::get_aise_value(NULL, NULL, AiseId::ReadSamaseFile_Ptr, 0, 0);
	std::string str;
	if (length == 0) {
		return str;
	}
	else {
		str = std::string(buf, length);
	}
	return str;
}

int get_generic_variable(u32 id) {
	return get_aise_value(NULL, NULL, AiseId::GenericVariable, id, 0);
}
CUnit* get_generic_rust_unit(CUnit* unit, u32 id) {
	u32 val = get_aise_value(unit, NULL, AiseId::GenericUnit, id, 0);
	return reinterpret_cast<CUnit*>(val);
}
/*CUnit* get_generic_unit(CUnit* unit, u32 id) {
	u32 val = get_aise_value(unit, NULL, AiseId::GenericUnit, id, 0);
	return reinterpret_cast<CUnit*>(val);
}
u32 get_generic_value(CUnit* u, u32 val) {
	return get_aise_value(u, NULL, AiseId::GenericValue, val, 0);
}
u32 get_generic_timer(CUnit* u, u32 val) {
	return get_aise_value(u, NULL, AiseId::GenericTimer, val, 0);
}
void set_generic_unit(CUnit* unit, u32 id, CUnit* targ) {
	scbw::sendGPTP_aise_cmd(unit, targ, GptpId::SetGenericUnit, id, 0, 0);
}*/
void set_generic_rust_unit(CUnit* unit, u32 id, CUnit* targ) {
	scbw::sendGPTP_aise_cmd(unit, targ, GptpId::SetGenericUnit, id, 0, 0);
}
void set_generic_variable(u32 id, int val) {
	scbw::sendGPTP_aise_cmd(NULL, NULL, GptpId::SetGenericVariable, id, val, 0);
}

u32 get_generic_rust_value(CUnit* u, u32 val) {
	return get_aise_value(u, NULL, AiseId::GenericValue, val, 0);
}
void set_generic_rust_value(CUnit* u, u32 id, u32 val) {
	scbw::sendGPTP_aise_cmd(u, NULL, GptpId::SetGenericValue, id, val, 0);
}
void setSpriteData(u32 entry, u32 index, u32 value) {
	return sendGPTP_aise_cmd(NULL, NULL, GptpId::SetSpriteData, entry, index, value);
}

/*void set_generic_value(CUnit* u, u32 id, u32 val) {
	scbw::sendGPTP_aise_cmd(u, NULL, GptpId::SetGenericValue, id, val, 0);
}
void add_generic_value(CUnit* u, u32 id, int val) {
	scbw::sendGPTP_aise_cmd(u, NULL, GptpId::AddGenericValue, id, val, 0);
}

void set_generic_timer(CUnit* u, u32 id, u32 val) {
	scbw::sendGPTP_aise_cmd(u, NULL, GptpId::SetGenericTimer, id, val, 0);
}*/

const u32 Func_CanTargetSpell = 0x00492020;
u32 get_aise_value(CUnit* unit1, CUnit* unit2, u32 command, u32 v1, u32 v2){
	u32 result = 0;
	u32 v3 = 0;//pointer
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

const u32 Func_PsiPlacement = 0x00473920;
u8 getUpgradeLevel(u8 playerId, u8 upgradeId) {
	assert(playerId < PLAYER_COUNT);
	//assert(upgradeId < UpgradeId::None);
	assert(upgradeId < 256);


	if (upgradeId < UpgradeId::PrismaticShield)
		return UpgradesSc->currentLevel[playerId][upgradeId];
	/*else if (upgradeId<UpgradeId::None)
		return UpgradesBw->currentLevel[playerId][upgradeId - UpgradeId::PStructurePlating];*/
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


u8 getUpgradeMaxLevel(u8 playerId, u8 upgradeId) {
	assert(playerId < PLAYER_COUNT);
//	assert(upgradeId < UpgradeId::None);
	assert(upgradeId < 256);
	if (upgradeId < UpgradeId::PrismaticShield)
		return UpgradesSc->maxLevel[playerId][upgradeId];
	/*else if (upgradeId<UpgradeId::None)
		return UpgradesBw->maxLevel[playerId][upgradeId - UpgradeId::PStructurePlating];*/
	else {
		u32 result = 0;
		u32 player_u32 = playerId;
		u32 upgrade_u32 = upgradeId;
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


void setUpgradeLevel(u8 playerId, u8 upgradeId, u8 level) {
	assert(playerId < PLAYER_COUNT);
	assert(upgradeId < UpgradeId::None);
	if (upgradeId < UpgradeId::PrismaticShield)
		UpgradesSc->currentLevel[playerId][upgradeId] = level;
	else
		UpgradesBw->currentLevel[playerId][upgradeId - UpgradeId::PrismaticShield] = level;
}
const u32 Func_getLastQueueSlotType = 0x0047B270;
u16 getLastQueueSlotType(CUnit* a1) {
	static int result;
	__asm {
		PUSHAD
		MOV EDX, a1
		CALL Func_getLastQueueSlotType
		MOV result, EAX
		POPAD
	}
	return (u16)result;
}
void setOffsetToParentSpecialOverlay(CImage* image) {

	const u32 Func_updateImagePositionOffset = 0x004D5A00;

	__asm {
		PUSHAD
		MOV ECX, image
		CALL Func_updateImagePositionOffset
		POPAD
	}
}

/*
CBullet* setMissileSourcePosRet(CUnit* unit, u32 weaponId, u32 valueId, s32 rotateCount, u32 overlayType, u32 frameset_offset) {
	if (unit->sprite == NULL || unit->orderTarget.unit==NULL) {
		return NULL;
	}
	auto rotate = scbw::get_generic_value(unit, valueId);
	const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(overlayType,
		unit->sprite->mainGraphic->id));
	u8 frameDirection = unit->sprite->mainGraphic->direction;
	Point8 offset = loFile->getOffset(frameDirection + (17 * frameset_offset), rotate);
	if (unit->sprite->mainGraphic->flags & CImage_Flags::Mirrored) {
		offset.x = -offset.x;
	}
	rotate++;
	if (rotate > rotateCount - 1) {
		rotate = 0;
	}
	scbw::set_generic_value(unit, valueId, rotate);
	auto X = unit->getX() + offset.x;
	auto Y = unit->getY() + offset.y;
	auto bullet = createBulletRet(weaponId, unit, X, Y, unit->playerId, scbw::getAngle(X, Y,
		unit->orderTarget.unit->position.x, unit->orderTarget.unit->position.y));
	return bullet;
}

bool isSlowed(CUnit* unit) {
	if (unit->ensnareTimer || unit->unusedTimer) {
		return true;
	}
	if (get_generic_timer(unit, ValueId::MaledictionSlow) > 1) {
		return true;
	}
	if (get_generic_timer(unit, ValueId::LobotomySlow) > 1) {
		return true;
	}
	if (unit->id==UnitId::TerranWyvern && get_generic_value(unit, ValueId::ReverseThrust) == 1) {
		return true;
	}
	if (unit->id == UnitId::TerranCyprian) {
		if (scbw::get_generic_value(unit, AiseId::SendCyprianValue) == 1) {
			return true;
		}
	}
	if (unit->id == UnitId::ProtossExemplar && scbw::get_generic_rust_value(unit, ValueId::ExemplarSwitch)) {
		return true;
	}
	return false;
}
*/
bool isStunned(CUnit* unit) {
	if (unit->maelstromTimer || unit->stasisTimer) {
		return true;
	}
	return false;
}

void adjustAddonX(u16 addonId, u16 sourceId, u16& x) {
	if (addonId == UnitId::TerranQuarry) {
		x = 128;
	}
}
void adjustAddonY(u16 addonId, u16 sourceId, u16& y) {
	if (addonId == UnitId::TerranQuarry) {
		y = 32;
	}
}

void adjustAddonCoordinates(u16 addonId, u16 sourceId, u16& x, u16& y) {
	adjustAddonX(addonId, sourceId, x);
	adjustAddonY(addonId, sourceId, y);
}


//-------- Map information --------//

const u32 Func_GetGroundHeightAtPos = 0x004BD0F0;
u32 getGroundHeightAtPos(s32 x, s32 y) {

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

//-------- Moving Units --------//

bool moveUnit(CUnit* unit, s16 x, s16 y) {
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
	}
	else {
		setUnitPosition(unit, prevPos.x, prevPos.y);
		return false;
	}

}

const u32 Func_DisableDialog = 0x00418640;
void disableDialog(BinDlg* dialog) {

	__asm {
		PUSHAD
		MOV ESI, dialog
		CALL Func_DisableDialog
		POPAD
	}

}

;

const u32 Func_ShowDialog = 0x004186A0;
void showDialog(BinDlg* dialog) {

	__asm {
		PUSHAD
		MOV ESI, dialog
		CALL Func_ShowDialog
		POPAD
	}

}

;

const u32 Func_HideDialog = 0x00418700;
void hideDialog(BinDlg* dialog) {

	__asm {
		PUSHAD
		MOV ESI, dialog
		CALL Func_HideDialog
		POPAD
	}

}

;

const u32 Func_Sub418E00 = 0x00418E00;
void function_418E00(BinDlg* dialog) {

	__asm {
		PUSHAD
		MOV ESI, dialog
		CALL Func_Sub418E00
		POPAD
	}

}

;

const u32 Func_UpdateDialog = 0x0041C400;
void updateDialog(BinDlg* dialog) {
	__asm {
		PUSHAD
		MOV EAX, dialog
		CALL Func_UpdateDialog
		POPAD
	}
}

;

const u32 Func_PrepareUnitMoveClearRefs = 0x00493CA0;
void prepareUnitMove(CUnit* unit, bool hideUnit) {

	assert(unit);

	static Bool32 _hideUnit;

	_hideUnit = hideUnit ? 1 : 0;

	__asm {
		PUSHAD
		PUSH _hideUnit
		MOV EDI, unit
		CALL Func_PrepareUnitMoveClearRefs
		POPAD
	}

}

const u32 Func_CheckUnitCollisionPos = 0x0049D3E0;
bool checkUnitCollisionPos(CUnit* unit, const Point16* inPos, Point16* outPos, Box16* moveArea, bool hideErrorMsg, u32 someFlag) {
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

const u32 Func_SetUnitPosition = 0x004EB9F0;
void setUnitPosition(CUnit* unit, u16 x, u16 y) {
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

const u32 Func_RefreshRevealUnitAfterMove = 0x00494160;
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

const u32 Func_CreateUnitAtPos = 0x004CD360; //AKA createUnitXY()
CUnit* createUnitAtPos(u16 unitType, u16 playerId, u32 x, u32 y) {

	if (unitType >= UNIT_TYPE_COUNT) 
		return NULL;

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

const u32 Func_fixTargetLocation = 0x00401FA0;
void fixTargetLocation(Point16* coords, u32 unitId) {

	__asm {
		PUSHAD
		MOV EAX, unitId
		MOV EDX, coords
		CALL Func_fixTargetLocation
		POPAD
	}
}

const u32 Func_CreateUnit = 0x004A09D0;
CUnit* CreateUnit(u32 unitId, int x, int y, u32 playerId) {
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
const u32 Func_Sub4A01F0 = 0x004A01F0;
void FinishUnit_Pre(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub4A01F0
		POPAD
	}
}
const u32 Func_UpdateUnitStrength = 0x0049FA40;
void updateUnitStrength(CUnit* unit) {
	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_UpdateUnitStrength
		POPAD
	}
}
const u32 Func_AI_TrainingUnit = 0x004A2830;
void InheritAI2(CUnit* result, CUnit* parent) {
	__asm {
		PUSHAD
		MOV ECX, parent
		MOV EAX, result
		CALL Func_AI_TrainingUnit
		POPAD
	}
};
CUnit* createUnitIngame(u16 unitId, u8 playerId, u32 x, u32 y, CUnit* creator) {
	Point16 pos;
	CUnit* unit;
	pos.x = x;
	pos.y = y;
	fixTargetLocation(&pos, unitId);
	unit = CreateUnit(unitId, pos.x, pos.y, playerId);
	if (unit == NULL) {
		displayLastNetErrForPlayer(playerId);
	}
	else {
		FinishUnit_Pre(unit);
		updateUnitStrength(unit);
		if (creator != NULL && creator->pAI != NULL) {
			InheritAI2(unit, creator);
		}
	}
	return unit;
}
const u32 Func_displayLastNetErrForPlayer = 0x0049E530;
void displayLastNetErrForPlayer(u32 playerId) {
	__asm {
		PUSHAD
		PUSH playerId
		CALL Func_displayLastNetErrForPlayer
		POPAD
	}
}
u32 getUnitOverlayAdjustment(CUnit* const unit) {
	if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
		return 1;
	else if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
		return 2;
	else
		return 0;
}

const u32 Func_OrdersHoldPositionSuicidal = 0x004EB5B0;
void makeToHoldPosition(CUnit* unit) {

	__asm {
		PUSHAD
		MOV ESI, unit
		CALL Func_OrdersHoldPositionSuicidal
		POPAD
	}
}
void refreshScreen(int left, int top, int right, int bottom) {

	left	>>= 4; right	= (right	+ 15) >> 4;
	top	 >>= 4; bottom = (bottom + 15) >> 4;

	if (left > right)
		std::swap(left, right);
	if (top > bottom)
		std::swap(top, bottom);

	//Rect out of bounds
	if (left >= 40 || right < 0 || top >= 30 || bottom < 0)
		return;

	left	= std::max(left,	0); right	 = std::min(right,	 40 - 1);
	top	 = std::max(top,	 0); bottom	= std::min(bottom,	30 - 1);

	for (int y = top; y <= bottom; ++y)
		memset(&refreshRegions[40 * y + left], 1, right - left + 1);

}

void refreshScreen() {
	memset(refreshRegions, 1, 1200);
}

/* /!\ Not working like 004DC550	RandBetween /!\ */
u32 randBetween(u32 min, u32 max) {
	assert(min <= max);
	return min + ((max - min + 1) * random() >> 15);
}

//similar to 004DC4A0 RandomizeShort, but don't update
//the RNG and don't use a parameter among possible
//differences
u16 random() {
	if (*IS_IN_GAME_LOOP) {
		*lastRandomNumber = 22695477 * (*lastRandomNumber) + 1;
		return (*lastRandomNumber >> 16) % 32768;	//Make a number between 0 and 32767
	}
	else
		return 0;
}

//-------- Needs research --------//

	//Logically equivalent to function @ 0x004C36C0
	void refreshConsole() {

		static u32*	const bCanUpdateCurrentButtonSet		= (u32*)		0x0068C1B0;
		static u8*	 const bCanUpdateSelectedUnitPortrait	= (u8*)			0x0068AC74;
		static u8*	 const bCanUpdateStatDataDialog			= (u8*)			0x0068C1F8;
		static BinDlg**	const someDialogUnknown				= (BinDlg**)	0x0068C1E8;
		static BinDlg**	const someDialogUnknownUser			= (BinDlg**)	0x0068C1EC;

		*bCanUpdateCurrentButtonSet = 1;
		*bCanUpdateSelectedUnitPortrait = 1;
		*bCanUpdateStatDataDialog = 1;
		*someDialogUnknown = NULL;
		*someDialogUnknownUser = NULL;

	}

	const u32 Func_MoveScreen = 0x0049C440;
	//Player check added based on how others functions test it
	//Values above or equal to 8 means all players
	//Equivalent to moveScreenToUnit @ 0x004E6020
	void MoveScreenToUnit(CUnit* unit, u32 playerId) {

		if(playerId >= 8 || playerId == *LOCAL_NATION_ID) {

			int x,y;

			x = (s16)unit->sprite->position.x;
			y = (s16)unit->sprite->position.y;

			if(x < 0) 
				x += 31;
			if(y < 0) 
				y += 31;

			x = ((x/32) - 10) * 32;
			y = ((y/32) - 6) * 32;

			__asm {
				PUSHAD
				MOV ECX, y
				MOV EAX, x
				CALL Func_MoveScreen
				POPAD
			}

		}

	}

	const u32 Func_minimapPing = 0x004A34C0;
	//Player check added based on how others functions test it
	//Values above or equal to 8 means all players
	//x and y are coords as gotten from unit->sprite->position
	void minimapPing(u32 x, u32 y, s32 color, u32 playerId) {

		if(playerId >= 8 || playerId == *LOCAL_NATION_ID)
			__asm {
				PUSHAD
				PUSH color
				PUSH y
				PUSH x
				CALL Func_minimapPing
				POPAD
			}

	}

	const u32 Func_hasOverlay = 0x0047B720;
	u8 hasOverlay(CUnit* unit) {

		static u8 result;

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_hasOverlay
			MOV result, AL
			POPAD
		}

		return result;

	}

	void createUnitFromAbility(int unitId, int count, u16 playerId, s32 x, s32 y, u32 removeTimer, CUnit* creator) {
		for (int i = 0; i < count; i++) {
			CUnit* unit = scbw::createUnitIngame(unitId, playerId, x, y, creator);
			if (unit != nullptr) {
				if (unitId == UnitId::ZergBroodling
				 || unitId == UnitId::ZergBroodlisk) { // Parasite & Incubators
					scbw::playSound(SoundId::Zerg_BROODLING_ZBrRdy00_WAV, unit);
					if (unit->removeTimer == 0 || unit->removeTimer > removeTimer) {
						unit->removeTimer = removeTimer;
					}
					u32 range = 6 * 32;
					scbw::UnitFinder unitFinder(unit->getX() - range,
						unit->getY() - range,
						unit->getX() + range,
						unit->getY() + range);
					unitFinder.forEach(gameteMeiosisProc(unit));
				}
				else if (unitId == UnitId::ProtossSimulacrum) {
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

	void rangeModifiedFire(u8 weaponId, CUnit* unit, s16 x, s16 y, u32 offset) {
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
		unit->orderTarget.unit = NULL;
		unit->orderTarget.pt.x = polar_x;
		unit->orderTarget.pt.y = polar_y;
		createBullet(weaponId, unit, x, y, unit->playerId, unit->currentDirection1);
		unit->orderTarget.unit = target_unit;
		unit->orderTarget.pt = target_pos;
	}

	bool canSpawnBroodling(CUnit* target) {
		if (units_dat::GroupFlags[target->id].isBuilding) return false;
		if (units_dat::SupplyProvided[target->id] > 0) return false;
		return true;
	}

	int getBroodlingCountFromSupply(CUnit* target) {
		if (!canSpawnBroodling(target)) return 0;
		bool ground = !(target->status & UnitStatus::InAir);
		bool onCreep = scbw::getActiveTileAt(target->getX(), target->getY()).hasCreep;
		double multiplier = (ground && onCreep) ? 1.5 : 1;
		return ceil(std::max(((int) units_dat::SupplyRequired[target->id]), 1) * multiplier);
	}


	const u32 Func_unitDeathSomething_0 = 0x004E6340;
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

	u32 get_sized_overlay(CUnit* unit, u32 initial) {
		u32 overlayImageId;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
			overlayImageId = initial + 1;
		else
			if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
				overlayImageId = initial + 2;
			else
				overlayImageId = initial;
		return overlayImageId;
	}

	bool canUseCliffAdvantage(CUnit* unit, u8 weapon) {
		return unit != nullptr
			&& !(unit->status & UnitStatus::InAir)
			&& unit->id != UnitId::ProtossPlasmaShell
			&& (weapon == NULL || weapons_dat::MaxRange[weapon] > 32)
			&& !(unit->status & UnitStatus::InBuilding)
			&& unit->id != UnitId::TerranHarakan;
	}

	s32 getCliffAdvantage(CUnit* unit, u8 weapon) {
		if (unit == nullptr)
			return 0;
		if (!canUseCliffAdvantage(unit, weapon))
			return 0;
		CUnit* targetUnit = unit->orderTarget.unit;
		if (targetUnit == nullptr)
			return 0;
		//if (targetUnit->status & UnitStatus::InAir)
		//	return 0;
		s32 sourceHeight = scbw::getActiveTileAt(unit->position.x, unit->position.y).groundHeight;
		s32 targetHeight = scbw::getActiveTileAt(targetUnit->position.x, targetUnit->position.y).groundHeight;
		return (sourceHeight - targetHeight) / 2;
	}

	s32 getCreepAdvantage(CUnit* unit, u8 weapon) {
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

/*	void restoreFunc(CUnit* unit) {
		u32 overlayImageId;
		CUnit* overlayTargetUnit;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
			overlayImageId = ImageId::RestorationHit_Medium;
		else if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
			overlayImageId = ImageId::RestorationHit_Large;
		else
			overlayImageId = ImageId::RestorationHit_Small;
		if (unit->subunit != NULL)
			overlayTargetUnit = unit->subunit;
		else
			overlayTargetUnit = unit;
		(overlayTargetUnit->sprite)->createTopOverlay(overlayImageId, 0, 0, 0);

		//remove status effects and corresponding overlays
		//timers set to 0 twice reflect the original code

		// Remove Parasite
		if (scbw::get_generic_timer(unit, ValueId::ParasiteArmTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::ParasiteArmTimer, 0);
			unit->removeOverlay(1031);
		}
		scbw::set_generic_unit(unit, ValueId::ParasiteSource, nullptr);

		// Remove default flags
		unit->parasiteFlags = 0;
		unit->isBlind = 0;

		// Remove Ensnare
		if (unit->ensnareTimer != 0) {
			unit->removeOverlay(ImageId::EnsnareOverlay_Small, ImageId::EnsnareOverlay_Large);
			unit->ensnareTimer = 0;
			//specific update following ensnare removal
			unit->updateSpeed();
		}

		// Remove Plague
		if (unit->plagueTimer != 0) {
			unit->removeOverlay(ImageId::PlagueOverlay_Small, ImageId::PlagueOverlay_Large);
			unit->plagueTimer = 0;
		}

		// Remove Irradiate
		if (unit->irradiateTimer != 0) {
			unit->removeOverlay(ImageId::Irradiate_Small, ImageId::Irradiate_Large);
			unit->irradiateTimer = 0;

			//specific updates following irradiate removal
			unit->irradiatedBy = NULL;
			unit->irradiatePlayerId = 8;
		}

		// Remove armor rend (generic)
		scbw::set_generic_value(unit, ValueId::ArmorReduce, 0);

		// Remove Entropy Gauntlets
		if (scbw::get_generic_timer(unit, ValueId::EntropyGauntlets) > 0) {
			scbw::set_generic_timer(unit, ValueId::EntropyGauntlets, 1);
		}

		// Remove Malediction
		if (scbw::get_generic_timer(unit, ValueId::MaledictionSlow) > 0) {
			scbw::set_generic_timer(unit, ValueId::MaledictionSlow, 1);
		}

		// Remove Observance
		if (scbw::get_generic_timer(unit, ValueId::ObservanceTimerDebuff) > 0) {
			scbw::set_generic_timer(unit, ValueId::ObservanceTimerDebuff, 1);
		}

		// Remove Knockout Drivers
		if (scbw::get_generic_timer(unit, ValueId::KnockoutDriversTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::KnockoutDriversTimer, 1);
		}

		// Remove Hallucination
		if (scbw::get_generic_timer(unit, ValueId::HallucinatingTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::HallucinatingTimer, 1);
		}

		// Remove Captivating Claws
		if (scbw::get_generic_timer(unit, ValueId::CaptivatingClawsTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::CaptivatingClawsTimer, 1);
		}

		// Remove Lobotomy Mine slow
		if (scbw::get_generic_timer(unit, ValueId::LobotomySlow) > 0) {
			scbw::set_generic_timer(unit, ValueId::LobotomySlow, 1);
		}

		// Remove Lockdown
		unit->removeLockdown();

		// Remove Maelstrom
		unit->removeMaelstrom();

		// Remove Devourer Acid Spores
		unit->removeAcidSpores();

		// Remove Deconstruction
		scbw::set_generic_value(unit, ValueId::Deconstruction, 0);
		unit->removeOverlay(1332);
		if (unit->subunit) {
			scbw::set_generic_value(unit->subunit, ValueId::Deconstruction, 0);
			unit->subunit->removeOverlay(1332);
		}

		//was hardcoded in original code
		scbw::refreshConsole();
	}

	void refreshDebuffs(CUnit* unit) {
		// NOTE: classic timer formula is (([real seconds / 0.042) / 8)
		// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

		// Reset Ensnare
		if (unit->ensnareTimer != 0) {
			unit->ensnareTimer = 37;
			//specific update following ensnare removal
			unit->updateSpeed();
		}

		// Refresh Plague
		if (unit->plagueTimer != 0) {
			unit->plagueTimer = 30;
		}

		// Reset Malediction
		if (scbw::get_generic_timer(unit, ValueId::MaledictionSlow) > 0) {
			scbw::set_generic_timer(unit, ValueId::MaledictionSlow, 4*24);
		}

		// Reset Observance
		if (scbw::get_generic_timer(unit, ValueId::ObservanceTimerDebuff) > 0) {
			scbw::set_generic_timer(unit, ValueId::ObservanceTimerDebuff, 5*24);
		}

		// Reset Knockout Drivers
		if (scbw::get_generic_timer(unit, ValueId::KnockoutDriversTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::KnockoutDriversTimer, 2*24);
		}

		// Reset Hallucination
		if (scbw::get_generic_timer(unit, ValueId::HallucinatingTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::HallucinatingTimer, 20*24);
		}

		// Reset Captivating Claws
		if (scbw::get_generic_timer(unit, ValueId::CaptivatingClawsTimer) > 0) {
			scbw::set_generic_timer(unit, ValueId::CaptivatingClawsTimer, 1*24);
		}

		// Reset Lobotomy Mine slow
		if (scbw::get_generic_timer(unit, ValueId::LobotomySlow) > 0) {
			scbw::set_generic_timer(unit, ValueId::LobotomySlow, 6*24);
		}

		// Reset Lockdown
		if (unit->lockdownTimer != 0)
			unit->lockdownTimer = 18;

		// Reset Maelstrom
		if (unit->maelstromTimer != 0)
			unit->maelstromTimer = 12;

		// Reset Devourer Acid Spores
		if (unit->acidSporeCount != 0)
			unit->acidSporeTime[unit->acidSporeCount] = 30;

		// Reset Irradiate
		if (unit->irradiateTimer != 0)
			unit->irradiateTimer = 24;

		//was hardcoded in original code
		scbw::refreshConsole();
	}

	void inheritDebuffs(CUnit* parent, CUnit* child) {
		// NOTE: classic timer formula is (([real seconds / 0.042) / 8)
		// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

		// Get overlay info
		s32 overlayOffset = getOverlayOffset(child->id);

		// Inherit Ensnare
		if (parent->ensnareTimer != 0) {
			child->ensnareTimer = parent->ensnareTimer;
			if (!(child->sprite->getOverlay(ImageId::EnsnareOverlay_Small + overlayOffset))) {
				child->sprite->createOverlay(ImageId::EnsnareOverlay_Small + overlayOffset);
			}
			child->updateSpeed();
		}

		// Inherit Plague
		if (parent->plagueTimer != 0) {
			child->plagueTimer = parent->plagueTimer;
			if (!(child->sprite->getOverlay(ImageId::PlagueOverlay_Small + overlayOffset))) {
				child->sprite->createOverlay(ImageId::PlagueOverlay_Small + overlayOffset);
			}
		}

		// Inherit Parasite
		if (scbw::get_generic_unit(parent, ValueId::ParasiteSource) != nullptr) {
			scbw::set_generic_unit(child, ValueId::ParasiteSource, scbw::get_generic_unit(parent, ValueId::ParasiteSource));
		}

		// Inherit armor rend (generic)
		if (scbw::get_generic_value(parent, ValueId::ArmorReduce) > 0) {
			scbw::set_generic_value(child, ValueId::ArmorReduce, scbw::get_generic_value(parent, ValueId::ArmorReduce));
		}

		// Inherit Malediction
		if (scbw::get_generic_timer(parent, ValueId::MaledictionSlow) > 0) {
			scbw::set_generic_timer(child, ValueId::MaledictionSlow, scbw::get_generic_timer(parent, ValueId::MaledictionSlow));
		}

		// Inherit Observance
		if (scbw::get_generic_timer(parent, ValueId::ObservanceTimerDebuff) > 0) {
			scbw::set_generic_timer(child, ValueId::ObservanceTimerDebuff, scbw::get_generic_timer(parent, ValueId::ObservanceTimerDebuff));
		}

		// Inherit Knockout Drivers
		if (scbw::get_generic_timer(parent, ValueId::KnockoutDriversTimer) > 0) {
			scbw::set_generic_timer(child, ValueId::KnockoutDriversTimer, scbw::get_generic_timer(parent, ValueId::KnockoutDriversTimer));
		}

		// Inherit Hallucination
		if (scbw::get_generic_timer(parent, ValueId::HallucinatingTimer) > 0) {
			scbw::set_generic_timer(child, ValueId::HallucinatingTimer, scbw::get_generic_timer(parent, ValueId::HallucinatingTimer));
		}

		// Inherit Captivating Claws
		if (scbw::get_generic_timer(parent, ValueId::CaptivatingClawsTimer) > 0) {
			scbw::set_generic_timer(child, ValueId::CaptivatingClawsTimer, scbw::get_generic_timer(parent, ValueId::CaptivatingClawsTimer));
		}

		// Inherit Lobotomy Mine slow
		if (scbw::get_generic_timer(parent, ValueId::LobotomySlow) > 0) {
			scbw::set_generic_timer(child, ValueId::LobotomySlow, scbw::get_generic_timer(parent, ValueId::LobotomySlow));
		}

		// Inherit Lockdown
		if (parent->lockdownTimer != 0)
			child->lockdownTimer = parent->lockdownTimer;

		// Inherit Maelstrom
		if (parent->maelstromTimer != 0)
			child->maelstromTimer = parent->maelstromTimer;

		// Inherit Devourer Acid Spores
		if (parent->acidSporeCount != 0)
			child->acidSporeCount = parent->acidSporeCount;

		// Inherit Irradiate
		if (parent->irradiateTimer != 0)
			child->irradiateTimer = parent->irradiateTimer;

		// Inherit Defensive Matrix
		if (parent->defensiveMatrixTimer > 0) {
			child->defensiveMatrixHp = parent->defensiveMatrixHp;
			child->defensiveMatrixTimer = parent->defensiveMatrixTimer;
		}

		// Inherit Stasis Field
		if (parent->stasisTimer > 0) {
			child->stasisTimer = parent->stasisTimer;
		}

		// Inherit Deconstruction
		if (scbw::get_generic_value(parent, ValueId::Deconstruction) > 0) {
			scbw::set_generic_value(child, ValueId::Deconstruction, scbw::get_generic_value(parent, ValueId::Deconstruction));
			if (child->subunit != nullptr) {
				scbw::set_generic_value(child->subunit, ValueId::Deconstruction, scbw::get_generic_value(parent, ValueId::Deconstruction));
			}
			if (!(child->sprite->getOverlay(1332 + overlayOffset))) {
				child->sprite->createOverlay(1332 + overlayOffset);
			}
		}

		// Inherit Entropy Gauntlets
		if (scbw::get_generic_timer(parent, ValueId::EntropyGauntlets) > 0) {
			scbw::set_generic_timer(child, ValueId::EntropyGauntlets, scbw::get_generic_timer(parent, ValueId::EntropyGauntlets));
			if (!(child->sprite->getOverlay(1107 + overlayOffset))) {
				child->sprite->createOverlay(1107 + overlayOffset);
			}
		}

		// Inherit Sublime Shepherd
		if (scbw::get_generic_value(parent, ValueId::SublimeShepherd) > 0) {
			scbw::set_generic_value(child, ValueId::SublimeShepherd, scbw::get_generic_value(parent, ValueId::SublimeShepherd));
		}

		// Inherit Cowardice

		// Inherit Malice

		// Inherit Terminal Surge
		if (scbw::get_generic_value(parent, ValueId::TerminalSurge) > 0) {
			scbw::set_generic_value(child, ValueId::TerminalSurge, scbw::get_generic_value(parent, ValueId::TerminalSurge));
		}

		// Inherit Astral Blessing
		if (scbw::get_generic_timer(parent, ValueId::EcclesiastTimer) > 0) {
			scbw::set_generic_value(child, ValueId::EcclesiastStack, scbw::get_generic_value(parent, ValueId::EcclesiastStack));
			scbw::set_generic_value(child, ValueId::EcclesiastPlayer, scbw::get_generic_value(parent, ValueId::EcclesiastPlayer));
			scbw::set_generic_timer(child, ValueId::EcclesiastTimer, scbw::get_generic_timer(parent, ValueId::EcclesiastTimer));
		}

		// Inherit Stellar Enforcement
		if (scbw::get_generic_value(parent, ValueId::StellarEnforcement) > 0) {
			scbw::set_generic_value(child, ValueId::StellarEnforcement, get_generic_value(parent, ValueId::StellarEnforcement));
		}

		// Inherit Adrenal Frenzy
		if (scbw::get_generic_timer(parent, ValueId::NathrokorTimer) > 0) {
			scbw::set_generic_timer(child, ValueId::NathrokorTimer, scbw::get_generic_timer(parent, ValueId::NathrokorTimer));
		}

		// Inherit Swarming Omen
		if (scbw::get_generic_timer(parent, ValueId::SwarmingOmenTimer) > 0) {
			scbw::set_generic_value(child, ValueId::SwarmingOmenStatus, scbw::get_generic_value(parent, ValueId::SwarmingOmenStatus));
			scbw::set_generic_timer(child, ValueId::SwarmingOmenTimer, scbw::get_generic_timer(parent, ValueId::SwarmingOmenTimer));
		}

		// Inherit Kaiser Rampage


		//was hardcoded in original code
		scbw::refreshConsole();
	}

	bool isDebuffed(CUnit* unit) {
		return scbw::get_generic_timer(unit, ValueId::MaledictionSlow) > 0
			|| scbw::get_generic_timer(unit, ValueId::KnockoutDriversTimer) > 0
			|| scbw::get_generic_timer(unit, ValueId::CaptivatingClawsTimer) > 0
			|| scbw::get_generic_timer(unit, ValueId::LobotomySlow) > 0
			|| scbw::get_generic_timer(unit, ValueId::ObservanceTimerDebuff) > 0
			|| scbw::get_generic_value(unit, ValueId::EntropyGauntlets) > 0
			|| scbw::get_generic_value(unit, ValueId::ArmorReduce) > 0
			|| unit->irradiateTimer != 0
			|| unit->ensnareTimer != 0
			|| unit->lockdownTimer != 0
			|| unit->maelstromTimer != 0
			|| unit->acidSporeCount != 0
			|| unit->plagueTimer != 0
			|| unit->parasiteFlags
			|| unit->isBlind;
		// TODO: cowardice, malice + also add them to status tooltip
	}
	*/
	void applyEnsnare(CUnit* unit) { // 004F45E0
		s32 baseProperty; // eax
		u8 overlayPadding; // al
		CUnit* victim; // ecx

		if (!(unit->ensnareTimer)
			&& !(unit->status & UnitStatus::Burrowed)) {

			// Calculate overlay ID offset based on units_dat flags
			overlayPadding = 0;
			baseProperty = units_dat::BaseProperty[unit->id];
			if (baseProperty & UnitProperty::MediumOverlay) {
				overlayPadding = 1;
			}
			else if (baseProperty & UnitProperty::LargeOverlay) {
				overlayPadding = 2;
			}
			victim = unit;
			if (unit->subunit != NULL) {
				victim = unit->subunit;
			}

			// Create overlay
			victim->sprite->createOverlay(ImageId::EnsnareOverlay_Small + overlayPadding);
		}

		// Apply effect
		unit->ensnareTimer = 37;
		unit->updateSpeed();
	}

	bool isEnabled(CUnit* unit) {
		if (unit->lockdownTimer
		||  unit->maelstromTimer
		||  unit->stasisTimer
		||  unit->status & UnitStatus::Disabled) {
			return false;
		}
		return true;
	}

	s32 getUpgradeType(CUnit* unit) {
		s32 race = unit->getRace();
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

	s32 getOverlayOffset(u16 unitId) {
		u32 overlayOffset = 0;
		if (units_dat::BaseProperty[unitId] & UnitProperty::MediumOverlay) {
			overlayOffset = 1;
		}
		else if (units_dat::BaseProperty[unitId] & UnitProperty::LargeOverlay) {
			overlayOffset = 2;
		}
		return overlayOffset;
	}

	s32 getFinalDamage(CUnit* unit, u16 weaponId) {
		s32 upgType = getUpgradeType(unit);
		s32 baseDamage = weapons_dat::DamageAmount[weaponId];
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
			||	  unit->id == UnitId::ZergLakizilisk); // LAME
	}

	// Identical to function OrderIsSpell @ 00436930 -- Pr0nogo
	// We should update the lardcode for HYDRA
	bool orderIsSpell(s32 orderId) {
		return orderId == OrderId::Plague
			|| orderId == OrderId::DarkSwarm
			|| orderId == OrderId::Ensnare
			|| orderId == OrderId::Irradiate
			|| orderId == OrderId::StasisField
			|| orderId == OrderId::EmpShockwave
			|| orderId == OrderId::MagnaPulse
			|| orderId == OrderId::PsiStorm
			|| orderId == OrderId::SummonBroodlings
			|| orderId == OrderId::FireYamatoGun1;
	}



	bool canBeParasited(CUnit* unit) {
		return !(units_dat::BaseProperty[unit->id] & UnitProperty::Building)
			&& !(unit->status & UnitStatus::Burrowed);
	}

/*	void applyParasite(CUnit* unit, u8 playerID, CUnit* queen) {
		if (!scbw::canBeParasited(unit)) return;
		int playerFlag = 1 << playerID;
		if (unit->parasiteFlags != playerFlag) { // <- Prevent overwriting timer for already parasited units by same player
			scbw::set_generic_timer(unit, ValueId::ParasiteArmTimer, DefaultArmParasiteTimer);
			unit->sprite->createOverlay(1031, 0, 0, 0);
		}
		scbw::set_generic_unit(unit, ValueId::ParasiteSource, queen);
		unit->parasiteFlags = playerFlag; // Parasite now overwrites other player's parasite
		scbw::refreshConsole();
	}
	*/
} //scbw