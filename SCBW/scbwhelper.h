#pragma once
#include "SCBW\scbwdata.h"

namespace helper {
    bool HasMoneyCanMake(CUnit* builder, uint32 unitToBuild);

    // For loadRaceUI
    void getRaceSoundList(int8 a1);
    int32 dialogGrpConstruct(int32 a1, int32 a2, int32 a3, int32(__fastcall* a4)(int32, int32, int32, int32));
    void loadGraphic(int8 a1, int32 a2, int32 a3, int8 a4, int32 a5);

    int randFunc();
    bool unitHPbelow33_percent(CUnit* a1);
    bool unitHPbetween33_66_percent(int32 a1, CUnit* a2);
    int32 getMaxHP(CUnit* a1);
    uint16 getLastQueueSlotType(CUnit* a1);
    void addTextToDialog(int32 a1, int16 a2, const char* a3);
    void drawEnergy(int32 a1);
    void hideOptionalStatusScreenControls(BinDlg* a1);
    void showControlGroup(BinDlg* a1, int16 a2, int16 a3);
    void addTextToDialog(int a1, int16 a2, int a3);
    void drawShieldUpgrade(int a1, uint16 a2);
    void hideDialog(BinDlg* a1);

    // For drawSupplyUnitInfo
    int getSuppliesUsed();
    uint16 getLastQueueSlotType(CUnit* a1);
    uint32 getSuppliesAvailable(uint8 a1, int a2);
    void drawArmor(int a1, int a2, uint16 a3);
    void unitStatAct_Standard(void* a1);

    // For AI_TargetEnemyProc
    bool AI_UnitCanAttack(CUnit* a1);

    // For drawBuildingInfo
    int needRedrawUI();
    CUnit* genericStatusUpdateDrawnValues();
    void drawProgressBar(int a1);
    void drawNukeSiloInfo(int a1);
    bool unitIsActiveTransport(CUnit* a1);
    bool isQueueSlotActive(CUnit* a1, int a2);
    void drawTrainingProgress(BinDlg* a1);
    bool isConstructingAddon(const CUnit* a1);
    bool isAttemptingProtossBuild(CUnit* a1);
    void drawAddonProgress(int a1);
    void drawTechProgress(int a1);
    void drawUpgradeProgress(BinDlg* a1);
    bool unitIsGeyser(CUnit* a1);
    void drawGasQuantity(BinDlg* a1);
    int8 unitGetGroundWeapon(CUnit* a1);
    void drawDisplayButtons(int a1);
    BinDlg* getControlFromIndex(BinDlg* a1, int16 a2);
    void setTextStr(int a1);
    void setUnitStatusStrText(int a1);
    int unitGetName(CUnit* a1);

    // For getUnitStrength
    int getCurrentHitPointsPlusShields(CUnit* a1);
    int getBaseStrength(int a1, CUnit* a2);
    int getMaxHP(CUnit* a1);

    // For AI_RestorationRequirementsProc
    int canWeaponTargetUnit(uint8 a1, CUnit* a2, CUnit* a3);

    // For AI_EMPShieldRequirementsProc
    int32 AI_TargetUnitIsWorthHitting(CUnit* a1, CUnit* a2);
    int createSplashWeaponProc(int16 a1, int16 a2, int16 a3, int(__fastcall* a4)(int, int), int a5);
    int unitShieldsClumpInAreaAccumulatorProc(CUnit* a1, CUnit* a2);

    // For hallucinationHit
    void unitWasHit(CUnit* a1, CUnit* a2, int a3);
    void doAttackNotifyEvent(CUnit* a1);
    CImage* createShieldOverlay(int a1, CUnit* a2);

    // For drawImageHPBar

    int unk_billions(int a1, int32 a2, int a3, int a4);
    int unk_billions2(int a1, int32 a2, int a3, int a4);
    int hpBarSurfaceBuffer(int a1, int a2, int* a3, int a4, int a5);
    int hpBarSurfaceBuffer2(int a1, int a2, uint16 a3, int a4);
    int renderHPBar(int a1, int a2, int* a3, int a4, int a5);
    int32 getUnitMaxEnergy(CUnit* a1);
    int getHpBarDrawSizes(int a1, int32 a2, int a3, int a4);

    // For canCastSpell
    bool unitIsFrozen(CUnit* a1);
    bool isUnitCasterNotHallucination(CUnit* a1);
    bool getActiveTileFlag(int16 a1, int16 a2, int a3, int a4);
    CUnit* getTargetUnitAbilityErrorStatString(CUnit* a1, char a2, int a3);

    // For modifyUnitShields
    int SErrSetLastError(char a1);

    // For compileHealthBar
    void updateImageDrawData(CImage* result);
    void playImageIscript(CImage* img, uint8 iscriptAnim);

    // For AI_FindTransport
    bool canEnterTransport(CUnit* a1, CUnit* a2);
    int16 SAI_GetRegionIdFromPxEx(int32 a1, int32 a2);
    int32 getUnitDistanceToHalt(int a1, CUnit* a2, int a3);

    // For mapDataTransfer
    int32 dataTransfer00(int a1, char a2, int a3);
    int dataTransfer01(uint32 a1, int a2, uint32 a3);
    int32 dataTransfer02(int a1, int a2, char a3);
    int32 dataTransfer03(int a1, char a2, int a3);
    int dataTransfer04(uint32 a1, int a2, int a3);
    int32 dataTransfer05(int a1, int a2);
    vectorHead* vectorTransfer(char a1, char a2);
    int dataTransfer06(int a1);

    // For getRandYesSFX
    int randGetSFXBetween(int a1, int a2);
    int16 SAI_GetRegionIdFromPxEx(int32 x, int32 y);

    // for AI_TrainUnit
    char AI_TrainingWorker(CUnit* result, CUnit* parent);
    void AI_TrainingOverlord(CUnit* result, CUnit* parent);
    void AI_TrainingNormal(CUnit* result, CUnit* parent);
    void AI_OrderToDestination(CUnit* unit, int orderId, int x, int y);
    void AI_AssignCaptainToSlowestUnit(AiCaptain* region);

