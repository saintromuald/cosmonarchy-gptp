#pragma once
#include "SCBW\scbwdata.h"

namespace helper
{
    bool HasMoneyCanMake(CUnit* builder, u32 unitToBuild);

    // For loadRaceUI
    void getRaceSoundList(s8 a1);
    s32 dialogGrpConstruct(s32 a1, s32 a2, s32 a3, s32(__fastcall* a4)(s32, s32, s32, s32));
    void loadGraphic(s8 a1, s32 a2, s32 a3, s8 a4, s32 a5);

    s32 randFunc();
    bool unitHPbelow33_percent(CUnit* a1);
    bool unitHPbetween33_66_percent(s32 a1, CUnit* a2);
    s32 getMaxHP(CUnit* a1);
    u16 getLastQueueSlotType(CUnit* a1);
    void addTextToDialog(s32 a1, s16 a2, const char* a3);
    void drawEnergy(s32 a1);
    void hideOptionalStatusScreenControls(BinDlg* a1);
    void showControlGroup(BinDlg* a1, s16 a2, s16 a3);
    void addTextToDialog(s32 a1, s16 a2, s32 a3);
    void drawShieldUpgrade(s32 a1, u16 a2);
    void hideDialog(BinDlg* a1);

    // For drawSupplyUnitInfo
    s32 getSuppliesUsed();
    u16 getLastQueueSlotType(CUnit* a1);
    u32 getSuppliesAvailable(u8 a1, s32 a2);
    void drawArmor(s32 a1, s32 a2, u16 a3);
    void unitStatAct_Standard(void* a1);

    // For AI_TargetEnemyProc
    bool AI_UnitCanAttack(CUnit* a1);

    // For drawBuildingInfo
    s32 needRedrawUI();
    CUnit* genericStatusUpdateDrawnValues();
    void drawProgressBar(s32 a1);
    void drawNukeSiloInfo(s32 a1);
    bool unitIsActiveTransport(CUnit* a1);
    bool isQueueSlotActive(CUnit* a1, s32 a2);
    void drawTrainingProgress(BinDlg* a1);
    bool isConstructingAddon(const CUnit* a1);
    bool isAttemptingProtossBuild(CUnit* a1);
    void drawAddonProgress(s32 a1);
    void drawTechProgress(s32 a1);
    void drawUpgradeProgress(BinDlg* a1);
    bool unitIsGeyser(CUnit* a1);
    void drawGasQuantity(BinDlg* a1);
    s8 unitGetGroundWeapon(CUnit* a1);
    void drawDisplayButtons(s32 a1);
    BinDlg* getControlFromIndex(BinDlg* a1, s16 a2);
    void setTextStr(s32 a1);
    void setUnitStatusStrText(s32 a1);
    s32 unitGetName(CUnit* a1);

    // For getUnitStrength
    s32 getCurrentHitPointsPlusShields(CUnit* a1);
    s32 getBaseStrength(s32 a1, CUnit* a2);
    s32 getMaxHP(CUnit* a1);

    // For AI_RestorationRequirementsProc
    s32 canWeaponTargetUnit(u8 a1, CUnit* a2, CUnit* a3);

    // For AI_EMPShieldRequirementsProc
    s32 AI_TargetUnitIsWorthHitting(CUnit* a1, CUnit* a2);
    s32 createSplashWeaponProc(s16 a1, s16 a2, s16 a3, s32(__fastcall* a4)(s32, s32), s32 a5);
    s32 unitShieldsClumpInAreaAccumulatorProc(CUnit* a1, CUnit* a2);

    // For hallucinationHit
    void unitWasHit(CUnit* a1, CUnit* a2, s32 a3);
    void doAttackNotifyEvent(CUnit* a1);
    CImage* createShieldOverlay(s32 a1, CUnit* a2);

    // For drawImageHPBar

    s32 unk_billions(s32 a1, s32 a2, s32 a3, s32 a4);
    s32 unk_billions2(s32 a1, s32 a2, s32 a3, s32 a4);
    s32 hpBarSurfaceBuffer(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5);
    s32 hpBarSurfaceBuffer2(s32 a1, s32 a2, u16 a3, s32 a4);
    s32 renderHPBar(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5);
    s32 getUnitMaxEnergy(CUnit* a1);
    s32 getHpBarDrawSizes(s32 a1, s32 a2, s32 a3, s32 a4);

    // For canCastSpell
    bool unitIsFrozen(CUnit* a1);
    bool isUnitCasterNotHallucination(CUnit* a1);
    bool getActiveTileFlag(s16 a1, s16 a2, s32 a3, s32 a4);
    CUnit* getTargetUnitAbilityErrorStatString(CUnit* a1, char a2, s32 a3);

    // For modifyUnitShields
    s32 SErrSetLastError(char a1);

    // For compileHealthBar
    void updateImageDrawData(CImage* result);
    void playImageIscript(CImage* img, u8 iscriptAnim);

    // For AI_FindTransport
    bool canEnterTransport(CUnit* a1, CUnit* a2);
    s16 SAI_GetRegionIdFromPxEx(s32 a1, s32 a2);
    s32 getUnitDistanceToHalt(s32 a1, CUnit* a2, s32 a3);

    // For mapDataTransfer
    s32 dataTransfer00(s32 a1, char a2, s32 a3);
    s32 dataTransfer01(u32 a1, s32 a2, u32 a3);
    s32 dataTransfer02(s32 a1, s32 a2, char a3);
    s32 dataTransfer03(s32 a1, char a2, s32 a3);
    s32 dataTransfer04(u32 a1, s32 a2, s32 a3);
    s32 dataTransfer05(s32 a1, s32 a2);
    vectorHead* vectorTransfer(char a1, char a2);
    s32 dataTransfer06(s32 a1);

    // For getRandYesSFX
    s32 randGetSFXBetween(s32 a1, s32 a2);
    s16 SAI_GetRegionIdFromPxEx(s32 x, s32 y);

    // for AI_TrainUnit
    char AI_TrainingWorker(CUnit* result, CUnit* parent);
    void AI_TrainingOverlord(CUnit* result, CUnit* parent);
    void AI_TrainingNormal(CUnit* result, CUnit* parent);
    void AI_OrderToDestination(CUnit* unit, s32 orderId, s32 x, s32 y);
    void AI_AssignCaptainToSlowestUnit(AiCaptain* region);

