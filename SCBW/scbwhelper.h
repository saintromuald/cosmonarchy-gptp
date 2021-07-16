#pragma once
#include "SCBW\scbwdata.h"

namespace helper
{
	bool HasMoneyCanMake(Unit* builder, u32 unitToBuild);

	// For loadRaceUI
	void getRaceSoundList(s8 a1);
	s32 dialogGrpConstruct(s32 a1, s32 a2, s32 a3, s32(__fastcall* a4)(s32, s32, s32, s32));
	void loadGraphic(s8 a1, s32 a2, s32 a3, s8 a4, s32 a5);

	s32 randFunc();
	bool unitHPbelow33_percent(Unit* a1);
	bool unitHPbetween33_66_percent(s32 a1, Unit* a2);
	s32 getMaxHP(Unit* a1);
	u16 getLastQueueSlotType(Unit* a1);
	void addTextToDialog(BinDlg* a1, s16 a2, const char* a3);
	void drawEnergy(BinDlg* a1);
	void hideOptionalStatusScreenControls(BinDlg* a1);
	void showControlGroup(BinDlg* dialog, s16 firstId, s16 lastId);
	void drawShieldUpgrade(s32 a1, u16 a2);
	void hideDialog(BinDlg* dialog);

	// For drawSupplyUnitInfo
	s32 getSuppliesUsed();
	u16 getLastQueueSlotType(Unit* a1);
	u32 getSuppliesAvailable(u8 a1, s32 a2);
	void drawArmor(s32 a1, s32 a2, u16 a3);
	void unitStatAct_Standard(void* a1);

	// For AI_TargetEnemyProc
	bool AI_UnitCanAttack(Unit* a1);

	// For drawBuildingInfo
	s32 needRedrawUI();
	Unit* genericStatusUpdateDrawnValues();
	void drawProgressBar(BinDlg* a1);
	void drawNukeSiloInfo(BinDlg* a1);
	bool unitIsActiveTransport(Unit* a1);
	bool isQueueSlotActive(Unit* unit, s32 slot);
	void drawTrainingProgress(BinDlg* dialog);
	bool isConstructingAddon(const Unit* a1);
	bool isAttemptingProtossBuild(Unit* a1);
	void drawAddonProgress(BinDlg* a1);
	void drawTechProgress(BinDlg* a1);
	void drawUpgradeProgress(BinDlg* a1);
	bool unitIsGeyser(Unit* a1);
	void drawGasQuantity(BinDlg* a1);
	s8 unitGetGroundWeapon(Unit* a1);
	void drawDisplayButtons(BinDlg* a1);
	BinDlg* getControlFromIndex(BinDlg* a1, s16 a2);
	void setTextStr(BinDlg* a1);
	void setUnitStatusStrText(BinDlg* a1);
	s32 unitGetName(Unit* a1);
	void drawSupplyUnitInfo(BinDlg* dialog); // 00427540

	// For getUnitName
	const char* getUnitNameStatString(u16 unitId); // 0047B090

	// For getUnitStrength
	s32 getCurrentHitPointsPlusShields(Unit* a1);
	s32 getBaseStrength(s32 a1, Unit* a2);
	s32 getMaxHP(Unit* a1);

	// For AI_RestorationRequirementsProc
	s32 canWeaponTargetUnit(u8 a1, Unit* a2, Unit* a3);

	// For AI_EMPShieldRequirementsProc
	s32 AI_TargetUnitIsWorthHitting(Unit* a1, Unit* a2);
	s32 createSplashWeaponProc(s16 a1, s16 a2, s16 a3, s32(__fastcall* a4)(s32, s32), s32 a5);
	s32 unitShieldsClumpInAreaAccumulatorProc(Unit* a1, Unit* a2);

	// For hallucinationHit
	void unitWasHit(Unit* a1, Unit* a2, s32 a3);
	void doAttackNotifyEvent(Unit* a1);
	Image* createShieldOverlay(s32 a1, Unit* a2);

	// For drawImageHPBar

	s32 unk_billions(s32 a1, s32 a2, s32 a3, s32 a4);
	s32 unk_billions2(s32 a1, s32 a2, s32 a3, s32 a4);
	s32 hpBarSurfaceBuffer(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5);
	s32 hpBarSurfaceBuffer2(s32 a1, s32 a2, u16 a3, s32 a4);
	s32 renderHPBar(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5);
	s32 getUnitMaxEnergy(Unit* a1);
	s32 getHpBarDrawSizes(s32 a1, s32 a2, s32 a3, s32 a4);

	// For canCastSpell
	bool unitIsFrozen(Unit* a1);
	bool isUnitCasterNotHallucination(Unit* a1);
	b32 getActiveTileFlag(s16 y, s16 x, s32 includeFlags, s32 excludeFlags);
	Unit* getTargetUnitAbilityErrorStatString(Unit* a1, char a2, s32 a3);

	// For modifyUnitShields
	s32 SErrSetLastError(char a1);

	// For compileHealthBar
	void updateImageDrawData(Image* result);
	void playImageIscript(Image* img, u8 iscriptAnim);

	// For AI_FindTransport
	bool canEnterTransport(Unit* a1, Unit* a2);
	s32 getUnitDistanceToHalt(s32 a1, Unit* a2, s32 a3);

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
	s32 randGetSFXBetween(s32 soundIdMin, s32 soundIdMax);										// 0048E940
	s16 SAI_GetRegionIdFromPxEx(s32 x, s32 y);													// 0049C9F0

	// For AI_TrainUnit
	char AI_TrainingWorker(Unit* result, Unit* parent);											// 00435700
	void AI_TrainingOverlord(Unit* result, Unit* parent);										// 00435770
	void AI_TrainingNormal(Unit* result, Unit* parent);											// 00435DB0
	void AI_OrderToDestination(Unit* unit, s32 orderId, s32 x, s32 y);							// 0043D5D0
	void AI_AssignCaptainToSlowestUnit(AiRegion* region);										// 00436F70

	// For AI_CreateNuke
	Unit* createUnit(s16 unitType, s32 x, s32 y, s32 playerId);
	void updateUnitStatsFinishBuilding(Unit* unit);
	u32 updateUnitStrengthAndApplyDefaultOrders(Unit* unit);
	void hideUnit(Unit* unit);

	// For AI_TrainingBroodling
	void AI_AssignMilitary(s32 regionType, AiRegion* region);
	void AI_AddMilitary(AiRegion* region, Unit* unit, s32 always_this_region);

	// For AI_TrainingWorker
	char AI_AddWorkerStructureAI(Unit* worker, AiTown* town);									// 00433DD0

	// For AI_AddUnitAI
	UnitAi* AI_CreateBuildingAi(BuildingAiArray* buildingAI);
	void orderComputerClear(Unit* unit, u8 orderId);
	Unit* setSecondaryOrder(Unit* result, char orderId);
	void AI_MakeGuard(s32 playerId, Unit* unit);
	bool unitIsActiveResourceDepot(Unit* unit);
	u32 getPlayerDefaultRefineryUnitType(u32 playerId);

	// for AI_UnitWasHit // some of these should probably be methods in api/etc
	char AI_CancelStructure(Unit* structure);
	Unit* AI_GetActualTarget(Unit* attacker, Unit* target);
	Unit* checkArbiterCloakingRange(Unit* unit, u16 id);
	void AI_OrderUnitCloaking(Unit* unit);
	void AI_OrderLurkerBurrow(Unit* unit);
	bool AI_IsUnderAttack(Unit* unit, s32 flag);
	bool unitIsActiveTransport(Unit* unit);
	s32 isUnitLoaded(Unit* unit);
	s32 AI_TryReturnHome(Unit* unit, s32 dontIssueOrder);
	s32 AI_TransportAction(Unit* unit);
	s32 AI_AskForHelp(Unit* self, Unit* attacker, s32 onlyNearbyGuards);
	s16 getRegionIdFromUnit(Unit* unit);
	s32 getEnemyAirStrengthInRegion(u16 regionId, s32 player);
	s32 getEnemyStrengthInRegion(u16 regionId, s32 player, s32 includeAir);
	bool unitIsPerformingUnbreakableCode(Unit* unit);
	bool AI_CastSpellBehavior(Unit* unit, bool isAggro);
	s32 AI_Flee(Unit* unit, Unit* enemy);
	bool unitHasGroundPath(Unit* unit, s16 x, s16 y);
	bool AI_HasNoDefenseBuild(s32 player);
	bool unitCanSeeCloakedTarget(Unit* target, Unit* unit);
	void AI_RespondToCloakedThreat(Unit* threat, Unit* unit);
	bool unitCanAttackTarget(Unit* target, Unit* unit, s32 checkDetection);
	s32 AI_UpdateAttackTarget(Unit* unit, s32 acceptIfSieged, s32 acceptCritters, s32 mustReach);
	__int64 AI_PrepareToFlee(Unit* enemy, Unit* unit);

