#include "orx.h"
#include "player.h"

orxOBJECT* orxFASTCALL RetrieveObjectFromConfig(const orxSTRING objectName) {
  orxConfig_PushSection("Runtime");
  orxOBJECT *object = orxOBJECT(orxStructure_Get(orxConfig_GetU64(objectName)));
  orxConfig_PopSection();

  return object;
}

orxSTATUS orxFASTCALL AllPlayersCallback(const orxFLOAT fDT, const orxSTRING section, const orxSTRING key, orxSTATUS (*callbackFcn)(const orxSTRING, const orxFLOAT)) {
  orxConfig_PushSection(section);
  orxS32 count = orxConfig_GetListCount(key);
  orxConfig_PopSection();

  for (orxS32 i = 0 ; i < count ; i++) {
    orxConfig_PushSection(section);
    const orxSTRING currentInput = orxConfig_GetListString(key, i);
    orxConfig_PopSection();

    if (callbackFcn(currentInput, fDT) == orxSTATUS_FAILURE) {
      return orxSTATUS_FAILURE;
    }
  }

  return orxSTATUS_SUCCESS;
}
