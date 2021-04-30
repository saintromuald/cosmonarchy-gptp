#pragma once
#include "SCBW\scbwdata.h"

bool HasMoneyCanMake(CUnit* builder, uint32 unitToBuild);


void removeOrderFromUnitQueue(CUnit* unit, COrder* order);

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


bool isConstructingAddon(CUnit* a1);


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


void updateImageDrawData(CImage* a1);


void playImageIscript(CImage* a1, char a2);

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

void AI_TrainingOverlord(CUnit* result, CUnit* parent);

int16 SAI_GetRegionIdFromPxEx(int32 x, int32 y);

CUnit* createUnit(int16 unitType, int32 x, int32 y, int32 playerId);
void updateUnitStatsFinishBuilding(CUnit* unit);				

uint32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit);

char AI_CancelStructure(CUnit* structure);

bool unitCantSeeCloakedTarget(CUnit* unit, CUnit* target);												//0x00401D60
bool isInfestableUnit(CUnit* unit);																		//0x00402210
bool unitCanInfest(CUnit* unit);																		//0x00402750
void AI_AttackUnit(CUnit* unit);																		//0x0043FFD0
CUnit* findBestAttackTarget(CUnit* unit);																//0x00443080
bool function_00462EA0(CUnit* unit, uint8 unknownByteValue);												//0x00462EA0
void function_00465780(CUnit* unit);																	//0x00465780
void removeOrderFromUnitQueue(CUnit* unit, COrder* order);												//0x004742D0
void function_00474A70(CUnit* unit, CUnit* target, uint8 orderId);											//0x00474A70
bool isTargetWithinMinMovementRange(CUnit* unit, CUnit* target, uint32 range);								//0x004763D0
bool function_00476610(CUnit* unit, int x, int y);														//0x00476610
bool isUnitInWeaponRange(CUnit* unit, CUnit* target);													//0x00476870
void getWeaponBeginIscript(CUnit* unit, IscriptAnimation::Enum anim);									//0x00476ED0
void function_00477510(CUnit* unit);																	//0x00477510
void function_00477820(CUnit* unit, uint8 orderId);														//0x00477820
bool function_00478370(CUnit* unit, uint8 orderId);														//0x00478370
bool function_00478540(CUnit* unit);																	//0x00478540
bool isUnitCritter(CUnit* unit);																		//0x0047B450
void function_00495400(CUnit* unit, CUnit* target);														//0x00495400

bool isUnitVisible(CUnit* unit);																		//0x004E5DB0
bool isTargetVisible(CUnit* unit, CUnit* target);														//0x004E5E30
uint8 getRightClickActionOrder(CUnit* unit);																//0x004E5EA0
void setNextWaypoint_Sub4EB290(CUnit* unit);															//0x004EB290
void makeToHoldPosition(CUnit* unit);																	//0x004EB5B0
bool moveToTarget(CUnit* unit, CUnit* target);															//0x004EB720
bool SetMoveTarget_xy(CUnit* unit, int x, int y);														//0x004EB820
bool function_004EB900(CUnit* unit, CUnit* target);														//0x004EB900
bool function_004EB9C0(CUnit* unit, int x, int y);														//0x004EB9C0

bool isDistanceGreaterThanHaltDistance(CUnit* unit, int x, int y, uint32 distance);	//01240
void function_0042D9A0(CUnit* unit);												//2D9A0
void AI_TrainingOverlord(CUnit* main_unit, CUnit* created_unit);					//35770
bool function_00467030(CUnit* unit);												//67030
CUnit* function_004678A0(CUnit* unit, uint32 unitId);									//678A0
void function_00498D70(CSprite* sprite, uint32 imageId, uint32 unk1, uint32 unk2, uint32 unk3);	//98D70
void replaceSpriteImages(CSprite* sprite, uint32 imageId, uint32 imageDirection);			//99BB0
void displayLastNetErrForPlayer(uint32 playerId);										//9E530
void updateUnitStrength(CUnit* unit);												//9FA40
void function_004A01F0(CUnit* unit);												//A01F0
CUnit* createUnit(uint32 unitId, int x, int y, uint32 playerId);							//A09D0
void iscript_OpcodeCases(CImage* image, uint32 offset_, uint32 unk1, uint32 unk2);			//D74C0
void function_004D8500(CImage* image);												//D8500
void makeToHoldPosition(CUnit* unit);												//EB5B0
bool function_004EB9C0(CUnit* unit, int x, int y);									//EB9C0
bool setThingyVisibilityFlags(CThingy* thingy);									//0x004878F0
CThingy* createThingy(uint32 spriteId, int16 x, int16 y, uint32 playerId);				//0x00488210
void setUnitStatTxtErrorMsg(char* message);										//0x0048CCB0
void playUnitSFX(uint32 soundId, uint16 x, uint16 y, uint32 unk1, uint32 unk2);				//0x0048EC10
uint32 RandomizeShort(uint32 unknown_index);											//0x004DC4A0
uint32 IterateUnitsAtLocationTargetProc_RecallTargets(CUnit* unit, Box16* coords);	//0x004E8280
void function_004EBAE0(CUnit* unit, int32 x, int32 y);								//0x004EBAE0