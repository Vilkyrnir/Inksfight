#include "orx.h"

extern orxVECTOR levelLimitsBR, levelLimitsTL;
extern orxS32 platformCFlag, dPlatformCFlag;

orxSTATUS orxFASTCALL level_Displaylayer(const orxSTRING sName, const orxS16 zIndex, const orxSTRING tileSetSection);
orxSTATUS orxFASTCALL level_GenerateLevelsSet();
orxSTATUS orxFASTCALL level_GenerateNextLevel();
orxSTATUS orxFASTCALL level_destroyCurrentLevel();
orxBOOL orxFASTCALL level_ClearConfigCallback(const orxSTRING _zSectionName, const orxSTRING _zKeyName);
