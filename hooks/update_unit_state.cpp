#include "update_unit_state.h"
#include "update_status_effects.h"
#include <SCBW/api.h>
#include <algorithm>
#include "hooks/weapons/weapon_damage.h"
#include "hooks/weapon_effect.h"
#include "hooks/hydra_unit_ai.h"
#include "globals.h"
#include "SCBW\UnitFinder.h"

namespace hooks
{

    /// This function regenerates energy for spellcasters and drains energy for cloaking units.
    /// Equivalent to function sub_4EB4B0 @ 0x004EB4B0

    void updateUnitEnergy(CUnit* unit)
    {
        //was hardcoded instead of function calling in original code
        if (unit->isValidCaster()
            && (unit->status & UnitStatus::Completed))
        {
            bool bStopHere = false;
            //Spend energy for cloaked units
            if (
                unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection)		// If the unit is cloaked
                && !(unit->status & UnitStatus::CloakingForFree)							// ...and must consume energy to stay cloaked (i.e. not under an Arbiter)
                )
            {
                u16 cloakingEnergyCost;
                //Equivalent to getCloakingEnergyConsumption @ 0x00491A90
                if (unit->id == UnitId::TerranEidolon
                    || unit->id == UnitId::Hero_SarahKerrigan
                    || unit->id == UnitId::Hero_InfestedKerrigan
                    || unit->id == UnitId::Hero_SamirDuran
                    || unit->id == UnitId::Hero_InfestedDuran
                    || unit->id == UnitId::Hero_AlexeiStukov
                    )
                {
                    cloakingEnergyCost = 10;
                }
                else
                    if (unit->id == UnitId::TerranWraith)
                    {
                        cloakingEnergyCost = 13;
                    }
                    else
                    {
                        cloakingEnergyCost = 0;
                    }

                if (!(unit->hasEnergy(cloakingEnergyCost)))
                {
                    if (unit->secondaryOrderId == OrderId::Cloak)
                    {
                        unit->setSecondaryOrder(OrderId::Nothing2); // Supposedly, immediately decloaks the unit.
                    }
                    bStopHere = true;
                }
                if (!bStopHere)
                { // the function prevent energy consumption if THE GATHERING enabled
                    unit->spendUnitEnergy(cloakingEnergyCost);
                }
            }
            else
                if (unit->id == UnitId::ProtossMindTyrant && scbw::get_aise_value(unit, nullptr, AiseId::SumMindControlCost, 0, 0) > 0)
                {
                    auto sum = scbw::get_aise_value(unit, nullptr, AiseId::SumMindControlCost, 0, 0) / 2;
                    unit->spendUnitEnergy(std::min(unit->energy, (u16)(16 * sum)));
                    if (unit->energy == 0)
                    {
                        scbw::sendGPTP_aise_cmd(unit, nullptr, GptpId::FreeMindControlled, 0, 0, 0);
                    }
                }
                else
                    if (unit->id == UnitId::TerranAzazel && scbw::get_generic_rust_value(unit, ValueId::SublimeShepherd) == 1)
                    {
                        unit->spendUnitEnergy(std::min(unit->energy, (u16)13));
                        if (unit->energy == 0)
                        {
                            scbw::set_generic_rust_value(unit, ValueId::SublimeShepherd, 0);
                        }
                    }
                    else
                        if (unit->id == UnitId::corsair && scbw::get_generic_rust_value(unit, ValueId::DisruptionWeb))
                        {
                            unit->spendUnitEnergy(std::min(unit->energy, (u16)13));
                            if (unit->energy == 0)
                            {
                                scbw::set_generic_rust_value(unit, ValueId::DisruptionWeb, 0);
                            }
                        }

                        else
                        { //EB526
                            if (unit == nullptr)
                            {
                                return;
                            }
                            s32 maxEnergy;
                            if (unit->id == UnitId::ProtossMindTyrant
                                && unit->mainOrderId == OrderId::CompletingArchonSummon
                                && unit->mainOrderState == 0)
                            {
                                maxEnergy = 12800;  //50 * 256; Identical to energy amount on spawn
                            }
                            else
                            {
                                maxEnergy = unit->getMaxEnergy();
                            }
                            bool phobosCondition = false;
                            if (unit->id == UnitId::TerranPhobos && scbw::get_generic_value(unit, ValueId::PhobosDischarge) > 0)
                            {
                                unit->spendUnitEnergy(426); // (200 * 256) / (5 * 24)  ~ 426.66
                                if (unit->energy < 426)
                                {
                                    unit->energy = 0;
                                    scbw::set_generic_value(unit, ValueId::PhobosDischarge, 0);
                                }
                                phobosCondition = true;
                            }
                            if (unit->energy == maxEnergy && !phobosCondition)
                            {
                                bStopHere = true;
                            }
                            else
                            {
                                unit->addUnitEnergy(8, maxEnergy);
                            }
                        }

            //If the unit is currently selected, redraw its graphics
            if (!bStopHere && unit->sprite->flags & CSprite_Flags::Selected)
            {
                helper::setAllImageGroupFlagsPal11(unit->sprite);
            }
        }
    }

    const u32 Func_SetUnitPathing = 0x00401360;
    void setUnitPathing(CUnit* unit, u8 unkPathRelated)
    {
        __asm {
            PUSHAD
            MOV AL, unkPathRelated
            MOV ECX, unit
            CALL Func_SetUnitPathing
            POPAD
        }
    }

    /// Updates unit timers, regenerates hp and shields, and burns down Terran buildings.
    /// Identical to function @ 0x004EC290
    void updateUnitStateHook(CUnit* unit)
    {
        if (unit->pAI != nullptr)
        {
            hydra_unit_ai(unit);
        }

        // Timers
        if (unit->mainOrderTimer)
        {
            unit->mainOrderTimer--;
        }
        if (unit->groundWeaponCooldown)
        {
            unit->groundWeaponCooldown--;
        }
        if (unit->airWeaponCooldown)
        {
            unit->airWeaponCooldown--;
        }
        if (unit->spellCooldown)
        {
            unit->spellCooldown--;
        }

        // Wyvern Reverse Thrust - fire while moving
        if (unit->id == UnitId::TerranWyvern && unit->groundWeaponCooldown == 0 && scbw::get_generic_value(unit, ValueId::ReverseThrust))
        {
            bool can_free_fire = true;
            if (unit->mainOrderId == OrderId::Attack1 || unit->mainOrderId == OrderId::Attack2)
            {
                if (unit->orderTarget.unit != nullptr)
                {
                    can_free_fire = false;
                }
            }

            if (can_free_fire)
            {
                auto range = weapons_dat::MaxRange[WeaponId::WyvernJacksonCannon];
                s32 forwardOffset = weapons_dat::ForwardOffset[WeaponId::WyvernJacksonCannon];
                auto x = unit->getX() + scbw::getPolarX(forwardOffset, unit->currentDirection1);
                auto y = unit->getY() + scbw::getPolarY(forwardOffset, unit->currentDirection1);

                unit->groundWeaponCooldown = weapons_dat::Cooldown[WeaponId::WyvernJacksonCannon];
                bool seek_unit = true;
                if (scbw::get_generic_value(unit, ValueId::WyvernReverseLandgrabOn) == 1)
                {
                    auto targ_x = scbw::get_generic_value(unit, ValueId::WyvernReverseLandgrabX);
                    auto targ_y = scbw::get_generic_value(unit, ValueId::WyvernReverseLandgrabY);
                    auto direction = scbw::getAngle(x, y, targ_x, targ_y);
                    if (scbw::getDistanceFast(unit->position.x, unit->position.y, targ_x, targ_y) <= range)
                    {
                        seek_unit = false;
                        scbw::artificalBulletFrom(WeaponId::WyvernJacksonCannon, unit->playerId, unit, nullptr, targ_x, targ_y);
                    }
                    else
                    {
                        scbw::set_generic_value(unit, ValueId::WyvernReverseLandgrabOn, 0);
                    }
                }

                if (seek_unit)
                {
                    auto a = scbw::UnitFinder::getNearestTarget(
                        unit->getX() - range, unit->getY() - range,
                        unit->getX() + range, unit->getY() + range,
                        unit, [&](CUnit* target)
                    {
                        if (target == unit)
                        {
                            return false;
                        }
                        if (target->status & UnitStatus::InAir)
                        {
                            return false;
                        }
                        if (scbw::isAlliedTo(unit->playerId, target->playerId))
                        {
                            return false;
                        }
                        if (scbw::getDistanceFast(target->sprite->position.x, target->sprite->position.y,
                            unit->sprite->position.x, unit->sprite->position.y) > weapons_dat::MaxRange[WeaponId::WyvernJacksonCannon])
                        {
                            return false;
                        }
                        return true;
                    });
                    if (a != nullptr)
                    {
                        auto direction = scbw::getAngle(x, y, a->getX(), a->getY());
                        scbw::artificalBulletFrom(WeaponId::WyvernJacksonCannon, unit->playerId, unit, a, a->position);
                    }
                }
                unit->updateSpeed();
            }
        }

        // Accantor Plasma Shells have shields equal to the Accantor that launches them
        if (unit->id == UnitId::ProtossAccantor)
        {
            CUnit* scarab = unit->carrier.inHangarChild;
            while (scarab != nullptr)
            {
                if (scarab->id == UnitId::ProtossPlasmaShell)
                {
                    scarab->shields = unit->shields;
                }
                scarab = scarab->interceptor.hangar_link.prev;
            }
        }

        // Add mining status, decrease the regeneration rate
        if (unit->mainOrderId == OrderId::MiningMinerals)
        {
            if (unit->orderTarget.unit != nullptr)
            {
                unit->orderTarget.unit->unusedTimer = 180;
            }
        }

        // Cowardice slow flag
        if (unit->_padding_0x132 & 0x10)
        {
            unit->_padding_0x132 &= ~0x10;
            unit->updateSpeed();
        }

        // Cowardice / Malice overlays
        if (unit->_unknown_0x08C & 0x1)
        {
            unit->_unknown_0x08C &= ~0x1;
            u32 overlayImageId = ImageId::Cowardice_Small + scbw::getOverlayOffset(unit->id);
            if (!unit->getOverlay(overlayImageId) && scbw::get_aise_value(unit, nullptr, AiseId::HasMalice, 0, 0) == 0)
            {
                unit->sprite->createTopOverlay(overlayImageId,
                    0, 0, 0);
                scbw::set_generic_value(unit, ValueId::HierophantOverlay, 1);
            }
        }

        if (scbw::get_generic_value(unit, ValueId::HierophantOverlay) == 1)
        {
            auto overlayId = ImageId::Cowardice_Small + scbw::getOverlayOffset(unit->id);
            auto overlay = unit->getOverlay(overlayId);
            if (scbw::get_aise_value(unit, nullptr, AiseId::HasMalice, 0, 0) == 1)
            {
                if (unit->getOverlay(overlayId))
                {
                    unit->removeOverlay(overlayId);
                }
                u32 maliceId = ImageId::Malice_Small + scbw::getOverlayOffset(unit->id);
                if (!unit->getOverlay(maliceId))
                {
                    unit->sprite->createTopOverlay(maliceId, 0, 0, 0);
                }
                scbw::set_generic_value(unit, ValueId::HierophantOverlay, 0);
            }
        }

        // Clear Malice & Cowardice overlays
        if (unit->_unknown_0x08C & 0x2)
        {
            unit->_unknown_0x08C &= ~0x2;
            u32 maliceId = ImageId::Malice_Small + scbw::getOverlayOffset(unit->id);
            u32 cowardiceId = ImageId::Cowardice_Small + scbw::getOverlayOffset(unit->id);
            scbw::set_generic_value(unit, ValueId::HierophantOverlay, 0);
            if (unit->getOverlay(maliceId))
            {
                unit->removeOverlay(maliceId);
            }
            if (unit->getOverlay(cowardiceId))
            {
                unit->removeOverlay(cowardiceId);
            }
        }

        // Something to do with player-specific Signify overlays?
        if (unit->_padding_0x132 & 0x20 && unit == playersSelections->unit[*ACTIVE_NATION_ID][0])
        {
            unit->_padding_0x132 &= ~0x20;
            scbw::refreshConsole();
        }

        if (units_dat::BaseProperty[unit->id] & UnitProperty::ResourceContainer)
        {
            // Decrement resource regen timers
            if (unit->unusedTimer > 0)
            {
                unit->unusedTimer--;
            }

            if (unit->_unknown_0x066 > 0)
            {
                unit->_unknown_0x066--;
            }
            else
            {
                // Set regen timers
                if (unit->unusedTimer > 0)
                {
                    unit->_unknown_0x066 = 72;
                    if (scbw::get_generic_timer(unit, ValueId::MineralSpecialRegenTimer) > 0)
                    {
                        unit->_unknown_0x066 = 36;
                    }
                }
                else
                {
                    unit->_unknown_0x066 = 36;
                    if (scbw::get_generic_timer(unit, ValueId::MineralSpecialRegenTimer) > 0)
                    {
                        unit->_unknown_0x066 = 18;
                    }
                }

                // Actual resource regeneration
                bool update_image = true;
                if (unit->building.resource.resourceAmount < 5000
                    && (unit->id == UnitId::ResourceVespeneGeyser
                        || scbw::get_generic_timer(unit, ValueId::MineralSpecialRegenTimer) > 0))
                {
                    unit->building.resource.resourceAmount++;
                    scbw::refreshConsole();
                }
                else if (unit->building.resource.resourceAmount < 5000)
                {
                    unit->building.resource.resourceAmount++;
                    if (unit->id >= UnitId::mineral_field_1 && unit->id <= UnitId::mineral_field_3)
                    {
                        helper::updateMineralPatchImage(unit);
                    }
                    scbw::refreshConsole();
                }
            }
        }

        // Shield regeneration
        if (units_dat::ShieldsEnabled[unit->id] != 0)
        {
            s32 maxShields = (s32)(units_dat::MaxShieldPoints[unit->id]) * 256;
            if (unit->id == UnitId::ProtossAugur && unit->status & UnitStatus::Completed)
            {
                unit->shields -= std::min(14, unit->shields);
            }
            else
            {
                if (unit->shields != maxShields && unit->id != UnitId::scarab)
                {
                    unit->shields += 7;
                    if (unit->shields > maxShields)
                    {
                        unit->shields = maxShields;
                    }
                    if (unit->sprite->flags & CSprite_Flags::Selected)
                    {  // If the unit is currently selected, redraw its graphics
                        helper::setAllImageGroupFlagsPal11(unit->sprite);
                    }
                }
            }
        }

        // Supposedly, allows Zerglings to attack after unburrowing without delay.
        if (unit->id == UnitId::ZergZergling)
        {
            if (unit->groundWeaponCooldown == 0)
            {
                unit->orderQueueTimer = 0;
            }
        }

        // Clear the healing flag every frame
        unit->isBeingHealed = 0;

        // Update unit status effects (stim, maelstrom, plague, etc.)
        if (unit->status & UnitStatus::Completed || !(unit->sprite->flags & CSprite_Flags::Hidden))
        {
            unit->cycleCounter++;
            if (unit->cycleCounter >= 8)
            {
                unit->cycleCounter = 0;
                helper::RestoreAllUnitStats(unit); // use update_status_effects hook if available
            }
        }

        // Only for fully-constructed units and buildings
        if (unit->status & UnitStatus::Completed)
        {
            bool bStopHere = false;

            // Remove units with timed lives
            if (unit->removeTimer != 0)
            {
                unit->removeTimer--;
                if (unit->removeTimer == 0)
                {
                    unit->remove();
                    bStopHere = true;
                }
            }

            // Update unit energy (energy regen/drain)
            updateUnitEnergy(unit);

            // Recent order timer
            if (unit->recentOrderTimer != 0)
            {
                unit->recentOrderTimer--;
            }

            // Self-destruct timer
            if (unit->id == UnitId::ProtossAugur && unit->shields == 0)
            {
                augurSacrifice(unit);
                bStopHere = true;
            }

            // TODO: Reduce defensiveMatrixTimer
            // Cleanse debuffs on initial Defensive Matrix application
            if (unit->defensiveMatrixTimer == 168)
            {
                scbw::restoreFunc(unit);
            }

            // HP regeneration
            if (units_dat::BaseProperty[unit->id] & UnitProperty::RegeneratesHP &&
                unit->hitPoints > 0 &&
                unit->hitPoints / 256 != unit->getMaxHpInGame())
            {
                unit->setHp(unit->hitPoints + 4);
            }

            // Deliberate healing start
            s32 tickHeal = 0;
            // Azazel Innervating Matrices - heal over time
            if (unit->defensiveMatrixHp != 0)
            {
                if (unit->hitPoints > 0
                    && unit->hitPoints / 256 != unit->getMaxHpInGame())
                {
                    auto caster = scbw::get_aise_value(unit, nullptr, AiseId::GetMatrixCasterPlayer, 0, 0);
                    if (caster < 8)
                    {
                        tickHeal += AzazelDefensiveMatrixHeal;
                    }
                }
            }
            // Akistrokor - Enthralling Assault HP regeneration
            if (scbw::get_generic_timer(unit, ValueId::EnthrallingAssaultHealTimer) > 0)
            {
                tickHeal += AkistrokorEnthrallingAssaultHeal;
            }

            if (tickHeal)
            {
                unit->heal(tickHeal);
            }
            // Deliberate healing end

            // Last Orders - Aurora passive
            if (unit->id == UnitId::ProtossAurora)
            {
                if (scbw::get_generic_value(unit, ValueId::LastOrdersInit) == 1)
                {
                    auto distance = scbw::getDistanceFast(unit->position.x, unit->position.y,
                        unit->orderTarget.pt.x, unit->orderTarget.pt.y);
                    if (distance < 32)
                    {
                        unit->status |= UnitStatus::Invincible;
                        CBullet* bullet = helper::createBulletRet(WeaponId::AuroraLastOrders, unit, unit->position.x, unit->position.y, unit->playerId, unit->currentDirection1);
                        hooks::splash(bullet, WeaponId::AuroraLastOrders, SplashType::FriendlyNew, nullptr);
                        if (bullet)
                        {
                            bullet->time_remaining = 0;
                        }
                        // TODO: Add impact graphic
                        unit->remove();
                        bStopHere = true;
                    }
                }
            }

            // Terran building burn-down
            if (!bStopHere)
            {
                if (unit->getRace() == RaceId::Terran)
                {
                    if (unit->status & UnitStatus::GroundedBuilding
                        || units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding)
                    {
                        if (helper::unitHpIsInRedZone(unit))
                        {
                            unit->damageHp(20, nullptr, unit->lastAttackingPlayer, true);
                        }
                    }
                }
            }
        }
    };

    void incrementUnitScoresEx(s32 count, CUnit* unit, s32 increment_scores)
    { // 0x00488D50
        if (unit->status & UnitStatus::IsHallucination)
        {
            return;
        }
        if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit)
        {
            return;
        }
        auto player = unit->playerId;
        auto unit_id = unit->id;
        completed_unit_counts[unit->id].player[unit->playerId] += count;

        s32 race = unit->getRace();
        s32 supply = units_dat::SupplyProvided[unit->id];
        if (scbw::isZergEggNew(unit, false, false))
        {
            if (unit->buildQueue[unit->buildQueueSlot % 5] == UnitId::ZergIroleth)
            {
                supply = units_dat::SupplyProvided[UnitId::ZergOvileth];
            }
            if (unit->buildQueue[unit->buildQueueSlot % 5] == UnitId::ZergOthstoleth)
            {
                supply = units_dat::SupplyProvided[UnitId::ZergIroleth];
            }
            if (unit->buildQueue[unit->buildQueueSlot % 5] == UnitId::ZergAlaszileth)
            {
                supply = units_dat::SupplyProvided[UnitId::ZergOthstoleth];
            }
        }

        if (*elapsedTimeFrames > 0)
        {
            u32 supply_new = scbw::get_generic_value(unit, ValueId::SupplyProvided);
            if (supply_new)
            {
                supply = supply_new;
            }
            scbw::set_generic_value(unit, ValueId::SupplyProvided, supply);
            auto supply_overlord_modifier = scbw::get_generic_value(unit, ValueId::OverlordSupplyGimmick);
            if (supply_overlord_modifier > 0)
            {
                switch (supply_overlord_modifier)
                {
                    case 1:
                        supply = units_dat::SupplyProvided[UnitId::ZergIroleth] - units_dat::SupplyProvided[UnitId::ZergOvileth];
                        break;
                    case 2:
                        supply = units_dat::SupplyProvided[UnitId::ZergOthstoleth] - units_dat::SupplyProvided[UnitId::ZergIroleth];
                        break;
                    case 3:
                        supply = units_dat::SupplyProvided[UnitId::ZergAlaszileth] - units_dat::SupplyProvided[UnitId::ZergOthstoleth];
                        break;
                    default:
                        break;
                }
            }
            else
                if (unit->id == UnitId::ZergOthstoleth)
                {
                    supply = units_dat::SupplyProvided[unit->id];
                }
            scbw::set_generic_value(unit, ValueId::OverlordSupplyGimmick, 0);
        }
        if (race <= RaceId::Protoss)
        {
            raceSupply[race].provided[player] += count * supply;
        }
        if (units_dat::GroupFlags[unit_id].isFactory)
        {
            factoriesOwned[unit->playerId] += count;
        }
        if (units_dat::GroupFlags[unit_id].isBuilding)
        {
            buildingsOwned[unit->playerId] += count;
        }
        if (units_dat::GroupFlags[unit_id].isMen)
        {
            unitsProduced[unit->playerId] += count;
        }
        if (increment_scores != 0)
        {
            if (units_dat::GroupFlags[unit_id].isMen)
            {
                if (!scbw::isAnyMorphMorphResult(unit_id))
                {
                    unitsOwned[unit->playerId] += count;
                }
                unitScore[unit->playerId] += count;
            }
            else if (units_dat::GroupFlags[unit_id].isBuilding)
            {
                if (!scbw::isZergBuildingMorphResult(unit_id))
                {
                    buildingsOwned[unit->playerId] += count;
                }
                buildingScore[unit->playerId] += count * units_dat::BuildScore[unit_id];
            }
        }
        if (completed_unit_counts[unit_id].player[unit->playerId] < 0)
        {
            completed_unit_counts[unit_id].player[unit->playerId] = 0;
        }
    }

    // Hooked to help with the cloak rewrite - Pr0nogo
    void updateUnits()
    { // 004ED000
        s32 v0;
        CUnit* v1;
        char v2;
        char i;
        CUnit* v4;
        CUnit* v5;
        s32 v6;
        CUnit* v7;
        CUnit* v8;
        CUnit* v9;
        char v10;
        CUnit* v11;
        CUnit* v12;
        CUnit* v13;
        CUnit* v14;
        s32 v15;
        CUnit* v16;
        CUnit* v17;

        *interceptorFocusedThisFrame = 0;
        v0 = helper::isSelectedUnitGroupEnabled();
        if (v0)
        {
            if (*IS_PLACING_BUILDING)
            {
                helper::refreshLayers3And4();
                helper::endBuildingPlacement();
            }
            if (*isTargeting)
            {
                helper::cancelTargetOrder();
            }
        }
        v1 = *firstVisibleUnit;
        if (!--(*orderTimerReassignCount))
        {
            v2 = 0;
            if (*firstVisibleUnit)
            {
                do
                {
                    v1->orderQueueTimer = v2++;
                    if (v2 == 8)
                        v2 = 0;
                    v1 = v1->link.next;
                }
                while (v1);
                v1 = *firstVisibleUnit;
            }
            *orderTimerReassignCount = 150;
        }
        if (!--(*secondaryOrderTimerReassignCount))
        {
            for (i = 0; v1; v1 = v1->link.next)
            {
                v1->secondaryOrderTimer = i++;
                if (i == 30)
                    i = 0;
            }
            *secondaryOrderTimerReassignCount = 300;
        }
        helper::updateDisruptionWebStatus();
        v4 = *firstDyingUnit;
        if (*firstDyingUnit)
        {
            do
            {
                v5 = v4->link.next;
                *activeIscriptEntity = v4;
                *activeIscriptUnit = v4;
                helper::progressDyingUnitFrame(v4);
                if (v4->sprite)
                {
                    if (*visionUpdated)
                    {
                        v6 = v4->playerId;
                        if (playerTable[v6].type == 2)
                        { // PLAYER_OCCUPIED
                            helper::revealSightAtLocation(
                                1,
                                playerVision->flags[v6],
                                v4->sprite->position.x,
                                v4->sprite->position.y,
                                (v4->status >> 2) & 1); // is air unit
                        }
                    }
                    helper::updateVisibility(v4);
                }
                v4 = v5;
            }
            while (v5);
        }
        v7 = *firstVisibleUnit;
        if (*firstVisibleUnit)
        {
            do
            {
                *activeIscriptEntity = v7;
                *activeIscriptUnit = v7;
                helper::progressActiveUnitFrame(v7);
                v7 = v7->link.next;
            }
            while (v7);
            v7 = *firstVisibleUnit;
        }
        if (*visionUpdated)
        {
            v8 = *firstScannerSweep;
            if (*firstScannerSweep)
            {
                do
                {
                    v9 = v8->link.next;
                    *activeIscriptEntity = v8;
                    *activeIscriptUnit = v8;
                    helper::refreshUnitVision(v8);
                    v8 = v9;
                }
                while (v9);
                v7 = *firstVisibleUnit;
            }
        }
        if (v7)
        {
            do
            {
                helper::updateVisibility(v7);
                if (v7->status & 0x300 && scbw::get_generic_timer(v7, ValueId::DecloakTimer) == 0)
                {
                    v10 = v7->secondaryOrderTimer;
                    v7->isCloaked = 0;
                    if (v10)
                    {
                        v7->secondaryOrderTimer = v10 - 1;
                    }
                    else
                    {
                        helper::checkUnitVisibility(v7);
                        v7->secondaryOrderTimer = 30;
                    }
                }
                v7 = v7->link.next;
            }
            while (v7);
            v7 = *firstVisibleUnit;
        }
        v11 = v7;
        if (v7)
        {
            do
            {
                v12 = v11->link.next;
                *activeIscriptEntity = v11;
                *activeIscriptUnit = v11;
                helper::progressUnitFrame(v11);
                v11 = v12;
            }
            while (v12);
        }
        v13 = *firstHiddenUnit;
        if (*firstHiddenUnit)
        {
            do
            {
                v14 = v13->link.next;
                if (v13->status & 0x300)
                {
                    v13->isCloaked = 0;
                }
                *activeIscriptEntity = v13;
                *activeIscriptUnit = v13;
                helper::unitUpdate(v13);
                v13 = v14;
            }
            while (v14);
        }
        updateBurrowingCloakingUnits();
        helper::updateUnitPower();
        v15 = (*lurkerSplashListIndex)++ + 1;
        if (*lurkerSplashListIndex == 32)
        {
            v15 = 0;
            *lurkerSplashListIndex = 0;
        }
        memset(&lurkerSplashList[32 * v15], 0, 0x80u);
        v16 = *firstScannerSweep;
        *lastBulletIndex = 0;
        if (*firstScannerSweep)
        {
            do
            {
                v17 = v16->link.next;
                *activeIscriptEntity = v16;
                *activeIscriptUnit = v16;
                helper::progressUnitFrame(v16);
                v16 = v17;
            }
            while (v17);
        }
        *activeIscriptEntity = 0;
        *activeIscriptUnit = 0;
    }

    // Hooked to help with the cloak rewrite - Pr0nogo
    void updateBurrowingCloakingUnits()
    { // 00492EB0
        CUnit* v0;
        s32 v1;
        CUnit* v2;
        s32 v3;
        s32 v4;
        s32 v5;

        v0 = *firstInvisibleUnit;
        v1 = 0;
        if (*firstInvisibleUnit)
        {
            while (1)
            {
                v2 = v0->burrow_link.next;
                if (helper::isUnitCloaking(v0))
                {
                    doCloak(v0, SoundId::Terran_PHOENIX_TPhClo01_WAV);
                }
                else
                {
                    v3 = v0->status;
                    if (v3 & UnitStatus::Burrowed || v0->secondaryOrderId != OrderId::Cloak || v0->isCloaked != 1 || !(v3 & 0x800))
                    {
                        v4 = 0;
                    }
                    else
                    {
                        v0->status = v3 & 0xFFFFF7FF;
                        v4 = 1;
                    }
                    v1 |= v4;
                    v5 = v0->status;
                    if (v5 & 0x100)
                        goto LABEL_13;
                    helper::playSoundFromDirect(v0, 273);
                }
                v1 = 1;
LABEL_13:
                v0 = v2;
                if (!v2)
                {
                    if (v1)
                    {
                        *forceButtonRefresh = 1;
                        portraitRedraw[0] = 1;
                        *statusScreenRedraw = 1;
                        *ctrlUnderMouse = 0;
                        *ctrlUnderMouseValue = 0;
                    }
                    return;
                }
            }
        }
    }

    // Hooked for cloak rewrite animations - Pr0nogo
    void doCloak(CUnit* unit, s32 soundId)
    { // 0049B440
        CUnit* v2;
        CUnit* v3;

        v2 = unit;
        if (!(unit->status & UnitStatus::Burrowed))
        {
            helper::playSoundFromSource(soundId, unit, 1, 0);
        }
        if (!IS_IN_REPLAY && v2->playerId == *LOCAL_NATION_ID)
        {
            *forceButtonRefresh = 1;
            portraitRedraw[0] = 1;
            *statusScreenRedraw = 1;
            *ctrlUnderMouse = 0;
            *ctrlUnderMouseValue = 0;
        }
        if (v2->status & UnitStatus::Burrowed)
        {
            helper::updateSpriteFlagOnPalette(v2->sprite);
            helper::showImagesHiddenByCloak(v2->sprite);
            v2->status &= 0xFFFFFCFF;
        }
        else
        {
            helper::updateSpriteImagesForCloak(v2->sprite);
            v2->status = v2->status & 0xFFFFFEFF | UnitStatus::Cloaked;
            v3 = v2->subunit;
            if (v3)
            {
                helper::updateSpriteImagesForCloak(v3->sprite);
                v3->status = v3->status & 0xFFFFFEFF | UnitStatus::Cloaked;
            }
        }
    }
}