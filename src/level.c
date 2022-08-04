#include "level.h"
#include "InksFight.h"
#include "utility.h"
#include "player.h"
#include "physics.h"

orxVECTOR levelLimitsBR = {0, 0, 0};
orxVECTOR levelLimitsTL = {0, 0, 0};
orxS32 platformCFlag = 0;
orxS32 dPlatformCFlag = 0;

orxSTATUS orxFASTCALL level_Displaylayer(const orxSTRING sName, const orxS16 zIndex, const orxSTRING tileSetSection) {
  orxVECTOR TileSize, MapSize, TilePos;

  // We first push the tileSetSection section to retrieve tiles width and height
  orxConfig_PushSection(tileSetSection);
  orxConfig_GetVector("TextureSize", &TileSize);
  orxConfig_PopSection();

  // We then push the sName section to get the layer's tiles data
  orxConfig_PushSection(sName);
  orxConfig_GetVector("Size", &MapSize);
  // levelLimits not yet defined
  if (levelLimitsBR.fX == 0 && levelLimitsBR.fY == 0) {
    levelLimitsBR.fX = MapSize.fX*TileSize.fX;
    levelLimitsBR.fY = MapSize.fY*TileSize.fY;
  }
  orxVector_Set(&TilePos, orxFLOAT_0, orxFLOAT_0, orxS2F(zIndex));
  const orxSTRING *TileNames = (const orxSTRING *)alloca(orxF2U(MapSize.fX) * orxF2U(MapSize.fY) * sizeof(orxSTRING));
  orxU32 TileCount = 0;
  for (orxU32 i=1 ; i <= orxF2U(MapSize.fY) ; i++) {
    orxCHAR Row[64];
    TilePos.fX = orxFLOAT_0;

    orxString_NPrint(Row, sizeof(Row) - 1, "Row%u", i);
    for (orxU32 j=0 ; j < orxF2U(MapSize.fX) ; j++) {
      const orxSTRING Tile = orxConfig_GetListString(Row, j);
      if (orxString_Compare(Tile, "NONE")) {
        orxCHAR Buffer[64];
        orxString_NPrint(Buffer, sizeof(Buffer) - 1, "%s-%u.%u", sName, j + 1, i);
        orxConfig_ClearSection(Buffer);
        orxConfig_PushSection(Buffer);
        orxConfig_SetString("Graphic", Tile);
        orxConfig_SetFloat("Alpha", 0.0f);
        //orxConfig_SetString("Group", "Map");
        orxString_NPrint(Buffer, sizeof(Buffer) - 1, "%sBody", Tile);
        if(orxConfig_HasSection(Buffer)) {
          orxConfig_SetString("Body", Buffer);
        }
        orxConfig_SetVector("Position", &TilePos);
        TileNames[TileCount++] = orxConfig_GetCurrentSection();
        orxConfig_PopSection();
      }

      TilePos.fX += TileSize.fX;
    }

    TilePos.fY += TileSize.fY;
  }
  orxConfig_SetListString("ChildList", TileNames, TileCount);
  orxConfig_PopSection();

  const orxSTRING levelSection = getGameState();

  orxConfig_PushSection(levelSection);
  orxConfig_AppendListString("ChildList", &sName, 1);
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL level_GenerateLevelsSet() {
  orxConfig_PushSection("Game");
  orxU32 levelsCount = orxConfig_GetU32("LevelsCount");
  orxU32 availableLevelsCount = orxConfig_GetListCount("AvailableLevels");

  for (orxU32 i=0 ; i<levelsCount ; i++) {
    const orxSTRING tmpLevel = orxConfig_GetString("AvailableLevels");
    orxConfig_AppendListString("Levels", &tmpLevel, 1);
  }
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL level_GenerateNextLevel() {
  orxConfig_PushSection("Game");
  orxS16 currentLevelIndex = orxConfig_GetU32("CurrentLevelIndex")+1;
  orxConfig_SetU32("CurrentLevelIndex", currentLevelIndex);
  const orxSTRING test = orxConfig_DuplicateRawValue("Levels");
  const orxSTRING levelSection = orxConfig_GetListString("Levels", currentLevelIndex);
  orxConfig_PopSection();

  setGameState(levelSection);

  orxCHAR filename[256];
  orxString_NPrint(filename, sizeof(filename) - 1, "../data/config/levels/%s.ini", levelSection);
  orxConfig_Load(filename);

  level_Displaylayer("fond", 1, "TilesGraphic");
  level_Displaylayer("deco", 0, "TilesGraphic");
  level_Displaylayer("calque1", 0, "TilesGraphic");
  orxOBJECT *level = orxObject_CreateFromConfig(levelSection);
  orxObject_AddFXRecursive(level, "FadeIn", 0.0f);

  // Characters creation
  AllPlayersCallback(0.0f, "Game", "InputList", &player_createCharacter);
  physics_enable(orxTRUE);
  
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL level_destroyCurrentLevel() {
  const orxSTRING currentLevelSection = getGameState();
  orxOBJECT *currentLevel = RetrieveObjectFromConfig(currentLevelSection);

  orxObject_SetLifeTime(currentLevel, 0);

  //orxConfig_Clear(&level_ClearConfigCallback);

  return orxSTATUS_SUCCESS;
}

orxBOOL orxFASTCALL level_ClearConfigCallback(const orxSTRING _zSectionName, const orxSTRING _zKeyName){
  orxBOOL bResult;
  orxCHAR filename[256];
  const orxSTRING currentLevelSection = getGameState();
  orxString_NPrint(filename, sizeof(filename)-1, "%s.ini", currentLevelSection);

  /* Updates result */
  bResult = (orxString_Compare(orxConfig_GetOrigin(_zSectionName), filename) == 0
              || orxConfig_GetOriginID(_zSectionName) == orxSTRINGID_UNDEFINED) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}
