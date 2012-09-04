#pragma once

#include <CLR/IROptimizer.h>

typedef enum IntrinsicCallType
{
	IntrinsicCallType___NULL = 0,
	IntrinsicCallType___System_Runtime_CompilerServices___RuntimeHelpers___get_OffsetToStringData,
	IntrinsicCallType___System___Buffer___ByteLengthInternal,
	IntrinsicCallType___System___Buffer___GetByteInternal,
	IntrinsicCallType___System___Buffer___SetByteInternal,
	IntrinsicCallType___System___GC___get_MaxGeneration,
	IntrinsicCallType___System___GC___GetGeneration,
	IntrinsicCallType___System___Environment___get_Platform,
	IntrinsicCallType___System___Environment___get_ProcessorCount,
	IntrinsicCallType___System___Environment___GetPageSize,
	IntrinsicCallType___Proton___Core___Interop___AddressAsObject,
	IntrinsicCallType___Proton___Core___Interop___ObjectAsAddress,
} IntrinsicCallType;

void IROptimizer_IntrinsicSubstitution(IRMethod* pMethod);