    // for AI_CreateNuke
    CUnit* createUnit(int16 unitType, int32 x, int32 y, int32 playerId);
    void updateUnitStatsFinishBuilding(CUnit* unit);
    uint32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit);
    void hideUnit(CUnit* unit);

    // for AI_TrainingBroodling
    void AI_AssignMilitary(int32 regionType, AiCaptain* region);
    void AI_AddMilitary(AiCaptain* region, CUnit* unit, int32 always_this_region);

    // for AI_AddUnitAi
    UnitAi* AI_CreateBuildingAi(BuildingAiArray* buildingAI);
    void orderComputerClear(CUnit* unit, uint8 orderId);
    CUnit* setSecondaryOrder(CUnit* result, char orderId);
    void AI_MakeGuard(int32 playerId, CUnit* unit);
    bool unitIsActiveResourceDepot(CUnit* unit);
    int32 getPlayerDefaultRefineryUnitType(int32 playerId);

    // for AI_UnitWasHit // some of these should probably be methods in api/etc
    char AI_CancelStructure(CUnit* structure);
    CUnit* AI_GetActualTarget(CUnit* attacker, CUnit* target);
    CUnit* checkArbiterCloakingRange(CUnit* unit, uint16 id);
    void AI_OrderUnitCloaking(CUnit* unit);
    void AI_OrderLurkerBurrow(CUnit* unit);
    bool AI_IsUnderAttack(CUnit* unit, int32 flag);
    bool unitIsActiveTransport(CUnit* unit);
    int32 isUnitLoaded(CUnit* unit);
    int32 AI_TryReturnHome(CUnit* unit, int32 dontIssueOrder);
    int32 AI_TransportAction(CUnit* unit);
    int32 AI_AskForHelp(CUnit* self, CUnit* attacker, int32 onlyNearbyGuards);
    int16 getRegionIdFromUnit(CUnit* unit);
    int32 getEnemyAirStrengthInRegion(uint16 regionId, int32 player);
    int32 getEnemyStrengthInRegion(uint16 regionId, int32 player, int32 includeAir);
    bool unitIsPerformingUnbreakableCode(CUnit* unit);
    bool AI_CastSpellBehavior(CUnit* unit, bool isAggro);
    int32 AI_Flee(CUnit* unit, CUnit* enemy);
    bool unitHasGroundPath(CUnit* unit, int16 x, int16 y);
    bool AI_HasNoDefenseBuild(int32 player);
    bool unitCanSeeCloakedTarget(CUnit* target, CUnit* unit);
    void AI_RespondToCloakedThreat(CUnit* threat, CUnit* unit);
    bool unitCanAttackTarget(CUnit* target, CUnit* unit, int32 checkDetection);
    int32 AI_UpdateAttackTarget(CUnit* unit, int32 acceptIfSieged, int32 acceptCritters, int32 mustReach);
    __int64 AI_PrepareToFlee(CUnit* enemy, CUnit* unit);

    // for AI_IterateBuildingAI
    uint16 AI_RemoveFromTowns(CUnit* worker);
    BuildingAi* AI_DeleteBuildingAI(BuildingAi* building, BuildingAiArray* buildingList);
    int32 AI_GetQueuedCount(CUnit* builder, uint32 unitId);
    void AI_PrepareGuardAI(uint16 y, uint32 player, uint16 unitId, uint16 x);

    // Attack extender

    // for AI_TrainAttackForce
    int32 AI_AddRegionToMilitary(AiCaptain* region, int32 unitId, int32 priority);
    uint32 AI_FindUnit(uint32 unitId, uint32 playerId, uint32 type);
    uint32 AI_AddSpendingRequest(uint16 unitId, uint32 type, uint8 player, char priority, void* value);
    uint32 AI_AddBaseUnitSpendingRequest(uint32 unitId, uint32 player, AiCaptain* region, char priority);

    // for AIRegion_GroupForAttack
    uint32 AI_RecalculateRegionStrength(AiCaptain* region);
    CUnit* AI_TargetEnemyInRegion(AiCaptain* region);
    int32 AI_MoveMilitaryToDefend(AiCaptain* region);
    void AI_GetRegionMilitaryTypes(AiCaptain* region, int32* hasGroundMilitary,
    int32* cantAttackGround, int32* cantAttackAir);
    int32 AI_CanTravelToRegion(AiCaptain* region1, AiCaptain* region2);
    CUnit* AI_PickAttackTarget_NoBuildings(AiCaptain* region);
    uint32 AI_MoveMilitaryToOtherRegion(AiCaptain* target, AiCaptain* source, uint32 pullUnitsNearRegion,
    uint32 sendPureSpellcasters, uint32 dontSendFromBunkers);
    AiTown* AI_IterateTownSomething(AiCaptain* region);
    uint32 AI_AttackTimerDecrement(uint32 player);
    int32 AI_HasForcesMovingToAttack(uint32 player);

    // for AI_AttackManager
    uint16 PopulateRegionsWithOwn(char* regionData, uint32 player);
    void PopulateRegionsWithEnemy(uint32 player, char* regionData);
    int32 AI_GetNeighborRegionTypes(char* regionData);
    void AI_FinishRegionAccessibility(char* regionData);
    int32 AI_SetAttackGroupingRegion(uint32 airAttack, uint32 alsoVeryNear, uint32 player, char* regionData,
        int32 x, int32 y, uint32 alwaysOverwriteLastAttack);
    int16 SAI_GetRegionIdFromPxEx(int32 x, int32 y);
    char AI_CancelStructure(CUnit* structure);

    bool unitCantSeeCloakedTarget(CUnit* unit, CUnit* target);												// 00401D60
    bool unitCanInfest(CUnit* unit);																		// 00402750
    void AI_AttackUnit(CUnit* unit);																		// 0043FFD0
    CUnit* findBestAttackTarget(CUnit* unit);																// 00443080
    bool function_00462EA0(CUnit* unit, uint8 unknownByteValue);											// 00462EA0
    void function_00465780(CUnit* unit);																	// 00465780
    void removeOrderFromUnitQueue(CUnit* unit, COrder* order);												// 004742D0
    void function_00474A70(CUnit* unit, CUnit* target, uint8 orderId);										// 00474A70
    bool isTargetWithinMinMovementRange(CUnit* unit, CUnit* target, uint32 range);							// 004763D0
    bool function_00476610(CUnit* unit, int x, int y);														// 00476610
    bool isUnitInWeaponRange(CUnit* unit, CUnit* target);													// 00476870
    void getWeaponBeginIscript(CUnit* unit, IscriptAnimation::Enum anim);									// 00476ED0
    void function_00477510(CUnit* unit);																	// 00477510
    void function_00477820(CUnit* unit, uint8 orderId);														// 00477820
    bool function_00478370(CUnit* unit, uint8 orderId);														// 00478370
    bool function_00478540(CUnit* unit);																	// 00478540
    bool isUnitCritter(CUnit* unit);																		// 0047B450
    void moveToCollide(CUnit* unit, CUnit* target);														    // 00495400
                                                                                                               
    bool isUnitVisible(CUnit* unit);																		// 004E5DB0
    bool isTargetVisible(CUnit* unit, CUnit* target);														// 004E5E30
    uint8 getRightClickActionOrder(CUnit* unit);															// 004E5EA0
    void setNextWaypoint(CUnit* unit);															            // 004EB290
    void makeToHoldPosition(CUnit* unit);																	// 004EB5B0
    bool moveToTarget(CUnit* unit, CUnit* target);															// 004EB720
    bool SetMoveTarget_xy(CUnit* unit, int x, int y);														// 004EB820
    bool function_004EB900(CUnit* unit, CUnit* target);														// 004EB900
    bool changeMovementTargetMoveTarget2(CUnit* unit, int x, int y);										// 004EB9C0

    bool isDistanceGreaterThanHaltDistance(CUnit* unit, int x, int y, uint32 distance);	                    // 00401240
    void checkUnstack(CUnit* unit);												                            // 0042D9A0
    bool beginBuilding(CUnit* unit);												                        // 00467030
    CUnit* beginGasBuilding(CUnit* unit, uint32 unitId);									                // 004678A0
    void addOverlayBelowMain(CSprite* sprite, uint32 imageId, uint32 unk1, uint32 unk2, uint32 unk3);	    // 00498D70
    void replaceSpriteImages(CSprite* sprite, uint32 imageId, uint32 imageDirection);			            // 00499BB0
    void displayLastNetErrorForPlayer(uint32 playerId);										                // 0049E530
    void updateUnitStrength(CUnit* unit);												                    // 0049FA40
    void iscript_OpcodeCases(CImage* image, uint32 offset_, uint32 unk1, uint32 unk2);			            // 004D74C0
    void switchToWarpTexture(CImage* image);												                // 004D8500
    bool setThingyVisibilityFlags(CThingy* thingy);									                        // 004878F0
    CThingy* createThingy(uint32 spriteId, int16 x, int16 y, uint32 playerId);		    	                // 00488210
    void setUnitStatTxtErrorMsg(char* message);										                        // 0048CCB0
    void playUnitSFX(uint32 soundId, uint16 x, uint16 y, uint32 unk1, uint32 unk2);				            // 0048EC10
    uint32 RandomizeShort(uint32 unknown_index);											                // 004DC4A0
    uint32 IterateUnitsAtLocationTargetProc_RecallTargets(CUnit* unit, Box16* coords);	                    // 004E8280
    void function_004EBAE0(CUnit* unit, int32 x, int32 y);								                    // 004EBAE0

    //// spider_mine
    CUnit* SpiderMine_AcquireTarget(CUnit* spiderMine);									                    // 00441270
    void instantCloak(CUnit* unit);												                            // 0049B1E0
    void unburrowGeneric(CUnit* unit);												                        // 004E99D0
    void function_004E9A30(CUnit* unit);												                    // 004E9A30

    // For orders_PlaceMine
    int techUseAllowed(CUnit* a1, __int16 a2, int a3);									                    // 0046DD80
    unsigned int getDistanceFast(int a1, int a2, int a3, int a4);						                    // 0040C360
    int orders_HoldPosSuicidal(CUnit* a1);												                    // 004EB5B0
    void fixTargetLocation(unsigned __int16 a1, Point32* a2);							                    // 00401FA0
    void turnUnit(CUnit* a1, char a2);													                    // 00495F20
    void toIdle(CUnit* a1);																                    // 004753A0
    int isUnitMovable(CUnit* a1);														                    // 00401DC0
    void orders_Nothing2(CUnit* a1);													                    // 00475000
    bool setMoveTargetXY(int a1, int a2, CUnit* a3);									                    // 004EB820

    // For burrowGeneric
    void toggleUnitPath(CUnit* unit);									                                    // 004E42A0
    char switchSpriteId(CSprite* sprite, int32 spriteId);				                                    // 00499860

    //// building_morph
    void refundBuildingCost(uint16 unitId, uint8 playerId);								                    // 002CE70
    void updateBuildingsAI(CUnit* unit);												                    // 0033FE0
    bool isValidMorph(uint16 buildingId);												                    // 005CC60
    void updateNewUnitVision(uint32 id, int x, int y);									                    // 005CE90
    void zergPlaceBuildingCntd(CUnit* unit);											                    // 005D410
    void stopCreepDisappearingAtBuilding(CUnit* unit);												        // 007D770
    void function_0047DE40(CSprite* sprite, uint32 unitId, int x, int y);				                    // 007DE40
    void incrementUnitScoresEx(CUnit* building, int32 unk1, int32 unk2);				                    // 0088D50
    void playBuildingCompleteSound(CUnit* building);									                    // 008F310
    void refreshSpriteData(CSprite* sprite);											                    // 00983A0
    void displayLastNetErrorForPlayer(uint32 playerId);									                    // 009E530
    void replaceUnitWithType(CUnit* unit, uint16 newUnitId);							                    // 009FED0

    //// building_terran
    bool isDistanceGreaterThanHaltDistance(CUnit* unit, int x, int y, uint32 distance);	                    // 00401240
    void fixTargetLocation(Point16* coords, uint32 unitId);								                    // 00401FA0
    bool isUnitPositions2Equal(CUnit* unit);											                    // 00402160
    bool32 buildingAddon(CUnit* unit, uint32 hpGain, bool32 canBeAborted);				                    // 004679A0
    void prependOrderTargetingNothing(CUnit* unit, COrder* order, uint8 orderId);					        // 00474760
    void addOrderTargetingNothing(CUnit* unit, COrder* order, uint8 orderId);					            // 004749A0
    bool canStillPlaceBuilding();														                    // 0048DDA0
    void endBuildingPlacement();															                // 0048E310
    void playBuildingCompleteSound(CUnit* building);									                    // 0048F310
    void function_00494BB0(CUnit* unit, int x, int y);									                    // 00494BB0
    void displayLastNetErrorForPlayer(uint32 playerId);									                    // 0049E530
    uint32 RandomizeShort(uint32 unknown_index);										                    // 004DC4A0
    uint32 RandBetween(uint32 min, uint32 max, uint32 someIndex);						                    // 004DC550
    void initBuilding(CUnit* unit, bool32 flag);									                        // 004E65E0
    bool SetMoveTarget_xy(CUnit* unit, int x, int y);									                    // 004EB820
    bool orderToMoveToTarget(CUnit* unit, CUnit* target);								                    // 004EB980
    bool32 unk_terrainCollision2(CUnit* unit, int x, int y);								                // 004F1870

    // For updatePlacementStateForPlacementBox
    int32 canBuildOnTileLocation(CUnit* a1, int32 player, int32 x_tile, int32 y_tile, int32 unitType,
    int32 placement_state_entry, int32 check_vision, int32 also_invisible, int32 without_vision);	        // 00473FB0
    // For refreshScreen
    int32 updatePlacementStateForPlacementBox_Func();                                                       // 0048DCE0
    int32 unk_buildingPlacementRelated(int32 a1, Point16* a2);                                              // 0048D7F0
    int32 initPlacementBoxPos(Point16* in_out);                                                             // 0048D660
    char unk_buildingPlacementRelatedMouse(int32 a1, int32 a2);                                             // 0048D7B0

    //// liftland
    void setUnitPathing(CUnit* unit, uint8 unkPathRelated);																				// 00401360
    bool isUnitPositions2Equal(CUnit* unit);																							// 00402160
    CUnit* connectAddonCheck(CUnit* unit);																								// 004404A0
    void function_00463470(CUnit* unit);																								// 00463470
    void function_004634E0(CUnit* unit);																								// 004634E0
    void function_00463640(CUnit* unit);																								// 00463640
    void completeAddon(CUnit* unit, CUnit* addon);																						// 00463D50
    void function_00464300(CUnit* unit);																								// 00464300
    void disconnectFromAddOn(CUnit* unit);																								// 00464930
    void function_00469EC0(CUnit* unit, int x, int y);																					// 00469EC0
    void function_00469F60(CUnit* unit, int x, int y);																					// 00469F60
    void function_0046A560(CUnit* unit);																								// 0046A560
    void function_0046A5A0(CUnit* unit);																								// 0046A5A0
    uint32 function_00473FB0(CUnit* unit, uint8 playerId, int x, int y, uint16 unitId, uint8 unk1, uint8 unk2, uint8 unk3, uint8 unk4);	// 00473FB0
    void actUnitReturnToIdle(CUnit* unit);																								// 00475420
    bool function_0047DF90(CUnit* unit);																								// 0047DF90
    void placebuildingProc();																											// 0048E6E0
    void playBuildingLandSound(CUnit* unit);																							// 0048F5A0
    void SetUnitMovementSpeed(CUnit* unit, uint32 newSpeed);																			// 004951C0
    void function_00498150(CSprite* sprite, uint8 newVerticalOffset);																	// 00498150
    void function_004997E0(CSprite* sprite);																							// 004997E0
    void function_004C3B40(CUnit* unit);																								// 004C3B40

    //// create_init_units
    bool32 spreadsCreep(uint32 unitId, bool32 defaultReturn);																			// 00413870
    void function_004148F0(int x, int y, uint32 unitId);																				// 004148F0
    void CreateBuildingLarva(CUnit* main_building);																						// 0049D660
    bool32 function_0049EC30(CUnit* unit);																								// 0049EC30
    CUnit* FindBestUnit(Box16* coords, uint32 searchProc);																				// 004E8830

    //// larva_creep_spawn
    CUnit* getUnitPointer();
    uint8 spreadCreep(uint32 unitId, int x, int y);																						// 00414680
    void AI_TrainingOverlord(CUnit* main_building, CUnit* larva);																		// 00435770
    void updateUnitStrength(CUnit* unit);																								// 0049FA40
    uint32 IterateUnitsAtLocationTargetProc_LarvaCount(CUnit* unit, Box16* coords);														// 004E8280
    void function_004E8DA0(CUnit* unit);																								// 004E8DA0
    bool function_004E8E10(CUnit* unit, Point32* coords);																				// 004E8E10

    // TODO: move somewhere else?
    // SC Variables and Buffers used in this file
    uint32* const mapTileBufWidth = (uint32*)(0x006D0F08);
    uint32* const mapTileBufHeight = (uint32*)(0x006D0C6C);
    uint16** const mapTileBuf = (uint16**)(0x006D0E84);
    Point32* const someCreepPointBuf = (Point32*)(0x006D7578);
    typedef uint32(*isTileVisible_func)(uint32 x, uint32 y);
    typedef bool(*setCreepRelationalData_callback)(uint16* tileBackup, uint8* creepOverlayVal, uint32* retval, uint16* tile, uint32 arg);
    isTileVisible_func* const isTileVisible_ptr = (isTileVisible_func*)(0x006D0C78);

    uint32 getNumSurroundingCreepTiles(uint32 x, uint32 y);																				// 00414290
    void setCreepRelationalData(uint32 x, uint32 y, setCreepRelationalData_callback callback, uint32 arg);								// 00413FA0
    uint32 iterateDirectionalCreepData(uint32 x, uint32 y, uint32 callback, uint32 arg);												// 00413B30

    // TODO: add helpers for these functions
    // for applyCreepAtLocationFromUnitType
    int32 modifyCreepState(int32 x_tile, int32 y_tile, int32(__stdcall* creepChange)(int32, int32, int32, int32, int32), void* param);	// 00413FA0
    int32 runCreepRandomizer(int16* originalTile, int32 creepTileBorders, int32* outCreepStatus, int16* currentTile, int32 unused);		// 004140A0 // "note possible desync"
    int32 unkCreepBorders(int32 a1, int32 a2);																							// 006D0C78
    uint32 forEachAdjacentTile(int32(__stdcall* callback)(int32, int32, int32, int32, void*), void* param, int32 x_tile, int32 y_tile);	// 00413B30
    int32 unkCreepTile(int32 pos, int32 originalTile, int32 x_tile, int32 y_tile, void* param);											// 00414440
    char unkSpreadCreep(int32 a1, int32 a2, int16 a3);																					// 00414680

    //// burrow_tech
    void actUnitReturnToIdle(CUnit* unit);																								// 00475420
    void function_004754F0(CUnit* unit, uint32 unitId, uint32 unk1, uint32 unk2, uint32 orderId, uint32 unk3, uint32 unk4, uint32 unk5);// 004754F0
    void function_004E97C0(CUnit* unit);																								// 004E97C0

    //// die_order
    void clearUnitSprites(CSprite* unit_sprite, CSprite* subunit_sprite);																// 00496F00
    void function_00497480(CSprite* thingySprite, uint32 visibility_flags);																// 00497480
    void _UnitDestructor(CUnit* unit);																									// 004A0740
    void UnitDestructor(CUnit* unit);																									// 004A0990
    void hideAndDisableUnit(CUnit* unit);																								// 004E6340

    //// CMDRECV_Cancel
    void CancelResearch_Proc(CUnit* unit);																								// 00453E30
    void CancelUpgrade_Proc(CUnit* unit);																								// 00454280
    void refundQueueSlot(CUnit* unit, uint16 slot);																						// 00466A70
    void function_00466E40(CUnit* unit);																								// 00466E40
    void CancelAddon_Proc(CUnit* unit);																									// 004E66E0

    //// CMDRECV_Build
    bool32 OrderAllowed(CUnit* unit, uint16 order, uint32 nationID);																	// 0046DC20
    bool placeBuildingMsg(uint32 result_function_473FB0);																				// 0048D930
    bool CMDRECV_PlaceBuildingAllowedHelper(CUnit* unit, uint8 orderId, uint16 builtUnitId);											// 0048DBD0
    void function_0048DE70(CUnit* unit, uint32 orderId, uint16 dimensionX, uint16 dimensionY, uint32 builtUnitId);						// 0048DE70
    void cmdRECV_PlaceBuilding(int16 x, int16 y, uint8 orderId, uint16 builtUnitId);													// 0048E190

    //// CMDRECV_MergeArchon
    CUnit* templarMergePartner(CUnit* unit, CUnit** units_list, uint32 units_list_length);												// 004BFA80

    //// CMDRECV_Stop
    void parseOrdersDatReqs();																											// 0046D450
    int parseRequirementOpcodes(CUnit* unit, uint32 orderId, uint32 playerId, uint32 address, uint32 req_offset);						// 0046D610

    //// give_unit
    void ApplySpeedUpgradeFromUnitType(CUnit* unit);																					// 00454370
    void function_00463040(CUnit* unit);																								// 00463040
    void function_00476D30(CUnit* unit);																								// 00476D30
    void incrementUnitScores(CUnit* unit, int32 unkScore);																				// 00488BF0
    void function_004A1E50(CUnit* unit, uint32 unk);																					// 004A1E50
    void CreateUnitSelection(CUnit* unit, uint32 selectionIndex);																		// 004E6180
    void DestroyUnitSelection(CUnit* unit);																								// 004E6290
    void function_004F3B70(CUnit* powerup);																								// 004F3B70

    //// cast_order
    uint32 getUpgradedWpnCooldown(CUnit* unit, uint8 weaponId);																			// 00475DC0
    bool function_00476640(CUnit* unit, uint32 weaponId);																				// 00476640
    uint32 get_statTxt_Str_0(CUnit* unit, uint32 playerId, char* message);																// 0048EF30
    bool canCastSpell_0(CUnit* unit);																									// 00492140
    uint32 function_00492620(CUnit* unit, bool32 wasMainOrderStateZero);																// 00492620

    void incrementUnitDeathScores(CUnit* unit, uint8 player);																			// 00488AF0
    void initializeEmptyUnitsLinkedListRef_Sub49E4E0(CUnit* unit, uint8 playerId);														// 0049E4E0
    void GiveUnitHelper(CUnit* unit, uint32 playerId, int32 unkScore);																	// 0049EFA0
    void changeUnitButtonSet_Sub_4E5D60(CUnit* unit, uint16 unitId);																	// 004E5D60
    void showAndEnableUnit(CUnit* unit);																								// 004E6490

    //// tech_target_check
    bool isInfestableUnit(CUnit* unit);																									// 

    //// trap_hooks
    // For updateUnitStrengthAndApplyDefaultOrders
    void playSoundFromDirection(CUnit* a1, int a2);																						// 0049B5B0
    void refreshUnitVision(CUnit* a1);																									// 004E5F30
    void initUnitTrapDoodad(CUnit* a1);																									// 004E6490
    COrder* removeOrderFromUnitQueue(COrder* a1, CUnit* a2);																			// 004742D0
    COrder* performAnotherOrder(uint8 a1, int a2, CUnit* a3, int16 a4, Point32* a5, CUnit* a6);											// 004745F0
    void order(int a1, CUnit* a2, int8 a3, Point32* a4, CUnit* a5, int a6);																// 00474810
    uint32 getUnitStrength(CUnit* a1, int a2, bool a3);																					// 00431800

    //// make_nydus_exit
    void playWorkerErrorSound(CUnit* unit, uint32 errorTextIndex);																		// 0048F6C0

    //// research_upgrade_orders
    bool isUnitUpgradeAvailable(CUnit* unit);																							// 004033D0
    void ApplySpeedUpgradeFromUpgradeType(CUnit* unit, uint8 upgradeType);																// 00454540
    void playUpgradeCompleteSound();																									// 0048F070
    void playResearchCompleteSound();																									// 0048F150
    void setUpgradeLevel(uint32 playerId, uint32 upgradeType, uint8 upgradeLevel);														// 004CE770

    //// merge_units
    bool advanceRemainingBuildTime(CUnit* unit);																				// 00466940
    uint8 function_0046A820(CUnit* unit);																								// 0046A820
    void orderImmediate(CUnit* unit, uint8 order);																						// 00474B40
    void playMorphingCompleteSound(CUnit* unit);																						// 0048F440
    void mergeStatusIntoUnit_Sub493180(CUnit* unit_dst, CUnit* unit_src);																// 00493180
    void function_0049B440(CUnit* unit, uint32 soundId);			 																	// 0049B440
    void changeUnitButtonSet_Sub4E5D60(CUnit* unit, uint16 unitId);																		// 004E5D60
    bool delayedSetMoveTarget_xy_Sub4EB960(CUnit* unit, uint16 x, uint16 y);															// 004EB960

    //// nuke_orders
    void setAllOverlayDirectionsGeneric(CUnit* unit, uint32 unk);																		// 00401140
    void AI_orderUnitCloaking(CUnit* unit);																								// 0043B970
    uint32 function_0043DB50(CUnit* unit);																								// 0043DB50

    //// target_related

    // for isUnitTargetOutOfMaxRange
    int getUnitMaxWeaponRange(CUnit* a1, unsigned __int8 a2);
    bool isTargetWithinMinMovementRange(CUnit* a1, unsigned int a2, CUnit* a3);

    // for isReadyToAttack
    unsigned int getDistanceToUnit(CUnit* a1, CUnit* a2);
    unsigned int distanceFromDboxCorner(CUnit* a1, __int16 a2, __int16 a3);
    bool isAngleInAttackAngle(int a1, int a2, CUnit* a3, unsigned __int8 a4);

    // for canCastOrderedSpellNow
    bool isTargetWithinMinRange(CUnit* a1, unsigned int a2, CUnit* a3);
    int getFlingyHaltDistance(int a1);

    // for ordersWatchTarget
    void unknownOrdersFunc(int a1, __int16 a2, __int16 a3);

    // for ordersStayInRange
    void prependOrderTargetingGround(__int8 a1, __int16 a2, __int16 a3, int a4);
    signed int moveTowards(CUnit* a1, CUnit* a2);
    int ordersHoldPositionSuicidal(CUnit* a1);
    int getDirectionFromPoints(int a1, int a2, int a3, int a4);
    CUnit* setSecondaryWaypoint(CUnit* a1, int a2, int a3);
    Point32 setResourceTarget(CUnit* a1);

    // for genericRightClickUnitProc2
    int32 isOrderValid(char a1, CUnit* a2, int32 a3, CUnit* a4, int32 a5);						// 00474D90
    uint16 getUnitAbilityErrorStatString(CUnit* a1, char a2, int32 a3);							// 00491E80

    // for CMDRECV_HoldPosition
    CUnit* getActivePlayerFirstSelection();														// 0049A850
    int32 orderAllowed(CUnit* a1, uint16 a2, int32 a3);											// 0046DC20
    void targetedOrder(int16 a1, int32 a2, CUnit* a3, int16 a4, int32 a5, int32 a6, char a7, int32 a8, int32 a9, int32 a10);	// 004754F0

    // for checkUnitVisibility
    uint32 getDetectingPlayers(CUnit* unit);													// 
    uint32 iteratePlayerUnitsSomething(uint32 aResult, CUnit* unit);							// 
    void makeSpriteVisible(CSprite* sprite);													// 
    int32 updateBuildingLandUnitSelection(uint32 unused, CUnit* selection);						// 
    char removeSelectionCircle(CSprite* sprite);												// 
    CImage* makeSpriteInvisible(CSprite* sprite);												// 

    //// receive_command
    void function_004756B0(CUnit* unit, uint8 orderId, uint32 unk1, uint32 unk2, uint32 bCommandType);								// 004756B0
    void function_004756E0(CUnit* unit, CUnit* target, uint32 orderId, uint32 bCommandType);										// 004756E0
    uint32 function_0049A410(CUnit* unit, uint8 orderId);																			// 0049A410
    bool CanTargetSelf(CUnit* unit, uint8 orderId);																					// 0049A480
    void function_0049A500(CUnit* unit, uint8* array_of_data);																		// 0049A500
    void function_0049A8C0(uint8* array_of_data, bool32 bCanBeObstructed);															// 0049A8C0

    //// load_unload_proc
    void IgnoreAllScriptAndGotoIdle(CUnit* unit);																					// 00476550
    uint32 function_004E76C0(CUnit* transport, CUnit* loaded_unit, Point16* pos);													// 004E76C0
    void function_004E7E10(CUnit* unit, uint32 unk);																				// 004E7E10

    //// stats_panel_display
    void StatsNukesCount(BinDlg* dialog, uint32 index);																				// 00425310
    void StatHangerCount(BinDlg* dialog, uint32 index);																				// 004253D0
    void StatsShieldLevel(BinDlg* dialog, uint32 index);																			// 00425510
    void StatsArmorLevel(BinDlg* dialog, uint32 index);																				// 00425600
    void StatsWeaponLevel(BinDlg* dialog, uint32 index, uint32 weaponId);															// 00425790
    bool StatSpidermineCount(BinDlg* dialog, uint32 index);																			// 00426300

    // For getUnitRankString
    __int8 unitGetRace(CUnit* a1);																									// 0047B150
    char* getStatTxtString(__int16 a1);																								// 004C36F0

    // For statsWeaponLevel / statsArmorLevel
    char getArmorCount(CUnit* unit, int32 a2);																						// 00453FC0
    void showDialog(BinDlg* dialog);																								// 004186A0
    int32 disableDialog(BinDlg* dialog);																							// 00418640
    void updateDialog(BinDlg* dialog);																								// 0041C400

    //// bunker_hooks
    CUnit* findRandomAttackTarget(CUnit* unit);																						// 00442FC0
    bool32 attackApplyCooldown(CUnit* unit);																						// 00478B40
    void setImageDirection(CImage* image, int8 direction);																			// 004D5F80

    //// api
    void ApplySpores(CUnit* unit);
    void createBullet(uint8 weaponId, CUnit* source, int16 x, int16 y, uint8 attackingPlayer, uint8 direction);
    CBullet* createBulletRet(uint8 weaponId, CUnit* source, int16 x, int16 y, uint8 attackingPlayer, uint8 direction);
    void weaponBulletShot(CUnit* target, CBullet* bullet, uint32 dmgDivisor);

    //// unit_morph
    uint8 setArchonPathing(CUnit* unit);																							// 0046A820
    bool hasSuppliesForUnit(uint32 unitId, uint32 playerId, bool32 canShowErrorMessage);											// 0042CF70
    CUnit* zergDualBirth(CUnit* unit);																							    // 0045D910
    void orderNewUnitToRally(CUnit* unit, CUnit* factory);																			// 00466F50
    void AI_TrainingUnit(CUnit* unit_creator, CUnit* created_unit);																	// 004A2830
    void removeAITownGasReferences(CUnit* unit);                                                                                    // 00432760
    void deleteBuildingAi(void* building_ai_list, BuildingAi* ai);                                                                  // 00404500
    void deleteWorkerAi(void* worker_ai_list, WorkerAi* ai);                                                                        // 00404470
    void deleteAiTownIfNeeded(AiTown* town);                                                                                        // 004347E0

    //// hallucination_spell
    // for orders_Hallucination1
    bool ordersSpell_Sub_4926D0(CUnit* unit, uint32 techId, uint16* techEnergyCost, uint32 sightRange, uint32 error_message_index);	// 004926D0

    void UnitDestructor(CUnit* unit);
    bool placeHallucination(CUnit* unit);																					// 004F66D0

    // for createHallucination
    void applyHallucinationStats(CUnit* halUnit);																					// 004F6180
    int32 hallucinateScarabs(CUnit* a1, CUnit* a2);																					// 004F65B0

    //// feedback_spell
    void replaceFeedbackSprite(CUnit* unit, uint32 spriteId);																		// 004F6350

    //// weapon_effect
    bool isUnitDistanceWithin(CUnit* target, CBullet* bullet, uint32 radius);
    void doWeaponHit(CUnit* target, CUnit* attacker, uint8 dmgDivisor);
    void Ai_UnitWasHit(uint32 main_target_reaction, uint32 dont_call_help, CUnit* attacker, CUnit* target);

    // for airSplash
    CUnit* findBestUnit(Box16* radius, int32 bullet, int32(__fastcall* func)(int32, int32));										// 004E8830
    int32 splashFunc(CBullet* bullet, CUnit* unit);																					// 0048B570

    //// mindcontrol_spell
    void MindControlHit(CUnit* attacker, CUnit* target);																			// 004F6910

    //// weapon_damage
    uint16 getUnitStrength(CUnit* unit, bool useGroundStrength);
    CUnit* createHallucination(CUnit* target, uint32 playerId);

    // For updateCorrosiveSpores
    uint32 getCorrosiveSporesImage(CUnit* unit);																					// 004F41F0

    //// recharge_shields
    void insertFirstOrder(CUnit* unit, uint8 orderId);			//0x004749D0
    uint8 hasOverlay(CUnit* unit);								//0x0047B720
    bool isHangarUnit(CUnit* unit);

    //// stopholdpos_orders
    CUnit* MedicHeal_TargetAcquire(CUnit* medic);																					// 004422A0
    uint32 doMedicHeal(CUnit* unit, CUnit* target);																					// 00463C40
    void function_00476FC0(CUnit* unit, CUnit* target, uint32 unk1, uint32 unk2);													// 00476FC0
    bool32 function_004770E0(CUnit* unit);																							// 004770E0

    //// medic_orders
    bool32 AI_OrderHeal(CUnit* unit, CUnit* target);																				// 00463530
    void function_4748E0(CUnit* unit, int x, int y, uint8 orderId);																	// 004748E0
    void function_474940(CUnit* unit, CUnit* target, uint8 orderId);																// 00474940
    void orderInterrupt(CUnit* unit, CUnit* target, uint16 x, uint16 y, uint32 orderId);											// 00474C30
    void unitOrderMoveToTargetUnit(CUnit* unit, CUnit* target);																		// 00479FE0
    bool32 AI_CastSpellBehaviour(CUnit* unit, uint32 unknown);																		// 004A13C0

    //// load_unload_orders
    bool32 function_00438E70(CUnit* unit, int x, int y);																			// 00438E70
    void function_0043DF30(CUnit* unit);																							// 0043DF30
    void unknownSpecialOrder(CUnit* unit, COrder* order, uint8 orderId, int16 x, int16 y, CUnit* target, uint16 targetUnitId);		// 00474540
    void _doOrder(CUnit* unit, uint32 orderId, int x, int y, bool stopPreviousOrders);												// 00474C70
    bool32 function_00477160(CUnit* unit, uint32 unk1, uint32 unk2, uint32 unk3);													// 00477160
    CUnit* getLoadedUnitFromIndex(CUnit* transport, int index);																		// 004E6C40
    bool32 getFirstLoadedWorker(CUnit* transport);																					// 004E6D90
    bool32 function_004E6EF0(CUnit* unit, CUnit** unitTargetOffset);																// 004E6EF0
    void loadUnitProc(CUnit* unit, CUnit* unitToLoad);																				// 004E78E0	(Hooked)
    bool32 unloadUnitProc(CUnit* unit);																								// 004E7F70 (Hooked)

    //// siege_transform
    void unitSetRetreatPoint(CUnit* unit, uint8 angleValue);																		// 004011A0
    bool getCloserToDestination_sub402BE0(CUnit* unit);																				// 00402BE0
    void orderReturnToIdle(CUnit* unit);																							// 00463770
    bool checkForCloseUnit_sub464290(CUnit* unit);																					// 00464290
    void playSubunitIscript(CUnit* unit, IscriptAnimation::Enum anim);																// 00474380

    //// unit_train
    uint32 getHPGainForRepair(CUnit* unit);																							// 00402C40
    void refundUnitTrainCost(uint32 unitId, uint8 playerId);																		// 0042CEC0
    void function_00432430(CUnit* unit, uint32 buildQueueSlot);																		// 00432430
    void addHangarUnit(CUnit* main_unit, CUnit* added_unit);																		// 00466300
    CUnit* attemptTrainHatchUnit(CUnit* trainer, uint32 builtUnitID, uint32 flag);													// 00468200

    //// weapon_fire
    // For progressBulletFlight
    void progressBulletMovement(CBullet* a1);																						// 0048B250
    void doMissileDmg(CBullet* bullet);																								// 0048B770

    // For bulletBehavior_Bounce
    CBullet* changeMovePos(CBullet* a1, int32 a2, int32 a3);																		// 00495240
    void progressBulletMovement_func(CBullet* a1);																					// 0048B250
    CUnit* findNextBounceTarget_func(CBullet* a1);																					// 0048B1E0

    // For findNextBounceTargetProc
    bool unitIsSubunit(CUnit* a1);																									// 00401D40
    bool unitIsCarrier(CUnit* a1);																									// 00401470
    bool unitIsReaver(CUnit* a1);																									// 00401490
    bool unitHasPathToUnit(CUnit* a1, CUnit* a2);																					// 0049CBB0
    int32 canBeInfested(CUnit* a1, CUnit* a2);																						// 00402210

    // For findNextBounceTarget
    int32 iterateUnitsAtLocationTargetProc(Box32* a1, int32(__fastcall* a2)(int32, int32), CUnit* a3);								// 004E8280

    // For writeBullets
    /*	
    void packBulletData(CBullet* a1, int32 a2); // 0048A9A0
    size_t _lockwrite(const void* a1, size_t a2, size_t a3, FILE* a4); // 00411931
    void* SMemAlloc(int32 a1, char* a2, int32 a3, int32 a4); // 0041006A
    int32 compressWrite(void* a1, uint32 a2, FILE* a3); // 004C3450
    bool SMemFree(void* a1, char* a2, int32 a3, char a4); // 00410070
    CBullet* unpackBulletData(CBullet* a1, int32 a2); // 0048A8D0
    */

    //// wpnspellhit
    bool function_0042D810(int x, int y, uint32 unitId);														// 0042D810
    CUnit** getAllUnitsInBounds(Box16* coords);																	// 0042FF80
    void incrementUnitKillCount(CUnit* unit);																	// 004759C0
    uint32 getUnitDamageBonus(CUnit* unit, uint8 weaponId);														// 00475EC0
    void attackOverlayAndNotify(CUnit* attacker, CUnit* target, uint32 weaponType, uint32 direction);			// 00479730
    CThingy* createThingy(uint32 spriteId, int16 x, int16 y, uint32 playerId);									// 00488210
    uint32 GetUnitBulletDamage(CBullet* bullet, CUnit* target);													// 0048ACD0
    uint32 getSpellStatString(CUnit* target, uint32 techId, uint8 playerId);									// 00491E80
    void function_492CC0(CUnit* unit);																			// 00492CC0
    uint8 function_492CC0_ret(CUnit* unit);																		// 00492CC0
    void setAllImageGroupFlagsPal11(CSprite* sprite);															// 00497430
    void IterateUnitsAtLocationTargetProc_CorrosiveAcid(uint32 attackingPlayerId, Box16* coords);				// 004E8280
    void IterateUnitsAtLocationTargetProc_Ensnare(CUnit* attacker, Box16* coords);								// 004E8280
    void IterateUnitsAtLocationTargetProc_Plague(CUnit* attacker, Box16* coords);								// 004E8280
    void IterateUnitsAtLocationTargetProc_Maelstrom(CUnit* attacker, Box16* coords);							// 004E8280
    void IterateUnitsAtLocationTargetProc_StasisField(CUnit* attacker, Box16* coords);							// 004E8280
    void createEnsnareOverlay(CUnit* a1);																		// 004F45E0

    // For createScanner
    int32 setUnitStatTxtErrorMsg(int32 a1);																		// 0048CCB0
    int32 playSoundFromSource(int32 a1, CUnit* a2, int32 a3, int32 a4);											// 0048ED50

    //// selection
    bool unitIsOwnedByCurrentPlayer(CUnit* unit); 																// 00401170
    bool isUnitBurrowed(CUnit* unit);																			// 00402A70
    void SC_memcpy_0(uint32 dest, uint32 src, uint32 memsize);													// 00408FD0
    void function_00468670(CUnit* unit);																		// 00468670
    bool unit_isUnselectable(uint16 unitId);																	// x046ED80
    void function_0046F040(CUnit* current_unit, CUnit** unit_list, CUnit* clicked_unit, uint32 list_length);	// x046F040
    uint32 combineLists_Sub_6F290(CUnit* unit, CUnit** unit_list_1, CUnit** unit_list_2, uint32 list_length);	// 0046F290
    void applyNewSelect_Sub_6FA00(CUnit** unit_list, uint32 unit_list_length);									// 0046FA00
    uint32 CUnitToUnitID(CUnit* unit);																			// 0047B1D0
    bool unit_IsStandardAndMovable(CUnit* unit); 																// 0047B770
    void selectBuildingSFX(CUnit* unit);																		// 0048F910
    bool32 selectSingleUnitFromID(uint32 unitIndex);															// 00496D30
    void CreateNewUnitSelectionsFromList(CUnit** unit_list, uint32 unit_list_length);							// 0049AE40
    void CMDACT_Select(CUnit** unit_list, uint32 unit_list_length); 											// 004C0860

    //// cloak_nearby_units
    void secondaryOrder_Cloak(CUnit* unit);																		// 00491790

    //// update_unit_state
    bool unitHpIsInRedZone(CUnit* unit);						// 0x004022C0
    void RestoreAllUnitStats(CUnit* unit);						// 0x00492F70 (hooked by hooks\update_status_effects.cpp)

    // For updateUnits
    bool isSelectedUnitGroupEnabled();																						// 0049A2C0
    char refreshLayers3And4();																								// 0048D9A0
    void cancelTargetOrder();																								// 0048CA10
    int32 updateDisruptionWebStatus();																						// 004EB2F0
    CUnit* progressDyingUnitFrame(CUnit* unit);																				// 004EB5E0
    void revealSightAtLocation(int32 sightRange, uint32 visionFlags, int32 ptX, int32 ptY, bool isAirUnit);					// 004806F0
    void updateVisibility(CUnit* unit);																						// 004EBE10
    void progressActiveUnitFrame(CUnit* unit);																				// 004EBC30
    void checkUnitVisibility(CUnit* unit);																					// 00443390
    void progressUnitFrame(CUnit* unit);																					// 004ECF70
    void unitUpdate(CUnit* unit);																							// 004EC420
    void updateUnitPower();																									// 00493790

    // For updateBurrowingCloakingUnits
    int32 isUnitCloaking(CUnit* unit);																						// 004916E0
    void playSoundFromDirect(CUnit* unit, int32 soundId);																	// 0049B5B0

    // For doCloak
    void updateSpriteFlagOnPalette(CSprite* sprite);																		// 00497E80
    void showImagesHiddenByCloak(CSprite* sprite);																			// 00498170
    void updateSpriteImagesForCloak(CSprite* sprite);																		// 00498A90

    //// select_larva

    uint32 getLarvaeUnitsFromList(CUnit** unitList, CUnit** outputList);													// 00423190
    bool32 selectSingleUnitFromID(uint32 unitIndex);																		// 00496D30
    void selectMultipleUnitsFromUnitList(CUnit** unitList, uint32 unitsCount, bool32 unk1, bool32 unk2);					// 0049AEF0

    //// uifeatures

    int32 BWFXN_PrintXY(const char* text, int32 x, int32 y);
    int32 GetColorHandle(uint8 color); // Colors: 1 - last color, 2 - 8, 14 - 17, 21 - 25, 27 - 31

    // For targetedOrder
    COrder* queueNewOrder(COrder* a1, int16 a2, int16 a3, CUnit* a4, uint8 a5, int32 a6, int16 a7, Point32* a8, CUnit* a9);	// 00474B90
    int32 showStatTxtToPlayer(int32 a1, int32 a2);																			// 0048CF00
    void setOrderTarget(uint8 a1, int16 a2, int16 a3, int32 a4, int32 a5, CUnit* a6, int32 a7, Point32* a8, CUnit* a9);		// 00475470

    // For selectUnits
    int32 selectSingleUnitFromID(int32 a1);																					// 00496D30
    int32 selectMultipleUnitsFromUnitList(int32 a1, CUnit** a2, uint8 a3, int32 a4);										// 0049AEF0
    int32 SNetGetTurnsInTransit(uint32* turns);
    uint8 sendTurn();
    int32 SErrGetLastError();
    uint8 packetErrHandle(int32 result, int32 a1, const char* a3, int32 a4, int32 a5);

    //// repair_order

    void getRepairInfo(CUnit* repairTarget, uint32* repairCostMineral, uint32* repairCostGas, int16* unkRepairInfo);		// 00466D10
    bool verifyResources(uint32 mineralCost, uint32 gasCost, uint32 playerId);												// 00466EE0
    bool ord_repair_subtract(CUnit* worker, CUnit* target);																	// 00466FB0
    // For isOrderValid
    bool unitIsResource(CUnit* unit);                                                                                       // 004688E0

    //// hydra_specific
    uint8 getHangerTrainCount(CUnit* unit);
    bool unitIsTrainingOrMorphing(CUnit* unit);
    uint8 initializeRandomizerInfo();
    void AI_Loop();
    int32 updateActiveTileInfo();
    int32 removeFOWCheat();
    void updateUnits();
    int32 imageDrawingBulletDrawing();
    void updateImages();
    void updateThingys();

    //// psi_field_util
    bool addPsiFieldSprite(CUnit* unit);

    // stim_packs
    void QueueGameCommand(uint8* params, uint32 param_length);

    // misc
    CThingy* createThingyFunc(uint32 spriteId, int16 x, int16 y, uint32 playerId);
    bool func_dounitscollide(CUnit* first, CUnit* second);
    void unburrow(CUnit* unit);
    CImage* CreateHealthBar(int a1, CSprite* a2);
    void RemoveHPBar(CImage* a1);
    void refundAllQueueSlots(CUnit* unit);
    void* SMemAlloc(uint32 amount, uint32 unkFilename, uint32 unkLine, uint32 defaultValue);	//1006A (based on Storm.dll definition)
    void SetCallbackTimer(BinDlg* dialog, uint32 unk1, uint32 unk2, uint32 unk3);			//16090
    void function_0049A7F0(CUnit* unit);											//9A7F0
    uint32 fxnInteract(BinDlg* dialog, uint32 data_struct_offset);
    void registerUserDialogAction(BinDlg* dialog, uint32 array_of_fxnInteractFunc, uint32 sizeOfArray);	//18100
    void function_418E00(BinDlg* dialog);																//18E00
    void replayStatBtns(BinDlg* dialog);																//27A80
    void function_4591D0();																				//591D0
    void function_459770();																				//59770
    void BINDLG_BlitSurface(BinDlg* dialog);															//C35F0
    uint32 fxnInteract(BinDlg* dialog, uint32 data_struct_offset);
    int32 req_check(uint32 reqFunc, uint16 reqVar, uint32 playerId, CUnit* unit);
    void updateMineralPatchImage(CUnit* mineralPatch);
    int32 canBuildOnTileLocation(CUnit* a1, int32 player, int32 x_tile, int32 y_tile, int32 unitType,
        int32 placement_state_entry, int32 check_vision, int32 also_invisible, int32 without_vision);
    void updateImagePositionOffset(CImage* image);

    //Identical to function @ 0x004F3AF0
    void setResourceAmountCarried(CUnit* worker, uint8 amountCarried, uint32 chunkImageId, bool isMineral);

    // ai misc
    int Ai_HasBuildingsNeedingPower(uint32 player);
    int Ai_CountEconomyUnits(uint32 player, uint32 unit_id);
    int getSuppliesRemaining(uint8 player, uint16 unit_id);
    int getSuppliesUsed(int byte_data, uint8 player);
    int32 AI_AddCostToNeeds(int32 a1, int32 a2, int32 a3);
    int32 AI_AddSpendingRequest(int16 a1, int32 a2, char a3, char a4, void* a5);
    bool isAIAllowedToBuild(int32 a1, int32 a2);

    //// misc
    int8 SETS(int x);
    int8 OFSUB(int x, int y);
}