    // for AI_CreateNuke
    CUnit* createUnit(s16 unitType, s32 x, s32 y, s32 playerId);
    void updateUnitStatsFinishBuilding(CUnit* unit);
    u32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit);
    void hideUnit(CUnit* unit);

    // for AI_TrainingBroodling
    void AI_AssignMilitary(s32 regionType, AiCaptain* region);
    void AI_AddMilitary(AiCaptain* region, CUnit* unit, s32 always_this_region);

    // for AI_AddUnitAi
    UnitAi* AI_CreateBuildingAi(BuildingAiArray* buildingAI);
    void orderComputerClear(CUnit* unit, u8 orderId);
    CUnit* setSecondaryOrder(CUnit* result, char orderId);
    void AI_MakeGuard(s32 playerId, CUnit* unit);
    bool unitIsActiveResourceDepot(CUnit* unit);
    s32 getPlayerDefaultRefineryUnitType(s32 playerId);

    // for AI_UnitWasHit // some of these should probably be methods in api/etc
    char AI_CancelStructure(CUnit* structure);
    CUnit* AI_GetActualTarget(CUnit* attacker, CUnit* target);
    CUnit* checkArbiterCloakingRange(CUnit* unit, u16 id);
    void AI_OrderUnitCloaking(CUnit* unit);
    void AI_OrderLurkerBurrow(CUnit* unit);
    bool AI_IsUnderAttack(CUnit* unit, s32 flag);
    bool unitIsActiveTransport(CUnit* unit);
    s32 isUnitLoaded(CUnit* unit);
    s32 AI_TryReturnHome(CUnit* unit, s32 dontIssueOrder);
    s32 AI_TransportAction(CUnit* unit);
    s32 AI_AskForHelp(CUnit* self, CUnit* attacker, s32 onlyNearbyGuards);
    s16 getRegionIdFromUnit(CUnit* unit);
    s32 getEnemyAirStrengthInRegion(u16 regionId, s32 player);
    s32 getEnemyStrengthInRegion(u16 regionId, s32 player, s32 includeAir);
    bool unitIsPerformingUnbreakableCode(CUnit* unit);
    bool AI_CastSpellBehavior(CUnit* unit, bool isAggro);
    s32 AI_Flee(CUnit* unit, CUnit* enemy);
    bool unitHasGroundPath(CUnit* unit, s16 x, s16 y);
    bool AI_HasNoDefenseBuild(s32 player);
    bool unitCanSeeCloakedTarget(CUnit* target, CUnit* unit);
    void AI_RespondToCloakedThreat(CUnit* threat, CUnit* unit);
    bool unitCanAttackTarget(CUnit* target, CUnit* unit, s32 checkDetection);
    s32 AI_UpdateAttackTarget(CUnit* unit, s32 acceptIfSieged, s32 acceptCritters, s32 mustReach);
    __int64 AI_PrepareToFlee(CUnit* enemy, CUnit* unit);

    // for AI_IterateBuildingAI
    u16 AI_RemoveFromTowns(CUnit* worker);
    BuildingAi* AI_DeleteBuildingAI(BuildingAi* building, BuildingAiArray* buildingList);
    s32 AI_GetQueuedCount(CUnit* builder, u32 unitId);
    void AI_PrepareGuardAI(u16 y, u32 player, u16 unitId, u16 x);

    // Attack extender

    // for AI_TrainAttackForce
    s32 AI_AddRegionToMilitary(AiCaptain* region, s32 unitId, s32 priority);
    u32 AI_FindUnit(u32 unitId, u32 playerId, u32 type);
    u32 AI_AddSpendingRequest(u16 unitId, u32 type, u8 player, char priority, void* value);
    u32 AI_AddBaseUnitSpendingRequest(u32 unitId, u32 player, AiCaptain* region, char priority);

    // for AIRegion_GroupForAttack
    u32 AI_RecalculateRegionStrength(AiCaptain* region);
    CUnit* AI_TargetEnemyInRegion(AiCaptain* region);
    s32 AI_MoveMilitaryToDefend(AiCaptain* region);
    void AI_GetRegionMilitaryTypes(AiCaptain* region, s32* hasGroundMilitary,
        s32* cantAttackGround, s32* cantAttackAir);
    s32 AI_CanTravelToRegion(AiCaptain* region1, AiCaptain* region2);
    CUnit* AI_PickAttackTarget_NoBuildings(AiCaptain* region);
    u32 AI_MoveMilitaryToOtherRegion(AiCaptain* target, AiCaptain* source, u32 pullUnitsNearRegion,
        u32 sendPureSpellcasters, u32 dontSendFromBunkers);
    AiTown* AI_IterateTownSomething(AiCaptain* region);
    u32 AI_AttackTimerDecrement(u32 player);
    s32 AI_HasForcesMovingToAttack(u32 player);

    // for AI_AttackManager
    u16 PopulateRegionsWithOwn(char* regionData, u32 player);
    void PopulateRegionsWithEnemy(u32 player, char* regionData);
    s32 AI_GetNeighborRegionTypes(char* regionData);
    void AI_FinishRegionAccessibility(char* regionData);
    s32 AI_SetAttackGroupingRegion(u32 airAttack, u32 alsoVeryNear, u32 player, char* regionData,
        s32 x, s32 y, u32 alwaysOverwriteLastAttack);
    s16 SAI_GetRegionIdFromPxEx(s32 x, s32 y);
    char AI_CancelStructure(CUnit* structure);

    bool unitCantSeeCloakedTarget(CUnit* unit, CUnit* target);												// 00401D60
    bool unitCanInfest(CUnit* unit);																		// 00402750
    void AI_AttackUnit(CUnit* unit);																		// 0043FFD0
    CUnit* findBestAttackTarget(CUnit* unit);																// 00443080
    bool function_00462EA0(CUnit* unit, u8 unknownByteValue);											// 00462EA0
    void function_00465780(CUnit* unit);																	// 00465780
    void removeOrderFromUnitQueue(CUnit* unit, COrder* order);												// 004742D0
    void function_00474A70(CUnit* unit, CUnit* target, u8 orderId);										// 00474A70
    bool isTargetWithinMinMovementRange(CUnit* unit, CUnit* target, u32 range);							// 004763D0
    bool function_00476610(CUnit* unit, s32 x, s32 y);														// 00476610
    bool isUnitInWeaponRange(CUnit* unit, CUnit* target);													// 00476870
    void getWeaponBeginIscript(CUnit* unit, IscriptAnimation::Enum anim);									// 00476ED0
    void function_00477510(CUnit* unit);																	// 00477510
    void function_00477820(CUnit* unit, u8 orderId);														// 00477820
    bool function_00478370(CUnit* unit, u8 orderId);														// 00478370
    bool function_00478540(CUnit* unit);																	// 00478540
    bool isUnitCritter(CUnit* unit);																		// 0047B450
    void moveToCollide(CUnit* unit, CUnit* target);														    // 00495400

    bool isUnitVisible(CUnit* unit);																		// 004E5DB0
    bool isTargetVisible(CUnit* unit, CUnit* target);														// 004E5E30
    u8 getRightClickActionOrder(CUnit* unit);															// 004E5EA0
    void setNextWaypoint(CUnit* unit);															            // 004EB290
    void makeToHoldPosition(CUnit* unit);																	// 004EB5B0
    bool moveToTarget(CUnit* unit, CUnit* target);															// 004EB720
    bool SetMoveTarget_xy(CUnit* unit, s32 x, s32 y);														// 004EB820
    bool function_004EB900(CUnit* unit, CUnit* target);														// 004EB900
    bool changeMovementTargetMoveTarget2(CUnit* unit, s32 x, s32 y);										// 004EB9C0

    bool isDistanceGreaterThanHaltDistance(CUnit* unit, s32 x, s32 y, u32 distance);	                    // 00401240
    void checkUnstack(CUnit* unit);												                            // 0042D9A0
    bool beginBuilding(CUnit* unit);												                        // 00467030
    CUnit* beginGasBuilding(CUnit* unit, u32 unitId);									                // 004678A0
    void addOverlayBelowMain(CSprite* sprite, u32 imageId, u32 unk1, u32 unk2, u32 unk3);	    // 00498D70
    void replaceSpriteImages(CSprite* sprite, u32 imageId, u32 imageDirection);			            // 00499BB0
    void displayLastNetErrorForPlayer(u32 playerId);										                // 0049E530
    void updateUnitStrength(CUnit* unit);												                    // 0049FA40
    void iscript_OpcodeCases(CImage* image, u32 offset_, u32 unk1, u32 unk2);			            // 004D74C0
    void switchToWarpTexture(CImage* image);												                // 004D8500
    bool setThingyVisibilityFlags(CThingy* thingy);									                        // 004878F0
    CThingy* createThingy(u32 spriteId, s16 x, s16 y, u32 playerId);		    	                // 00488210
    void setUnitStatTxtErrorMsg(char* message);										                        // 0048CCB0
    void playUnitSFX(u32 soundId, u16 x, u16 y, u32 unk1, u32 unk2);				            // 0048EC10
    u32 RandomizeShort(u32 unknown_index);											                // 004DC4A0
    u32 IterateUnitsAtLocationTargetProc_RecallTargets(CUnit* unit, Box16* coords);	                    // 004E8280
    void function_004EBAE0(CUnit* unit, s32 x, s32 y);								                    // 004EBAE0

    //// spider_mine
    CUnit* SpiderMine_AcquireTarget(CUnit* spiderMine);									                    // 00441270
    void instantCloak(CUnit* unit);												                            // 0049B1E0
    void unburrowGeneric(CUnit* unit);												                        // 004E99D0
    void function_004E9A30(CUnit* unit);												                    // 004E9A30

    // For orders_PlaceMine
    s32 techUseAllowed(CUnit* a1, __int16 a2, s32 a3);									                    // 0046DD80
    u32 getDistanceFast(s32 a1, s32 a2, s32 a3, s32 a4);						                    // 0040C360
    s32 orders_HoldPosSuicidal(CUnit* a1);												                    // 004EB5B0
    void fixTargetLocation(unsigned __int16 a1, Point32* a2);							                    // 00401FA0
    void turnUnit(CUnit* a1, char a2);													                    // 00495F20
    void toIdle(CUnit* a1);																                    // 004753A0
    s32 isUnitMovable(CUnit* a1);														                    // 00401DC0
    void orders_Nothing2(CUnit* a1);													                    // 00475000
    bool setMoveTargetXY(s32 a1, s32 a2, CUnit* a3);									                    // 004EB820

    // For burrowGeneric
    void toggleUnitPath(CUnit* unit);									                                    // 004E42A0
    char switchSpriteId(CSprite* sprite, s32 spriteId);				                                    // 00499860

    //// building_morph
    void refundBuildingCost(u16 unitId, u8 playerId);								                    // 002CE70
    void updateBuildingsAI(CUnit* unit);												                    // 0033FE0
    bool isValidMorph(u16 buildingId);												                    // 005CC60
    void updateNewUnitVision(u32 id, s32 x, s32 y);									                    // 005CE90
    void zergPlaceBuildingCntd(CUnit* unit);											                    // 005D410
    void stopCreepDisappearingAtBuilding(CUnit* unit);												        // 007D770
    void function_0047DE40(CSprite* sprite, u32 unitId, s32 x, s32 y);				                    // 007DE40
    void incrementUnitScoresEx(CUnit* building, s32 unk1, s32 unk2);				                    // 0088D50
    void playBuildingCompleteSound(CUnit* building);									                    // 008F310
    void refreshSpriteData(CSprite* sprite);											                    // 00983A0
    void displayLastNetErrorForPlayer(u32 playerId);									                    // 009E530
    void replaceUnitWithType(CUnit* unit, u16 newUnitId);							                    // 009FED0

    //// building_terran
    bool isDistanceGreaterThanHaltDistance(CUnit* unit, s32 x, s32 y, u32 distance);	                    // 00401240
    void fixTargetLocation(Point16* coords, u32 unitId);								                    // 00401FA0
    bool isUnitPositions2Equal(CUnit* unit);											                    // 00402160
    b32 buildingAddon(CUnit* unit, u32 hpGain, b32 canBeAborted);				                    // 004679A0
    void prependOrderTargetingNothing(CUnit* unit, COrder* order, u8 orderId);					        // 00474760
    void addOrderTargetingNothing(CUnit* unit, COrder* order, u8 orderId);					            // 004749A0
    bool canStillPlaceBuilding();														                    // 0048DDA0
    void endBuildingPlacement();															                // 0048E310
    void playBuildingCompleteSound(CUnit* building);									                    // 0048F310
    void function_00494BB0(CUnit* unit, s32 x, s32 y);									                    // 00494BB0
    void displayLastNetErrorForPlayer(u32 playerId);									                    // 0049E530
    u32 RandomizeShort(u32 unknown_index);										                    // 004DC4A0
    u32 RandBetween(u32 min, u32 max, u32 someIndex);						                    // 004DC550
    void initBuilding(CUnit* unit, b32 flag);									                        // 004E65E0
    bool SetMoveTarget_xy(CUnit* unit, s32 x, s32 y);									                    // 004EB820
    bool orderToMoveToTarget(CUnit* unit, CUnit* target);								                    // 004EB980
    b32 unk_terrainCollision2(CUnit* unit, s32 x, s32 y);								                // 004F1870

    // For updatePlacementStateForPlacementBox
    s32 canBuildOnTileLocation(CUnit* a1, s32 player, s32 x_tile, s32 y_tile, s32 unitType,
        s32 placement_state_entry, s32 check_vision, s32 also_invisible, s32 without_vision);	        // 00473FB0
        // For refreshScreen
    s32 updatePlacementStateForPlacementBox_Func();                                                       // 0048DCE0
    s32 unk_buildingPlacementRelated(s32 a1, Point16* a2);                                              // 0048D7F0
    s32 initPlacementBoxPos(Point16* in_out);                                                             // 0048D660
    char unk_buildingPlacementRelatedMouse(s32 a1, s32 a2);                                             // 0048D7B0

    //// liftland
    void setUnitPathing(CUnit* unit, u8 unkPathRelated);																				// 00401360
    bool isUnitPositions2Equal(CUnit* unit);																							// 00402160
    CUnit* connectAddonCheck(CUnit* unit);																								// 004404A0
    void function_00463470(CUnit* unit);																								// 00463470
    void function_004634E0(CUnit* unit);																								// 004634E0
    void function_00463640(CUnit* unit);																								// 00463640
    void completeAddon(CUnit* unit, CUnit* addon);																						// 00463D50
    void function_00464300(CUnit* unit);																								// 00464300
    void disconnectFromAddOn(CUnit* unit);																								// 00464930
    void function_00469EC0(CUnit* unit, s32 x, s32 y);																					// 00469EC0
    void function_00469F60(CUnit* unit, s32 x, s32 y);																					// 00469F60
    void function_0046A560(CUnit* unit);																								// 0046A560
    void function_0046A5A0(CUnit* unit);																								// 0046A5A0
    u32 function_00473FB0(CUnit* unit, u8 playerId, s32 x, s32 y, u16 unitId, u8 unk1, u8 unk2, u8 unk3, u8 unk4);	// 00473FB0
    void actUnitReturnToIdle(CUnit* unit);																								// 00475420
    bool function_0047DF90(CUnit* unit);																								// 0047DF90
    void placebuildingProc();																											// 0048E6E0
    void playBuildingLandSound(CUnit* unit);																							// 0048F5A0
    void SetUnitMovementSpeed(CUnit* unit, u32 newSpeed);																			// 004951C0
    void function_00498150(CSprite* sprite, u8 newVerticalOffset);																	// 00498150
    void function_004997E0(CSprite* sprite);																							// 004997E0
    void function_004C3B40(CUnit* unit);																								// 004C3B40

    //// create_init_units
    b32 spreadsCreep(u32 unitId, b32 defaultReturn);																			// 00413870
    void function_004148F0(s32 x, s32 y, u32 unitId);																				// 004148F0
    void CreateBuildingLarva(CUnit* main_building);																						// 0049D660
    b32 function_0049EC30(CUnit* unit);																								// 0049EC30
    CUnit* FindBestUnit(Box16* coords, u32 searchProc);																				// 004E8830

    //// larva_creep_spawn
    CUnit* getUnitPointer();
    u8 spreadCreep(u32 unitId, s32 x, s32 y);																						// 00414680
    void AI_TrainingOverlord(CUnit* main_building, CUnit* larva);																		// 00435770
    void updateUnitStrength(CUnit* unit);																								// 0049FA40
    u32 IterateUnitsAtLocationTargetProc_LarvaCount(CUnit* unit, Box16* coords);														// 004E8280
    void function_004E8DA0(CUnit* unit);																								// 004E8DA0
    bool function_004E8E10(CUnit* unit, Point32* coords);																				// 004E8E10

    // TODO: move somewhere else?
    // SC Variables and Buffers used in this file
    u32* const mapTileBufWidth = (u32*)(0x006D0F08);
    u32* const mapTileBufHeight = (u32*)(0x006D0C6C);
    u16** const mapTileBuf = (u16**)(0x006D0E84);
    Point32* const someCreepPointBuf = (Point32*)(0x006D7578);
    typedef u32(*isTileVisible_func)(u32 x, u32 y);
    typedef bool(*setCreepRelationalData_callback)(u16* tileBackup, u8* creepOverlayVal, u32* retval, u16* tile, u32 arg);
    isTileVisible_func* const isTileVisible_ptr = (isTileVisible_func*)(0x006D0C78);

    u32 getNumSurroundingCreepTiles(u32 x, u32 y);																				// 00414290
    void setCreepRelationalData(u32 x, u32 y, setCreepRelationalData_callback callback, u32 arg);								// 00413FA0
    u32 iterateDirectionalCreepData(u32 x, u32 y, u32 callback, u32 arg);												// 00413B30

    // TODO: add helpers for these functions
    // for applyCreepAtLocationFromUnitType
    s32 modifyCreepState(s32 x_tile, s32 y_tile, s32(__stdcall* creepChange)(s32, s32, s32, s32, s32), void* param);	// 00413FA0
    s32 runCreepRandomizer(s16* originalTile, s32 creepTileBorders, s32* outCreepStatus, s16* currentTile, s32 unused);		// 004140A0 // "note possible desync"
    s32 unkCreepBorders(s32 a1, s32 a2);																							// 006D0C78
    u32 forEachAdjacentTile(s32(__stdcall* callback)(s32, s32, s32, s32, void*), void* param, s32 x_tile, s32 y_tile);	// 00413B30
    s32 unkCreepTile(s32 pos, s32 originalTile, s32 x_tile, s32 y_tile, void* param);											// 00414440
    char unkSpreadCreep(s32 a1, s32 a2, s16 a3);																					// 00414680

    //// burrow_tech
    void actUnitReturnToIdle(CUnit* unit);																								// 00475420
    void function_004754F0(CUnit* unit, u32 unitId, u32 unk1, u32 unk2, u32 orderId, u32 unk3, u32 unk4, u32 unk5);// 004754F0
    void function_004E97C0(CUnit* unit);																								// 004E97C0

    //// die_order
    void clearUnitSprites(CSprite* unit_sprite, CSprite* subunit_sprite);																// 00496F00
    void function_00497480(CSprite* thingySprite, u32 visibility_flags);																// 00497480
    void _UnitDestructor(CUnit* unit);																									// 004A0740
    void UnitDestructor(CUnit* unit);																									// 004A0990
    void hideAndDisableUnit(CUnit* unit);																								// 004E6340

    //// CMDRECV_Cancel
    void CancelResearch_Proc(CUnit* unit);																								// 00453E30
    void CancelUpgrade_Proc(CUnit* unit);																								// 00454280
    void refundQueueSlot(CUnit* unit, u16 slot);																						// 00466A70
    void function_00466E40(CUnit* unit);																								// 00466E40
    void CancelAddon_Proc(CUnit* unit);																									// 004E66E0

    //// CMDRECV_Build
    b32 OrderAllowed(CUnit* unit, u16 order, u32 nationID);																	// 0046DC20
    bool placeBuildingMsg(u32 result_function_473FB0);																				// 0048D930
    bool CMDRECV_PlaceBuildingAllowedHelper(CUnit* unit, u8 orderId, u16 builtUnitId);											// 0048DBD0
    void function_0048DE70(CUnit* unit, u32 orderId, u16 dimensionX, u16 dimensionY, u32 builtUnitId);						// 0048DE70
    void cmdRECV_PlaceBuilding(s16 x, s16 y, u8 orderId, u16 builtUnitId);													// 0048E190

    //// CMDRECV_MergeArchon
    CUnit* templarMergePartner(CUnit* unit, CUnit** units_list, u32 units_list_length);												// 004BFA80

    //// CMDRECV_Stop
    void parseOrdersDatReqs();																											// 0046D450
    s32 parseRequirementOpcodes(CUnit* unit, u32 orderId, u32 playerId, u32 address, u32 req_offset);						// 0046D610

    //// give_unit
    void ApplySpeedUpgradeFromUnitType(CUnit* unit);																					// 00454370
    void function_00463040(CUnit* unit);																								// 00463040
    void function_00476D30(CUnit* unit);																								// 00476D30
    void incrementUnitScores(CUnit* unit, s32 unkScore);																				// 00488BF0
    void function_004A1E50(CUnit* unit, u32 unk);																					// 004A1E50
    void CreateUnitSelection(CUnit* unit, u32 selectionIndex);																		// 004E6180
    void DestroyUnitSelection(CUnit* unit);																								// 004E6290
    void function_004F3B70(CUnit* powerup);																								// 004F3B70

    //// cast_order
    u32 getUpgradedWpnCooldown(CUnit* unit, u8 weaponId);																			// 00475DC0
    bool function_00476640(CUnit* unit, u32 weaponId);																				// 00476640
    u32 get_statTxt_Str_0(CUnit* unit, u32 playerId, char* message);																// 0048EF30
    bool canCastSpell_0(CUnit* unit);																									// 00492140
    u32 function_00492620(CUnit* unit, b32 wasMainOrderStateZero);																// 00492620

    void incrementUnitDeathScores(CUnit* unit, u8 player);																			// 00488AF0
    void initializeEmptyUnitsLinkedListRef_Sub49E4E0(CUnit* unit, u8 playerId);														// 0049E4E0
    void GiveUnitHelper(CUnit* unit, u32 playerId, s32 unkScore);																	// 0049EFA0
    void changeUnitButtonSet_Sub_4E5D60(CUnit* unit, u16 unitId);																	// 004E5D60
    void showAndEnableUnit(CUnit* unit);																								// 004E6490

    //// tech_target_check
    bool isInfestableUnit(CUnit* unit);																									// 

    //// trap_hooks
    // For updateUnitStrengthAndApplyDefaultOrders
    void playSoundFromDirection(CUnit* a1, s32 a2);																						// 0049B5B0
    void refreshUnitVision(CUnit* a1);																									// 004E5F30
    void initUnitTrapDoodad(CUnit* a1);																									// 004E6490
    COrder* removeOrderFromUnitQueue(COrder* a1, CUnit* a2);																			// 004742D0
    COrder* performAnotherOrder(u8 a1, s32 a2, CUnit* a3, s16 a4, Point32* a5, CUnit* a6);											// 004745F0
    void order(s32 a1, CUnit* a2, s8 a3, Point32* a4, CUnit* a5, s32 a6);																// 00474810
    u32 getUnitStrength(CUnit* a1, s32 a2, bool a3);																					// 00431800

    //// make_nydus_exit
    void playWorkerErrorSound(CUnit* unit, u32 errorTextIndex);																		// 0048F6C0

    //// research_upgrade_orders
    bool isUnitUpgradeAvailable(CUnit* unit);																							// 004033D0
    void ApplySpeedUpgradeFromUpgradeType(CUnit* unit, u8 upgradeType);																// 00454540
    void playUpgradeCompleteSound();																									// 0048F070
    void playResearchCompleteSound();																									// 0048F150
    void setUpgradeLevel(u32 playerId, u32 upgradeType, u8 upgradeLevel);														// 004CE770

    //// merge_units
    bool advanceRemainingBuildTime(CUnit* unit);																				// 00466940
    u8 function_0046A820(CUnit* unit);																								// 0046A820
    void orderImmediate(CUnit* unit, u8 order);																						// 00474B40
    void playMorphingCompleteSound(CUnit* unit);																						// 0048F440
    void mergeStatusIntoUnit_Sub493180(CUnit* unit_dst, CUnit* unit_src);																// 00493180
    void function_0049B440(CUnit* unit, u32 soundId);			 																	// 0049B440
    void changeUnitButtonSet_Sub4E5D60(CUnit* unit, u16 unitId);																		// 004E5D60
    bool delayedSetMoveTarget_xy_Sub4EB960(CUnit* unit, u16 x, u16 y);															// 004EB960

    //// nuke_orders
    void setAllOverlayDirectionsGeneric(CUnit* unit, u32 unk);																		// 00401140
    void AI_orderUnitCloaking(CUnit* unit);																								// 0043B970
    u32 function_0043DB50(CUnit* unit);																								// 0043DB50

    //// target_related

    // for isUnitTargetOutOfMaxRange
    s32 getUnitMaxWeaponRange(CUnit* a1, unsigned __int8 a2);
    bool isTargetWithinMinMovementRange(CUnit* a1, u32 a2, CUnit* a3);

    // for isReadyToAttack
    u32 getDistanceToUnit(CUnit* a1, CUnit* a2);
    u32 distanceFromDboxCorner(CUnit* a1, __int16 a2, __int16 a3);
    bool isAngleInAttackAngle(s32 a1, s32 a2, CUnit* a3, unsigned __int8 a4);

    // for canCastOrderedSpellNow
    bool isTargetWithinMinRange(CUnit* a1, u32 a2, CUnit* a3);
    s32 getFlingyHaltDistance(s32 a1);

    // for ordersWatchTarget
    void unknownOrdersFunc(s32 a1, __int16 a2, __int16 a3);

    // for ordersStayInRange
    void prependOrderTargetingGround(__int8 a1, __int16 a2, __int16 a3, s32 a4);
    s32 moveTowards(CUnit* a1, CUnit* a2);
    s32 ordersHoldPositionSuicidal(CUnit* a1);
    s32 getDirectionFromPoints(s32 a1, s32 a2, s32 a3, s32 a4);
    CUnit* setSecondaryWaypoint(CUnit* a1, s32 a2, s32 a3);
    Point32 setResourceTarget(CUnit* a1);

    // for genericRightClickUnitProc2
    s32 isOrderValid(char a1, CUnit* a2, s32 a3, CUnit* a4, s32 a5);						// 00474D90
    u16 getUnitAbilityErrorStatString(CUnit* a1, char a2, s32 a3);							// 00491E80

    // for CMDRECV_HoldPosition
    CUnit* getActivePlayerFirstSelection();														// 0049A850
    s32 orderAllowed(CUnit* a1, u16 a2, s32 a3);											// 0046DC20
    void targetedOrder(s16 a1, s32 a2, CUnit* a3, s16 a4, s32 a5, s32 a6, char a7, s32 a8, s32 a9, s32 a10);	// 004754F0

    // for checkUnitVisibility
    u32 getDetectingPlayers(CUnit* unit);													// 
    u32 iteratePlayerUnitsSomething(u32 aResult, CUnit* unit);							// 
    void makeSpriteVisible(CSprite* sprite);													// 
    s32 updateBuildingLandUnitSelection(u32 unused, CUnit* selection);						// 
    char removeSelectionCircle(CSprite* sprite);												// 
    CImage* makeSpriteInvisible(CSprite* sprite);												// 

    //// receive_command
    void function_004756B0(CUnit* unit, u8 orderId, u32 unk1, u32 unk2, u32 bCommandType);								// 004756B0
    void function_004756E0(CUnit* unit, CUnit* target, u32 orderId, u32 bCommandType);										// 004756E0
    u32 function_0049A410(CUnit* unit, u8 orderId);																			// 0049A410
    bool CanTargetSelf(CUnit* unit, u8 orderId);																					// 0049A480
    void function_0049A500(CUnit* unit, u8* array_of_data);																		// 0049A500
    void function_0049A8C0(u8* array_of_data, b32 bCanBeObstructed);															// 0049A8C0

    //// load_unload_proc
    void IgnoreAllScriptAndGotoIdle(CUnit* unit);																					// 00476550
    u32 function_004E76C0(CUnit* transport, CUnit* loaded_unit, Point16* pos);													// 004E76C0
    void function_004E7E10(CUnit* unit, u32 unk);																				// 004E7E10

    //// stats_panel_display
    void StatsNukesCount(BinDlg* dialog, u32 index);																				// 00425310
    void StatHangerCount(BinDlg* dialog, u32 index);																				// 004253D0
    void StatsShieldLevel(BinDlg* dialog, u32 index);																			// 00425510
    void StatsArmorLevel(BinDlg* dialog, u32 index);																				// 00425600
    void StatsWeaponLevel(BinDlg* dialog, u32 index, u32 weaponId);															// 00425790
    bool StatSpidermineCount(BinDlg* dialog, u32 index);																			// 00426300

    // For getUnitRankString
    __int8 unitGetRace(CUnit* a1);																									// 0047B150
    char* getStatTxtString(__int16 a1);																								// 004C36F0

    // For statsWeaponLevel / statsArmorLevel
    char getArmorCount(CUnit* unit, s32 a2);																						// 00453FC0
    void showDialog(BinDlg* dialog);																								// 004186A0
    s32 disableDialog(BinDlg* dialog);																							// 00418640
    void updateDialog(BinDlg* dialog);																								// 0041C400

    //// bunker_hooks
    CUnit* findRandomAttackTarget(CUnit* unit);																						// 00442FC0
    b32 attackApplyCooldown(CUnit* unit);																						// 00478B40
    void setImageDirection(CImage* image, s8 direction);																			// 004D5F80

    //// api
    void ApplySpores(CUnit* unit);
    void createBullet(u8 weaponId, CUnit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction);
    CBullet* createBulletRet(u8 weaponId, CUnit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction);
    void weaponBulletShot(CUnit* target, CBullet* bullet, u32 dmgDivisor);

    //// unit_morph
    u8 setArchonPathing(CUnit* unit);																							// 0046A820
    bool hasSuppliesForUnit(u32 unitId, u32 playerId, b32 canShowErrorMessage);											// 0042CF70
    CUnit* zergDualBirth(CUnit* unit);																							    // 0045D910
    void orderNewUnitToRally(CUnit* unit, CUnit* factory);																			// 00466F50
    void AI_TrainingUnit(CUnit* unit_creator, CUnit* created_unit);																	// 004A2830
    void removeAITownGasReferences(CUnit* unit);                                                                                    // 00432760
    void deleteBuildingAi(void* building_ai_list, BuildingAi* ai);                                                                  // 00404500
    void deleteWorkerAi(void* worker_ai_list, WorkerAi* ai);                                                                        // 00404470
    void deleteAiTownIfNeeded(AiTown* town);                                                                                        // 004347E0

    //// hallucination_spell
    // for orders_Hallucination1
    bool ordersSpell_Sub_4926D0(CUnit* unit, u32 techId, u16* techEnergyCost, u32 sightRange, u32 error_message_index);	// 004926D0

    void UnitDestructor(CUnit* unit);
    bool placeHallucination(CUnit* unit);																					// 004F66D0

    // for createHallucination
    void applyHallucinationStats(CUnit* halUnit);																					// 004F6180
    s32 hallucinateScarabs(CUnit* a1, CUnit* a2);																					// 004F65B0

    //// feedback_spell
    void replaceFeedbackSprite(CUnit* unit, u32 spriteId);																		// 004F6350

    //// weapon_effect
    bool isUnitDistanceWithin(CUnit* target, CBullet* bullet, u32 radius);
    void doWeaponHit(CUnit* target, CUnit* attacker, u8 dmgDivisor);
    void Ai_UnitWasHit(u32 main_target_reaction, u32 dont_call_help, CUnit* attacker, CUnit* target);

    // for airSplash
    CUnit* findBestUnit(Box16* radius, s32 bullet, s32(*func)(CUnit*, CBullet*));										// 004E8830
    s32 splashFunc(CBullet* bullet, CUnit* unit);																					// 0048B570

    //// mindcontrol_spell
    void MindControlHit(CUnit* attacker, CUnit* target);																			// 004F6910

    //// weapon_damage
    u16 getUnitStrength(CUnit* unit, bool useGroundStrength);
    CUnit* createHallucination(CUnit* target, u32 playerId);

    // For updateCorrosiveSpores
    u32 getCorrosiveSporesImage(CUnit* unit);																					// 004F41F0

    //// recharge_shields
    void insertFirstOrder(CUnit* unit, u8 orderId);			//0x004749D0
    u8 hasOverlay(CUnit* unit);								//0x0047B720
    bool isHangarUnit(CUnit* unit);

    //// stopholdpos_orders
    CUnit* MedicHeal_TargetAcquire(CUnit* medic);																					// 004422A0
    u32 doMedicHeal(CUnit* unit, CUnit* target);																					// 00463C40
    void function_00476FC0(CUnit* unit, CUnit* target, u32 unk1, u32 unk2);													// 00476FC0
    b32 function_004770E0(CUnit* unit);																							// 004770E0

    //// medic_orders
    b32 AI_OrderHeal(CUnit* unit, CUnit* target);																				// 00463530
    void function_4748E0(CUnit* unit, s32 x, s32 y, u8 orderId);																	// 004748E0
    void function_474940(CUnit* unit, CUnit* target, u8 orderId);																// 00474940
    void orderInterrupt(CUnit* unit, CUnit* target, u16 x, u16 y, u32 orderId);											// 00474C30
    void unitOrderMoveToTargetUnit(CUnit* unit, CUnit* target);																		// 00479FE0
    b32 AI_CastSpellBehaviour(CUnit* unit, u32 unknown);																		// 004A13C0

    //// load_unload_orders
    b32 function_00438E70(CUnit* unit, s32 x, s32 y);																			// 00438E70
    void function_0043DF30(CUnit* unit);																							// 0043DF30
    void unknownSpecialOrder(CUnit* unit, COrder* order, u8 orderId, s16 x, s16 y, CUnit* target, u16 targetUnitId);		// 00474540
    void _doOrder(CUnit* unit, u32 orderId, s32 x, s32 y, bool stopPreviousOrders);												// 00474C70
    b32 function_00477160(CUnit* unit, u32 unk1, u32 unk2, u32 unk3);													// 00477160
    CUnit* getLoadedUnitFromIndex(CUnit* transport, s32 index);																		// 004E6C40
    b32 getFirstLoadedWorker(CUnit* transport);																					// 004E6D90
    b32 function_004E6EF0(CUnit* unit, CUnit** unitTargetOffset);																// 004E6EF0
    void loadUnitProc(CUnit* unit, CUnit* unitToLoad);																				// 004E78E0	(Hooked)
    b32 unloadUnitProc(CUnit* unit);																								// 004E7F70 (Hooked)

    //// siege_transform
    void unitSetRetreatPoint(CUnit* unit, u8 angleValue);																		// 004011A0
    bool getCloserToDestination_sub402BE0(CUnit* unit);																				// 00402BE0
    void orderReturnToIdle(CUnit* unit);																							// 00463770
    bool checkForCloseUnit_sub464290(CUnit* unit);																					// 00464290
    void playSubunitIscript(CUnit* unit, IscriptAnimation::Enum anim);																// 00474380

    //// unit_train
    u32 getHPGainForRepair(CUnit* unit);																							// 00402C40
    void refundUnitTrainCost(u32 unitId, u8 playerId);																		// 0042CEC0
    void function_00432430(CUnit* unit, u32 buildQueueSlot);																		// 00432430
    void addHangarUnit(CUnit* main_unit, CUnit* added_unit);																		// 00466300
    CUnit* attemptTrainHatchUnit(CUnit* trainer, u32 builtUnitID, u32 flag);													// 00468200

    //// weapon_fire
    // For progressBulletFlight
    void progressBulletMovement(CBullet* a1);																						// 0048B250
    void doMissileDmg(CBullet* bullet);																								// 0048B770

    // For bulletBehavior_Bounce
    CBullet* changeMovePos(CBullet* a1, s32 a2, s32 a3);																		// 00495240
    void progressBulletMovement_func(CBullet* a1);																					// 0048B250
    CUnit* findNextBounceTarget_func(CBullet* a1);																					// 0048B1E0

    // For findNextBounceTargetProc
    bool unitIsSubunit(CUnit* a1);																									// 00401D40
    bool unitIsCarrier(CUnit* a1);																									// 00401470
    bool unitIsReaver(CUnit* a1);																									// 00401490
    bool unitHasPathToUnit(CUnit* a1, CUnit* a2);																					// 0049CBB0
    s32 canBeInfested(CUnit* a1, CUnit* a2);																						// 00402210

    // For findNextBounceTarget
    s32 iterateUnitsAtLocationTargetProc(Box32* a1, s32(__fastcall* a2)(s32, s32), CUnit* a3);								// 004E8280

    // For writeBullets
    /*
    void packBulletData(CBullet* a1, s32 a2); // 0048A9A0
    size_t _lockwrite(const void* a1, size_t a2, size_t a3, FILE* a4); // 00411931
    void* SMemAlloc(s32 a1, char* a2, s32 a3, s32 a4); // 0041006A
    s32 compressWrite(void* a1, u32 a2, FILE* a3); // 004C3450
    bool SMemFree(void* a1, char* a2, s32 a3, char a4); // 00410070
    CBullet* unpackBulletData(CBullet* a1, s32 a2); // 0048A8D0
    */

    //// wpnspellhit
    bool function_0042D810(s32 x, s32 y, u32 unitId);														    // 0042D810
    CUnit** getAllUnitsInBounds(Box16* coords);																	// 0042FF80
    void incrementUnitKillCount(CUnit* unit);																	// 004759C0
    u32 getUnitDamageBonus(CUnit* unit, u8 weaponId);														    // 00475EC0
    void attackOverlayAndNotify(CUnit* attacker, CUnit* target, u32 weaponType, u32 direction);			        // 00479730
    CThingy* createThingy(u32 spriteId, s16 x, s16 y, u32 playerId);									        // 00488210
    u32 GetUnitBulletDamage(CBullet* bullet, CUnit* target);													// 0048ACD0
    u32 getSpellStatString(CUnit* target, u32 techId, u8 playerId);									            // 00491E80
    void disableUnitFunc(CUnit* unit);																			// 00492CC0
    void setAllImageGroupFlagsPal11(CSprite* sprite);															// 00497430
    void IterateUnitsAtLocationTargetProc_CorrosiveAcid(u32 attackingPlayerId, Box16* coords);				    // 004E8280
    void IterateUnitsAtLocationTargetProc_Ensnare(CUnit* attacker, Box16* coords);								// 004E8280
    void IterateUnitsAtLocationTargetProc_Plague(CUnit* attacker, Box16* coords);								// 004E8280
    void IterateUnitsAtLocationTargetProc_Maelstrom(CUnit* attacker, Box16* coords);							// 004E8280
    void IterateUnitsAtLocationTargetProc_StasisField(CUnit* attacker, Box16* coords);							// 004E8280
    void createEnsnareOverlay(CUnit* a1);																		// 004F45E0

    // For createScanner
    s32 setUnitStatTxtErrorMsg(s32 a1);																		    // 0048CCB0
    s32 playSoundFromSource(s32 a1, CUnit* a2, s32 a3, s32 a4);											        // 0048ED50
    void returnAllInterceptors(CUnit* unit);                                                                    // 004655F0
    char ignoreAllScriptsAndGoToIdle(CUnit* unit);                                                              // 00476550

    // For maelstromProc
    void orders_SelfDestructing(CUnit* unit);                                                                   // 00475710
    char setMaelstromTimer(CUnit* unit, u8 time);                                                               // 004553F0

    // For CBulletDamage
    s32 lurkerSplashProc(CUnit* target, CBullet* bullet);                                                       // 0048B680
    s32 groundSplashProc(CUnit* target, CBullet* bullet);                                                       // 0048B750
    s32 airSplashProc(CUnit* unit, CBullet* bullet);                                                            // 0048B150
    s32 airSplashProc2(CUnit* unit, CBullet* bullet);                                                           // 0048B730
    bool unitHasNoMainOrderType(CUnit* unit);                                                                   // 00475A50
    s32 setEventUnitComplete();                                                                                 // 004C36C0

    //// selection
    bool unitIsOwnedByCurrentPlayer(CUnit* unit); 																// 00401170
    bool isUnitBurrowed(CUnit* unit);																			// 00402A70
    void SC_memcpy_0(u32 dest, u32 src, u32 memsize);													        // 00408FD0
    void function_00468670(CUnit* unit);																		// 00468670
    bool unit_isUnselectable(u16 unitId);																	    // x046ED80
    void function_0046F040(CUnit* current_unit, CUnit** unit_list, CUnit* clicked_unit, u32 list_length);	    // x046F040
    u32 combineLists_Sub_6F290(CUnit* unit, CUnit** unit_list_1, CUnit** unit_list_2, u32 list_length);	        // 0046F290
    void applyNewSelect_Sub_6FA00(CUnit** unit_list, u32 unit_list_length);									    // 0046FA00
    u32 CUnitToUnitID(CUnit* unit);																			    // 0047B1D0
    bool unit_IsStandardAndMovable(CUnit* unit); 																// 0047B770
    void selectBuildingSFX(CUnit* unit);																		// 0048F910
    b32 selectSingleUnitFromID(u32 unitIndex);															        // 00496D30
    void CreateNewUnitSelectionsFromList(CUnit** unit_list, u32 unit_list_length);							    // 0049AE40
    void CMDACT_Select(CUnit** unit_list, u32 unit_list_length); 											    // 004C0860

    //// cloak_nearby_units
    void secondaryOrder_Cloak(CUnit* unit);																		// 00491790

    //// update_unit_state
    bool unitHpIsInRedZone(CUnit* unit);						                                                // 0x004022C0
    void RestoreAllUnitStats(CUnit* unit);						                                                // 0x00492F70 (hooked by hooks\update_status_effects.cpp)

    // For updateUnits
    bool isSelectedUnitGroupEnabled();																						// 0049A2C0
    char refreshLayers3And4();																								// 0048D9A0
    void cancelTargetOrder();																								// 0048CA10
    s32 updateDisruptionWebStatus();																						// 004EB2F0
    CUnit* progressDyingUnitFrame(CUnit* unit);																				// 004EB5E0
    void revealSightAtLocation(s32 sightRange, u32 visionFlags, s32 ptX, s32 ptY, bool isAirUnit);					        // 004806F0
    void updateVisibility(CUnit* unit);																						// 004EBE10
    void progressActiveUnitFrame(CUnit* unit);																				// 004EBC30
    void checkUnitVisibility(CUnit* unit);																					// 00443390
    void progressUnitFrame(CUnit* unit);																					// 004ECF70
    void unitUpdate(CUnit* unit);																							// 004EC420
    void updateUnitPower();																									// 00493790

    // For updateBurrowingCloakingUnits
    s32 isUnitCloaking(CUnit* unit);																						// 004916E0
    void playSoundFromDirect(CUnit* unit, s32 soundId);																	    // 0049B5B0

    // For doCloak
    void updateSpriteFlagOnPalette(CSprite* sprite);																		// 00497E80
    void showImagesHiddenByCloak(CSprite* sprite);																			// 00498170
    void updateSpriteImagesForCloak(CSprite* sprite);																		// 00498A90

    //// select_larva

    u32 getLarvaeUnitsFromList(CUnit** unitList, CUnit** outputList);													    // 00423190
    b32 selectSingleUnitFromID(u32 unitIndex);																		        // 00496D30
    void selectMultipleUnitsFromUnitList(CUnit** unitList, u32 unitsCount, b32 unk1, b32 unk2);					            // 0049AEF0

    //// uifeatures

    s32 BWFXN_PrintXY(const char* text, s32 x, s32 y);
    s32 GetColorHandle(u8 color); // Colors: 1 - last color, 2 - 8, 14 - 17, 21 - 25, 27 - 31

    // For targetedOrder
    COrder* queueNewOrder(COrder* a1, s16 a2, s16 a3, CUnit* a4, u8 a5, s32 a6, s16 a7, Point32* a8, CUnit* a9);	        // 00474B90
    s32 showStatTxtToPlayer(s32 a1, s32 a2);																			    // 0048CF00
    void setOrderTarget(u8 a1, s16 a2, s16 a3, s32 a4, s32 a5, CUnit* a6, s32 a7, Point32* a8, CUnit* a9);		            // 00475470

    // For selectUnits
    s32 selectSingleUnitFromID(s32 a1);																					    // 00496D30
    s32 selectMultipleUnitsFromUnitList(s32 a1, CUnit** a2, u8 a3, s32 a4);										            // 0049AEF0
    s32 SNetGetTurnsInTransit(u32* turns);
    u8 sendTurn();
    s32 SErrGetLastError();
    u8 packetErrHandle(s32 result, s32 a1, const char* a3, s32 a4, s32 a5);

    //// repair_order

    void getRepairInfo(CUnit* repairTarget, u32* repairCostMineral, u32* repairCostGas, s16* unkRepairInfo);		        // 00466D10
    bool verifyResources(u32 mineralCost, u32 gasCost, u32 playerId);												        // 00466EE0
    bool ord_repair_subtract(CUnit* worker, CUnit* target);																	// 00466FB0
    // For isOrderValid
    bool unitIsResource(CUnit* unit);                                                                                       // 004688E0

    //// hydra_specific
    u8 getHangerTrainCount(CUnit* unit);
    bool unitIsTrainingOrMorphing(CUnit* unit);
    u8 initializeRandomizerInfo();
    void AI_Loop();
    s32 updateActiveTileInfo();
    s32 removeFOWCheat();
    void updateUnits();
    s32 imageDrawingBulletDrawing();
    void updateImages();
    void updateThingys();

    //// psi_field_util
    bool addPsiFieldSprite(CUnit* unit);

    // stim_packs
    void QueueGameCommand(u8* params, u32 param_length);

    // misc
    CThingy* createThingyFunc(u32 spriteId, s16 x, s16 y, u32 playerId);
    bool func_dounitscollide(CUnit* first, CUnit* second);
    void unburrow(CUnit* unit);
    CImage* CreateHealthBar(s32 a1, CSprite* a2);
    void RemoveHPBar(CImage* a1);
    void refundAllQueueSlots(CUnit* unit);
    void* SMemAlloc(u32 amount, u32 unkFilename, u32 unkLine, u32 defaultValue);	                    // 0041006A (based on Storm.dll definition)
    void SetCallbackTimer(BinDlg* dialog, u32 unk1, u32 unk2, u32 unk3);			                    // 00416090
    void function_0049A7F0(CUnit* unit);											                    // 0049A7F0
    u32 fxnInteract(BinDlg* dialog, u32 data_struct_offset);                                               
    void registerUserDialogAction(BinDlg* dialog, u32 array_of_fxnInteractFunc, u32 sizeOfArray);	    // 00418100
    void function_418E00(BinDlg* dialog);																// 00418E00
    void replayStatBtns(BinDlg* dialog);																// 00427A80
    void function_4591D0();																				// 004591D0
    void function_459770();																				// 00459770
    void BINDLG_BlitSurface(BinDlg* dialog);															// 004C35F0
    u32 fxnInteract(BinDlg* dialog, u32 data_struct_offset);
    s32 req_check(u32 reqFunc, u16 reqVar, u32 playerId, CUnit* unit);
    void updateMineralPatchImage(CUnit* mineralPatch);
    s32 canBuildOnTileLocation(CUnit* a1, s32 player, s32 x_tile, s32 y_tile, s32 unitType,
        s32 placement_state_entry, s32 check_vision, s32 also_invisible, s32 without_vision);
    void updateImagePositionOffset(CImage* image);

    //Identical to function @ 0x004F3AF0
    void setResourceAmountCarried(CUnit* worker, u8 amountCarried, u32 chunkImageId, bool isMineral);

    // ai misc
    s32 Ai_HasBuildingsNeedingPower(u32 player);
    s32 Ai_CountEconomyUnits(u32 player, u32 unit_id);
    s32 getSuppliesRemaining(u8 player, u16 unit_id);
    s32 getSuppliesUsed(s32 byte_data, u8 player);
    s32 AI_AddCostToNeeds(s32 a1, s32 a2, s32 a3);
    s32 AI_AddSpendingRequest(s16 a1, s32 a2, char a3, char a4, void* a5);
    bool isAIAllowedToBuild(s32 a1, s32 a2);
    void applyCloakingOrder(CUnit* unit);                                                               // 00491B30

    //// use_tech
    const void issueTriTargetOrder(u8 order1, u8 order2, u8 order3);                                     // 0048CAF0
    s32 checkEnergyForTechUse(s32 tech_id);                                                              // 00423660

    //// misc
    s8 SETS(s32 x);
    s8 OFSUB(s32 x, s32 y);
}