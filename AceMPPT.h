#ifndef ACEMPPT_H
#define ACEMPPT_H

#include "sig.h"

#define ACEMPPT_STATUS (SIG_PRIORITY_MEDIUM | 0x40)

#define ACEMPPT_VPV2                                                      \
  (ACEMPPT_STATUS | SIG_WORD | SIG_OFF0 | SIG_CENT | SIG_UINT)
#define ACEMPPT_ICH2                                                      \
  (ACEMPPT_STATUS | SIG_WORD | SIG_OFF2 | SIG_DECI | SIG_UINT)
#define ACEMPPT_VPV3                                                      \
  (ACEMPPT_STATUS | SIG_WORD | SIG_OFF4 | SIG_CENT | SIG_UINT)
#define ACEMPPT_ICH3                                                      \
  (ACEMPPT_STATUS | SIG_WORD | SIG_OFF6 | SIG_DECI | SIG_UINT)

#define ACEMPPT_NAMES                                                     \
  {"Vpv2", ACEMPPT_VPV2}, {"Ich2", ACEMPPT_ICH2},                    \
      {"Vpv3", ACEMPPT_VPV3}, {"Ich3", ACEMPPT_ICH3}

#endif // ACEMPPT_H