	// for AI_IterateBuildingAI
	u16 AI_RemoveFromTowns(Unit* worker);
	s32 AI_GetQueuedCount(Unit* builder, u32 unitId);
	void AI_PrepareGuardAI(u16 y, u32 player, u16 unitId, u16 x);

	// For AI_SendSuicide
	bool isThreat(Unit* target, Unit* unit);														// 00476930
	void prepSuicide(Unit* unit);																	// 0043CD40
	u32 AI_AttackClear(s32 player);																	// 00439500
	bool AI_AttackManager(s32 player, s32 x, s32 y, s32 bOverrideLastAttack, s32 bFallbackAirOnly); // 0043ABB0

	// Attack extender

	// for AI_TrainAttackForce
	s32 AI_AddRegionToMilitary(AiRegion* region, s32 unitId, s32 priority);
	u32 AI_FindUnit(u32 unitId, u32 playerId, u32 type);
	u32 AI_AddSpendingRequest(u16 unitId, u32 type, u8 player, char priority, void* value);
	u32 AI_AddBaseUnitSpendingRequest(u32 unitId, u32 player, AiRegion* region, char priority);

	// for AIRegion_GroupForAttack
	u32 AI_RecalculateRegionStrength(AiRegion* region);
	Unit* AI_TargetEnemyInRegion(AiRegion* region);
	s32 AI_MoveMilitaryToDefend(AiRegion* region);
	void AI_GetRegionMilitaryTypes(AiRegion* region, s32* hasGroundMilitary,
		s32* cantAttackGround, s32* cantAttackAir);
	s32 AI_CanTravelToRegion(AiRegion* region1, AiRegion* region2);
	Unit* AI_PickAttackTarget_NoBuildings(AiRegion* region);
	u32 AI_MoveMilitaryToOtherRegion(AiRegion* target, AiRegion* source, u32 pullUnitsNearRegion,
		u32 sendPureSpellcasters, u32 dontSendFromBunkers);
	AiTown* AI_IterateTownSomething(AiRegion* region);
	u32 AI_AttackTimerDecrement(u32 player);
	s32 AI_HasForcesMovingToAttack(u32 player);

	// for AI_AttackManager
	u16 PopulateRegionsWithOwn(char* regionData, u32 player);
	void PopulateRegionsWithEnemy(u32 player, char* regionData);
	s32 AI_GetNeighborRegionTypes(char* regionData);
	void AI_FinishRegionAccessibility(char* regionData);
	s32 AI_SetAttackGroupingRegion(u32 airAttack, u32 alsoVeryNear, u32 player, char* regionData,
		s32 x, s32 y, u32 alwaysOverwriteLastAttack);
	char AI_CancelStructure(Unit* structure);

	bool unitCantSeeCloakedTarget(Unit* unit, Unit* target);												// 00401D60
	bool unitCanInfest(Unit* unit);																			// 00402750
	void AI_AttackUnit(Unit* unit);																			// 0043FFD0
	Unit* findBestAttackTarget(Unit* unit);																	// 00443080
	bool function_00462EA0(Unit* unit, u8 unknownByteValue);												// 00462EA0
	void function_00465780(Unit* unit);																		// 00465780
	void removeOrderFromUnitQueue(Unit* unit, Order* order);												// 004742D0
	void prependOrderTargetingUnit(Unit* unit, Unit* target, u8 orderId);									// 00474A70
	bool isTargetWithinMinMovementRange(Unit* unit, Unit* target, u32 range);								// 004763D0
	bool function_00476610(Unit* unit, s32 x, s32 y);														// 00476610
	bool isUnitInWeaponRange(Unit* unit, Unit* target);														// 00476870
	void getWeaponBeginIscript(Unit* unit, IscriptAnimation::Enum anim);									// 00476ED0
	void function_00477510(Unit* unit);																		// 00477510
	void function_00477820(Unit* unit, u8 orderId);															// 00477820
	bool function_00478370(Unit* unit, u8 orderId);															// 00478370
	bool function_00478540(Unit* unit);																		// 00478540
	bool isUnitCritter(Unit* unit);																			// 0047B450
	void moveToCollide(Unit* unit, Unit* target);														    // 00495400

	bool isUnitVisible(Unit* unit);																			// 004E5DB0
	bool isTargetVisible(Unit* unit, Unit* target);															// 004E5E30
	u8 getRightClickActionOrder(Unit* unit);																// 004E5EA0
	void setNextWaypoint(Unit* unit);															            // 004EB290
	void makeToHoldPosition(Unit* unit);																	// 004EB5B0
	bool moveToTarget(Unit* target, Unit* unit);															// 004EB720
	bool SetMoveTarget_xy(Unit* unit, s32 x, s32 y);														// 004EB820
	s32 moveTowards(Unit* unit, Unit* target);																// 004EB900
	bool changeMovementTargetMoveTarget2(Unit* unit, s32 x, s32 y);											// 004EB9C0

	bool isDistanceGreaterThanHaltDistance(Unit* unit, s32 x, s32 y, u32 distance);							// 00401240
	void checkUnstack(Unit* unit);												                            // 0042D9A0
	bool beginBuilding(Unit* unit);																			// 00467030
	Unit* beginGasBuilding(Unit* unit, u32 unitId);															// 004678A0
	void addOverlayBelowMain(Sprite* sprite, u32 imageId, u32 x_offset, u32 y_offset, u32 direction);		// 00498D70
	void replaceSpriteImages(Sprite* sprite, u32 imageId, u32 imageDirection);								// 00499BB0
	void displayLastNetErrorForPlayer(u32 playerId);										                // 0049E530
	void updateUnitStrength(Unit* unit);												                    // 0049FA40
	void iscript_OpcodeCases(Image* image, u32 offset_, u32 unk1, u32 unk2);								// 004D74C0
	void switchToWarpTexture(Image* image);																	// 004D8500
	bool setThingyVisibilityFlags(Thingy* thingy);									                        // 004878F0
	Thingy* createThingy(u32 spriteId, s16 x, s16 y, u32 playerId);		    								// 00488210
	void setUnitStatTxtErrorMsg(char* message);										                        // 0048CCB0
	void playUnitSFX(u32 soundId, u16 x, u16 y, u32 unk1, u32 unk2);										// 0048EC10
	u32 RandomizeShort(u32 unknown_index);																	// 004DC4A0
	u32 IterateUnitsAtLocationTargetProc_RecallTargets(Unit* unit, Box16* coords);							// 004E8280
	void moveUnit_Duplicate(Unit* unit, s32 x, s32 y);														// 004EBAE0

	//// spider_mine
	Unit* SpiderMine_AcquireTarget(Unit* spiderMine);									                    // 00441270
	void instantCloak(Unit* unit);												                            // 0049B1E0
	void unburrowGeneric(Unit* unit);												                        // 004E99D0
	void burrowGeneric(Unit* unit);																			// 004E9A30

	// For orders_PlaceMine
	s32 techUseAllowed(Unit* a1, __int16 a2, s32 a3);									                    // 0046DD80
	u32 getDistanceFast(s32 a1, s32 a2, s32 a3, s32 a4);													// 0040C360
	s32 orders_HoldPosSuicidal(Unit* a1);												                    // 004EB5B0
	void turnUnit(Unit* a1, char a2);													                    // 00495F20
	void toIdle(Unit* a1);																                    // 004753A0
	s32 isUnitMovable(Unit* a1);														                    // 00401DC0
	void orders_Nothing2(Unit* a1);																			// 00475000
	bool setMoveTargetXY(s32 a1, s32 a2, Unit* a3);															// 004EB820

