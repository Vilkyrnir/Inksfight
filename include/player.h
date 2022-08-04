#define JUMPSOUNDSECTION "JumpSound"
#define CHARACTEROBJECTSECTION "CharacterObject"
#define JUMPSPEEDKEY "JumpS"
#define GJUMPSPEEDKEY "JumpSpeed"
#define JUMPTIMEKEY "JumpTime"
#define STATEKEY "State"
#define VELOCITYKEY "Velocity"
#define MAXJUMPTIME 0.5f
#define JUMPDECREASEFACTOR 0.7f
#define MARGIN 2.f
#define VERTICALRAYS 4
#define HORIZONTALRAYS 4

// Player possible states
#define STATEFALLING 1
#define STATEGROUNDED 2
#define STATEJUMPING 3

orxSTATUS orxFASTCALL player_controlPlayer(const orxSTRING input, const orxFLOAT fDT);
orxSTATUS orxFASTCALL player_createCharacter(const orxSTRING input, const orxFLOAT fDT);
orxVECTOR orxFASTCALL player_getLeftSpeed();
orxVECTOR orxFASTCALL player_getRightSpeed();
orxVECTOR orxFASTCALL player_getJumpSpeed(const orxSTRING playerSection, const orxBOOL strict);
orxVECTOR orxFASTCALL player_getGenericJumpSpeed();
orxSTATUS orxFASTCALL player_setJumpSpeed(const orxSTRING playerSection, const orxVECTOR* jumpSpeed);
orxSTATUS orxFASTCALL player_clearJumpSpeed(const orxSTRING playerSection);
orxFLOAT orxFASTCALL player_getJumpTime(const orxSTRING playerSection);
orxSTATUS orxFASTCALL player_setJumpTime(const orxSTRING playerSection, const orxFLOAT jumpTime);
orxVECTOR orxFASTCALL player_getFlipRight();
orxVECTOR orxFASTCALL player_getFlipLeft();
orxSTATUS orxFASTCALL player_gotHit(const orxSTRING playerSection);
orxSTATUS orxFASTCALL player_applyGravity(const orxSTRING input, orxFLOAT fDT);
orxU32 orxFASTCALL player_getState(const orxSTRING player);
orxSTATUS orxFASTCALL player_setState(const orxSTRING player, orxU32 state);
orxVECTOR orxFASTCALL player_getVelocity(const orxSTRING player);
orxSTATUS orxFASTCALL player_setVelocity(const orxSTRING player, const orxVECTOR* velocity);
orxSTATUS orxFASTCALL player_updatePosition(const orxSTRING input, orxFLOAT fDT);
orxVECTOR orxFASTCALL player_getPosition(const orxSTRING player);
orxSTATUS orxFASTCALL player_setPosition(const orxSTRING player, const orxVECTOR* position);
orxSTATUS orxFASTCALL player_checkDownwardRays(const orxSTRING input, orxFLOAT fDT);
orxSTATUS orxFASTCALL player_checkUpwardRays(const orxSTRING input, orxFLOAT fDT);
orxSTATUS orxFASTCALL player_checkLeftRays(const orxSTRING input, orxFLOAT fDT);
orxSTATUS orxFASTCALL player_checkRightRays(const orxSTRING input, orxFLOAT fDT);
