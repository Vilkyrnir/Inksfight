#define PLATFORMSID "platforms"
#define DPLATFORMSID "dPlatforms"

extern orxOBJECT *playersSelection, *characterSelector;

orxSTATUS orxFASTCALL ps_AddCharacterSelector(const orxU32 selectorNumber, const orxSTRING inputSection);
orxSTATUS orxFASTCALL ps_DisplayCharactersList();
orxSTATUS orxFASTCALL ps_SelectCharacter(const orxSTRING selectorSection,
  orxOBJECT* characterSelector, orxVECTOR coordinates, orxBOOL smoothMove);
orxSTATUS orxFASTCALL ps_ValidateSelection(const orxSTRING inputSection);
orxSTATUS orxFASTCALL ps_UndoSelection(const orxSTRING inputSection);
orxVECTOR orxFASTCALL ps_GetCharactersListPosition(const orxSTRING selectorSection);
orxVECTOR orxFASTCALL ps_GetFirstUnselectedChar();
orxSTATUS orxFASTCALL ps_CheckValidatedSelection(const orxSTRING input, orxFLOAT fDT);
orxSTATUS orxFASTCALL ps_ControlSelector(const orxSTRING input, const orxFLOAT fDT);
orxSTATUS orxFASTCALL ps_AddNewPlayer(const orxSTRING input, const orxFLOAT fDT);
