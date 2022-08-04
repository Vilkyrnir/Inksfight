#include "physics.h"

orxSTATUS orxFASTCALL physics_enable(const orxBOOL enable) {
  orxConfig_PushSection(PHYSICSSECTION);
  orxConfig_SetBool(ENABLEPHYSICSKEY, enable);
  orxConfig_PopSection();
  
  return orxSTATUS_SUCCESS;
}

orxBOOL orxFASTCALL physics_isEnabled() {
  orxConfig_PushSection(PHYSICSSECTION);
  orxBOOL result = orxConfig_GetBool(ENABLEPHYSICSKEY);
  orxConfig_PopSection();
  
  return result;
}

orxVECTOR orxFASTCALL physics_getGravity() {
  orxVECTOR gravity;
  orxConfig_PushSection(PHYSICSSECTION);
  orxConfig_GetVector(GRAVITYKEY, &gravity);
  orxConfig_PopSection();
  
  return gravity;
}

orxU32 orxFASTCALL physics_getMaxFallVelocity() {
  orxU32 maxFallVelocity;
  orxConfig_PushSection(PHYSICSSECTION);
  maxFallVelocity = orxConfig_GetU32(MAXFALLVKEY);
  orxConfig_PopSection();
  
  return maxFallVelocity;
}
