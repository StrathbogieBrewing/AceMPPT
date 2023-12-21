#ifndef ACEMPPT_H
#define ACEMPPT_H

#include "AceBMS.h"

#define ACEMPPT_STATUS (ACEBMS_MPPT_STATUS | SIG_SIZE7)
#define ACEMPPT_COMMAND (ACEBMS_MPPT_COMMAND)
#define ACEMPPT_SENSOR (ACEBMS_MPPT_SENSOR | SIG_SIZE6)

#define ACEMPPT_ICH                                                      \
  (ACEMPPT_STATUS | SIG_WORD | SIG_OFF0 | SIG_UNIT | SIG_UINT)
#define ACEMPPT_VPV                                                      \
  (ACEMPPT_STATUS | SIG_BYTE | SIG_OFF2 | SIG_UNIT | SIG_UINT)
#define ACEMPPT_BERR                                                      \
    (ACEMPPT_STATUS | SIG_BYTE | SIG_OFF3 | SIG_UNIT | SIG_UINT)

#define ACEMPPT_INSOL \
      (ACEMPPT_SENSOR | SIG_WORD | SIG_OFF0 | SIG_UNIT | SIG_UINT)
#define ACEMPPT_EXTTMP \
      (ACEMPPT_SENSOR | SIG_WORD | SIG_OFF2 | SIG_DECI)
#define ACEMPPT_OFFPOW \
      (ACEMPPT_SENSOR | SIG_WORD | SIG_OFF4 | SIG_UNIT | SIG_UINT)

#define ACEMPPT_NAMES               \
      {"mVs", ACEMPPT_VPV}, \
      {"mIb", ACEMPPT_ICH}, \
      {"mErr", ACEMPPT_BERR}, \
      {"mSol", ACEMPPT_INSOL}, \
      {"mTmp", ACEMPPT_EXTTMP}, \
      {"mPsh", ACEMPPT_OFFPOW}

#endif // ACEMPPT_H