	// For burrowGeneric
	void toggleUnitPath(Unit* unit);									                                    // 004E42A0
	char switchSpriteId(Sprite* sprite, s32 spriteId);														// 00499860

	//// building_morph
	void refundBuildingCost(u16 unitId, u8 playerId);														// 002CE70
	void updateBuildingsAI(Unit* unit);																		// 0033FE0
	bool isValidMorph(u16 buildingId);																		// 005CC60
	void updateNewUnitVision(u32 id, s32 x, s32 y);															// 005CE90
	void zergPlaceBuildingCntd(Unit* unit);																	// 005D410
	void stopCreepDisappearingAtBuilding(Unit* unit);												        // 007D770
	void updateCreepDisappearance(Sprite* sprite, u32 unitId, s32 x, s32 y);								// 007DE40
	void incrementUnitScoresEx(Unit* building, s32 unk1, s32 unk2);											// 0088D50
	void playBuildingCompleteSound(Unit* building);															// 008F310
	void refreshSpriteData(Sprite* sprite);																	// 00983A0
	void displayLastNetErrorForPlayer(u32 playerId);									                    // 009E530
	void replaceUnitWithType(Unit* unit, u16 newUnitId);													// 009FED0

	//// building_terran
	bool isDistanceGreaterThanHaltDistance(Unit* unit, s32 x, s32 y, u32 distance);							// 00401240
	void fixTargetLocation(Point16* coords, u32 unitId);								                    // 00401FA0
	bool isUnitPositions2Equal(Unit* unit);																	// 00402160
	b32 buildingAddon(Unit* unit, u32 hpGain, b32 canBeAborted);											// 004679A0
	void prependOrderTargetingNothing(Unit* unit, Order* order, u8 orderId);								// 00474760
	void addOrderTargetingNothing(Unit* unit, Order* order, u8 orderId);									// 004749A0
	bool canStillPlaceBuilding();														                    // 0048DDA0
	void endBuildingPlacement();															                // 0048E310
	void playBuildingCompleteSound(Unit* building);															// 0048F310
	void setSecondaryWaypoint(Unit* unit, s32 x, s32 y);									                // 00494BB0
	void displayLastNetErrorForPlayer(u32 playerId);									                    // 0049E530
	u32 RandomizeShort(u32 unknown_index);																	// 004DC4A0
	u32 RandBetween(u32 min, u32 max, u32 someIndex);														// 004DC550
	void initBuilding(Unit* unit, b32 flag);																// 004E65E0
	bool SetMoveTarget_xy(Unit* unit, s32 x, s32 y);									                    // 004EB820
	bool orderToMoveToTarget(Unit* unit, Unit* target);														// 004EB980
	b32 unk_terrainCollision2(Unit* unit, s32 x, s32 y);													// 004F1870

	// For updatePlacementState_Units
	bool doesBuildingBlock(Unit* unit, s32 xTile, s32 yTile);												// 00473410
	s32 isTileBlockedBy(Unit** array, Unit* builder, s32 xTile, s32 yTile,									// 00473300
		char bDontIgnoreReacting, char bAlsoInvisible);

	// For updatePlacementStateForPlacementBox
	s32 canBuildOnTileLocation(Unit* builder, s32 playerId, s32 xTile, s32 yTile, s32 unitType,				// 00473FB0
		s32 placementStateEntry, s32 bCheckVision, s32 bAlsoInvisible, s32 bWithoutVision);
	
	// For refreshScreen
	s32 updatePlacementStateForPlacementBox_Func();															// 0048DCE0
	s32 unk_buildingPlacementRelated(s32 a1, Point16* a2);													// 0048D7F0
	s32 initPlacementBoxPos(Point16* in_out);																// 0048D660
	char unk_buildingPlacementRelatedMouse(s32 a1, s32 a2);													// 0048D7B0

	//// liftland
	void setUnitPathing(Unit* unit, u8 unkPathRelated);																				// 00401360
	bool isUnitPositions2Equal(Unit* unit);																							// 00402160
	Unit* connectAddonCheck(Unit* unit);																							// 004404A0
	void function_00463470(Unit* unit);																								// 00463470
	void function_004634E0(Unit* unit);																								// 004634E0
	void function_00463640(Unit* unit);																								// 00463640
	void completeAddon(Unit* unit, Unit* addon);																					// 00463D50
	void function_00464300(Unit* unit);																								// 00464300
	void disconnectFromAddOn(Unit* unit);																							// 00464930
	void function_00469EC0(Unit* unit, s32 x, s32 y);																				// 00469EC0
	void function_00469F60(Unit* unit, s32 x, s32 y);																				// 00469F60
	void function_0046A560(Unit* unit);																								// 0046A560
	void function_0046A5A0(Unit* unit);																								// 0046A5A0
	void actUnitReturnToIdle(Unit* unit);																							// 00475420
	bool function_0047DF90(Unit* unit);																								// 0047DF90
	void placebuildingProc();																										// 0048E6E0
	void playBuildingLandSound(Unit* unit);																							// 0048F5A0
	void SetUnitMovementSpeed(Unit* unit, u32 newSpeed);																			// 004951C0
	void function_00498150(Sprite* sprite, u8 newVerticalOffset);																	// 00498150
	void function_004997E0(Sprite* sprite);																							// 004997E0
	void function_004C3B40(Unit* unit);																								// 004C3B40

	//// create_init_units
	b32 spreadsCreep(u32 unitId, b32 defaultReturn);																				// 00413870
	void function_004148F0(s32 x, s32 y, u32 unitId);																				// 004148F0
	void CreateBuildingLarva(Unit* main_building);																					// 0049D660
	b32 function_0049EC30(Unit* unit);																								// 0049EC30
	Unit* FindBestUnit(Box16* coords, u32 searchProc);																				// 004E8830

	//// larva_creep_spawn
	Unit* getUnitPointer();
	u8 spreadCreep(u32 unitId, s32 x, s32 y);																						// 00414680
	void AI_TrainingOverlord(Unit* main_building, Unit* larva);																		// 00435770
	void updateUnitStrength(Unit* unit);																							// 0049FA40
	u32 IterateUnitsAtLocationTargetProc_LarvaCount(Unit* unit, Box16* coords);														// 004E8280
	void function_004E8DA0(Unit* unit);																								// 004E8DA0
	bool getLarvaSpawnPos(Unit* unit, Vector32* coords);																			// 004E8E10

	// TODO: move somewhere else?
	// SC Variables and Buffers used in this file
	u32* const mapTileBufWidth = (u32*)(0x006D0F08);
	u32* const mapTileBufHeight = (u32*)(0x006D0C6C);
	u16** const mapTileBuf = (u16**)(0x006D0E84);
	Vector32* const someCreepPointBuf = (Vector32*)(0x006D7578);
	typedef u32(*isTileVisible_func)(u32 x, u32 y);
	typedef bool(*setCreepRelationalData_callback)(u16* tileBackup, u8* creepOverlayVal, u32* retval, u16* tile, u32 arg);
	isTileVisible_func* const isTileVisible_ptr = (isTileVisible_func*)(0x006D0C78);

	u32 getNumSurroundingCreepTiles(u32 x, u32 y);																						// 00414290
	void setCreepRelationalData(u32 x, u32 y, setCreepRelationalData_callback callback, u32 arg);										// 00413FA0
	u32 iterateDirectionalCreepData(u32 x, u32 y, u32 callback, u32 arg);																// 00413B30

	// TODO: add helpers for these functions
	// for applyCreepAtLocationFromUnitType
	s32 modifyCreepState(s32 x_tile, s32 y_tile, s32(__stdcall* creepChange)(s32, s32, s32, s32, s32), void* param);					// 00413FA0
	s32 runCreepRandomizer(s16* originalTile, s32 creepTileBorders, s32* outCreepStatus, s16* currentTile, s32 unused);					// 004140A0 // "note possible desync"
	s32 unkCreepBorders(s32 a1, s32 a2);																								// 006D0C78
	u32 forEachAdjacentTile(s32(__stdcall* callback)(s32, s32, s32, s32, void*), void* param, s32 x_tile, s32 y_tile);					// 00413B30
	s32 unkCreepTile(s32 pos, s32 originalTile, s32 x_tile, s32 y_tile, void* param);													// 00414440
	char unkSpreadCreep(s32 a1, s32 a2, s16 a3);																						// 00414680

