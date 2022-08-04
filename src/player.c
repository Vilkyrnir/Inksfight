#include "orx.h"
#include "utility.h"
#include "player.h"
#include "InksFight.h"
#include "pause.h"
#include "playersSelection.h"
#include "level.h"
#include "physics.h"

orxSTATUS orxFASTCALL player_controlPlayer(const orxSTRING input, const orxFLOAT fDT) {
  /*if (orxConsole_IsEnabled()) {
    return orxSTATUS_FAILURE;
  }*/
  orxInput_SelectSet(input);

  orxConfig_PushSection(input);
  const orxSTRING currentPlayer = orxConfig_GetString("PlayerObject");
  orxConfig_PopSection();

  orxOBJECT *character = RetrieveObjectFromConfig(currentPlayer);
  if (character == NULL) {
    return orxSTATUS_FAILURE;
  }
  orxOBJECT *charactersPen = orxObject_GetChild(character);

  if (orxInput_HasBeenActivated("Pause")) {
    if (getGamePaused() == orxFALSE) {
      // We retrieve the current gameState
      const orxSTRING currentLevelSection = getGameState();
      orxOBJECT *currentLevel = RetrieveObjectFromConfig(currentLevelSection);

      orxObject_PauseRecursive(currentLevel, orxTRUE);
      setGamePaused(orxTRUE);
      pause_displayMenu(PAUSEMENUSECTION);
    } else {
      const orxSTRING currentLevelSection = getGameState();
      orxOBJECT *currentLevel = RetrieveObjectFromConfig(currentLevelSection);

      orxObject_PauseRecursive(currentLevel, orxFALSE);
      setGamePaused(orxFALSE);
      orxOBJECT *pauseMenu = RetrieveObjectFromConfig(PAUSEMENUSECTION);
      //orxObject_Delete(pauseMenu);
      orxObject_AddFXRecursive(pauseMenu, "FadeOut", orxFLOAT_0);
    }
  }

  if (getGamePaused() == orxFALSE) {
    // Keyboard inputs
    if (orxInput_IsActive("ShootRight")) {
      orxVECTOR flipRight = player_getFlipRight();
      orxObject_SetScale(character, &flipRight);
      orxVECTOR characterScale;
      orxObject_GetScale(character, &characterScale);
      orxObject_SetRotation(charactersPen, characterScale.fX == flipRight.fX ? 0 : M_PI);
      orxObject_Enable(charactersPen, orxTRUE);
    } else if (orxInput_IsActive("ShootLeft")) {
      orxVECTOR flipLeft = player_getFlipLeft();
      orxObject_SetScale(character, &flipLeft);
      orxVECTOR characterScale;
      orxObject_GetScale(character, &characterScale);
      orxObject_SetRotation(charactersPen, characterScale.fX == flipLeft.fX ? 0 : M_PI);
      orxObject_Enable(charactersPen, orxTRUE);
    } else {
      orxObject_Enable(charactersPen, orxFALSE);
    }

    if (orxInput_IsActive("GoLeft")){
      orxVECTOR leftSpeed = player_getLeftSpeed();
      orxVECTOR velocity = player_getVelocity(currentPlayer);
      velocity.fX = leftSpeed.fX;
      player_setVelocity(currentPlayer, &velocity);
      //orxObject_ApplyImpulse(character, &leftSpeed, orxNULL);
      orxObject_SetTargetAnim(character, "CharacterRun");
    } else if (orxInput_IsActive("GoRight")){
      orxVECTOR rightSpeed = player_getRightSpeed();
      orxVECTOR velocity = player_getVelocity(currentPlayer);
      velocity.fX = rightSpeed.fX;
      player_setVelocity(currentPlayer, &velocity);
      //orxObject_ApplyImpulse(character, &rightSpeed, orxNULL);
      orxObject_SetTargetAnim(character, "CharacterRun");
    } else {
      orxVECTOR velocity = player_getVelocity(currentPlayer);
      velocity.fX = 0.f;
      player_setVelocity(currentPlayer, &velocity);
      orxObject_SetTargetAnim(character, "CharacterIdle");
    }

    if (orxInput_IsActive("Jump") && orxInput_HasNewStatus("Jump")){
      if (player_getState(currentPlayer) == STATEGROUNDED) { 
        orxSOUND *jumpSound;
        jumpSound = orxSound_CreateFromConfig(JUMPSOUNDSECTION);
        orxSound_Play(jumpSound);
        orxVECTOR velocity = player_getVelocity(currentPlayer);
        orxVECTOR jumpSpeed = player_getJumpSpeed(currentPlayer, orxFALSE);
        player_setJumpSpeed(currentPlayer, &jumpSpeed);
        velocity.fY = jumpSpeed.fY;
        player_setVelocity(currentPlayer, &velocity);
        player_setState(currentPlayer, STATEJUMPING);
        orxObject_Detach(character);
      }
    } else if (orxInput_IsActive("Jump") && !orxInput_HasNewStatus("Jump")) {
      orxFLOAT jumpTime = player_getJumpTime(currentPlayer);
      orxVECTOR jumpSpeed = player_getJumpSpeed(currentPlayer, orxTRUE);
      if (jumpTime < MAXJUMPTIME && !orxVector_IsNull(&jumpSpeed)) {
        jumpSpeed.fY = jumpSpeed.fY*JUMPDECREASEFACTOR;
        orxVECTOR velocity = player_getVelocity(currentPlayer);
        velocity.fY += jumpSpeed.fY;
        player_setVelocity(currentPlayer, &velocity);

        player_setJumpSpeed(currentPlayer, &jumpSpeed);
        player_setJumpTime(currentPlayer, jumpTime+fDT);
      }
    }
  } else {
    orxBOOL isMoving = orxFALSE;
    orxConfig_PushSection(PAUSEMENUSECTION);
    const orxSTRING selectorSection = orxConfig_GetString("Selector");
    orxConfig_PopSection();

    orxConfig_PushSection(selectorSection);
    isMoving = orxConfig_GetBool("Moving");
    orxConfig_PopSection();

    orxConfig_PushSection(PAUSEMENUSECTION);
    const orxSTRING confirmationNeeded = orxConfig_GetString("ConfirmationNeeded");
    orxConfig_PopSection();

    orxSTATUS selectedElement = orxSTATUS_FAILURE;
    // If the selector isn't moving'
    if (isMoving == orxFALSE) {
      // Case where the user validates selection
      if (orxInput_HasBeenActivated("ValidateMenu")) {
        orxConfig_PushSection(selectorSection);
        const orxSTRING selectedMenuItem = orxConfig_GetString("Selected");
        orxConfig_PopSection();

        if (orxString_Compare(selectedMenuItem, "MenuResume") == 0) {
          const orxSTRING currentLevelSection = getGameState();
          orxOBJECT *currentLevel = RetrieveObjectFromConfig(currentLevelSection);

          orxObject_PauseRecursive(currentLevel, orxFALSE);
          setGamePaused(orxFALSE);
          orxOBJECT *pauseMenu = RetrieveObjectFromConfig(PAUSEMENUSECTION);
          orxObject_Delete(pauseMenu);
        } else if (orxString_Compare(selectedMenuItem, "MenuQuit") == 0 ||
          orxString_Compare(confirmationNeeded, "MenuQuit") == 0) {
          if (orxString_Compare(confirmationNeeded, "") == 0) {
            pause_displayConfirm("MenuQuit", PAUSEMENUSECTION);
            orxConfig_PushSection(PAUSEMENUSECTION);
            orxConfig_SetString("ConfirmationNeeded", selectedMenuItem);
            orxConfig_PopSection();
          } else if (orxString_Compare(selectedMenuItem, "ConfirmYes") == 0) {
            orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
          } else {
            orxConfig_PushSection(PAUSEMENUSECTION);
            orxConfig_SetString("ConfirmationNeeded", "");
            orxConfig_PopSection();

            pause_hideConfirm("ConfirmationPH", PAUSEMENUSECTION);
          }
        }
      } else { // If the user uses arrows to navigate the menu
        const orxSTRING menuSection = (orxString_Compare(confirmationNeeded, "") == 0) ?
          PAUSEMENUSECTION : confirmationNeeded;

        orxVECTOR menuSize = pause_getMenuSize(menuSection);
        orxFLOAT XAxis = menuSize.fX > 1 ?
          orxInput_GetValue("SelectRight") - orxInput_GetValue("SelectLeft") : orxFLOAT_0;
        orxFLOAT YAxis = menuSize.fY > 1 ?
          orxInput_GetValue("SelectDown") - orxInput_GetValue("SelectUp") : orxFLOAT_0;

        orxVECTOR menuMove = {XAxis, YAxis, orxFLOAT_0};

        if(!orxVector_IsNull(&menuMove)) {
          orxVECTOR selectorMenuPosition = pause_GetMenuPosition(PAUSEMENUSECTION);
          orxSOUND *selectSound;
          selectSound = orxSound_CreateFromConfig(SELECTSOUNDSECTION);
          orxSound_Play(selectSound);

          while (selectedElement == orxSTATUS_FAILURE) {
            orxVector_Add(&selectorMenuPosition, &selectorMenuPosition, &menuMove);
            selectedElement = pause_SelectMenu(selectorSection, selectorMenuPosition, orxTRUE, menuSection);
          }
        }
      }
    }
  }

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL player_createCharacter(const orxSTRING input, const orxFLOAT fDT) {
  orxConfig_PushSection("Game");
  const orxSTRING map = orxConfig_GetString("GameState");
  orxConfig_PopSection();

  orxOBJECT *staticMap = RetrieveObjectFromConfig(map);

  orxConfig_PushSection(input);
  const orxSTRING selector = orxConfig_GetString("SelectorObject");
  const orxSTRING player = orxConfig_GetString("PlayerObject");
  orxConfig_PopSection();

  orxConfig_PushSection(selector);
  const orxSTRING charName = orxConfig_GetString("Selected");
  orxConfig_PopSection();

  const orxSTRING levelSection = getGameState();

  // We get the position of a random character spawn point and delete it from the list
  orxConfig_PushSection(levelSection);
  const orxSTRING spawnPoint = orxConfig_GetString("Spawners");
  const orxS32 spawnPointsCount = orxConfig_GetListCount("Spawners");
  const orxSTRING spawnPoints[spawnPointsCount-1];
  orxU32 index = 0;
  for (orxU32 c=0 ; c<spawnPointsCount ; c++) {
    const orxSTRING curSpawnPoint = orxConfig_GetListString("Spawners", c);
    if (orxString_Compare(spawnPoint, curSpawnPoint) != 0) {
      spawnPoints[index] = curSpawnPoint;
      index++;
    }
  }

  orxConfig_ClearValue("Spawners");
  orxConfig_AppendListString("Spawners", spawnPoints, spawnPointsCount-1);
  orxConfig_PopSection();

  orxConfig_PushSection(spawnPoint);
  orxVECTOR position;
  orxConfig_GetVector("Position", &position);
  orxConfig_PopSection();

  orxConfig_PushSection(player);
  orxConfig_SetVector("Position", &position);

  // We also create each player's health bar
  orxOBJECT *ch = orxObject_CreateFromConfig(orxConfig_GetString("HealthBarObject"));
  orxObject_SetOwner(ch, staticMap);

  orxConfig_PopSection();

  orxConfig_SetParent(charName, "CharacterObject");
  orxConfig_SetParent(player, charName);

  ch = orxObject_CreateFromConfig(player);
  orxObject_SetOwner(ch, staticMap);

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL player_gotHit(const orxSTRING playerSection) {
  orxOBJECT *player = RetrieveObjectFromConfig(playerSection);
  orxConfig_PushSection(playerSection);
  if (orxConfig_GetBool("Invincible") == orxFALSE) {
    orxU32 lives = orxConfig_GetU32("Lives");
    orxVECTOR livesVector = {lives > 1 ? lives-1 : 1, 1, 0};
    if (lives > 1) {
      lives--;
    } else {
      orxObject_AddFX(player, "FadeOut");
      /*level_destroyCurrentLevel();
      level_GenerateNextLevel();*/
      return orxSTATUS_SUCCESS;
    }
    orxObject_AddUniqueFX(player, "FlashFX");
    orxConfig_SetU32("Lives", lives);

    const orxSTRING healthBarSection = orxConfig_GetString("HealthBarObject");
    orxOBJECT *healthBarObject = RetrieveObjectFromConfig(healthBarSection);
    orxOBJECT *heart = orxObject_GetChild(healthBarObject);

    while (orxObject_IsEnabled(heart) == orxFALSE) {
      heart = orxObject_GetSibling(heart);
    }
    orxObject_Enable(heart, orxFALSE);
  }
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL player_checkDownwardRays(const orxSTRING input, orxFLOAT fDT) {
  orxConfig_PushSection(input);
  const orxSTRING currentPlayer = orxConfig_GetString("PlayerObject");
  orxConfig_PopSection();
  
  orxU32 playerState = player_getState(currentPlayer);
  // Rays checking is useless
  if (playerState != STATEGROUNDED && playerState != STATEFALLING) {
    return orxSTATUS_SUCCESS;
  }

  orxOBJECT *ch = RetrieveObjectFromConfig(currentPlayer);
  
  if (ch == orxNULL) {
    return orxSTATUS_FAILURE;
  }
  
  orxVECTOR startPoint, endPoint;
  orxFLOAT distance;
  orxOBOX box;
  orxVECTOR center, size, scale;
  orxVECTOR velocity = player_getVelocity(currentPlayer);
  
  orxObject_GetBoundingBox(ch, &box);
  orxOBox_GetCenter(&box, &center);
  orxObject_GetSize(ch, &size);
  orxObject_GetScale(ch, &scale);
  orxVector_Mul(&size, &size, &scale);
  
  distance = MARGIN + (playerState == STATEGROUNDED ? MARGIN : orxMath_Abs(velocity.fY * fDT));
  orxVector_Set(&startPoint, center.fX - size.fX / 2 + MARGIN, center.fY + size.fY / 2 - MARGIN, orxFLOAT_0);
  orxVector_Set(&endPoint, center.fX + size.fX / 2 - MARGIN, center.fY + size.fY / 2 - MARGIN, orxFLOAT_0);

  for (orxS32 i = 0; i < VERTICALRAYS; i ++) {
    orxVECTOR origin, end, contact, dContact;

    orxFLOAT lerpAmount = orx2F(i) / orx2F(VERTICALRAYS - 1);
    orxVector_Lerp(&origin, &startPoint, &endPoint, lerpAmount);
    orxVector_Copy(&end, &origin);
    end.fY += distance;

    orxBODY* pBody = orxBody_Raycast(&origin, &end, 0xFFFF, platformCFlag, orxTRUE, &contact, orxNULL);
    orxBODY* dPBody = orxBody_Raycast(&origin, &end, 0xFFFF, dPlatformCFlag, orxTRUE, &dContact, orxNULL);

    if (pBody != orxNULL || dPBody != orxNULL) {
      orxVECTOR position;
      player_setState(currentPlayer, STATEGROUNDED);
      position = player_getPosition(currentPlayer);
      position.fY = contact.fY;
      //player_setPosition(currentPlayer, &position);
      velocity.fY = orxFLOAT_0;
      player_setVelocity(currentPlayer, &velocity);
      //break;
      player_setJumpTime(currentPlayer, orxFLOAT_0);
      player_clearJumpSpeed(currentPlayer);
      if (dPBody != orxNULL) {
        orxOBJECT* dPlatform = orxOBJECT(orxStructure_GetOwner(dPBody));
        //orxLOG("%s", orxObject_GetName(dPlatform));
        orxObject_Attach(ch, dPlatform);
      }
      return orxSTATUS_SUCCESS;
    } else {
      player_setState(currentPlayer, STATEFALLING);
      orxObject_Detach(ch);
    }
  }
  
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL player_checkUpwardRays(const orxSTRING input, orxFLOAT fDT) {
  orxConfig_PushSection(input);
  const orxSTRING currentPlayer = orxConfig_GetString("PlayerObject");
  orxConfig_PopSection();
  
  orxU32 collisionFlag = orxPhysics_GetCollisionFlagValue("platforms");
  orxU32 playerState = player_getState(currentPlayer);
  // Rays checking is useless
  if (playerState != STATEJUMPING) {
    return orxSTATUS_SUCCESS;
  }

  orxOBJECT *ch = RetrieveObjectFromConfig(currentPlayer);
  
  if (ch == orxNULL) {
    return orxSTATUS_FAILURE;
  }
  
  orxVECTOR startPoint, endPoint;
  orxFLOAT distance;
  orxOBOX box;
  orxVECTOR center, size, scale;
  orxVECTOR velocity = player_getVelocity(currentPlayer);
  
  orxObject_GetBoundingBox(ch, &box);
  orxOBox_GetCenter(&box, &center);
  orxObject_GetSize(ch, &size);
  orxObject_GetScale(ch, &scale);
  orxVector_Mul(&size, &size, &scale);
  
  distance = MARGIN + (velocity.fY >= orxFLOAT_0 ? MARGIN : orxMath_Abs(velocity.fY * fDT));
  orxVector_Set(&startPoint, center.fX - size.fX / 2 + MARGIN, center.fY - size.fY / 2 + MARGIN, orxFLOAT_0);
  orxVector_Set(&endPoint, center.fX + size.fX / 2 - MARGIN, center.fY - size.fY / 2 + MARGIN, orxFLOAT_0);

  for (orxS32 i = 0; i < VERTICALRAYS; i ++) {
    orxVECTOR origin, end, contact;

    orxFLOAT lerpAmount = orx2F(i) / orx2F(VERTICALRAYS - 1);
    orxVector_Lerp(&origin, &startPoint, &endPoint, lerpAmount);
    orxVector_Copy(&end, &origin);
    end.fY -= distance;

    orxBODY* pBody = orxBody_Raycast(&origin, &end, 0xFFFF, platformCFlag, orxTRUE, &contact, orxNULL);
    orxBODY* dPBody = orxBody_Raycast(&origin, &end, 0xFFFF, dPlatformCFlag, orxTRUE, &contact, orxNULL);

    if (pBody != orxNULL || dPBody != orxNULL) {
      player_setState(currentPlayer, STATEFALLING);
      velocity.fY = orxFLOAT_0;
      player_setVelocity(currentPlayer, &velocity);
      player_setJumpTime(currentPlayer, MAXJUMPTIME);
      return orxSTATUS_SUCCESS;
    }
  }
  
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL player_checkLeftRays(const orxSTRING input, orxFLOAT fDT) {
  orxConfig_PushSection(input);
  const orxSTRING currentPlayer = orxConfig_GetString("PlayerObject");
  orxConfig_PopSection();
  
  orxU32 collisionFlag = orxPhysics_GetCollisionFlagValue("platforms");
  orxU32 playerState = player_getState(currentPlayer);

  orxOBJECT *ch = RetrieveObjectFromConfig(currentPlayer);
  
  if (ch == orxNULL) {
    return orxSTATUS_FAILURE;
  }
  
  orxVECTOR startPoint, endPoint;
  orxFLOAT distance;
  orxOBOX box;
  orxVECTOR center, size, scale;
  orxVECTOR velocity = player_getVelocity(currentPlayer);
  
  orxObject_GetBoundingBox(ch, &box);
  orxOBox_GetCenter(&box, &center);
  orxObject_GetSize(ch, &size);
  orxObject_GetScale(ch, &scale);
  orxVector_Mul(&size, &size, &scale);
  
  distance = -(MARGIN + (velocity.fX == orxFLOAT_0 ? MARGIN : orxMath_Abs(velocity.fX * fDT)));
  orxVector_Set(&startPoint, center.fX - size.fX / 2 + MARGIN, center.fY - size.fY / 2 + MARGIN, orxFLOAT_0);
  orxVector_Set(&endPoint, center.fX - size.fX / 2 + MARGIN, center.fY + size.fY / 2 - MARGIN, orxFLOAT_0);

  for (orxS32 i = 0; i < HORIZONTALRAYS; i ++) {
    orxVECTOR origin, end, contact;

    orxFLOAT lerpAmount = orx2F(i) / orx2F(HORIZONTALRAYS - 1);
    orxVector_Lerp(&origin, &startPoint, &endPoint, lerpAmount);
    orxVector_Copy(&end, &origin);
    end.fX += distance;

    orxBODY* pBody = orxBody_Raycast(&origin, &end, 0xFFFF, platformCFlag, orxTRUE, &contact, orxNULL);
    orxBODY* dPBody = orxBody_Raycast(&origin, &end, 0xFFFF, dPlatformCFlag, orxTRUE, &contact, orxNULL);


    if ((pBody != orxNULL || dPBody != orxNULL) && velocity.fX < orxFLOAT_0) {
      //player_setState(currentPlayer, STATEFALLING);
      velocity.fX = orxFLOAT_0;
      player_setVelocity(currentPlayer, &velocity);
      return orxSTATUS_SUCCESS;
    }
  }
  
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL player_checkRightRays(const orxSTRING input, orxFLOAT fDT) {
  orxConfig_PushSection(input);
  const orxSTRING currentPlayer = orxConfig_GetString("PlayerObject");
  orxConfig_PopSection();
  
  orxU32 collisionFlag = orxPhysics_GetCollisionFlagValue("platforms");
  orxU32 playerState = player_getState(currentPlayer);

  orxOBJECT *ch = RetrieveObjectFromConfig(currentPlayer);
  
  if (ch == orxNULL) {
    return orxSTATUS_FAILURE;
  }
  
  orxVECTOR startPoint, endPoint;
  orxFLOAT distance;
  orxOBOX box;
  orxVECTOR center, size, scale;
  orxVECTOR velocity = player_getVelocity(currentPlayer);
  
  orxObject_GetBoundingBox(ch, &box);
  orxOBox_GetCenter(&box, &center);
  orxObject_GetSize(ch, &size);
  orxObject_GetScale(ch, &scale);
  orxVector_Mul(&size, &size, &scale);
  
  distance = MARGIN + (velocity.fX == orxFLOAT_0 ? MARGIN : orxMath_Abs(velocity.fX * fDT));
  orxVector_Set(&startPoint, center.fX + size.fX / 2 - MARGIN, center.fY - size.fY / 2 + MARGIN, orxFLOAT_0);
  orxVector_Set(&endPoint, center.fX + size.fX / 2 - MARGIN, center.fY + size.fY / 2 - MARGIN, orxFLOAT_0);

  for (orxS32 i = 0; i < HORIZONTALRAYS; i ++) {
    orxVECTOR origin, end, contact;

    orxFLOAT lerpAmount = orx2F(i) / orx2F(HORIZONTALRAYS - 1);
    orxVector_Lerp(&origin, &startPoint, &endPoint, lerpAmount);
    orxVector_Copy(&end, &origin);
    end.fX += distance;

    orxBODY* pBody = orxBody_Raycast(&origin, &end, 0xFFFF, platformCFlag, orxTRUE, &contact, orxNULL);
    orxBODY* dPBody = orxBody_Raycast(&origin, &end, 0xFFFF, dPlatformCFlag, orxTRUE, &contact, orxNULL);


    if ((pBody != orxNULL || dPBody != orxNULL) && velocity.fX > orxFLOAT_0) {
      velocity.fX = orxFLOAT_0;
      player_setVelocity(currentPlayer, &velocity);
      return orxSTATUS_SUCCESS;
    }
  }
  
  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL player_updatePosition(const orxSTRING input, orxFLOAT fDT) {
  orxVECTOR deltaVector;
  
  orxConfig_PushSection(input);
  const orxSTRING currentPlayer = orxConfig_GetString("PlayerObject");
  orxConfig_PopSection();

  orxOBJECT *character = RetrieveObjectFromConfig(currentPlayer);
  if (character == NULL) {
    return orxSTATUS_FAILURE;
  }
  
  orxVECTOR position = player_getPosition(currentPlayer);
  orxVECTOR velocity = player_getVelocity(currentPlayer);
  orxVector_Mulf(&deltaVector, &velocity, fDT);
  orxVector_Add(&position, &position, &deltaVector);
  player_setPosition(currentPlayer, &position);
}

orxSTATUS orxFASTCALL player_applyGravity(const orxSTRING input, orxFLOAT fDT) {
  orxConfig_PushSection(input);
  const orxSTRING currentPlayer = orxConfig_GetString("PlayerObject");
  orxConfig_PopSection();

  orxOBJECT *character = RetrieveObjectFromConfig(currentPlayer);
  if (character == NULL) {
    return orxSTATUS_FAILURE;
  }
  
  orxU32 playerState = player_getState(currentPlayer);
  orxVECTOR velocity = player_getVelocity(currentPlayer);
  orxVECTOR NV = velocity;

  if (playerState != STATEGROUNDED) {
    orxVECTOR gravity = physics_getGravity();
    orxU32 maxFallVelocity = physics_getMaxFallVelocity();
    NV.fY = orxMIN(velocity.fY + gravity.fY * fDT, maxFallVelocity);
    player_setVelocity(currentPlayer, &NV);
  }

  if(velocity.fY > orxFLOAT_0) {
    player_setState(currentPlayer, STATEFALLING);
  }
  
  return orxSTATUS_SUCCESS;
}

orxU32 orxFASTCALL player_getState(const orxSTRING player) {
  orxConfig_PushSection(player);
  orxU32 state = orxConfig_GetU32(STATEKEY);
  orxConfig_PopSection();
  
  return state;
}

orxSTATUS orxFASTCALL player_setState(const orxSTRING player, orxU32 state) {
  orxConfig_PushSection(player);
  orxConfig_SetU32(STATEKEY, state);
  orxConfig_PopSection();
  return orxSTATUS_SUCCESS;
}

orxVECTOR orxFASTCALL player_getVelocity(const orxSTRING player) {
  orxVECTOR velocity;
  orxConfig_PushSection(player);
  orxConfig_GetVector(VELOCITYKEY, &velocity);
  orxConfig_PopSection();
  
  return velocity;
}

orxSTATUS orxFASTCALL player_setVelocity(const orxSTRING player, const orxVECTOR* velocity) {
  orxConfig_PushSection(player);
  orxConfig_SetVector(VELOCITYKEY, velocity);
  orxConfig_PopSection();
  
  return orxSTATUS_SUCCESS;
}

orxVECTOR orxFASTCALL player_getPosition(const orxSTRING player) {
  orxVECTOR position;
  
  orxOBJECT *ch = RetrieveObjectFromConfig(player);
  
  if (ch == orxNULL) {
    position = orxVECTOR_0;
  } else {
    orxObject_GetWorldPosition(ch, &position);
  }
  
  return position;
}

orxSTATUS orxFASTCALL player_setPosition(const orxSTRING player, const orxVECTOR* position) {
  orxOBJECT *ch = RetrieveObjectFromConfig(player);
  
  if (ch == orxNULL) {
    return orxSTATUS_FAILURE;
  }
  
  orxObject_SetWorldPosition(ch, position);
  
  return orxSTATUS_SUCCESS;
}

orxVECTOR orxFASTCALL player_getLeftSpeed() {
  orxConfig_PushSection("CharacterObject");
  orxVECTOR leftSpeed;
  orxConfig_GetVector("LeftSpeed", &leftSpeed);
  orxConfig_PopSection();

  return leftSpeed;
}

orxVECTOR orxFASTCALL player_getRightSpeed() {
  orxConfig_PushSection("CharacterObject");
  orxVECTOR rightSpeed;
  orxConfig_GetVector("RightSpeed", &rightSpeed);
  orxConfig_PopSection();

  return rightSpeed;
}

orxVECTOR orxFASTCALL player_getGenericJumpSpeed() {
  orxVECTOR jumpSpeed;
  orxConfig_PushSection(CHARACTEROBJECTSECTION);
  orxConfig_GetVector(GJUMPSPEEDKEY, &jumpSpeed);
  orxConfig_PopSection();

  return jumpSpeed;
}

orxVECTOR orxFASTCALL player_getJumpSpeed(const orxSTRING playerSection, orxBOOL strict) {
  orxVECTOR jumpSpeed = orxVECTOR_0;

  orxConfig_PushSection(playerSection);
  if (orxConfig_HasValue(JUMPSPEEDKEY)) {
    orxConfig_GetVector(JUMPSPEEDKEY, &jumpSpeed);
  } else if (strict == orxFALSE) {
    jumpSpeed = player_getGenericJumpSpeed();
  }
  orxConfig_PopSection();

  return jumpSpeed;
}

orxFLOAT orxFASTCALL player_getJumpTime(const orxSTRING playerSection) {
  orxFLOAT jumpTime;

  orxConfig_PushSection(playerSection);
  jumpTime = orxConfig_HasValue(JUMPTIMEKEY) ? orxConfig_GetFloat(JUMPTIMEKEY) : orxFLOAT_0;
  orxConfig_PopSection();

  return jumpTime;
}

orxSTATUS orxFASTCALL player_setJumpTime(const orxSTRING playerSection, const orxFLOAT jumpTime) {
  orxConfig_PushSection(playerSection);
  orxConfig_SetFloat(JUMPTIMEKEY, jumpTime);
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL player_setJumpSpeed(const orxSTRING playerSection, const orxVECTOR* jumpSpeed) {
  orxConfig_PushSection(playerSection);
  orxConfig_SetVector(JUMPSPEEDKEY, jumpSpeed);
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL player_clearJumpSpeed(const orxSTRING playerSection) {
  orxConfig_PushSection(playerSection);
  orxConfig_ClearValue(JUMPSPEEDKEY);
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

orxVECTOR orxFASTCALL player_getFlipRight() {
  orxConfig_PushSection("CharacterObject");
  orxVECTOR flipRight;
  orxConfig_GetVector("FlipRight", &flipRight);
  orxConfig_PopSection();

  return flipRight;
}

orxVECTOR orxFASTCALL player_getFlipLeft() {
  orxConfig_PushSection("CharacterObject");
  orxVECTOR flipLeft;
  orxConfig_GetVector("FlipLeft", &flipLeft);
  orxConfig_PopSection();

  return flipLeft;
}


