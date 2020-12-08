#include "weapon_effect.h"

#include <SCBW/UnitFinder.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include "hooks/weapons/wpnspellhit.h"
#include "globals.h"
namespace {
  bool isUnitDistanceWithin(CUnit* target, CBullet* bullet, u32 radius);
  void weaponBulletShot(CUnit* target, CBullet* bullet, u32 dmgDivisor);
  void doWeaponHit(CUnit* target, CUnit* attacker, u8 dmgDivisor);
  void Ai_UnitWasHit(u32 main_target_reaction, u32 dont_call_help, CUnit* attacker, CUnit* target);

  // SC internal data for lurker spines
  struct SpineHitTable {
    struct {
      CUnit* source;
      CUnit* target;
    } hits[32][16];
  };
  SpineHitTable* const spineHitTable = (SpineHitTable*)(0x0064DEC8);
  u32* const spineHitRow = (u32*)(0x64EEC8);
  u32* const spineHitCol = (u32*)(0x64DEA8);
}//unnamed namespace

namespace hooks {

   void darkSwarmDamage(u16 x, u16 y, u16 r, CBullet* bullet, u32 weaponId) {
	    CUnit* dsw = scbw::getDarkSwarm(x,y,r);
	    if(dsw!=NULL){
			//scbw::printText("Dark swarm detected");
			u32 damage = (weapons_dat::DamageAmount[weaponId]*256)+
					(weapons_dat::DamageBonus[weaponId]*256*scbw::getUpgradeLevel(bullet->srcPlayer,weapons_dat::DamageUpgrade[weaponId]));
			if(bullet->hitFlags & 2){
				//hallucination
				damage /= 2;
			}
			if(damage>=dsw->hitPoints){
				dsw->remove();
				//scbw::printText("Destroy dark swarm");
			}
			else {
				u32 old = dsw->hitPoints;
				dsw->hitPoints-=damage;
				//char buf[64];
				//sprintf(buf,"Dark swarm hit: %d --> %d",old,dsw->hitPoints);
				//scbw::printText(buf);
			}
	    }
   }
   bool nathrokorCondition(CBullet* bullet, CUnit* target) {
	   if (bullet->sourceUnit == NULL || target==NULL) {
		   return true;
	   }
	   if (bullet->weaponType == WeaponId::PlasmidBreath) {
		   CUnit* nathrokor = bullet->sourceUnit;
		   u16 angle = scbw::getAngle(nathrokor->position.x, nathrokor->position.y,
			   target->position.x, target->position.y);
		   u16 unit_angle = nathrokor->velocityDirection1;
		   int arc = (double)15 / 1.40625;
		   //30 degrees
		   int delta = unit_angle - angle;
		   delta = abs((delta + 128) % 256 - 128);
		   if (delta > arc) {
			   return false;
		   }
		   else {
		   }
	   }
	   return true;
   }
	void friendlySplash(CBullet* bullet, u32 weaponId, u8 splash_type, CUnit* guaranteed_hit){
		int splash = splash_type;
		if (bullet->sourceUnit != NULL && bullet->sourceUnit->sprite!=NULL) {
			if (bullet->sourceUnit->isBlind) {
				splash = SplashType::Radial;
			}
			
			//must be moved to aise
			if (bullet->sourceUnit->id == UnitId::TerranSalamander) {
				auto hit = bullet->attackTarget.pt;
				if (bullet->attackTarget.unit != NULL) {
					hit = bullet->attackTarget.unit->position;
				}
				Globals.createSalamanderFire(hit.x, hit.y, 11, 5, true);
			}
/*			if (bullet->sourceUnit->id == WeaponId::IonCannon) {
				auto hit = bullet->attackTarget.pt;
				if (bullet->attackTarget.unit != NULL) {
					hit = bullet->attackTarget.unit->position;
				}
				Globals.createIonField(hit.x, hit.y, 11, 5, true);
			}*/
		}
		
		s16 x,y,r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::OuterSplashRadius[weaponId];

		// Hammerfall Shells - Siege Tank upgrade
		if (weaponId == WeaponId::ArcliteShockCannon && scbw::getUpgradeLevel(bullet->srcPlayer, UpgradeId::HammerfallShells)) {
			r += r / 3;
		}
		
		// Safety Off - Cyprian ability
		if (isUnsafeCyprianBullet(bullet,weaponId)){
			r = 40;
		}
		// Probing Incisors - Gorgoleth passive
		if (isChargedGorgolethBullet(bullet, weaponId)) {
			r = 45;
		}
		if (weaponId == WeaponId::PlasmidBreath) {
			r = 32;
		}

		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(x-r, y-r, x+r, y+r);
		//
		darkSwarmDamage(x, y, r, bullet, weaponId);
		//

		bool hasGuaranteedHitUnit = guaranteed_hit != NULL;
		if (hasGuaranteedHitUnit && nathrokorCondition(bullet, bullet->attackTarget.unit)) {
			weaponBulletShot(guaranteed_hit, bullet, 1);
		}
		int divisor = 1;
		if (weaponId == WeaponId::DispersiveSalvo) {
			divisor = Globals.get_divider(bullet);
		}
		for (int i = 0; i < unitsInSplash.getUnitCount(); i++){
			CUnit* target = unitsInSplash.getUnit(i);
			if (hasGuaranteedHitUnit && target == guaranteed_hit) { // skips target check if hasGuaranteedHitUnit is unset, just for little bit of performance
				continue; // guaranteed hit unit was already damaged before splash!
			}

			// Units can't splash themselves -- except psi storm
			if (target == bullet->sourceUnit && bullet->weaponType != WeaponId::PsiStorm) continue;
      
			// Don't hit allied units -- normally just checks target->playerId == bullet->srcPlayer (also checks if type is 'splash enemy' or 'nuclear missile')
	//      if(scbw::isUnitEnemy(bullet->srcPlayer, target) == false && target != bullet->attackTarget.unit) continue;

			if (splash==SplashType::Enemy){
				if(bullet->srcPlayer==target->playerId && target != bullet->attackTarget.unit) continue;
			}

			else if (splash==SplashType::FriendlyNew){
				if(scbw::isUnitEnemy(bullet->srcPlayer, target) == false && target != bullet->attackTarget.unit) continue;
			}    

			// Can the unit be targeted?
			if(!scbw::canWeaponTargetUnit(bullet->weaponType, target, target)) continue;
      
			// We found units in a square around the target, but is the target within the splash radius?
			s16 outer = weapons_dat::OuterSplashRadius[bullet->weaponType];

			// Hammerfall Shells - Siege Tank upgrade
			if (weaponId == WeaponId::ArcliteShockCannon && scbw::getUpgradeLevel(bullet->srcPlayer, UpgradeId::HammerfallShells)) {
				outer += outer / 3;
			}

			// Safety Off - Cyprian ability
			if(isUnsafeCyprianBullet(bullet,weaponId)){
				outer = 40;
			}
			// Probing Incisors - Gorgoleth passive
			if (isChargedGorgolethBullet(bullet, weaponId)) {
				outer = 45;
			}
			if (weaponId == WeaponId::PlasmidBreath) {
				outer = 32;
			}
			if(!isUnitDistanceWithin(target, bullet, outer)) continue;
      
			// Don't storm a unit that's already being stormed
			if(bullet->weaponType == WeaponId::PsiStorm){
				if(target->isUnderStorm) continue;
				target->isUnderStorm = 1;
			}
      
			// Inner radius 
			s16 inner = weapons_dat::InnerSplashRadius[bullet->weaponType];

			// Hammerfall Shells - Siege Tank upgrade
			if (weaponId == WeaponId::ArcliteShockCannon && scbw::getUpgradeLevel(bullet->srcPlayer, UpgradeId::HammerfallShells)) {
				inner += inner / 3;
			}

			// Safety Off - Cyprian ability
			if (isUnsafeCyprianBullet(bullet,weaponId)){
				inner = 15;
			}
			// Probing Incisors - Gorgoleth passive
			if (isChargedGorgolethBullet(bullet, weaponId)) {
				inner = 30;
			}

			if(isUnitDistanceWithin(target, bullet, inner) && nathrokorCondition(bullet,target)){
				weaponBulletShot(target, bullet, divisor); // 100% damage
				continue;
			}
      
			// Burrowed units can only be hit by inner radius
			if(target->status & UnitStatus::Burrowed) continue;
      
			// Medium radius
			s16 medium = weapons_dat::MediumSplashRadius[bullet->weaponType];

			// Hammerfall Shells - Siege Tank upgrade
			if (weaponId == WeaponId::ArcliteShockCannon && scbw::getUpgradeLevel(bullet->srcPlayer, UpgradeId::HammerfallShells)) {
				medium += medium / 3;
			}

			// Safety Off - Cyprian ability
			if(isUnsafeCyprianBullet(bullet,weaponId)){
				medium = 25;
			}
			// Probing Incisors - Gorgoleth passive
			if (isChargedGorgolethBullet(bullet, weaponId)) {
				medium = 37;
			}
			if (weaponId == WeaponId::PlasmidBreath) {
				r = 32;
			}
			if(isUnitDistanceWithin(target, bullet, medium) && nathrokorCondition(bullet, target)){
				weaponBulletShot(target, bullet, divisor*2); // 50% damage
				continue;
			}
      
			// Outer radius
			if (nathrokorCondition(bullet, target)) {
				weaponBulletShot(target, bullet, divisor*4); // 25% damage
			}
		}
	}

