#include "orx.h"

#define MLISTKEY "List"
#define MSIZEKEY "Size"
#define PAUSEMENUSECTION "PauseMenu"
#define SELECTSOUNDSECTION "SelectSound"

orxSTATUS orxFASTCALL pause_displayMenu(const orxSTRING menuSection);
orxSTATUS orxFASTCALL pause_displayConfirm(const orxSTRING menuItemSection, const orxSTRING menuSection);
orxSTATUS orxFASTCALL pause_SelectMenu(const orxSTRING selectorSection,
  orxVECTOR coordinates, orxBOOL smoothMove, const orxSTRING menuSection);
orxSTATUS orxFASTCALL pause_addMenuSelector(const orxU32 selectorNumber, const orxSTRING inputSection);
orxVECTOR orxFASTCALL pause_GetFirstUnselectedElement(const orxSTRING menuSection);
orxVECTOR orxFASTCALL pause_GetMenuPosition(const orxSTRING menuSection);
orxSTATUS orxFASTCALL pause_hideConfirm(const orxSTRING confirmSection, const orxSTRING menuSection);
orxVECTOR orxFASTCALL pause_getMenuSize(const orxSTRING menuSection);
