#include "orx.h"

#define PHYSICSSECTION "Physics"
#define ENABLEPHYSICSKEY "Enable"
#define MAXFALLVKEY "MaxFallVelocity"
#define GRAVITYKEY "Gravity"

orxSTATUS orxFASTCALL physics_enable(const orxBOOL enable);
orxBOOL orxFASTCALL physics_isEnabled();
orxVECTOR orxFASTCALL physics_getGravity();
orxU32 orxFASTCALL physics_getMaxFallVelocity();
