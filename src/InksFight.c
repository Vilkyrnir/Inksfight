//! Includes
#include "time.h"
#include "orx.h"
#include "InksFight.h"
#include "playersSelection.h"
#include "level.h"
#include "player.h"
#include "utility.h"
#include "command.h"
#include "physics.h"

//! Defines

#if defined(__orxX86_64__) || defined(__orxPPC64__) || defined(__orxARM64__)
  #define CAST_HELPER (orxU64)
#else /* __orxX86_64__ || __orxPPC64__ || __orxARM64__ */
  #define CAST_HELPER (orxU32)
#endif /* __orxX86_64__ || __orxPPC64__ || __orxARM64__ */

#ifdef __orxMSVC__

/* Requesting high performance dedicated GPU on hybrid laptops */
__declspec(dllexport) unsigned long NvOptimusEnablement        = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#endif // __orxMSVC__

//! Structures



//! Variables
static orxVIEWPORT *pstViewport;
static orxOBJECT *character, *charactersPen, *title, *staticMap, *camHandle;

orxFLOAT maxCamDistX = 100;
orxFLOAT maxCamDistY = 200;
orxFLOAT VPWidth = 0;
orxFLOAT VPHeight = 0;
orxU32 alreadyLoggued = 0;

orxSTATUS orxFASTCALL initScreenControls(const orxSTRING input, const orxFLOAT fDT) {
  orxInput_SelectSet(input);
  if(orxInput_IsActive("Quit"))
  {
    orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
  }
  if (orxInput_HasBeenActivated("Start")) {
    orxObject_Delete(title);
    title = NULL;
    ps_AddCharacterSelector(1, input);
    ps_DisplayCharactersList();

    orxConfig_PushSection("Game");
    orxConfig_AppendListString("InputList", &input, 1);
    orxConfig_PopSection();

    orxEvent_AddHandler(orxEVENT_TYPE_FX, FXEventHandler);

    setGameState("playersSelection");
  }

  return orxSTATUS_SUCCESS;
}

void orxFASTCALL Update(const orxCLOCK_INFO *_pstInfo, void *_pContext) {
  const orxSTRING gameState = getGameState();
  const orxSTRING defaultInput = orxInput_GetCurrentSet();

  if (orxString_Compare(gameState, "title") == 0) {
    // Player 1 will be the first one to press Enter or Start

    //orxSTATUS (*initScreenControlsPtr)(const orxSTRING) = &initScreenControls;
    AllPlayersCallback(_pstInfo->fDT, "Game", "AvailableInputs", &initScreenControls);
  } else if (orxString_Compare(gameState, "playersSelection") == 0) {
    extern orxOBJECT *playersSelection;

    // We first check for controls of added players
    AllPlayersCallback(_pstInfo->fDT, "Game", "InputList", &ps_ControlSelector);

    // Then we check for wannabe players
    AllPlayersCallback(_pstInfo->fDT, "Game", "AvailableInputs", &ps_AddNewPlayer);

  } else {

    CameraFollowCharacters(camHandle);

    AllPlayersCallback(_pstInfo->fDT, "Game", "InputList", &player_controlPlayer);
    
    if (physics_isEnabled()) {
      AllPlayersCallback(_pstInfo->fDT, "Game", "InputList", &player_applyGravity);
      AllPlayersCallback(_pstInfo->fDT, "Game", "InputList", &player_checkDownwardRays);
      AllPlayersCallback(_pstInfo->fDT, "Game", "InputList", &player_checkUpwardRays);
      AllPlayersCallback(_pstInfo->fDT, "Game", "InputList", &player_checkLeftRays);
      AllPlayersCallback(_pstInfo->fDT, "Game", "InputList", &player_checkRightRays);
      AllPlayersCallback(_pstInfo->fDT, "Game", "InputList", &player_updatePosition);
    }
  }

  orxInput_SelectSet(defaultInput);
}