	void friendlyLurkerSplash(CBullet* bullet, u32 weaponId, u8 splash) {
		s16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::OuterSplashRadius[weaponId];

		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);

		for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
			CUnit* target = unitsInSplash.getUnit(i);

			// Units can't splash themselves
			if (target == bullet->sourceUnit) continue;

			// Don't hit allied units -- normally just checks target->playerId == bullet->srcPlayer
		//     if (scbw::isUnitEnemy(bullet->srcPlayer, target) == false && target != bullet->attackTarget.unit) continue;
		if(splash==SplashType::Enemy){
			if (bullet->srcPlayer==target->playerId && target != bullet->attackTarget.unit) continue;
		}
		else if(splash==SplashType::FriendlyNew){
			bool lurkerOverride = false;
			if (bullet->sourceUnit && (CUnit*)scbw::get_generic_value(bullet->sourceUnit, ValueId::LurkerPickTarget) == target) {
				lurkerOverride = true;
			}
			if (scbw::isUnitEnemy(bullet->srcPlayer, target) == false && !lurkerOverride && target != bullet->attackTarget.unit) {
				continue;
			}
		}    

			// Can the unit be targetted?
			if (!scbw::canWeaponTargetUnit(bullet->weaponType, target, target)) continue;

			// We found units in a square around the target, but is the target within the splash radius?
			if (!isUnitDistanceWithin(target, bullet, weapons_dat::InnerSplashRadius[bullet->weaponType])) continue;

