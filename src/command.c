#include "command.h"

void orxFASTCALL command_MoveBetween(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult) {
  orxOBJECT *object;
  orxVECTOR currentSpeed, speed, position, to, from, swap;

  object = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));
  orxObject_GetPosition(object, &position);
  orxObject_GetSpeed(object, &currentSpeed);

  from = _astArgList[1].vValue;
  to = _astArgList[2].vValue;

  // If object position equals the "to" parameter, we swap from and to
  if (currentSpeed.fX < 0 && to.fX > from.fX ||
      currentSpeed.fX > 0 && to.fX < from.fX || currentSpeed.fY < 0 && to.fY > from.fY ||
      currentSpeed.fY > 0 && to.fY < from.fY) {
    from = _astArgList[2].vValue;
    to = _astArgList[1].vValue;
  }
  
  if (currentSpeed.fX < 0 && position.fX <= to.fX ||
      currentSpeed.fY < 0 && position.fY <= to.fY ||
      currentSpeed.fX > 0 && position.fX >= to.fX ||
      currentSpeed.fY > 0 && position.fY >= to.fY) {
    swap = from;       
    from = to;
    to = swap;
  }
  
  orxVector_Sub(&speed, &to, &from);
  orxVector_Divf(&speed, &speed, _astArgList[3].fValue);

  if (!orxVector_AreEqual(&currentSpeed, &speed)) {
    orxObject_SetSpeed(object, &speed);
  }

  _pstResult->u64Value = _astArgList[0].u64Value;
  return;
}
