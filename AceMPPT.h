#ifndef ACEMPPT_H
#define ACEMPPT_H

#include "AceBMS.h"

#define ACEMPPT_STATUS (ACEBMS_MPPT_STATUS | SIG_SIZE7)
#define ACEMPPT_COMMAND (ACEBMS_MPPT_COMMAND)
#define ACEMPPT_SENSOR (ACEBMS_MPPT_SENSOR | SIG_SIZE6)

#define ACEMPPT_ICH2                                                      \
  (ACEMPPT_STATUS | SIG_WORD | SIG_OFF0 | SIG_UNIT | SIG_UINT)
#define ACEMPPT_ICH3                                                      \
  (ACEMPPT_STATUS | SIG_WORD | SIG_OFF2 | SIG_UNIT | SIG_UINT)
#define ACEMPPT_VPV2                                                      \
  (ACEMPPT_STATUS | SIG_BYTE | SIG_OFF4 | SIG_UNIT | SIG_UINT)
#define ACEMPPT_VPV3                                                      \
    (ACEMPPT_STATUS | SIG_BYTE | SIG_OFF5 | SIG_UNIT | SIG_UINT)
#define ACEMPPT_BERR                                                      \
    (ACEMPPT_STATUS | SIG_BYTE | SIG_OFF6 | SIG_UNIT | SIG_UINT)

#define ACEMPPT_INSOL \
      (ACEMPPT_SENSOR | SIG_WORD | SIG_OFF0 | SIG_UNIT | SIG_UINT)
#define ACEMPPT_EXTTMP \
      (ACEMPPT_SENSOR | SIG_WORD | SIG_OFF2 | SIG_DECI)
#define ACEMPPT_OFFPOW \
      (ACEMPPT_SENSOR | SIG_WORD | SIG_OFF4 | SIG_UNIT | SIG_UINT)

#define ACEMPPT_NAMES               \
      {"m2Vs", ACEMPPT_VPV2}, \
      {"m2Ib", ACEMPPT_ICH2}, \
      {"m3Vs", ACEMPPT_VPV3}, \
      {"m3Ib", ACEMPPT_ICH3}, \
      {"mErr", ACEMPPT_BERR}, \
      {"mInsol", ACEMPPT_INSOL}, \
      {"mExtTmp", ACEMPPT_EXTTMP}, \
      {"mPoffice", ACEMPPT_OFFPOW}

#endif // ACEMPPT_H