orxSTATUS orxFASTCALL FXEventHandler(const orxEVENT *_pstEvent) {
  orxOBJECT *pstSenderObject;
  orxFX_EVENT_PAYLOAD *pstPayload;
  pstSenderObject = orxOBJECT(_pstEvent->hSender);

  pstPayload = _pstEvent->pstPayload;

  const orxSTRING senderSectionName = orxObject_GetName(pstSenderObject);

  orxConfig_PushSection(senderSectionName);
  orxBOOL senderHasJumpSContact = orxConfig_HasValue("JumpSContact");
  orxConfig_PopSection();

  if (orxString_Compare(pstPayload->zFXName, "SelectorPosFX") == 0) {
    orxConfig_PushSection(orxObject_GetName(pstSenderObject));
    switch (_pstEvent->eID) {
      case orxFX_EVENT_START:
        orxConfig_SetBool("Moving", orxTRUE);
        break;
      case orxFX_EVENT_STOP:
        orxConfig_SetBool("Moving", orxFALSE);
        break;
    }
    orxConfig_PopSection();
  } else if (senderHasJumpSContact == orxTRUE) { // The target object is a playable character
    if (orxString_Compare(pstPayload->zFXName, "FlashFX") == 0) {
      orxConfig_PushSection(senderSectionName);
      // FlashFX on character activated
      switch (_pstEvent->eID) {
        case orxFX_EVENT_START:
          orxConfig_SetBool("Invincible", orxTRUE);
          break;
        case orxFX_EVENT_STOP:
          orxConfig_SetBool("Invincible", orxFALSE);
          break;
      }
      orxConfig_PopSection();
    }
  } else if (orxString_Compare(pstPayload->zFXName, "ScaleTransFX") == 0) {
    switch (_pstEvent->eID) {
      case orxFX_EVENT_START:
        orxObject_RemoveFX(pstSenderObject, "ScaleFX");
        break;
      case orxFX_EVENT_STOP:
        orxObject_AddFX(pstSenderObject, "ScaleFX");
        break;
    }
  } else if (orxString_Compare(pstPayload->zFXName, "FadeOut") == 0 &&
    _pstEvent->eID == orxFX_EVENT_STOP) {
    orxObject_SetLifeTime(pstSenderObject, 0);
  }

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL setGameState(const orxSTRING gameState) {
  orxConfig_PushSection("Game");
  orxConfig_SetString("GameState", gameState);
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

const orxSTRING orxFASTCALL getGameState() {
  orxConfig_PushSection("Game");
  const orxSTRING gameState = orxConfig_GetString("GameState");
  orxConfig_PopSection();

  return gameState;
}

orxSTATUS orxFASTCALL setGamePaused(orxBOOL gamePaused) {
  orxConfig_PushSection("Game");
  orxConfig_SetBool("GamePaused", gamePaused);
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

orxBOOL orxFASTCALL getGamePaused() {
  orxConfig_PushSection("Game");
  orxBOOL gamePaused = orxConfig_GetBool("GamePaused");
  orxConfig_PopSection();

  return gamePaused;
}

orxSTATUS orxFASTCALL PhysicsEventHandler(const orxEVENT *_pstEvent) {
  if (_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_ADD) {
    orxOBJECT *pstRecipientObject, *pstSenderObject;
    const orxSTRING senderSectionName;
    orxBOOL senderHasJumpSContact;
    const orxSTRING recipientSectionName;
    orxBOOL recipientHasJumpSContact;

    pstSenderObject = orxOBJECT(_pstEvent->hSender);
    pstRecipientObject = orxOBJECT(_pstEvent->hRecipient);

    senderSectionName = orxObject_GetName(pstSenderObject);
    orxConfig_PushSection(senderSectionName);
    senderHasJumpSContact = orxConfig_HasValue("JumpSContact");
    orxConfig_PopSection();

    recipientSectionName = orxObject_GetName(pstRecipientObject);
    orxConfig_PushSection(recipientSectionName);
    recipientHasJumpSContact = orxConfig_HasValue("JumpSContact");
    orxConfig_PopSection();

    if (senderHasJumpSContact || recipientHasJumpSContact){
      orxPHYSICS_EVENT_PAYLOAD* payload = (orxPHYSICS_EVENT_PAYLOAD*)_pstEvent->pstPayload;
      if (orxString_Compare(payload->zSenderPartName, "JumpSensorPart") == 0) {
        //const orxSTRING sectionName = orxObject_GetName(pstSenderObject);
        orxConfig_PushSection(senderSectionName);
        orxConfig_SetU32("JumpSContact", orxConfig_GetU32("JumpSContact")+1);
        orxConfig_PopSection();

        player_setJumpTime(senderSectionName, orxFLOAT_0);
        //orxLOG("%s : %f", senderSectionName, jumpSpeed.fY);
        player_clearJumpSpeed(senderSectionName);
        
        orxObject_Attach(pstSenderObject, pstRecipientObject);
        orxLOG("%s", orxObject_GetName(pstRecipientObject));
      } else if (orxString_Compare(payload->zRecipientPartName, "JumpSensorPart") == 0) {
        //const orxSTRING sectionName = orxObject_GetName(pstRecipientObject);
        orxConfig_PushSection(recipientSectionName);
        orxConfig_SetU32("JumpSContact", orxConfig_GetU32("JumpSContact")+1);
        orxConfig_PopSection();

        player_setJumpTime(recipientSectionName, orxFLOAT_0);
        player_clearJumpSpeed(recipientSectionName);
        orxObject_Attach(pstRecipientObject, pstSenderObject);
        orxLOG("%s", orxObject_GetName(pstRecipientObject));
      }

      //const orxSTRING senderObjectName = orxObject_GetName(pstSenderObject);
      //const orxSTRING recipientObjectName = orxObject_GetName(pstRecipientObject);

      if (orxString_Compare(payload->zSenderPartName, "LeftBulletSensorPart") == 0
            && orxString_Compare(recipientSectionName, "InkBulletObject") == 0 ||
          orxString_Compare(payload->zRecipientPartName, "LeftBulletSensorPart") == 0
            && orxString_Compare(senderSectionName, "InkBulletObject") == 0) {
        if (orxString_Compare(recipientSectionName, "InkBulletObject") == 0) {
          player_gotHit(senderSectionName);
          orxObject_SetLifeTime(pstRecipientObject, 0);
        } else {
          player_gotHit(recipientSectionName);
          orxObject_SetLifeTime(pstSenderObject, 0);
        }
      }

      if (orxString_Compare(payload->zSenderPartName, "RightBulletSensorPart") == 0
            && orxString_Compare(recipientSectionName, "InkBulletObject") == 0 ||
          orxString_Compare(payload->zRecipientPartName, "RightBulletSensorPart") == 0
            && orxString_Compare(senderSectionName, "InkBulletObject") == 0) {
        if (orxString_Compare(recipientSectionName, "InkBulletObject") == 0) {
          player_gotHit(senderSectionName);
          orxObject_SetLifeTime(pstRecipientObject, 0);
        } else if (orxString_Compare(senderSectionName, "InkBulletObject") == 0) {
          player_gotHit(recipientSectionName);
          orxObject_SetLifeTime(pstSenderObject, 0);
        }
      }

      // Contact with an enemy bodypart
      if (orxString_Compare(recipientSectionName, "Spike1Object") == 0) {
        player_gotHit(senderSectionName);
      } else if (orxString_Compare(senderSectionName, "Spike1Object") == 0) {
        player_gotHit(recipientSectionName);
      }
    }
  } else if (_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_REMOVE) {
    orxOBJECT *pstRecipientObject, *pstSenderObject;
    const orxSTRING senderSectionName;
    orxBOOL senderHasJumpSContact;
    const orxSTRING recipientSectionName;
    orxBOOL recipientHasJumpSContact;

    pstSenderObject = orxOBJECT(_pstEvent->hSender);

    pstRecipientObject = orxOBJECT(_pstEvent->hRecipient);

    senderSectionName = orxObject_GetName(pstSenderObject);
    orxConfig_PushSection(senderSectionName);
    senderHasJumpSContact = orxConfig_HasValue("JumpSContact");
    orxConfig_PopSection();

    recipientSectionName = orxObject_GetName(pstRecipientObject);
    orxConfig_PushSection(recipientSectionName);
    recipientHasJumpSContact = orxConfig_HasValue("JumpSContact");
    orxConfig_PopSection();

    if (senderHasJumpSContact || recipientHasJumpSContact){
      orxPHYSICS_EVENT_PAYLOAD* payload = (orxPHYSICS_EVENT_PAYLOAD*)_pstEvent->pstPayload;
      if (orxString_Compare(payload->zSenderPartName, "JumpSensorPart") == 0) {
        orxConfig_PushSection(senderSectionName);
        orxConfig_SetU32("JumpSContact", orxConfig_GetU32("JumpSContact")-1);
        orxConfig_PopSection();
        orxObject_Detach(pstSenderObject);
      } else if (orxString_Compare(payload->zRecipientPartName, "JumpSensorPart") == 0) {
        orxConfig_PushSection(recipientSectionName);
        orxConfig_SetU32("JumpSContact", orxConfig_GetU32("JumpSContact")-1);
        orxConfig_PopSection();
        orxObject_Detach(pstRecipientObject);
      }
    }
  }

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL CameraFollowCharacters(orxOBJECT *camHandle) {
  if (getGamePaused() == orxTRUE) {
    return orxSTATUS_SUCCESS;
  }

  orxVECTOR cameraPosition = { 0,0,0 };
  orxObject_GetPosition(camHandle, &cameraPosition);

  orxVECTOR currentPosition;
  orxVECTOR minPosition;
  orxVECTOR middlePosition;
  orxVECTOR maxPosition;

  orxConfig_PushSection("Game");
  orxS32 count = orxConfig_GetListCount("InputList");
  orxConfig_PopSection();

  for (orxS32 i = 0 ; i < count ; i++) {
    orxConfig_PushSection("Game");
    const orxSTRING currentInput = orxConfig_GetListString("InputList", i);
    orxConfig_PopSection();

    orxConfig_PushSection(currentInput);
    const orxSTRING currentPlayer = orxConfig_GetString("PlayerObject");
    orxConfig_PopSection();

    orxOBJECT *playerObject = RetrieveObjectFromConfig(currentPlayer);
    if (playerObject == NULL) {
      return orxSTATUS_FAILURE;
    }
    orxObject_GetWorldPosition(playerObject, &currentPosition);

    if (i == 0) {
      minPosition = currentPosition;
      maxPosition = currentPosition;
    } else {
      if (currentPosition.fX < minPosition.fX) {
        minPosition.fX = currentPosition.fX;
      } else if (currentPosition.fX > maxPosition.fX) {
        maxPosition.fX = currentPosition.fX;
      }

      if (currentPosition.fY < minPosition.fY) {
        minPosition.fY = currentPosition.fY;
      } else if (currentPosition.fY > maxPosition.fY) {
        maxPosition.fY = currentPosition.fY;
      }
    }
  }

  middlePosition.fX = minPosition.fX+(maxPosition.fX-minPosition.fX)/2;
  middlePosition.fY = minPosition.fY+(maxPosition.fY-minPosition.fY)/2;

  orxVECTOR camMaxSpeed;
  orxConfig_PushSection("Camera");
  orxConfig_GetVector("MaxSpeed", &camMaxSpeed);
  orxConfig_PopSection();
  
  float maxXSpeed = camMaxSpeed.fX;

  if (floor(middlePosition.fX-cameraPosition.fX) > 0) {
    // Calculate percentage of difference
    float perc = fabs(floor(middlePosition.fX)-cameraPosition.fX)/maxCamDistX;
    cameraPosition.fX += perc*maxXSpeed;
  } else if (floor(cameraPosition.fX-middlePosition.fX) > 0) {
    float perc = fabs(floor(middlePosition.fX)-cameraPosition.fX)/maxCamDistX;
    cameraPosition.fX -= perc*maxXSpeed;
  }

  float maxYSpeed = camMaxSpeed.fX;

  if (floor(middlePosition.fY-cameraPosition.fY) > 0) {
    // Calculate percentage of difference
    float perc = fabs(floor(middlePosition.fY)-cameraPosition.fY)/maxCamDistY;
    cameraPosition.fY += perc*maxYSpeed;
  } else if (floor(cameraPosition.fY-middlePosition.fY) > 0) {
    float perc = fabs(floor(middlePosition.fY)-cameraPosition.fY)/maxCamDistY;
    cameraPosition.fY -= perc*maxYSpeed;
  }

  if (cameraPosition.fX-VPWidth/2 < levelLimitsTL.fX) {
    if (levelLimitsBR.fX-levelLimitsTL.fX < VPWidth) {
      cameraPosition.fX = (levelLimitsBR.fX-levelLimitsTL.fX)/2;
    } else {
      cameraPosition.fX = levelLimitsTL.fX+VPWidth/2;
    }
  } else if (cameraPosition.fX+VPWidth/2 > levelLimitsBR.fX) {
    if (levelLimitsBR.fX-levelLimitsTL.fX < VPWidth) {
      cameraPosition.fX = (levelLimitsBR.fX-levelLimitsTL.fX)/2;
    } else {
      cameraPosition.fX = levelLimitsBR.fX-VPWidth/2;
    }
  }

  if (cameraPosition.fY-VPHeight/2 < levelLimitsTL.fY) {
    if (levelLimitsBR.fY-levelLimitsTL.fY < VPHeight) {
      cameraPosition.fY = (levelLimitsBR.fY-levelLimitsTL.fY)/2;
    } else {
      cameraPosition.fY = levelLimitsTL.fY+VPHeight/2;
    }
  } else if (cameraPosition.fY+VPHeight/2 > levelLimitsBR.fY) {
    if (levelLimitsBR.fY-levelLimitsTL.fY < VPHeight) {
      cameraPosition.fY = (levelLimitsBR.fY-levelLimitsTL.fY)/2;
    } else {
      cameraPosition.fY = levelLimitsBR.fY-VPHeight/2;
    }
  }

  orxObject_SetPosition(camHandle, &cameraPosition);

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL Init() {
  // Textures preloading
  if (orxObject_CreateFromConfig("Doves130Preloading") &&
  orxObject_CreateFromConfig("Doves40Preloading") &&
  orxObject_CreateFromConfig("Doves30Preloading") &&
  orxObject_CreateFromConfig("TitleBackgroundPreloading")) {

    // Registering Vector.AreEqual command
    const orxCOMMAND_VAR_DEF paramsList1[2] = {{"vector1", orxCOMMAND_VAR_TYPE_VECTOR}, {"vector2", orxCOMMAND_VAR_TYPE_VECTOR}};
    const orxCOMMAND_VAR_DEF ret1 = {"ret", orxCOMMAND_VAR_TYPE_BOOL};
    
    //Registering Object.MoveBetween command
    const orxCOMMAND_VAR_DEF paramsList2[4] = {{"object", orxCOMMAND_VAR_TYPE_U64}, {"from", orxCOMMAND_VAR_TYPE_VECTOR}, {"to", orxCOMMAND_VAR_TYPE_VECTOR}, {"time", orxCOMMAND_VAR_TYPE_FLOAT}};
    const orxCOMMAND_VAR_DEF ret2 = {"ret", orxCOMMAND_VAR_TYPE_U64};
    orxCommand_Register("Object_MoveBetween", &command_MoveBetween, 4, 0, paramsList2, &ret2);

    time_t t;
    srand((unsigned) time(&t));

    orxConfig_PushSection("Powered");
    orxCHAR buffer[255];
    const orxSTRING randWord = orxConfig_GetString("RandWord");
    orxString_NPrint(buffer, sizeof(buffer) - 1, "Made with %s with ORX", randWord);
    orxConfig_SetString("String", buffer);
    //camera = orxCamera_Get("Camera");
    orxConfig_PopSection();

    setGameState("title");
    //orxClock_Register(orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE), Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_LOWER);

    orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

    // Creates viewport
    pstViewport = orxViewport_CreateFromConfig("Viewport");
    orxSTRUCTURE_ASSERT(pstViewport);
    orxViewport_GetSize(pstViewport, &VPWidth, &VPHeight);

    //orxConfig_PushSection("Runtime");
    orxCAMERA *camera = orxCamera_Get("Camera");
    camHandle = orxObject_CreateFromConfig("CamHandleObject");
    orxCamera_SetParent(camera, camHandle);
    //orxConfig_PopSection();

    orxConfig_PushSection("Game");
    for (orxS32 i = 0, count = orxConfig_GetListCount("AvailableInputs") ; i < count ; i++) {
      orxInput_EnableSet(orxConfig_GetListString("AvailableInputs", i), orxTRUE);
    }
    orxConfig_PopSection();

    title = orxObject_CreateFromConfig("Title");

    // Done!
    return orxSTATUS_SUCCESS;
  }
}

orxSTATUS orxFASTCALL Run() {
  // Done!
  return orxSTATUS_SUCCESS;
}

void orxFASTCALL Exit() {
  // We could delete everything we created here but orx will do it for us anyway as long as we didn't do any direct memory allocations =)
}

orxSTATUS orxFASTCALL Bootstrap() {
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Adds config resource storages
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../data/config", orxFALSE);
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../../data/config", orxFALSE);
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../../../data/config", orxFALSE);

  // Done!
  return eResult;
}

int main(int argc, char **argv) {
  // Sets config bootstrap
  orxConfig_SetBootstrap(Bootstrap);

  // Executes orx
  orx_Execute(argc, argv, Init, Run, Exit);

  // Done!
  return EXIT_SUCCESS;
}