	//// burrow_tech
	void actUnitReturnToIdle(Unit* unit);																								// 00475420
	void TargetedOrderCopy(Unit* unit, u32 unitId, u32 unk1, u32 unk2, u32 orderId, u32 unk3, u32 unk4, u32 unk5);						// 004754F0
	void function_004E97C0(Unit* unit);																								// 004E97C0

	//// die_order
	void clearUnitSprites(Sprite* unit_sprite, Sprite* subunit_sprite);																// 00496F00
	void function_00497480(Sprite* thingySprite, u32 visibility_flags);																// 00497480
	void _UnitDestructor(Unit* unit);																									// 004A0740
	void UnitDestructor(Unit* unit);																									// 004A0990
	void hideAndDisableUnit(Unit* unit);																								// 004E6340

	//// CMDRECV_Cancel
	void CancelResearch_Proc(Unit* unit);																								// 00453E30
	void CancelUpgrade_Proc(Unit* unit);																								// 00454280
	void refundQueueSlot(Unit* unit, u16 slot);																						// 00466A70
	void function_00466E40(Unit* unit);																								// 00466E40
	void CancelAddon_Proc(Unit* unit);																									// 004E66E0

	//// CMDRECV_Build
	b32 OrderAllowed(Unit* unit, u16 order, u32 nationID);																				// 0046DC20
	bool placeBuildingMsg(u32 result_function_473FB0);																					// 0048D930
	bool CMDRECV_PlaceBuildingAllowedHelper(Unit* unit, u8 orderId, u16 builtUnitId);													// 0048DBD0
	void function_0048DE70(Unit* unit, u32 orderId, u16 dimensionX, u16 dimensionY, u32 builtUnitId);									// 0048DE70
	void cmdRECV_PlaceBuilding(s16 x, s16 y, u8 orderId, u16 builtUnitId);																// 0048E190

	//// CMDRECV_MergeArchon
	Unit* templarMergePartner(Unit* unit, Unit** units_list, u32 units_list_length);													// 004BFA80

	//// CMDRECV_Stop
	void parseOrdersDatReqs();																											// 0046D450
	s32 parseRequirementOpcodes(Unit* unit, u32 orderId, u32 playerId, u32 address, u32 req_offset);									// 0046D610

	//// give_unit
	void ApplySpeedUpgradeFromUnitType(Unit* unit);																						// 00454370
	void giveAI(Unit* unit);																									// 00463040
	void recheckAttacks(Unit* unit);																									// 00476D30
	void incrementUnitScores(Unit* unit, s32 unkScore);																					// 00488BF0
	void AI_RemoveUnitAI(Unit* unit, UnitAi* unitAI);																					// 004A1E50
	void CreateUnitSelection(Unit* unit, u32 selectionIndex);																			// 004E6180
	void DestroyUnitSelection(Unit* unit);																								// 004E6290
	void function_004F3B70(Unit* powerup);																								// 004F3B70

	//// cast_order
	u32 getUpgradedWpnCooldown(Unit* unit, u8 weaponId);																				// 00475DC0
	bool function_00476640(Unit* unit, u32 weaponId);																					// 00476640
	u32 get_statTxt_Str_0(Unit* unit, u32 playerId, char* message);																		// 0048EF30
	bool canCastSpell_0(Unit* unit);																									// 00492140
	u32 function_00492620(Unit* unit, b32 wasMainOrderStateZero);																		// 00492620

	// For spellOrder
	bool getOrderTargetErrorString(u16 a1, Unit* a2, s16 a3, Unit* a4, s16 a5, s16 a6, u16* a7);										// 00492020
	s32 spellOrder_MoveTowardsTarget(Unit* caster, u32 range);																			// 00492450

	// For canTargetSpell
	s16 canTargetSpellOnFOWUnit(s32 techId, s16 unitId);																				// 004914B0

	void incrementUnitDeathScores(Unit* unit, u8 player);																				// 00488AF0
	void initializeEmptyUnitsLinkedListRef(Unit* unit, u8 playerId);																	// 0049E4E0
	void GiveUnitHelper(Unit* unit, u32 playerId, s32 unkScore);																		// 0049EFA0
	void changeUnitButtonSet(Unit* unit, u16 unitId);																					// 004E5D60
	void showAndEnableUnit(Unit* unit);																									// 004E6490

	//// tech_target_check
	bool isInfestableUnit(Unit* unit);																									// 00402210

	//// trap_hooks
	// For updateUnitStrengthAndApplyDefaultOrders
	void AnimCloak(Unit* a1, s32 a2);																									// 0049B5B0
	void refreshUnitVision(Unit* a1);																									// 004E5F30
	void initUnitTrapDoodad(Unit* a1);																									// 004E6490
	Order* removeOrderFromUnitQueue(Order* a1, Unit* a2);																				// 004742D0
	Order* performAnotherOrder(u8 a1, s32 a2, Unit* a3, s16 a4, Vector32* a5, Unit* a6);													// 004745F0
	void order(s32 a1, Unit* a2, s8 a3, Vector32* a4, Unit* a5, s32 a6);																	// 00474810
	u32 getUnitStrength(Unit* a1, s32 a2, bool a3);																						// 00431800

	//// make_nydus_exit
	void playWorkerErrorSound(Unit* unit, u32 errorTextIndex);																			// 0048F6C0

	//// research_upgrade_orders
	bool isUnitUpgradeAvailable(Unit* unit);																							// 004033D0
	void ApplySpeedUpgradeFromUpgradeType(Unit* unit, u8 upgradeType);																	// 00454540
	void playUpgradeCompleteSound();																									// 0048F070
	void playResearchCompleteSound();																									// 0048F150
	void setUpgradeLevel(u32 playerId, u32 upgradeType, u8 upgradeLevel);																// 004CE770

	//// merge_units
	bool advanceRemainingBuildTime(Unit* unit);																							// 00466940
	u8 function_0046A820(Unit* unit);																									// 0046A820
	void orderImmediate(Unit* unit, u8 order);																							// 00474B40
	void playMorphingCompleteSound(Unit* unit);																							// 0048F440
	void mergeStatusIntoUnit_Sub493180(Unit* unit_dst, Unit* unit_src);																	// 00493180
	void animDecloak(Unit* unit, u32 soundId);			 																				// 0049B440
	bool delayedSetMoveTarget_xy_Sub4EB960(Unit* unit, u16 x, u16 y);																	// 004EB960

	//// nuke_orders
	void setAllOverlayDirectionsGeneric(Unit* unit, u32 unk);																			// 00401140
	void AI_orderUnitCloaking(Unit* unit);																								// 0043B970
	u32 function_0043DB50(Unit* unit);																									// 0043DB50

	//// target_related

	// for isUnitTargetOutOfMaxRange
	s32 getUnitMaxWeaponRange(Unit* a1, unsigned __int8 a2);
	bool isTargetWithinMinMovementRange(Unit* a1, u32 a2, Unit* a3);

	// for isReadyToAttack
	u32 getDistanceToUnit(Unit* a1, Unit* a2);
	u32 distanceFromDboxCorner(Unit* a1, __int16 a2, __int16 a3);
	bool isAngleInAttackAngle(s32 a1, s32 a2, Unit* a3, unsigned __int8 a4);

	// for canCastOrderedSpellNow
	bool isTargetWithinMinRange(Unit* unit, u32 minRange, Unit* target);
	s32 getFlingyHaltDistance(s32 a1);

	// for ordersWatchTarget
	void unknownOrdersFunc(s32 a1, __int16 a2, __int16 a3);

	// for ordersStayInRange
	void prependOrderTargetingGround(__int8 a1, __int16 a2, __int16 a3, s32 a4);
	s32 ordersHoldPositionSuicidal(Unit* a1);
	s32 getDirectionFromPoints(s32 a1, s32 a2, s32 a3, s32 a4);
	Vector32 setResourceTarget(Unit* a1);

