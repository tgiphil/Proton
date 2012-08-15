#pragma once

#include <CLR/IROptimizer.h>

typedef enum IntrinsicCallType
{
	IntrinsicCallType___NULL = 0,
	IntrinsicCallType___System_Runtime_CompilerServices___RuntimeHelpers___get_OffsetToStringData,
	IntrinsicCallType___System___Buffer___ByteLengthInternal,
	IntrinsicCallType___System___Buffer___GetByteInternal,
	IntrinsicCallType___System___Buffer___SetByteInternal,
} IntrinsicCallType;

void IROptimizer_IntrinsicSubstitution(IRMethod* pMethod);