			if (bullet->sourceUnit != NULL) {
				/*
				bool cont = false; // supposed to be a return within the loops, but since we can't it's a bool
				for (int r = 0; r < 32 && !cont; r++) {
					for (int c = 0; c < 16; c++) {
					if (spineHitTable->hits[r][c].source == bullet->sourceUnit && spineHitTable->hits[r][c].target == target) {
						cont = true; // unit has already been hit
						break;
					}
					}
				}
				if (cont) continue;

				if (*spineHitCol != 16) {
					spineHitTable->hits[*spineHitRow][*spineHitCol].source = bullet->sourceUnit;
					spineHitTable->hits[*spineHitRow][*spineHitCol].target = target;
					(*spineHitCol)++;
				}*/
				bool cont = false;
				if (Globals.spine_pair_exists(bullet->sourceUnit, target)) {
					cont = true;
					break;
				}
				if (cont)
					continue;
				Globals.add_lurker_spine_pair(bullet->sourceUnit, target);
			}
			weaponBulletShot(target, bullet, 1);
		}
	}

	bool isChargedGorgolethBullet(CBullet* bullet, u32 weaponId) {
		if (weaponId == WeaponId::SerratedCleavers) {
			if (bullet != NULL && bullet->sourceUnit != NULL && bullet->sourceUnit->id==UnitId::ZergGorgoleth) {
				if (bullet->sourceUnit->energy >= 5 * 256) {
					return true;
				}
			}
		}
		return false;
	}

	bool isUnsafeCyprianBullet(CBullet* bullet, u32 weaponId){
		if(weaponId==26){
			if(bullet!=NULL){
				if(bullet->sourceUnit!=NULL) {
					if (scbw::get_aise_value(bullet->sourceUnit,NULL,AiseId::SendCyprianValue,0,0)==1) {
						return true;
					}
				}
			}
		}
		return false;
	}

	const u32 Func_turnUnit = 0x00495F20;
	void turnUnit(CUnit* a1, char a2) {
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX BL, a2
			CALL Func_turnUnit
			POPAD
		}
	}

	// clone of 'Normal' damage for reference
	void normalDamage(CBullet* bullet, u32 weaponId) {
		if (bullet->sourceUnit!=NULL && weaponId == WeaponId::Lockdown) {
			CUnit* mine = scbw::createUnitIngame(UnitId::TerranLobotomyMine, bullet->srcPlayer, bullet->position.x, bullet->position.y);
			if (mine != NULL)
			{
				turnUnit(mine, bullet->sourceUnit->currentDirection1);
			}
		}
		//re-route cyprian bullet
		//if(bullet->sourceUnit != NULL && bullet->attackTarget.unit!=NULL)
		if(isUnsafeCyprianBullet(bullet,weaponId)) {
			friendlySplash(bullet, weaponId, SplashType::Radial);
			//scbw::printText("Cyprian - Execute splash instead of normal");
			return;
		}
		if (isChargedGorgolethBullet(bullet, weaponId)) {
			bullet->sourceUnit->spendUnitEnergy(5 * 256);
			friendlySplash(bullet, weaponId, SplashType::FriendlyNew, bullet->attackTarget.unit);
			return;
		}
		CUnit* target = bullet->attackTarget.unit;
		if (target == NULL) return;
		darkSwarmDamage(bullet->attackTarget.unit->position.x,bullet->attackTarget.unit->position.y,1,bullet,weaponId);
		u8 divisor = 1;
		if (weaponId == WeaponId::DispersiveSalvo) {
			divisor = Globals.get_divider(bullet);
		}
		else if (weaponId == WeaponId::ViscousAcid) {
//			divisor = Globals.get_multihit_count(bullet);
			divisor = 1;
//			scbw::printFormattedText("Get divisor %d, bullet: %X",divisor,bullet);
		}
		else if (weaponId == WeaponId::AcidSpore
		&&		bullet->sourceUnit
		&&		bullet->sourceUnit->sprite
		&&		target->sprite) {
			auto X = scbw::get_generic_value(bullet->sourceUnit, ValueId::GuardianMissileLaunchPosX);
			auto Y = scbw::get_generic_value(bullet->sourceUnit, ValueId::GuardianMissileLaunchPosY);
			if (X != 0 && Y != 0) {
				divisor = scbw::getDistanceFast(X, Y, target->getX(), target->getY()) / (32 * 3);
			}
		}
		if (bullet->hitFlags & 1) { // Miss ?
		  if (target->pAI == NULL) return;
		  if (bullet->sourceUnit == NULL) return;
		  doWeaponHit(target, bullet->sourceUnit, divisor);
		  return;
		}
		else {
			if (weapons_dat::Behavior[weaponId] == WeaponBehavior::Bounce) {
				bool mutalisk = true;
				bool bounces = true;
				if(bullet !=NULL && bullet->sourceUnit!=NULL){
					if (bullet->sourceUnit->id==129){
						if(bullet->remainingBounces<=1){
							mutalisk = false;
						}
						else {
							bounces = false;
						}
					}
				}
				if (bounces) {
					if (mutalisk) {
						int bounceCount = 2;
						divisor = bounceCount - bullet->remainingBounces;
						/*
						for (int i = 2 - bullet->remainingBounces; i != 0; i--) {
							divisor *= 3;
						}*/
						//1-3-9  2-1-0 - old
						//1-2-3  2-1-0 - new
					}
					else {
						divisor *= 3;
					}
				}        
			}	  
		weaponBulletShot(target, bullet, divisor);
		}
	}

  // clone of 'Yamato' damage for reference
  void yamatoDamage(CBullet* bullet, u32 weaponId) {
    if (bullet->attackTarget.unit == NULL) return;
    darkSwarmDamage(bullet->attackTarget.unit->position.x,bullet->attackTarget.unit->position.y,1,bullet,weaponId);
	weaponBulletShot(bullet->attackTarget.unit, bullet, 1);
  }

	class parasiteProc : public scbw::UnitFinderCallbackProcInterface {
	private:
		u8 _playerID;
		CUnit* _queen;
	public:
		parasiteProc(u8 playerID, CUnit* queen) {
			_playerID = playerID;
			_queen = queen;
		}

		void proc(CUnit* unit) {
			if (units_dat::BaseProperty[unit->id] & UnitProperty::Building) return;
			if (unit->status & UnitStatus::Burrowed) return;
			int playerFlag = 1 << _playerID;
			constexpr int defaultArmParasiteTimer = (24 * 2) + 1;
			if (unit->parasiteFlags != playerFlag) { // <- Prevent overwriting timer for already parasited units by same player
				scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::ParasiteArmTimer, defaultArmParasiteTimer, 0);
				unit->sprite->createOverlay(1031, 0, 0, 0);
			}
			scbw::sendGPTP_aise_cmd(unit, _queen, GptpId::SetGenericUnit, ValueId::ParasiteSource, 0, 0);
			unit->parasiteFlags = playerFlag; // Parasite now overwrites other players' parasite
			scbw::refreshConsole();
		}
	};

  void parasite(CBullet* bullet, u32 weaponId) {
	  if (bullet->sourceUnit == NULL) return;
      constexpr int radius = 96;
      int left = bullet->position.x - radius / 2;
      int right = bullet->position.x + radius / 2;
      int top = bullet->position.y - radius / 2;
      int bottom = bullet->position.y + radius / 2;
      scbw::UnitFinder(left, top, right, bottom).forEach(parasiteProc(bullet->sourceUnit->playerId, bullet->sourceUnit));
  }
  
  void broodlings(CBullet* bullet, u32 weaponId) {
	  if (bullet->sourceUnit == NULL) return;
	  CUnit* target = bullet->attackTarget.unit;
	// if (target != NULL) {
	//	  if (!target->isDead()) {
//			  Ai_UnitWasHit(1, 0, bullet->sourceUnit, target);
			 // BroodlingHit(bullet->sourceUnit, target,bullet);
	  hooks::BroodlingHit(bullet->sourceUnit, target, bullet);
	//	  }
	//  }
  }

	void lockdown(CBullet* bullet, u32 weaponId) {
	/*	CUnit* target = bullet->attackTarget.unit; 
		//original code 
		if (target) {
			if (!target->isDead()) {
				hooks::LockdownHit(target,0x83,bullet);
			}
		}*/
		auto targPos = bullet->targetPosition;
		s16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::OuterSplashRadius[WeaponId::LobotomyMine];
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);
		for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
			CUnit* target = unitsInSplash.getUnit(i);
			if (!target->isDead()
				&& !units_dat::GroupFlags[target->id].isBuilding
				&& !(units_dat::BaseProperty[target->id] & UnitProperty::ResourceContainer)
				&& target->id!=UnitId::TerranSpiderMine
				&& target->id!=UnitId::TerranLobotomyMine) {
					if (units_dat::BaseProperty[target->id] & UnitProperty::Organic) {
						scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericTimer, ValueId::LobotomySlow, 6 * 24, 0);
					}
					else {
						hooks::LockdownHit(target, 18, bullet);
					}
				}
			}
		}
}//hooks