	// for genericRightClickUnitProc2
	s32 isOrderValid(char a1, Unit* a2, s32 a3, Unit* a4, s32 a5);																	// 00474D90
	u16 getUnitAbilityErrorStatString(Unit* a1, char a2, s32 a3);																	// 00491E80

	// for CMDRECV_HoldPosition
	Unit* getActivePlayerFirstSelection();																							// 0049A850
	s32 orderAllowed(Unit* a1, u16 a2, s32 a3);																						// 0046DC20
	void targetedOrder(s16 a1, s32 a2, Unit* a3, s16 a4, s32 a5, s32 a6, char a7, s32 a8, s32 a9, s32 a10);							// 004754F0

	// for checkUnitVisibility
	u32 getDetectingPlayers(Unit* unit);																							// 00441130
	u32 iteratePlayerUnitsSomething(u32 aResult, Unit* unit);																		// 0049E8A0
	void makeSpriteVisible(Sprite* sprite);																							// 00497ED0
	s32 updateBuildingLandUnitSelection(u32 unused, Unit* selection);																// 004C3B40
	char removeSelectionCircle(Sprite* sprite);																						// 004975D0
	Image* makeSpriteInvisible(Sprite* sprite);																						// 00498A10

	//// receive_command
	void function_004756B0(Unit* unit, u8 orderId, u32 unk1, u32 unk2, u32 bCommandType);											// 004756B0
	void function_004756E0(Unit* unit, Unit* target, u32 orderId, u32 bCommandType);												// 004756E0
	u32 function_0049A410(Unit* unit, u8 orderId);																					// 0049A410
	bool CanTargetSelf(Unit* unit, u8 orderId);																						// 0049A480
	void function_0049A500(Unit* unit, u8* array_of_data);																			// 0049A500
	void function_0049A8C0(u8* array_of_data, b32 bCanBeObstructed);																// 0049A8C0

	//// load_unload_proc
	void IgnoreAllScriptAndGotoIdle(Unit* unit);																					// 00476550
	u32 canUnload(Unit* transport, Unit* loaded_unit, Point16* pos);																// 004E76C0
	void function_004E7E10(Unit* unit, u32 unk);																					// 004E7E10

	//// stats_panel_display
	void StatsNukesCount(BinDlg* dialog, u32 index);																				// 00425310
	void StatHangerCount(BinDlg* dialog, u32 index);																				// 004253D0
	void StatsShieldLevel(BinDlg* dialog, u32 index);																				// 00425510
	void StatsArmorLevel(BinDlg* dialog, u32 index);																				// 00425600
	void StatsWeaponLevel(BinDlg* dialog, u32 index, u32 weaponId);																	// 00425790
	bool StatSpidermineCount(BinDlg* dialog, u32 index);																			// 00426300

	// For getUnitRankString and checkHotkeyChar
	__int8 unitGetRace(Unit* a1);																									// 0047B150
	char* getStatTxtString(u16 a1);																									// 004C36F0

	// For statsWeaponLevel / statsArmorLevel
	char getArmorCount(Unit* unit, s32 a2);																							// 00453FC0
	void showDialog(BinDlg* dialog);																								// 004186A0
	s32 disableDialog(BinDlg* dialog);																								// 00418640
	void updateDialog(BinDlg* dialog);																								// 0041C400

	//// bunker_hooks
	Unit* findRandomAttackTarget(Unit* unit);																						// 00442FC0
	b32 attackApplyCooldown(Unit* unit);																							// 00478B40
	void setImageDirection(Image* image, s8 direction);																				// 004D5F80

	//// api
	void ApplySpores(Unit* unit);
	Bullet* createBullet(u8 weaponId, Unit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction);
	void weaponBulletShot(Unit* target, Bullet* bullet, u32 dmgDivisor);
	Unit* createUnitAtPos(u16 unitType, u16 playerId, u32 x, u32 y);																// 004CD360
	Flingy* createFlingy(u32 flingyId, u32 playerId, u32 x, u32 y);																	// 00496460

	//// unit_morph
	u8 setArchonPathing(Unit* unit);																								// 0046A820
	bool hasSuppliesForUnit(u32 unitId, u32 playerId, b32 canShowErrorMessage);														// 0042CF70
	Unit* zergDualBirth(Unit* unit);																							    // 0045D910
	void orderNewUnitToRally(Unit* unit, Unit* factory);																			// 00466F50
	void AI_TrainingUnit(Unit* unit, Unit* creator);																				// 004A2830
	void deleteBuildingAI(BuildingAi* building, BuildingAiArray** buildingList);                                                    // 00404500
	void deleteWorkerAI(WorkerAi* ai, WorkerAiArray** workerArray);                                                                 // 00404470
	void deleteAITownIfNeeded(AiTown* town);                                                                                        // 004347E0

	//// hallucination_spell
	// for orders_Hallucination1
	bool spellOrder(Unit* unit, u32 techId, u16* techEnergyCost, u32 sightRange, u32 error_message_index);							// 004926D0

	void UnitDestructor(Unit* unit);
	bool placeHallucination(Unit* unit);																							// 004F66D0

	// for createHallucination
	void applyHallucinationStats(Unit* halUnit);																					// 004F6180
	s32 hallucinateScarabs(Unit* a1, Unit* a2);																						// 004F65B0

	//// feedback_spell
	void replaceFeedbackSprite(Unit* unit, u32 spriteId);																			// 004F6350

	//// weapon_effect
	bool isUnitDistanceWithin(Unit* target, Bullet* bullet, u32 radius);
	void doWeaponHit(Unit* target, Unit* attacker, u8 dmgDivisor);
	void Ai_UnitWasHit(u32 main_target_reaction, u32 dont_call_help, Unit* attacker, Unit* target);

	// for airSplash
	Unit* findBestUnit(Box16* radius, s32 bullet, s32(*func)(Unit*, Bullet*));														// 004E8830
	s32 splashFunc(Bullet* bullet, Unit* unit);																						// 0048B570

	// For iterateLoadedUnits
	Unit* iterateLoadedUnits(Unit* unit, Unit*(__fastcall* func)(Unit*, Unit*), s32 a3);											// 004E6D00

	//// mindcontrol_spell
	void MindControlHit(Unit* attacker, Unit* target);																				// 004F6910

	//// weapon_damage
	u16 getUnitStrength(Unit* unit, bool useGroundStrength);
	Unit* createHallucination(Unit* target, u32 playerId);

	// For updateCorrosiveSpores
	u32 getCorrosiveSporesImage(Unit* unit);																						// 004F41F0

	//// recharge_shields
	void insertFirstOrder(Unit* unit, u8 orderId);																					// 004749D0
	u8 hasOverlay(Unit* unit);																										// 0047B720
	bool isHangarUnit(Unit* unit);

	//// stopholdpos_orders
	Unit* MedicHeal_TargetAcquire(Unit* medic);																						// 004422A0
	u32 doMedicHeal(Unit* unit, Unit* target);																						// 00463C40
	void function_00476FC0(Unit* unit, Unit* target, u32 unk1, u32 unk2);															// 00476FC0
	b32 function_004770E0(Unit* unit);																								// 004770E0

	//// medic_orders
	b32 AI_OrderHeal(Unit* unit, Unit* target);																						// 00463530
	void function_4748E0(Unit* unit, s32 x, s32 y, u8 orderId);																		// 004748E0
	void function_474940(Unit* unit, Unit* target, u8 orderId);																		// 00474940
	void orderInterrupt(Unit* unit, Unit* target, u16 x, u16 y, u32 orderId);														// 00474C30
	void unitOrderMoveToTargetUnit(Unit* unit, Unit* target);																		// 00479FE0
	b32 AI_CastSpellBehaviour(Unit* unit, u32 unknown);																				// 004A13C0

