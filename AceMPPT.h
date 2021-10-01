#ifndef ACEMPPT_H
#define ACEMPPT_H

#include "sig.h"

#define ACEMPPT_STATUS (SIG_PRIORITY_MEDIUM | 0x40)

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

#define ACEMPPT_NAMES                                                     \
  {"mppt/2/vpv", ACEMPPT_VPV2}, {"mppt/2/ich", ACEMPPT_ICH2},             \
  {"mppt/3/vpv", ACEMPPT_VPV3}, {"mppt/3/ich", ACEMPPT_ICH3},             \
  {"mppt/berr", ACEMPPT_BERR}

#endif // ACEMPPT_H