namespace {
   /*** helper functions that I couldn't find in gptp ***/
	const u32 Func_0043F320 = 0x0043F320;
	void Ai_UnitWasHit(u32 main_target_reaction, u32 dont_call_help, CUnit* attacker, CUnit* target) {
		__asm {
			PUSHAD
			PUSH main_target_reaction
			PUSH dont_call_help
			MOV ECX,target
			MOV EDX,attacker
			CALL Func_0043F320
			POPAD
		}
	}

  // checks if the distance between target and bullet's positions is < radius
  const u32 Helper_IsUnitDistanceWithin = 0x0048ADB0;
  bool isUnitDistanceWithin(CUnit* target, CBullet* bullet, u32 radius){
    u32 result;
    __asm {
      PUSHAD
      PUSH radius;
      PUSH bullet;
      MOV ECX, target
      CALL Helper_IsUnitDistanceWithin
      MOV result, EAX
      POPAD
    }
    return result;
  }
  
  // creates attack overlay & deals damage from the bullet (just calls overlayAndNotify if the weapon deals no damage, or calls target->damageWith if it does
  const u32 Helper_WeaponBulletShot = 0x00479AE0;
  void weaponBulletShot(CUnit* target, CBullet* bullet, u32 dmgDivisor){

	if (bullet != NULL && target!=NULL && bullet->sourceUnit && bullet->weaponType == WeaponId::IconoclastBlades) {
		if (!Globals.was_hit_by_pariah(bullet->sourceUnit, target)) {
			Globals.register_pariah_hit(bullet->sourceUnit, target);
		}
		else {
			return;
		}
	}
    __asm {
		  PUSHAD
		  PUSH dmgDivisor
		  MOV EAX, target
		  MOV EDX, bullet
		  CALL Helper_WeaponBulletShot
		  POPAD
    }
  }

  const u32 Helper_DoWeaponHit = 0x004795D0;
  void doWeaponHit(CUnit* target, CUnit* attacker, u8 dmgDivisor) {
    u32 divisor = dmgDivisor;
	

    __asm {
      PUSHAD
      PUSH divisor
      PUSH attacker
      MOV EBX, target
      CALL Helper_DoWeaponHit
      POPAD
    }
  }

}//unnamed namespace