	//// load_unload_orders
	b32 Ai_AreOnConnectedRegions(Unit* unit, s32 x, s32 y);																			// 00438E70
	void function_0043DF30(Unit* unit);																								// 0043DF30
	void unknownSpecialOrder(Unit* unit, Order* order, u8 orderId, s16 x, s16 y, Unit* target, u16 targetUnitId);					// 00474540
	void _doOrder(Unit* unit, u32 orderId, s32 x, s32 y, bool stopPreviousOrders);													// 00474C70
	b32 Ai_UpdateAttackTarget(Unit* unit, u32 unk1, u32 unk2, u32 unk3);															// 00477160
	Unit* getLoadedUnitFromIndex(Unit* transport, s32 index);																		// 004E6C40
	b32 getFirstLoadedWorker(Unit* transport);																						// 004E6D90
	b32 Ai_FindTransport(Unit* unit, Unit** unitTargetOffset);																		// 004E6EF0
	void loadUnitProc(Unit* unit, Unit* unitToLoad);																				// 004E78E0	(Hooked)
	b32 unloadUnitProc(Unit* unit);																									// 004E7F70 (Hooked)
	void issueOrderTargetingGround(Unit* unit, u8 orderId, s16 x, s16 y);															// 00475260

	//// siege_transform
	void unitSetRetreatPoint(Unit* unit, u8 angleValue);																			// 004011A0
	bool getCloserToDestination_sub402BE0(Unit* unit);																				// 00402BE0
	void orderReturnToIdle(Unit* unit);																								// 00463770
	bool checkForCloseUnit(Unit* unit);																								// 00464290
	void playSubunitIscript(Unit* unit, IscriptAnimation::Enum anim);																// 00474380

	//// unit_train
	u32 getHPGainForRepair(Unit* unit);																								// 00402C40
	void refundUnitTrainCost(u32 unitId, u8 playerId);																				// 0042CEC0
	void function_00432430(Unit* unit, u32 buildQueueSlot);																			// 00432430
	void addHangarUnit(Unit* main_unit, Unit* added_unit);																			// 00466300
	Unit* attemptTrainHatchUnit(Unit* trainer, u32 builtUnitID, u32 flag);															// 00468200

	//// weapon_fire
	// For progressBulletFlight
	void progressBulletMovement(Bullet* a1);																						// 0048B250
	void doMissileDmg(Bullet* bullet);																								// 0048B770

	// For bulletBehavior_Bounce
	Bullet* changeMovePos(Bullet* a1, s32 a2, s32 a3);																				// 00495240
	void progressBulletMovement_func(Bullet* a1);																					// 0048B250
	Unit* findNextBounceTarget_func(Bullet* a1);																					// 0048B1E0

	// For findNextBounceTargetProc
	bool unitIsSubunit(Unit* a1);																									// 00401D40
	bool unitIsCarrier(Unit* a1);																									// 00401470
	bool unitIsReaver(Unit* a1);																									// 00401490
	bool unitHasPathToUnit(Unit* a1, Unit* a2);																						// 0049CBB0
	s32 canBeInfested(Unit* a1, Unit* a2);																							// 00402210

	// For findNextBounceTarget
	Unit* iterateUnitsAtLocationTargetProc(Box16* a1, Unit*(__fastcall* a2)(Unit*, Unit*), Unit* a3);								// 004E8280

	// For createBullet
	s32 initializeBullet(char playerId, Unit* parent, s16 y, Bullet* bulletResult, s32 weaponId, s32 x, s32 angle);					// 0048BEC0

	// For writeBullets
	/*
	void packBulletData(Bullet* a1, s32 a2); // 0048A9A0
	size_t _lockwrite(const void* a1, size_t a2, size_t a3, FILE* a4); // 00411931
	void* SMemAlloc(s32 a1, char* a2, s32 a3, s32 a4); // 0041006A
	s32 compressWrite(void* a1, u32 a2, FILE* a3); // 004C3450
	bool SMemFree(void* a1, char* a2, s32 a3, char a4); // 00410070
	Bullet* unpackBulletData(Bullet* a1, s32 a2); // 0048A8D0
	*/

	//// wpnspellhit
	bool function_0042D810(s32 x, s32 y, u32 unitId);														    // 0042D810
	Unit** getAllUnitsInBounds(Box16* coords);																	// 0042FF80
	void incrementUnitKillCount(Unit* unit);																	// 004759C0
	u32 getUnitDamageBonus(Unit* unit, u8 weaponId);														    // 00475EC0
	void attackOverlayAndNotify(Unit* attacker, Unit* target, u32 weaponType, u32 direction);			        // 00479730
	Thingy* createThingy(u32 spriteId, s16 x, s16 y, u32 playerId);									        // 00488210
	u32 GetUnitBulletDamage(Bullet* bullet, Unit* target);													// 0048ACD0
	u32 getSpellStatString(Unit* target, u32 techId, u8 playerId);									            // 00491E80
	void disableUnitFunc(Unit* unit);																			// 00492CC0
	void setAllImageGroupFlagsPal11(Sprite* sprite);															// 00497430
	void IterateUnitsAtLocationTargetProc_CorrosiveAcid(u32 attackingPlayerId, Box16* coords);				    // 004E8280
	void IterateUnitsAtLocationTargetProc_Ensnare(Unit* attacker, Box16* coords);								// 004E8280
	void IterateUnitsAtLocationTargetProc_Plague(Unit* attacker, Box16* coords);								// 004E8280
	void IterateUnitsAtLocationTargetProc_Maelstrom(Unit* attacker, Box16* coords);							// 004E8280
	void IterateUnitsAtLocationTargetProc_StasisField(Unit* attacker, Box16* coords);							// 004E8280
	void createEnsnareOverlay(Unit* a1);																		// 004F45E0

	// For createScanner
	s32 setUnitStatTxtErrorMsg(s32 a1);																		    // 0048CCB0
	s32 playSoundFromSource(s32 soundId, Unit* unit, s32 bPlay, s32 minVolume);								// 0048ED50
	void returnAllInterceptors(Unit* unit);                                                                    // 004655F0
	char ignoreAllScriptsAndGoToIdle(Unit* unit);                                                              // 00476550

	// For maelstromProc
	void orders_SelfDestructing(Unit* unit);                                                                   // 00475710
	char setMaelstromTimer(Unit* unit, u8 time);                                                               // 004553F0

	// For CBulletDamage
	s32 _fastcall lurkerSplashProc(Unit* target, Bullet* bullet);                                             // 0048B680
	s32 _fastcall groundSplashProc(Unit* target, Bullet* bullet);                                             // 0048B750
	s32 _fastcall airSplashProc(Unit* unit, Bullet* bullet);                                                  // 0048B150
	s32 _fastcall airSplashProc2(Unit* unit, Bullet* bullet);                                                 // 0048B730
	bool unitHasNoMainOrderType(Unit* unit);                                                                   // 00475A50
	s32 setEventUnitComplete();                                                                                 // 004C36C0

	// For findBestUnit
	Unit** findAllUnits(Box16* coords);																		// 004308A0

	//// selection
	bool unitIsOwnedByCurrentPlayer(Unit* unit); 																// 00401170
	bool isUnitBurrowed(Unit* unit);																			// 00402A70
	void SC_memcpy_0(u32 dest, u32 src, u32 memsize);													        // 00408FD0
	void function_00468670(Unit* unit);																		// 00468670
	bool unit_isUnselectable(u16 unitId);																	    // 0046ED80
	void function_0046F040(Unit* current_unit, Unit** unit_list, Unit* clicked_unit, u32 list_length);	    // 0046F040
	u32 combineLists_Sub_6F290(Unit* unit, Unit** unit_list_1, Unit** unit_list_2, u32 list_length);	        // 0046F290
	void applyNewSelect_Sub_6FA00(Unit** unit_list, u32 unit_list_length);									    // 0046FA00
	u32 CUnitToUnitID(Unit* unit);																			    // 0047B1D0
	bool unit_IsStandardAndMovable(Unit* unit); 																// 0047B770
	void selectBuildingSFX(Unit* unit);																		// 0048F910
	b32 selectSingleUnitFromID(u32 unitIndex);															        // 00496D30
	void CreateNewUnitSelectionsFromList(Unit** unit_list, u32 unit_list_length);							    // 0049AE40
	void CMDACT_Select(Unit** unit_list, u32 unit_list_length); 											    // 004C0860

	//// cloak_nearby_units
	void secondaryOrder_Cloak(Unit* unit);																		// 00491790

	//// update_unit_state
	bool unitHpIsInRedZone(Unit* unit);						                                                // 004022C0
	void RestoreAllUnitStats(Unit* unit);						                                                // 00492F70 (hooked by hooks\update_status_effects.cpp)

