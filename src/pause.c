#include "pause.h"
#include "utility.h"
#include "playersSelection.h"

orxSTATUS orxFASTCALL pause_hideConfirm(const orxSTRING confirmSection, const orxSTRING menuSection) {
  orxOBJECT *confirm = RetrieveObjectFromConfig(confirmSection);

  for(orxOBJECT *child = orxOBJECT(orxObject_GetChild(confirm));
      child != orxNULL; child = orxOBJECT(orxObject_GetSibling(child))) {
    orxObject_SetLifeTime(child, 0);
  }

  orxObject_Enable(confirm, orxFALSE);

  orxConfig_PushSection(menuSection);
  const orxSTRING selectorSection = orxConfig_GetString("Selector");
  orxConfig_PopSection();

  orxVECTOR elemToSelect = pause_GetFirstUnselectedElement(menuSection);
  pause_SelectMenu(selectorSection, elemToSelect, orxFALSE, menuSection);

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL pause_displayConfirm(const orxSTRING menuItemSection, const orxSTRING menuSection) {
  const orxSTRING confirmList;
  const orxSTRING selectorSection;
  orxVECTOR listSize, listGap, listPosition;
  orxConfig_PushSection(menuItemSection);
  confirmList = orxConfig_GetString("List");
  orxConfig_PopSection();

  orxOBJECT *confirmationPH = RetrieveObjectFromConfig("ConfirmationPH");
  orxObject_Enable(confirmationPH, orxTRUE);

  orxConfig_PushSection(confirmList);
  orxConfig_GetVector("Size", &listSize);
  orxConfig_GetVector("Gap", &listGap);
  orxConfig_GetVector("Position", &listPosition);

  orxVECTOR prevElementScale, prevElementSize = {0, 0, 0};

  // Looping on RowXs
  for (orxU32 j=0 ; j < orxF2U(listSize.fY) ; j++) {
    orxCHAR Buffer[16];
    orxString_NPrint(Buffer, sizeof(Buffer) - 1, "Row%u", j + 1);
    // Looping on each RowX list item
    for (orxU32 i=0 ; i < orxF2U(listSize.fX) ; i++) {
      const orxSTRING curSection = orxConfig_GetListString(Buffer, i);

      // We create the list element object
      orxOBJECT *elementObject = orxObject_CreateFromConfig(curSection);
      // We set the current scene as it's owner so that when we delete the scene
      // it will be deleted aswell
      orxObject_SetParent(elementObject, confirmationPH);
      orxObject_SetOwner(elementObject, confirmationPH);
      // And set it's position
      orxVECTOR elementPosition = {listPosition.fX+(listGap.fX+prevElementSize.fX)*i,
        listPosition.fY+(listGap.fY+prevElementSize.fY)*j, 1};
      orxObject_SetPosition(elementObject, &elementPosition);

      orxObject_GetSize(elementObject, &prevElementSize);
      orxObject_GetScale(elementObject, &prevElementScale);
      orxVector_Mul(&prevElementSize, &prevElementSize, &prevElementScale);
    }
  }
  orxConfig_PopSection();

  orxConfig_PushSection(menuSection);
  selectorSection = orxConfig_GetString("Selector");
  orxConfig_PopSection();

  orxVECTOR elemToSelect = {0, 0, 0};
  pause_SelectMenu(selectorSection, elemToSelect, orxFALSE, menuItemSection);

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL pause_displayMenu(const orxSTRING menuSection) {
  orxVECTOR clPosition, clGap, clSize;
  const orxSTRING selectorSection;
  const orxSTRING listSection;
  orxOBJECT *selectorObject, *menu, *confirmationPH;

  // We create menu object and selector object
  menu = orxObject_CreateFromConfig(menuSection);

  confirmationPH = RetrieveObjectFromConfig("ConfirmationPH");

  if (confirmationPH != NULL) {
    orxObject_Enable(confirmationPH, orxFALSE);
  }

  orxConfig_PushSection(menuSection);
  selectorSection = orxConfig_GetString("Selector");
  listSection = orxConfig_GetString("List");
  orxConfig_PopSection();

  selectorObject = orxObject_CreateFromConfig(selectorSection);
  orxObject_SetOwner(selectorObject, menu);

  // Pushing List section and getting some positioning and sizing info
  orxConfig_PushSection(listSection);
  orxConfig_GetVector("Size", &clSize);
  orxConfig_GetVector("Gap", &clGap);
  orxConfig_GetVector("Position", &clPosition);

  orxVECTOR prevElementScale, prevElementSize = {0, 0, 0};

  // Looping on RowXs
  for (orxU32 j=0 ; j < orxF2U(clSize.fY) ; j++) {
    orxCHAR Buffer[16];
    orxString_NPrint(Buffer, sizeof(Buffer) - 1, "Row%u", j + 1);
    // Looping on each RowX list item
    for (orxU32 i=0 ; i < orxF2U(clSize.fX) ; i++) {
      const orxSTRING curSection = orxConfig_GetListString(Buffer, i);

      // We create the list element object
      orxOBJECT *elementObject = orxObject_CreateFromConfig(curSection);
      // We set the current scene as it's owner so that when we delete the scene
      // it will be deleted aswell
      orxObject_SetOwner(elementObject, menu);
      // And set it's position
      orxVECTOR elementPosition = {clPosition.fX+(clGap.fX+prevElementSize.fX)*i,
        clPosition.fY+(clGap.fY+prevElementSize.fY)*j, 1};
      orxObject_SetPosition(elementObject, &elementPosition);

      orxObject_GetSize(elementObject, &prevElementSize);
      orxObject_GetScale(elementObject, &prevElementScale);
      orxVector_Mul(&prevElementSize, &prevElementSize, &prevElementScale);
    }
  }
  orxConfig_PopSection();

  // We select the first unselected character
  orxVECTOR elemToSelect = pause_GetFirstUnselectedElement(menuSection);
  pause_SelectMenu(selectorSection, elemToSelect, orxFALSE, menuSection);

  orxObject_AddFXRecursive(menu, "FadeIn", 0.0f);

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL pause_SelectMenu(const orxSTRING selectorSection,
  orxVECTOR coordinates, orxBOOL smoothMove, const orxSTRING menuSection) {
  orxCHAR rowKey[16];
  orxVECTOR menuItemPos, selectorPos, listSize;
  const orxSTRING listSection;
  orxString_NPrint(rowKey, sizeof(rowKey) - 1, "Row%u", orxF2U(coordinates.fY)+1);

  orxConfig_PushSection(menuSection);
  listSection = orxConfig_GetString("List");
  orxConfig_PopSection();

  orxConfig_PushSection(listSection);
  // Errors handling.
  orxConfig_GetVector("Size", &listSize);
  orxS32 itemsCount = orxConfig_GetListCount(rowKey);
  orxConfig_PopSection();

  if (coordinates.fX < 0) {
    coordinates.fX = itemsCount-1;
    if (pause_SelectMenu(selectorSection, coordinates, smoothMove, menuSection) == orxSTATUS_FAILURE) {
      coordinates.fX--;
      while (pause_SelectMenu(selectorSection, coordinates, smoothMove, menuSection) == orxSTATUS_FAILURE) {
        coordinates.fX--;
      }
      return orxSTATUS_SUCCESS;
    } else {
      return orxSTATUS_SUCCESS;
    }
  } else if (coordinates.fY < 0) {
    coordinates.fY = orxF2U(listSize.fY)-1;
    if (pause_SelectMenu(selectorSection, coordinates, smoothMove, menuSection) == orxSTATUS_FAILURE) {
      coordinates.fY--;
      while (pause_SelectMenu(selectorSection, coordinates, smoothMove, menuSection) == orxSTATUS_FAILURE) {
        coordinates.fY--;
      }
      return orxSTATUS_SUCCESS;
    } else {
      return orxSTATUS_SUCCESS;
    }
  } else if (orxF2U(coordinates.fY)+1 > orxF2U(listSize.fY)) {
    coordinates.fY = 0;
    if (pause_SelectMenu(selectorSection, coordinates, smoothMove, menuSection) == orxSTATUS_FAILURE) {
      coordinates.fY++;
      while (pause_SelectMenu(selectorSection, coordinates, smoothMove, menuSection) == orxSTATUS_FAILURE) {
        coordinates.fY++;
      }
      return orxSTATUS_SUCCESS;
    } else {
      return orxSTATUS_SUCCESS;
    }
  } else if (orxF2U(coordinates.fX) > itemsCount-1) {
    coordinates.fX = 0;
    if (pause_SelectMenu(selectorSection, coordinates, smoothMove, menuSection) == orxSTATUS_FAILURE) {
      coordinates.fX++;
      while (pause_SelectMenu(selectorSection, coordinates, smoothMove, menuSection) == orxSTATUS_FAILURE) {
        coordinates.fX++;
      }
      return orxSTATUS_SUCCESS;
    } else {
      return orxSTATUS_SUCCESS;
    }
  }

  orxConfig_PushSection(listSection);
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
  orxOBJECT *menuItem = RetrieveObjectFromConfig(itemToSelect);
  // Get its position
  orxObject_GetWorldPosition(menuItem, &menuItemPos);
  orxOBJECT *menuSelector = RetrieveObjectFromConfig(selectorSection);
  // Let's do the magic
  orxVECTOR menuItemSize, chScale, chRes;
  orxObject_GetSize(menuItem, &menuItemSize);
  orxVECTOR offset = {5, 5, 5};
  orxVector_Add(&menuItemSize, &menuItemSize, &offset);
  orxVECTOR endScale;
  orxVector_Add(&endScale, &menuItemSize, &offset);

  orxConfig_PushSection("ScaleFX");
  orxConfig_SetVector("StartValue", &menuItemSize);
  orxConfig_SetVector("EndValue", &endScale);
  orxConfig_PopSection();

  if (smoothMove == orxTRUE) {
    orxVECTOR selectorSize;
    orxObject_GetScale(menuSelector, &selectorSize);
    orxConfig_PushSection("ScaleTransFX");
    orxConfig_SetVector("StartValue", &selectorSize);
    orxConfig_SetVector("EndValue", &menuItemSize);
    orxConfig_PopSection();

    orxObject_GetPosition(menuSelector, &selectorPos);
    orxConfig_PushSection("SelectorPosFX");
    orxConfig_SetVector("StartValue", &selectorPos);
    orxConfig_SetVector("EndValue", &menuItemPos);
    orxConfig_PopSection();
    orxObject_AddUniqueFX(menuSelector, "ScaleTransFX");
    orxObject_AddUniqueFX(menuSelector, "SelectorPosFX");

    //orxConfig_PopSection();
  } else {
    orxObject_RemoveFX(menuSelector, "ScaleFX");
    orxObject_SetPosition(menuSelector, &menuItemPos);
    orxObject_AddFX(menuSelector, "ScaleFX");
  }

  // We finally set the position in SelectorObject section
  orxConfig_PushSection(selectorSection);
  orxConfig_SetVector("ListPosition", &coordinates);
  // And the Selected key to keep track of the selected character
  orxConfig_SetString("Selected", orxObject_GetName(menuItem));
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

orxVECTOR orxFASTCALL pause_GetMenuPosition(const orxSTRING menuSection) {
  const orxSTRING selectorSection;
  orxVECTOR selectorMenuPosition;

  orxConfig_PushSection(menuSection);
  selectorSection = orxConfig_GetString("Selector");
  orxConfig_PopSection();

  orxConfig_PushSection(selectorSection);
  orxConfig_GetVector("ListPosition", &selectorMenuPosition);
  orxConfig_PopSection();

  return selectorMenuPosition;
}

orxSTATUS orxFASTCALL pause_addMenuSelector(const orxU32 selectorNumber, const orxSTRING inputSection) {
  if (selectorNumber < 1 || selectorNumber > 4) {
    return orxSTATUS_FAILURE;
  }

  orxCHAR selectorSection[32];
  orxString_NPrint(selectorSection, sizeof(selectorSection) - 1, "SelectorObject%u", selectorNumber);
  orxOBJECT *selTemp = RetrieveObjectFromConfig(selectorSection);
  if (selTemp != NULL) {
    orxObject_Delete(selTemp);
  }

  orxOBJECT *selectorObject = orxObject_CreateFromConfig(selectorSection);
  orxObject_SetOwner(selectorObject, RetrieveObjectFromConfig("PauseMenu"));

  orxConfig_PushSection(selectorSection);
  orxConfig_SetBool("Moving", orxFALSE);
  orxConfig_SetBool("Locked", orxFALSE);
  orxConfig_PopSection();

  orxConfig_PushSection("PauseMenu");
  orxConfig_SetString("SelectorObject", selectorSection);
  orxConfig_PopSection();

  return orxSTATUS_SUCCESS;
}

orxVECTOR orxFASTCALL pause_GetFirstUnselectedElement(const orxSTRING menuSection) {
  orxVECTOR result, clSize;
  const orxSTRING listSection;
  orxBOOL singleSelector = orxFALSE;

  orxConfig_PushSection(menuSection);
  listSection = orxConfig_GetString("List");
  if (orxConfig_GetBool("SingleSelector") == orxTRUE) {
    singleSelector = orxTRUE;
  }
  orxConfig_PopSection();

  orxConfig_PushSection(listSection);

  if (singleSelector == orxFALSE) {
    orxConfig_GetVector("Size", &clSize);

    // Looping on RowXs
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
  } else {
    result.fX = 0;
    result.fY = 0;
  }
  orxConfig_PopSection();

  return result;
}

orxVECTOR orxFASTCALL pause_getMenuSize(const orxSTRING menuSection) {
  orxConfig_PushSection(menuSection);
  const orxSTRING menuListSection = orxConfig_GetString(MLISTKEY);
  orxConfig_PopSection();

  orxConfig_PushSection(menuListSection);
  orxVECTOR size;
  orxConfig_GetVector(MSIZEKEY, &size);
  orxConfig_PopSection();

  return size;
}
