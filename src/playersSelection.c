#include "orx.h"
#include "playersSelection.h"
#include "utility.h"
#include "InksFight.h"
#include "level.h"
#include "player.h"

orxOBJECT *playersSelection, *characterSelector;

orxSTATUS orxFASTCALL ps_AddCharacterSelector(const orxU32 selectorNumber, const orxSTRING inputSection) {
  if (selectorNumber < 1 || selectorNumber > 4) {
    return orxSTATUS_FAILURE;
  }

  orxCHAR selectorSection[32];
  orxString_NPrint(selectorSection, sizeof(selectorSection) - 1, "SelectorObject%u", selectorNumber);
  orxOBJECT *selTemp = RetrieveObjectFromConfig(selectorSection);
  if (selTemp != NULL) {
    return orxSTATUS_FAILURE;
  }

  orxOBJECT *selectorObject = orxObject_CreateFromConfig(selectorSection);
  orxObject_SetOwner(selectorObject, playersSelection);

  orxCHAR playerSection[16];
  orxString_NPrint(playerSection, sizeof(playerSection) - 1, "Player%u", selectorNumber);
  orxConfig_PushSection(playerSection);
  //orxConfig_SetString("SelectorObject", selectorSection);
  orxConfig_SetString("InputSet", inputSection);
  orxConfig_PopSection();

  // We link current input and player
  orxConfig_PushSection(inputSection);
  orxConfig_SetString("PlayerObject", playerSection);
  orxConfig_SetString("SelectorObject", selectorSection);
  orxConfig_PopSection();

  orxConfig_PushSection(selectorSection);
  orxConfig_SetBool("Moving", orxFALSE);
  orxConfig_SetBool("Locked", orxFALSE);
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL ps_DisplayCharactersList() {
  orxVECTOR clPosition, clGap, clSize;

  // We create PlayerSelection object and retrieve characterSelector object
  playersSelection = orxObject_CreateFromConfig("PlayersSelection");

  orxOBJECT *charSelector = RetrieveObjectFromConfig("SelectorObject1");

  // Pushing CharactersList section and getting some positioning and sizing info
  orxConfig_PushSection("CharactersList");
  orxConfig_GetVector("Size", &clSize);
  orxConfig_GetVector("Gap", &clGap);
  orxConfig_GetVector("Position", &clPosition);

  orxVECTOR prevElementScale, prevElementSize = {0, 0, 0};

  // Looping on CharactersList RowXs
  for (orxU32 j=0 ; j < orxF2U(clSize.fY) ; j++) {
    orxCHAR Buffer[16];
    orxString_NPrint(Buffer, sizeof(Buffer) - 1, "Row%u", j + 1);
    // Looping on each RowX list item
    for (orxU32 i=0 ; i < orxF2U(clSize.fX) ; i++) {
      const orxSTRING curSection = orxConfig_GetListString(Buffer, i);

      // We create the character object
      orxOBJECT *characterObject = orxObject_CreateFromConfig(curSection);
      // We set the current scene as it's owner so that when we delete the scene
      // it will be deleted aswell
      orxObject_SetOwner(characterObject, playersSelection);
      // And set it's position
      orxVECTOR characterPosition = {clPosition.fX+(clGap.fX+prevElementSize.fX)*i,
        clPosition.fY+(clGap.fY+prevElementSize.fY)*j, 1};
      orxObject_SetPosition(characterObject, &characterPosition);

      orxObject_GetSize(characterObject, &prevElementSize);
      orxObject_GetScale(characterObject, &prevElementScale);
      orxVector_Mul(&prevElementSize, &prevElementSize, &prevElementScale);
    }
  }
  orxConfig_PopSection();

  // We select the first unselected character
  orxVECTOR charToSelect = ps_GetFirstUnselectedChar();
  ps_SelectCharacter("SelectorObject1", charSelector, charToSelect, orxFALSE);

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL ps_SelectCharacter(const orxSTRING selectorSection,
  orxOBJECT* characterSelector, orxVECTOR coordinates, orxBOOL smoothMove) {
  orxCHAR rowKey[16];
  orxVECTOR characterPos, selectorPos, listSize;
  orxString_NPrint(rowKey, sizeof(rowKey) - 1, "Row%u", orxF2U(coordinates.fY)+1);

  orxConfig_PushSection("CharactersList");


  // Errors handling.
  orxConfig_GetVector("Size", &listSize);
  orxS32 itemsCount = orxConfig_GetListCount(rowKey);
  if (coordinates.fX < 0) {
    coordinates.fX = itemsCount-1;
    if (ps_SelectCharacter(selectorSection, characterSelector, coordinates, smoothMove) == orxSTATUS_FAILURE) {
      coordinates.fX--;
      while (ps_SelectCharacter(selectorSection, characterSelector, coordinates, smoothMove) == orxSTATUS_FAILURE) {
        coordinates.fX--;
      }
      return orxSTATUS_SUCCESS;
    } else {
      return orxSTATUS_SUCCESS;
    }
  } else if (coordinates.fY < 0) {
    coordinates.fY = orxF2U(listSize.fY)-1;
    if (ps_SelectCharacter(selectorSection, characterSelector, coordinates, smoothMove) == orxSTATUS_FAILURE) {
      coordinates.fY--;
      while (ps_SelectCharacter(selectorSection, characterSelector, coordinates, smoothMove) == orxSTATUS_FAILURE) {
        coordinates.fY--;
      }
      return orxSTATUS_SUCCESS;
    } else {
      return orxSTATUS_SUCCESS;
    }
  } else if (orxF2U(coordinates.fY)+1 > orxF2U(listSize.fY)) {
    coordinates.fY = 0;
    if (ps_SelectCharacter(selectorSection, characterSelector, coordinates, smoothMove) == orxSTATUS_FAILURE) {
      coordinates.fY++;
      while (ps_SelectCharacter(selectorSection, characterSelector, coordinates, smoothMove) == orxSTATUS_FAILURE) {
        coordinates.fY++;
      }
      return orxSTATUS_SUCCESS;
    } else {
      return orxSTATUS_SUCCESS;
    }
  } else if (orxF2U(coordinates.fX) > itemsCount-1) {
    coordinates.fX = 0;
    if (ps_SelectCharacter(selectorSection, characterSelector, coordinates, smoothMove) == orxSTATUS_FAILURE) {
      coordinates.fX++;
      while (ps_SelectCharacter(selectorSection, characterSelector, coordinates, smoothMove) == orxSTATUS_FAILURE) {
        coordinates.fX++;
      }
      return orxSTATUS_SUCCESS;
    } else {
      return orxSTATUS_SUCCESS;
    }
  }

  // We get the RowX item according to the given coordinates
  const orxSTRING itemToSelect = orxConfig_GetListString(rowKey, coordinates.fX);
  orxConfig_PopSection();

  orxConfig_PushSection("Game");
  orxS32 count = orxConfig_GetListCount("InputList");
  orxConfig_PopSection();
  // We check that the character hasn't been already selected
  for (orxS32 i = 0 ; i < count ; i++) {
    orxConfig_PushSection("Game");
    const orxSTRING input = orxConfig_GetListString("InputList", i);
    orxConfig_PopSection();

    orxConfig_PushSection(input);
    const orxSTRING inputSelector = orxConfig_GetString("SelectorObject");
    orxConfig_PopSection();

    if (orxString_Compare(selectorSection, inputSelector) != 0) {
      orxConfig_PushSection(inputSelector);
      const orxBOOL locked = orxConfig_GetBool("Locked");
      const orxSTRING charName = orxConfig_GetString("Selected");
      orxConfig_PopSection();
      if (locked == orxTRUE && orxString_Compare(charName, itemToSelect) == 0) {
        return orxSTATUS_FAILURE;
      }

    }
  }

  // We retrieve the corresponding object
  orxOBJECT *character = RetrieveObjectFromConfig(itemToSelect);
  // Get its position
  orxObject_GetPosition(character, &characterPos);
  // Let's do the magic

  orxVECTOR chSize, chScale, endScale;
  orxObject_GetSize(character, &chSize);
  orxObject_GetScale(character, &chScale);
  orxVector_Mul(&chSize, &chSize, &chScale);
  orxConfig_PushSection("SelectorObject");
  orxVECTOR offset;
  orxConfig_GetVector("Offset", &offset);
  orxConfig_PopSection();
  orxVector_Add(&chSize, &chSize, &offset);
  orxVector_Add(&endScale, &chSize, &offset);

  orxConfig_PushSection("SelectorValidateFX");
  orxConfig_SetVector("StartValue", &endScale);
  orxConfig_SetVector("EndValue", &chSize);
  orxConfig_PopSection();

  orxConfig_PushSection("ScaleFX");
  orxConfig_SetVector("StartValue", &chSize);
  orxConfig_SetVector("EndValue", &endScale);
  orxConfig_PopSection();
  orxObject_RemoveFX(characterSelector, "ScaleFX");
  orxObject_AddFX(characterSelector, "ScaleFX");

  if (smoothMove == orxTRUE) {
    orxObject_GetPosition(characterSelector, &selectorPos);
    orxConfig_PushSection("SelectorPosFX");
    orxConfig_SetVector("StartValue", &selectorPos);
    orxConfig_SetVector("EndValue", &characterPos);
    orxConfig_PopSection();
    orxObject_AddUniqueFX(characterSelector, "SelectorPosFX");

    //orxConfig_PopSection();
  } else {
    orxObject_SetPosition(characterSelector, &characterPos);
  }

  // We finally set the position in SelectorObject section
  orxConfig_PushSection(selectorSection);
  orxConfig_SetVector("ListPosition", &coordinates);
  // And the Selected key to keep track of the selected character
  orxConfig_SetString("Selected", orxObject_GetName(character));
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

orxVECTOR orxFASTCALL ps_GetFirstUnselectedChar() {
  orxVECTOR result, clSize;

  orxConfig_PushSection("CharactersList");
  orxConfig_GetVector("Size", &clSize);

  // Looping on CharactersList RowXs
  for (orxU32 j=0 ; j < orxF2U(clSize.fY) ; j++) {
    orxCHAR Buffer[16];
    orxString_NPrint(Buffer, sizeof(Buffer) - 1, "Row%u", j + 1);
    // Looping on each RowX list item
    for (orxU32 i=0 ; i < orxF2U(clSize.fX) ; i++) {
      orxBOOL alreadySelected = orxFALSE;
      const orxSTRING charToCheck = orxConfig_GetListString(Buffer, i);

      orxConfig_PushSection("Game");
      orxS32 count = orxConfig_GetListCount("InputList");
      orxConfig_PopSection();
      // We check that the character hasn't been already selected
      for (orxS32 c = 0 ; c < count ; c++) {
        orxConfig_PushSection("Game");
        const orxSTRING input = orxConfig_GetListString("InputList", c);
        orxConfig_PopSection();

        orxConfig_PushSection(input);
        const orxSTRING inputSelector = orxConfig_GetString("SelectorObject");
        orxConfig_PopSection();

        orxConfig_PushSection(inputSelector);
        const orxSTRING selectedChar = orxConfig_GetString("Selected");
        orxConfig_PopSection();

        if (orxString_Compare(charToCheck, selectedChar) == 0) {
          alreadySelected = orxTRUE;
        }
      }

      if (alreadySelected == orxFALSE) {
        result.fX = i;
        result.fY = j;
        orxConfig_PopSection();
        return result;
      }
    }
  }
  orxConfig_PopSection();

  return result;
}

orxSTATUS orxFASTCALL ps_ValidateSelection(const orxSTRING inputSection) {
  orxConfig_PushSection(inputSection);
  const orxSTRING playerSection = orxConfig_GetString("PlayerObject");
  const orxSTRING selectorSection = orxConfig_GetString("SelectorObject");
  orxConfig_PopSection();

  orxConfig_PushSection(selectorSection);
  const orxSTRING charName = orxConfig_GetString("Selected");
  orxConfig_SetBool("Locked", orxTRUE);
  orxConfig_PopSection();

  orxConfig_PushSection(charName);
  const orxSTRING charAnimSet = orxConfig_GetString("AnimationSet");
  const orxSTRING charGraphic = orxConfig_GetString("Graphic");
  orxConfig_PopSection();

  orxConfig_PushSection("CharacterObject");
  orxConfig_SetString("AnimationSet", charAnimSet);
  orxConfig_SetString("Graphic", charGraphic);
  orxConfig_PopSection();

  orxOBJECT *selector = RetrieveObjectFromConfig(selectorSection);
  if (selector != NULL) {
    orxObject_RemoveFX(selector, "ScaleFX");
    orxObject_AddUniqueFX(selector, "SelectorValidateFX");
  }
}

orxSTATUS orxFASTCALL ps_UndoSelection(const orxSTRING inputSection) {
  orxConfig_PushSection(inputSection);
  const orxSTRING playerSection = orxConfig_GetString("PlayerObject");
  const orxSTRING selectorSection = orxConfig_GetString("SelectorObject");
  orxConfig_PopSection();

  orxConfig_PushSection(selectorSection);
  orxConfig_SetBool("Locked", orxFALSE);
  orxConfig_PopSection();

  orxOBJECT *selector = RetrieveObjectFromConfig(selectorSection);
  if (selector != NULL) {
    orxObject_RemoveFX(selector, "SelectorValidateFX");
    orxObject_AddUniqueFX(selector, "ScaleFX");
  }
}

orxVECTOR orxFASTCALL ps_GetCharactersListPosition(const orxSTRING selectorSection) {
  orxVECTOR selectorListPosition;
  orxConfig_PushSection(selectorSection);
  orxConfig_GetVector("ListPosition", &selectorListPosition);
  orxConfig_PopSection();

  return selectorListPosition;
}

orxBOOL orxFASTCALL ps_CheckValidatedSelection(const orxSTRING input, const orxFLOAT fDT) {
  orxConfig_PushSection(input);
  const orxSTRING selector = orxConfig_GetString("SelectorObject");
  orxConfig_PopSection();

  orxConfig_PushSection(selector);
  const orxSTRING selectedCharacter = orxConfig_GetString("Selected");
  const orxBOOL locked = orxConfig_GetBool("Locked");
  orxConfig_PopSection();

  if (orxString_Compare(selectedCharacter, "") == 0 || locked == orxFALSE) {
    return orxSTATUS_FAILURE;
  }

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL ps_ControlSelector(const orxSTRING input, const orxFLOAT fDT) {
  orxInput_SelectSet(input);

  orxConfig_PushSection(input);
  const orxSTRING selector = orxConfig_GetString("SelectorObject");
  orxConfig_PopSection();

  orxOBJECT *charSelector = RetrieveObjectFromConfig(selector);

  orxConfig_PushSection(selector);
  const orxBOOL selectorMoving = orxConfig_GetBool("Moving");
  const orxBOOL selectorLocked = orxConfig_GetBool("Locked");
  orxConfig_PopSection();

  orxConfig_PushSection("Game");
  orxS32 count = orxConfig_GetListCount("InputList");
  orxConfig_PopSection();

  //orxSTATUS (*allSelectionsValidatedPtr)(const orxSTRING) = &ps_CheckValidatedSelection;

  if (count > 1 && orxInput_HasBeenActivated("EndSelectionScreen")
  && AllPlayersCallback(fDT, "Game", "InputList", &ps_CheckValidatedSelection) == orxSTATUS_SUCCESS) {
    orxObject_Delete(playersSelection);
    playersSelection = NULL;

    // Setting up collision flags for physics
    platformCFlag = orxPhysics_GetCollisionFlagValue(PLATFORMSID);
    dPlatformCFlag = orxPhysics_GetCollisionFlagValue(DPLATFORMSID);

    level_GenerateLevelsSet();
    level_GenerateNextLevel();

    orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, PhysicsEventHandler);

    return orxSTATUS_SUCCESS;
  }

  if (selectorMoving == orxFALSE && selectorLocked == orxFALSE) {
    orxSTATUS selectedCharacter = orxSTATUS_FAILURE;
    if (orxInput_HasBeenActivated("SelectRight")) {
      orxVECTOR selectorListPosition = ps_GetCharactersListPosition(selector);
      while (selectedCharacter == orxSTATUS_FAILURE) {
        selectorListPosition.fX++;
        selectedCharacter = ps_SelectCharacter(selector, charSelector,
          selectorListPosition, orxTRUE);
      }
    } else if (orxInput_HasBeenActivated("SelectLeft")) {
      orxVECTOR selectorListPosition = ps_GetCharactersListPosition(selector);
      while (selectedCharacter == orxSTATUS_FAILURE) {
        selectorListPosition.fX--;
        selectedCharacter = ps_SelectCharacter(selector, charSelector,
          selectorListPosition, orxTRUE);
      }
    } else if (orxInput_HasBeenActivated("SelectDown")) {
      orxVECTOR selectorListPosition = ps_GetCharactersListPosition(selector);
      while (selectedCharacter == orxSTATUS_FAILURE) {
        selectorListPosition.fY++;
        selectedCharacter = ps_SelectCharacter(selector, charSelector,
          selectorListPosition, orxTRUE);
      }
    } else if (orxInput_HasBeenActivated("SelectUp")) {
      orxVECTOR selectorListPosition = ps_GetCharactersListPosition(selector);
      while (selectedCharacter == orxSTATUS_FAILURE) {
        selectorListPosition.fY--;
        selectedCharacter = ps_SelectCharacter(selector, charSelector,
          selectorListPosition, orxTRUE);
      }
    } else if (orxInput_HasBeenActivated("ConfirmSelection") && playersSelection != NULL) {
      ps_ValidateSelection(input);
    }
  } else if (orxInput_HasBeenActivated("UndoSelection")) {
    ps_UndoSelection(input);
  }

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL ps_AddNewPlayer(const orxSTRING input, const orxFLOAT fDT) {
  orxInput_SelectSet(input);
  if(orxInput_HasBeenActivated("AddPlayer")){
    for (orxS32 p = 1 ; p < 5 ; p++) {
      orxCHAR playerSection[16];
      orxString_NPrint(playerSection, sizeof(playerSection) - 1, "Player%u", p);
      orxConfig_PushSection(playerSection);
      const orxSTRING inputSet = orxConfig_GetString("InputSet");
      orxConfig_PopSection();
      if (orxString_Compare(inputSet, input) == 0) {
        break;
      } else if (orxString_Compare(inputSet, "") == 0) {
        orxConfig_PushSection(playerSection);
        orxConfig_SetString("InputSet", input);
        orxConfig_PopSection();

        // We add character selector for the new player
        ps_AddCharacterSelector(p, input);

        orxConfig_PushSection(input);
        const orxSTRING selectorSection = orxConfig_GetString("SelectorObject");
        orxConfig_PopSection();

        orxOBJECT *charSelector = RetrieveObjectFromConfig(selectorSection);

        // We select the first unselected character
        orxVECTOR charToSelect = ps_GetFirstUnselectedChar();
        ps_SelectCharacter(selectorSection, charSelector, charToSelect, orxFALSE);

        orxConfig_PushSection("Game");
        orxConfig_AppendListString("InputList", &input, 1);
        orxConfig_PopSection();

        // We just want to add 1 player, so we break here
        return orxSTATUS_SUCCESS;
      }
    }
  }
  return orxSTATUS_SUCCESS;
}