	// For updateUnits
	bool isSelectedUnitGroupEnabled();																						// 0049A2C0
	char refreshLayers3And4();																								// 0048D9A0
	void cancelTargetOrder();																								// 0048CA10
	s32 updateDisruptionWebStatus();																						// 004EB2F0
	Unit* progressDyingUnitFrame(Unit* unit);																				// 004EB5E0
	void revealSightAtLocation(s32 sightRange, u32 visionFlags, s32 ptX, s32 ptY, bool isAirUnit);					        // 004806F0
	void updateVisibility(Unit* unit);																						// 004EBE10
	void progressActiveUnitFrame(Unit* unit);																				// 004EBC30
	void checkUnitVisibility(Unit* unit);																					// 00443390
	void progressUnitFrame(Unit* unit);																						// 004ECF70
	void unitUpdate(Unit* unit);																							// 004EC420
	void updateUnitPower();																									// 00493790

	// For updateBurrowingCloakingUnits
	s32 isUnitCloaking(Unit* unit);																							// 004916E0
	void playSoundFromDirect(Unit* unit, s32 soundId);																	    // 0049B5B0

	// For doCloak
	void updateSpriteFlagOnPalette(Sprite* sprite);																			// 00497E80
	void showImagesHiddenByCloak(Sprite* sprite);																			// 00498170
	void updateSpriteImagesForCloak(Sprite* sprite);																		// 00498A90

	// For progressUnitFrame
	char refreshSprite(Sprite* sprite, s32 spriteId);																		// 004EBBD0
	void updateUnitTimers(Unit* unit);																						// 004EC290
	void progressOrder(Unit* unit);																							// 004EC4D0
	void performSecondaryOrders(Unit* result, s32 unused);																	// 004ECF70
	void spriteToIscriptLoop(Sprite* sprite);																				// 00497920

	//// select_larva

	u32 getLarvaeUnitsFromList(Unit** unitList, Unit** outputList);															// 00423190
	b32 selectSingleUnitFromID(u32 unitIndex);																		        // 00496D30
	void selectMultipleUnitsFromUnitList(Unit** unitList, u32 unitsCount, b32 unk1, b32 unk2);					            // 0049AEF0

	//// uifeatures

	s32 BWFXN_PrintXY(const char* text, s32 x, s32 y);
	s32 GetColorHandle(u8 color); // Colors: 1 - last color, 2 - 8, 14 - 17, 21 - 25, 27 - 31

	// For targetedOrder
	Order* queueNewOrder(Order* a1, s16 a2, s16 a3, Unit* a4, u8 a5, s32 a6, s16 a7, Vector32* a8, Unit* a9);	        // 00474B90
	s32 showStatTxtToPlayer(s32 a1, s32 a2);																			    // 0048CF00
	void setOrderTarget(u8 a1, s16 a2, s16 a3, s32 a4, s32 a5, Unit* a6, s32 a7, Vector32* a8, Unit* a9);		            // 00475470

	// For selectUnits
	s32 selectSingleUnitFromID(s32 a1);																					    // 00496D30
	s32 selectMultipleUnitsFromUnitList(s32 a1, Unit** a2, u8 a3, s32 a4);										            // 0049AEF0
	s32 SNetGetTurnsInTransit(u32* turns);
	u8 sendTurn();
	s32 SErrGetLastError();
	u8 packetErrHandle(s32 result, s32 a1, const char* a3, s32 a4, s32 a5);

	//// repair_order

	void getRepairInfo(Unit* repairTarget, u32* repairCostMineral, u32* repairCostGas, s16* unkRepairInfo);		        // 00466D10
	bool verifyResources(u32 mineralCost, u32 gasCost, u32 playerId);												        // 00466EE0
	bool ord_repair_subtract(Unit* worker, Unit* target);																	// 00466FB0
	// For isOrderValid
	bool unitIsResource(Unit* unit);                                                                                       // 004688E0

	//// cosmonarchy_specific
	u8 getHangerTrainCount(Unit* unit);
	bool unitIsTrainingOrMorphing(Unit* unit);
	u8 initializeRandomizerInfo();
	void AI_Loop();
	s32 updateActiveTileInfo();
	s32 removeFOWCheat();
	void updateUnits();
	s32 imageDrawingBulletDrawing();
	void updateImages();
	void updateThingys();

	// createUnit
	Unit* unitConstructor(s32 x, s32 y, u16 unitType, s32 playerId, s32 unused);						// 004A06C0
	Unit* CListPushBackUsedUnitEntry(Unit* unit);														// 0049DCA0
	Unit* CListPushBackHiddenUnitEntry(Unit* unit);														// 0049DCE0
	Sprite* unitDestructor(Unit* unit);																	// 004A0990
	void* getImageAttackFrame(void* resultArg, Sprite* sprite, s32 index, u8 playerId);					// 00401DF0
	void setSpriteMainImageOffset(Sprite* sprite, char x, char y);										// 00401E40

	//// psi_field_util
	bool addPsiFieldSprite(Unit* unit);

	// stim_packs
	void QueueGameCommand(u8* params, u32 param_length);

	// misc
	Thingy* createThingyFunc(u32 spriteId, s16 x, s16 y, u32 playerId);
	bool func_dounitscollide(Unit* first, Unit* second);
	void unburrow(Unit* unit);
	Image* CreateHealthBar(s32 a1, Sprite* a2);
	void RemoveHPBar(Image* a1);
	void refundAllQueueSlots(Unit* unit);
	void* SMemAlloc(u32 amount, u32 unkFilename, u32 unkLine, u32 defaultValue);	                    // 0041006A (based on Storm.dll definition)
	void SetCallbackTimer(BinDlg* dialog, u32 unk1, u32 unk2, u32 unk3);			                    // 00416090
	void removeFromSelections(Unit* unit);																// 0049A7F0
	u32 fxnInteract(BinDlg* dialog, u32 data_struct_offset);
	void registerUserDialogAction(BinDlg* dialog, u32 array_of_fxnInteractFunc, u32 sizeOfArray);	    // 00418100
	void enableControl(BinDlg* dialog);																	// 00418E00
	void replayStatBtns(BinDlg* dialog);																// 00427A80
	void updateCommandCard();																			// 004591D0
	void function_459770();																				// 00459770
	void BINDLG_BlitSurface(BinDlg* dialog);															// 004C35F0
	u32 fxnInteract(BinDlg* dialog, u32 data_struct_offset);
	s32 req_check(u32 reqFunc, u16 reqVar, u32 playerId, Unit* unit);
	void updateMineralPatchImage(Unit* mineralPatch);
	void updateImagePositionOffset(Image* image);

	// Identical to function @ 0x004F3AF0
	void setResourceAmountCarried(Unit* worker, u8 amountCarried, u32 chunkImageId, bool isMineral);

	// ai misc
	s32 Ai_HasBuildingsNeedingPower(u32 player);
	s32 Ai_CountEconomyUnits(u32 player, u32 unit_id);
	s32 getSuppliesRemaining(u8 player, u16 unit_id);
	s32 getSuppliesUsed(s32 byte_data, u8 player);
	s32 AI_AddCostToNeeds(s32 a1, s32 a2, s32 a3);
	s32 AI_AddSpendingRequest(s16 a1, s32 a2, char a3, char a4, void* a5);
	bool isAIAllowedToBuild(s32 a1, s32 a2);
	void applyCloakingOrder(Unit* unit);																	// 00491B30

	//// use_tech
	const void issueTriTargetOrder(u8 order1, u8 order2, u8 order3);										// 0048CAF0
	s32 checkEnergyForTechUse(s32 tech_id);																	// 00423660

	//// pathfinding
	s32 isPointAtUnitBorder(s32 unused, Unit* other, Unit* self, s32 position_xy);							// 004EB720
	s32 AI_PrepareMovingTo(Unit* unit, s32 x, s32 y);														// 004E7420

	// order attack behavior
	bool hasGroundWeaponGraphic(Unit* unit);
	bool isUnreachable(Unit* target, Unit* unit);
	bool isUnitTargetOutOfMaxRange(Unit* unit, Unit* target);												// 00476430
	s32 moveForMeleeRange(Unit* target, Unit* unit);

