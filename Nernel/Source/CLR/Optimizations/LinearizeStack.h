#pragma once


#include <CLR/IROptimizer.h>

void IROptimizer_LinearizeStack(IRMethod* pMethod, IRBranch* pBranches, uint32_t pBranchCount);
