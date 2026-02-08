#include "RNGUtility.h"

int32 URNGUtility::MakeDeterministicSeed(int32 BaseSeed, int32 Salt)
{
    // Simple mixing function — deterministic and repeatable
    uint32 a = (uint32)BaseSeed;
    uint32 b = (uint32)Salt;
    a ^= b + 0x9e3779b9 + (a<<6) + (a>>2);
    return (int32)a;
}