	// For createUpgradeLevelTooltip
	s16 CreateTooltip(char* text);																			// 004591A6
	char refreshSelectionScreen();																			// 00481480

	// lobby.cpp
	
	// For initGame
	void switchAllAvailableComputerSlotsToOpen();															// 004DBBE0
	bool setLastOpenSlotToComputer(s8 race);																// 004DBB70

	// For startGame
	s32 bootReason(u8 player);																				// 004B8870
	s32 isAuthedMap(void* a1);																				// 004C4870
	void setTimerFunc();																					// 00472E00
	s32 getTickCount();																						// 004FE0C4

	// For initializeBullet
	Flingy* initializeFlingy(s32 flingyId, s16 y, s32 x, Flingy* flingy, char player, char angle);			// 00496360
	void bulletBehaviorReassign(Bullet* bullet);															// 0048BD50
	s32 weaponBehaviorCanHitTargetCheck(Unit* target, Unit* attacker);										// 004765B0
	void appearOnTarget(Bullet* bullet, s16 x, s16 y);														// 004955C0
	Bullet* attackFlyToTarget(Bullet* bullet, s32 x, s32 y);												// 00494B70

	// For AI_ProgressBuild
	void displayErrorMessageIDWithSFX(s32 playerId, s32 messageId, s32 soundId);							// 0048EE30

	// For AIManageBuildingPlacement
	s32 getBunkerPlacementPosition(s32 placementId, s32* regionId);											// 004442E0
	Unit* getDisabledUnitInRange(s32 owner, Point16 position);												// 00440420
	void AIGetBuildLocation(s32 unitId, char* placementData, s32 playerId, Point16 tryPosition,				// 004461B0
		s32 maxRange);
	void AIPlacementDataSomething(s8* placementData, Point16 tryPosition);									// 00443610
	u32 getTileDistance(s32 x, s32 y, s32 x2, s32 y2);														// 00444100

	// For initializeUnitCounts
	char teamGame_CountRaces(char force, char* bIsLegal, char race);										// 004857B0

	// For StatRes_MainProc
	void showSupplyInfo(BinDlg* dialog, s16 graphic, s16 mathA, u16 mathB);									// 004E53B0
	void printStatResInfo(BinDlg* dialog, s16 graphic, char** a3, char* a4, s32 bSomething);				// 004E52E0

	// For redrawSupplyInfo
	bool unitStatCond_Standard();																			// 00424980

	// For drawSupplyUnitStatus
	void drawShieldUpgrade(BinDlg* dialog, u16 statusScreenIndex);											// 00425510
	void drawArmor(BinDlg* dialog, s32 unitId, u16 statusScreenIndex);										// 00425600

	// For progressBulletFrame
	void bulletBehaviorFly(Bullet* bullet);																	// 0048B3B0
	void bulletBehaviorFollow(Bullet* bullet);																// 0048B500
	void bulletBehaviorBounce(Bullet* bullet);																// 0048B2D0
	void bulletBehaviorPersist(Bullet* bullet);																// 0048BC70
	void bulletBehaviorInstant(Bullet* bullet);																// 0048A560
	void bulletBehaviorArea(Bullet* bullet);																// 0048B440

	// For giveUnitToPlayer
	void giveAllUnitPropertiesToPlayer(Unit* unit, s8 playerId);											// 0047C9F0
	Unit* getHiddenMiner(Unit* unit);																		// 00468A60
	bool unitHasHanger(Unit* unit);																			// identical to 004014B0
	s32 forHangarUnits(Unit* unit, Unit* (__fastcall* func)(Unit*, Unit*), s32 arg);						// 00465200
	Unit* getCreatingUnit(Unit* unit);																		// 00466A30

	// For UnitMove
	void MoveSprite(Sprite* sprite, s16 x, s16 y);															// 00497A10
	u32 updateUnitSortingInfo(u32 x_diff, Unit* unit, u32 y_diff);											// 0046A000

	// For bulletBehaviorReassign
	void playSpriteIscript(Sprite* sprite, char scriptId, s32 bPlay);										// 00499D00
	Bullet* assignWeaponData(Bullet* bullet, char behaviorType);											// 0048AA80

	// For AI_TrainingNormal
	AiRegion* getAiRegionInfoFromUnitLoc(Unit* unit);														// 00438BF0
	void AI_AddGuardAiToUnit(Unit* unit);																	// 00462960

	s8 createUnitAllowed(u16 unitId, Unit* unit, s32 playerID);												// 0046E1C0

	b32 SBmpLoadImage(const char *filename, void* palette, void* buffer, u32 buffersize, u32* width, u32* height, u32* bpp); // 00410154

	// For AI_MilitaryShouldWaitForGroup
	bool AI_AreOnConnectedRegions(Unit* unit1, Unit* unit2);												// 00438E10
	bool AI_CheckMilitaryGrouping(Unit* unit);																// 00439550

	// For AI_RemoveUnitAI
	s32 AI_RemoveFromAIStructs(Unit* unit, UnitAi* unitAI);													// 00439D60
	s32 AI_RemoveWorkerOrBuildingAI(Unit* unit, UnitAi* unitAI);											// 00434C90
	void AI_MedicRemove(Unit* unit);																		// 004477C0

	// AI utility
	void giveAi(Unit* unit);
	void AI_TrainingSimpleMilitary(Unit* newUnit, Unit* sourceUnit);										// 0043E280

	// For deleteAITownIfNeeded
	AiTown* tryBeginDeleteTown(AiTown* town);																// 00434330
	aiscript* deleteAIScriptsUsingTown(AiTown* town);														// 0045AE20

	// For displayEstablishingShot
	void* loadInitCreditsBIN(char* a1);																		// 004D8DF0
	bool SFileOpenArchive(char* archiveName, u32 dwPriority, u32 dwFlags, void* const* handle);				// 004100B2
	bool SFileCloseArchive(void* archive);																	// 004100BE
	bool SFileOpenFileEx(void* hMPQ, const char* szFileName, u32 dwSearchScope, void** phFile);				// 004100C4
	bool SFileCloseFile(void* hFile);																		// 004100C4

	// For AI_AreOnConnectedRegions
	bool AI_RegionsAreConnected(u32 playerId, u32 region1, u32 region2);									// 00437E70

	// For BTNSCOND_ZergBuildingRally
	u32 BTNSCOND_Rally(u32 unused, u32 player, Unit* unit);													// 00429520

	// For AI_GetBuildLocation
	u32 Ai_GetBuildingPlacementData(u32 maxRange, u32 unitId, char* placementData, u32 playerId, Point16 desiredPos);	// 00445D60
	char* Ai_RemoveUnreachableRegions(char* placementData, Point16 desiredPos);											// 00443900
	u32 Ai_RemoveDangerousRegions(u32 unitId, char* placementData, u32 placementId, Point16 desiredPos);				// 00443870
	u32 Ai_RemoveFarAwayTiles(char* placementData, Point16 desiredPos, u32 maxRange);									// 00444E20
	u32 Ai_RemoveDifferentHeight(char* placementData, Point16 desiredPos);												// 004436C0
	u32 Ai_PlacementRelatedSomething(u16 unitId, char* placementData, Point16 desiredPos);								// 00443E30
	void Ai_RemoveTilesIfUnfit(u32 unitId, char* placementData, u32 playerId, Point16 desiredPos);						// 00444CE0
	u32 Ai_RemoveAddonPlaces(u32 unitId, char* placementData, u32 playerId, Point16 desiredPos);						// 00443CC0
	u32 getTurretPlacementPosition(char* placementData, u32 playerId, Point16 desiredPos, u16 unitId, u32 unkFlag);		// 00443AC0
	u32 getPylonPlacementPosition(char* placementData, u8 playerId, Point16 desiredPos);								// 004448D0
	u32 getDefaultPlacementPosition(u32 unitId, char* placementData, u32 playerId, Point16 desiredPos);					// 004449A0

	// For AI_CheckMilitaryGrouping
	void getRegionCenter(u16 regionId, s32* out_x, s32* out_y);															// 0049CAA0

	// For ignoreIscriptGoToIdle
	void playIdleIscript(Sprite* sprite);																				// 00497880

	//// misc
	s8 SETS(s32 x);
	s8 OFSUB(s32 x, s32 y);
}