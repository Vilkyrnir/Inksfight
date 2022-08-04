typedef orxSTATUS (*callback)(orxSTRING);

orxOBJECT* orxFASTCALL RetrieveObjectFromConfig(const orxSTRING objectName);
orxSTATUS orxFASTCALL AllPlayersCallback(const orxFLOAT fDT, const orxSTRING section, const orxSTRING key, orxSTATUS (*callbackFcn)(const orxSTRING, const orxFLOAT));

