#ifndef ACEMPPT_H
#define ACEMPPT_H

#include "AceBMS.h"

#define ACEMPPT_STATUS (ACEBMS_MPPT_STATUS | SIG_SIZE8)
#define ACEMPPT_COMMAND (ACEBMS_MPPT_COMMAND)
#define ACEMPPT_SENSOR (ACEBMS_MPPT_SENSOR | SIG_SIZE6)

#define ACEMPPT_ICH_0                                                      \
  (ACEMPPT_STATUS | SIG_WORD | SIG_OFF0 | SIG_UNIT | SIG_DECI)
#define ACEMPPT_ICH_1                                                      \
  (ACEMPPT_STATUS | SIG_WORD | SIG_OFF2 | SIG_UNIT | SIG_DECI)
#define ACEMPPT_VPV_0                                                      \
  (ACEMPPT_STATUS | SIG_WORD | SIG_OFF4 | SIG_UNIT | SIG_DECI)
#define ACEMPPT_VPV_1                                                      \
  (ACEMPPT_STATUS | SIG_WORD | SIG_OFF6 | SIG_UNIT | SIG_DECI)

#define ACEMPPT_NAMES               \
      {"mVs0", ACEMPPT_VPV_0}, \
      {"mIb0", ACEMPPT_ICH_0}, \
      {"mVs1", ACEMPPT_VPV_1}, \
      {"mIb1", ACEMPPT_ICH_1}

#endif // ACEMPPT_H
