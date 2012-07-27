#include <CLR/GC.h>
#include <CLR/JIT.h>
#include <CLR/JIT/x86/Layout.h>
#include <CLR/JIT/x86/x86-codegen.h>

#define PRIMARY_REG X86_EAX
#define SECONDARY_REG X86_EBX
#define THIRD_REG X86_ECX
#define FOURTH_REG X86_EDX

#define STACK_FLAGS_REG X86_ESI
#define APP_DOMAIN_REG X86_EDI

//#define MemoryCorruptionChecks

#ifdef MemoryCorruptionChecks

#define Move_Destination_Default(parentFunc, type) \
default: \
	Panic(#parentFunc ": Unknown destination type for " #type "!"); \
	break;

#define Destination_Default(parentFunc) \
default: \
	Panic(#parentFunc ": Unknown destination type!"); \
	break;

#define Source_Default(parentFunc) \
default: \
	Panic(#parentFunc ": Unknown source type!"); \
	break; 
#else

#define Move_Destination_Default(parentFunc, type)
#define Destination_Default(parentFunc)
#define Source_Default(parentFunc)

#endif

// These source types aren't valid destinations
#define Define_Bad_Destinations() \
	case SourceType_Null: \
	case SourceType_ParameterAddress: \
	case SourceType_ConstantI4: \
	case SourceType_ConstantI8: \
	case SourceType_ConstantR4: \
	case SourceType_ConstantR8: \
	case SourceType_LocalAddress: \
	case SourceType_FieldAddress: \
	case SourceType_StaticFieldAddress: \
	case SourceType_SizeOf: \
	case SourceType_ArrayElementAddress: \
	case SourceType_ArrayLength: \
	{ \
		Panic("This should not be happening!"); \
		break; \
	}

#define Define_Move_To_Destination(sourceBaseReg, sourceOffset, destinationBaseReg, destinationOffset, intermediateReg, sourceAligned, destinationAligned) \
	switch (sizeOfDestination) \
	{ \
		case 1: \
		case 2: \
		case 3: \
			if (!sourceAligned && destinationAligned) \
				x86_alu_reg_reg(pCompiledCode, X86_XOR, intermediateReg, intermediateReg); \
		case 4: \
			if (sourceAligned) \
				x86_mov_reg_membase(pCompiledCode, intermediateReg, sourceBaseReg, sourceOffset, gSizeOfPointerInBytes); \
			else \
				x86_mov_reg_membase(pCompiledCode, intermediateReg, sourceBaseReg, sourceOffset, sizeOfSource); \
			\
			if (destinationAligned) \
				x86_mov_membase_reg(pCompiledCode, destinationBaseReg, destinationOffset, intermediateReg, gSizeOfPointerInBytes); \
			else \
				x86_mov_membase_reg(pCompiledCode, destinationBaseReg, destinationOffset, intermediateReg, sizeOfDestination); \
			break; \
		default: \
		{ \
			uint32_t count = sizeOfDestination >> gPointerDivideShift; \
			for (uint32_t index = 0; index < count; index++) \
			{ \
				x86_mov_reg_membase(pCompiledCode, intermediateReg, sourceBaseReg, (sourceOffset) + (index << gPointerDivideShift), gSizeOfPointerInBytes); \
				x86_mov_membase_reg(pCompiledCode, destinationBaseReg, (destinationOffset) + (index << gPointerDivideShift), intermediateReg, gSizeOfPointerInBytes); \
			} \
			if (!destinationAligned) \
			{ \
				uint32_t remainder = sizeOfDestination & (gSizeOfPointerInBytes - 1); \
				if (remainder) \
				{ \
					x86_mov_reg_membase(pCompiledCode, intermediateReg, sourceBaseReg, (sourceOffset) + (count << gPointerDivideShift), remainder); \
					x86_mov_membase_reg(pCompiledCode, destinationBaseReg, (destinationOffset) + (count << gPointerDivideShift), intermediateReg, remainder); \
				} \
			} \
			break; \
		} \
	}


char* JIT_Emit_Load(char* pCompiledCode, IRMethod* pMethod, SourceTypeData* pSource, X86_Reg_No pRegister1, X86_Reg_No pRegister2, X86_Reg_No pRegister3, size_t* pSize)
{
	uint32_t sizeOfSource = 0;
	switch (pSource->Type)
	{
		case SourceType_Null:
		{
			sizeOfSource = gSizeOfPointerInBytes;
			x86_mov_reg_imm(pCompiledCode, pRegister1, 0);
			break;
		}
		case SourceType_Local:
		{
			sizeOfSource = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pSource->Data.LocalVariable.LocalVariableIndex]->VariableType));
			switch (sizeOfSource)
			{
				case 4:
					x86_mov_reg_membase(pCompiledCode, pRegister1, X86_EBP, -pMethod->LocalVariables[pSource->Data.LocalVariable.LocalVariableIndex]->Offset, 4);
					break;
				default:
				{
					x86_adjust_stack(pCompiledCode, -((int32_t)sizeOfSource));
					uint32_t count = sizeOfSource >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_EBP, -(pMethod->LocalVariables[pSource->Data.LocalVariable.LocalVariableIndex]->Offset - (index << gPointerDivideShift)), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, X86_ESP, (index << gPointerDivideShift), pRegister2, gSizeOfPointerInBytes);
					}
					break;
				}
			}
			break;
		}
		case SourceType_LocalAddress:
		{
			sizeOfSource = gSizeOfPointerInBytes;
			x86_mov_reg_reg(pCompiledCode, pRegister1, X86_EBP, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_SUB, pRegister1, pMethod->LocalVariables[pSource->Data.LocalVariableAddress.LocalVariableIndex]->Offset);
			break;
		}
		case SourceType_Parameter:
		{
			sizeOfSource = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pSource->Data.Parameter.ParameterIndex]->Type));
			switch (sizeOfSource)
			{
				case 4:
					x86_mov_reg_membase(pCompiledCode, pRegister1, X86_EBP, pMethod->Parameters[pSource->Data.Parameter.ParameterIndex]->Offset, 4);
					break;
				default:
				{
					x86_adjust_stack(pCompiledCode, -((int32_t)sizeOfSource));
					uint32_t count = sizeOfSource >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_EBP, (pMethod->Parameters[pSource->Data.Parameter.ParameterIndex]->Offset + (index << gPointerDivideShift)), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, X86_ESP, (index << gPointerDivideShift), pRegister2, gSizeOfPointerInBytes);
					}
					break;
				}
			}
			break;
		}
		case SourceType_ParameterAddress:
		{
			sizeOfSource = gSizeOfPointerInBytes;
			x86_mov_reg_reg(pCompiledCode, pRegister1, X86_EBP, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, pMethod->Parameters[pSource->Data.ParameterAddress.ParameterIndex]->Offset);
			break;
		}
		case SourceType_ConstantI4:
		{
			sizeOfSource = 4;
			x86_mov_reg_imm(pCompiledCode, pRegister1, pSource->Data.ConstantI4.Value);
			break;
		}
		case SourceType_ConstantI8:
		{
			sizeOfSource = 8;
			x86_push_imm(pCompiledCode, pSource->Data.ConstantI8.Value >> 32);
			x86_push_imm(pCompiledCode, pSource->Data.ConstantI8.Value);
			break;
		}
		case SourceType_ConstantR4:
		{
			sizeOfSource = 4;
			x86_mov_reg_imm(pCompiledCode, pRegister1, pSource->Data.ConstantR4.Value);
			break;
		}
		case SourceType_ConstantR8:
		{
			sizeOfSource = 8;
			x86_push_imm(pCompiledCode, pSource->Data.ConstantR8.Value >> 32);
			x86_push_imm(pCompiledCode, pSource->Data.ConstantR8.Value);
			break;
		}
		case SourceType_Field:
		{
			JIT_CalculateFieldLayout(pSource->Data.Field.ParentType);
			IRField* sourceField = pSource->Data.Field.ParentType->Fields[pSource->Data.Field.FieldIndex];
			sizeOfSource = JIT_GetStackSizeOfType(sourceField->FieldType);
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sourceField->Offset);
			switch (sizeOfSource)
			{
				case 1:
				case 2:
				case 3:
					x86_alu_reg_reg(pCompiledCode, X86_XOR, pRegister1, pRegister1);
				case 4:
					x86_mov_reg_membase(pCompiledCode, pRegister1, pRegister3, 0, sizeOfSource);
					break;
				default:
				{
					x86_adjust_stack(pCompiledCode, -((int32_t)JIT_StackAlign(sizeOfSource)));
					uint32_t count = sizeOfSource >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, pRegister3, (index << gPointerDivideShift), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, X86_ESP, (index << gPointerDivideShift), pRegister2, gSizeOfPointerInBytes);
					}
					uint32_t remainder = sizeOfSource & (gSizeOfPointerInBytes - 1);
					if (remainder)
					{
						x86_alu_reg_reg(pCompiledCode, X86_XOR, pRegister2, pRegister2);
						x86_mov_reg_membase(pCompiledCode, pRegister2, pRegister3, (count << gPointerDivideShift), remainder);
						x86_mov_membase_reg(pCompiledCode, X86_ESP, (count << gPointerDivideShift), pRegister2, remainder);
					}
					break;
				}
			}
			break;
		}
		case SourceType_FieldAddress:
		{
			JIT_CalculateFieldLayout(pSource->Data.FieldAddress.ParentType);
			sizeOfSource = gSizeOfPointerInBytes;
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.FieldAddress.FieldSource, pRegister1, pRegister2, pRegister3, NULL);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, pSource->Data.FieldAddress.ParentType->Fields[pSource->Data.FieldAddress.FieldIndex]->Offset);
			break;
		}
		case SourceType_StaticField:
		{
			IRField* field = pSource->Data.StaticField.Field;
			sizeOfSource = JIT_GetStackSizeOfType(field->FieldType);
			x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
			x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
			switch (sizeOfSource)
			{
				case 1:
				case 2:
				case 3:
					x86_alu_reg_reg(pCompiledCode, X86_XOR, pRegister1, pRegister1);
				case 4:
					x86_mov_reg_membase(pCompiledCode, pRegister1, pRegister3, 0, sizeOfSource);
					break;
				default:
				{
					x86_adjust_stack(pCompiledCode, -((int32_t)JIT_StackAlign(sizeOfSource)));
					uint32_t count = sizeOfSource >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, pRegister3, (index << gPointerDivideShift), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, X86_ESP, (index << gPointerDivideShift), pRegister2, gSizeOfPointerInBytes);
					}
					uint32_t remainder = sizeOfSource & (gSizeOfPointerInBytes - 1);
					if (remainder)
					{
						x86_alu_reg_reg(pCompiledCode, X86_XOR, pRegister2, pRegister2);
						x86_mov_reg_membase(pCompiledCode, pRegister2, pRegister3, (count << gPointerDivideShift), remainder);
						x86_mov_membase_reg(pCompiledCode, X86_ESP, (count << gPointerDivideShift), pRegister2, remainder);
					}
					break;
				}
			}
			break;
		}
		case SourceType_StaticFieldAddress:
		{
			IRField* field = pSource->Data.StaticFieldAddress.Field;
			sizeOfSource = gSizeOfPointerInBytes;
			x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
			x86_mov_reg_membase(pCompiledCode, pRegister1, pRegister3, 0, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, field->Offset);
			break;
		}
		case SourceType_Indirect:
		{
			sizeOfSource = JIT_GetStackSizeOfType(pSource->Data.Indirect.Type);
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
			switch (sizeOfSource)
			{
				case 1:
				case 2:
				case 3:
					x86_alu_reg_reg(pCompiledCode, X86_XOR, pRegister1, pRegister1);
				case 4:
					x86_mov_reg_membase(pCompiledCode, pRegister1, pRegister3, 0, sizeOfSource);
					break;
				default:
				{
					x86_adjust_stack(pCompiledCode, -((int32_t)JIT_StackAlign(sizeOfSource)));
					uint32_t count = sizeOfSource >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, pRegister3, (index << gPointerDivideShift), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, X86_ESP, (index << gPointerDivideShift), pRegister2, gSizeOfPointerInBytes);
					}
					uint32_t remainder = sizeOfSource & (gSizeOfPointerInBytes - 1);
					if (remainder)
					{
						x86_alu_reg_reg(pCompiledCode, X86_XOR, pRegister2, pRegister2);
						x86_mov_reg_membase(pCompiledCode, pRegister2, pRegister3, (count << gPointerDivideShift), remainder);
						x86_mov_membase_reg(pCompiledCode, X86_ESP, (count << gPointerDivideShift), pRegister2, remainder);
					}
					break;
				}
			}
			break;
		}
		case SourceType_SizeOf:
		{
			sizeOfSource = 4;
			x86_mov_reg_imm(pCompiledCode, pRegister1, JIT_GetSizeOfType(pSource->Data.SizeOf.Type));
			break;
		}
		case SourceType_ArrayElement:
		{
			sizeOfSource = JIT_GetStackSizeOfType(pSource->Data.ArrayElement.ElementType);
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
			if (pSource->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
			{
				x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfSource * pSource->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
			}
			else
			{
				x86_push_reg(pCompiledCode, pRegister3);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
				x86_pop_reg(pCompiledCode, pRegister3);
				if (sizeOfSource != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfSource);
				x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
			}
			switch (sizeOfSource)
			{
				case 1:
				case 2:
				case 3:
					x86_alu_reg_reg(pCompiledCode, X86_XOR, pRegister1, pRegister1);
				case 4:
					x86_mov_reg_membase(pCompiledCode, pRegister1, pRegister3, 0, sizeOfSource);
					break;
				default:
				{
					x86_adjust_stack(pCompiledCode, -((int32_t)JIT_StackAlign(sizeOfSource)));
					uint32_t count = sizeOfSource >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, pRegister3, (index << gPointerDivideShift), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, X86_ESP, (index << gPointerDivideShift), pRegister2, gSizeOfPointerInBytes);
					}
					uint32_t remainder = sizeOfSource & (gSizeOfPointerInBytes - 1);
					if (remainder)
					{
						x86_alu_reg_reg(pCompiledCode, X86_XOR, pRegister2, pRegister2);
						x86_mov_reg_membase(pCompiledCode, pRegister2, pRegister3, (count << gPointerDivideShift), remainder);
						x86_mov_membase_reg(pCompiledCode, X86_ESP, (count << gPointerDivideShift), pRegister2, remainder);
					}
					break;
				}
			}
			break;
		}
		case SourceType_ArrayElementAddress:
		{
			sizeOfSource = gSizeOfPointerInBytes;
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.ArrayElementAddress.ArraySource, pRegister1, pRegister2, pRegister3, NULL);
			if (pSource->Data.ArrayElementAddress.IndexSource->Type == SourceType_ConstantI4)
			{
				x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, sizeOfSource * pSource->Data.ArrayElementAddress.IndexSource->Data.ConstantI4.Value);
			}
			else
			{
				x86_push_reg(pCompiledCode, pRegister1);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.ArrayElementAddress.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
				x86_pop_reg(pCompiledCode, pRegister1);
				if (sizeOfSource != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfSource);
				x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister1, pRegister2);
			}
			break;
		}
		case SourceType_ArrayLength:
		{
			sizeOfSource = 4;
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.ArrayLength.ArraySource, pRegister1, pRegister2, pRegister3, NULL);
			x86_mov_reg_membase(pCompiledCode, pRegister1, pRegister1, -gSizeOfPointerInBytes, gSizeOfPointerInBytes);
			x86_mov_reg_membase(pCompiledCode, pRegister1, pRegister1, offsetof(GCObjectHeader, Array.Length), 4);
			break;
		}
		Source_Default(JIT_Emit_Load);
	}
	if (pSize) *pSize = sizeOfSource;
	return pCompiledCode;
}

char* JIT_Emit_Store(char* pCompiledCode, IRMethod* pMethod, SourceTypeData* pDestination, X86_Reg_No pRegister1, X86_Reg_No pRegister2, X86_Reg_No pRegister3, size_t* pSize)
{
	uint32_t sizeOfDestination = 0;
	switch (pDestination->Type)
	{
		case SourceType_Local:
		{
			sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->VariableType));
			switch (sizeOfDestination)
			{
				case 4:
					x86_mov_membase_reg(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister1, 4);
					break;
				default:
				{
					uint32_t count = sizeOfDestination >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_ESP, (index << gPointerDivideShift), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, X86_EBP, -(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset - (index << gPointerDivideShift)), pRegister2, gSizeOfPointerInBytes);
					}
					x86_adjust_stack(pCompiledCode, ((int32_t)sizeOfDestination));
					break;
				}
			}
			break;
		}
		case SourceType_Parameter:
		{
			sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Type));
			switch (sizeOfDestination)
			{
				case 4:
					x86_mov_membase_reg(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister1, 4);
					break;
				default:
				{
					uint32_t count = sizeOfDestination >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_ESP, (index << gPointerDivideShift), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, X86_EBP, (pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset + (index << gPointerDivideShift)), pRegister2, gSizeOfPointerInBytes);
					}
					x86_adjust_stack(pCompiledCode, ((int32_t)sizeOfDestination));
					break;
				}
			}
			break;
		}
		case SourceType_Field:
		{
			JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
			IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
			sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
			if (sizeOfDestination <= 8)
			{
				x86_push_reg(pCompiledCode, pRegister1);
				if (sizeOfDestination > 4)
				{
					x86_push_reg(pCompiledCode, pRegister2);
				}
			}
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
			if (sizeOfDestination <= 8)
			{
				if (sizeOfDestination > 4)
				{
					x86_pop_reg(pCompiledCode, pRegister2);
				}
				x86_pop_reg(pCompiledCode, pRegister1);
			}
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
			switch (sizeOfDestination)
			{
				case 1:
				case 2:
				case 3:
				case 4:
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, sizeOfDestination);
					break;
				default:
				{
					uint32_t count = sizeOfDestination >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_ESP, (index << gPointerDivideShift), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, pRegister3, (index << gPointerDivideShift), pRegister2, gSizeOfPointerInBytes);
					}
					uint32_t remainder = sizeOfDestination & (gSizeOfPointerInBytes - 1);
					if (remainder)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_ESP, (count << gPointerDivideShift), remainder);
						x86_mov_membase_reg(pCompiledCode, pRegister3, (count << gPointerDivideShift), pRegister2, remainder);
					}
					x86_adjust_stack(pCompiledCode, ((int32_t)JIT_StackAlign(sizeOfDestination)));
					break;
				}
			}
			break;
		}
		case SourceType_StaticField:
		{
			IRField* field = pDestination->Data.StaticField.Field;
			sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
			x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
			x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
			switch (sizeOfDestination)
			{
				case 1:
				case 2:
				case 3:
				case 4:
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, sizeOfDestination);
					break;
				default:
				{
					uint32_t count = sizeOfDestination >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_ESP, (index << gPointerDivideShift), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, pRegister3, (index << gPointerDivideShift), pRegister2, gSizeOfPointerInBytes);
					}
					uint32_t remainder = sizeOfDestination & (gSizeOfPointerInBytes - 1);
					if (remainder)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_ESP, (count << gPointerDivideShift), remainder);
						x86_mov_membase_reg(pCompiledCode, pRegister3, (count << gPointerDivideShift), pRegister2, remainder);
					}
					x86_adjust_stack(pCompiledCode, ((int32_t)JIT_StackAlign(sizeOfDestination)));
					break;
				}
			}
			break;
		}
		case SourceType_Indirect:
		{
			sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.Indirect.Type);
			if (sizeOfDestination <= 8)
			{
				x86_push_reg(pCompiledCode, pRegister1);
				if (sizeOfDestination > 4)
				{
					x86_push_reg(pCompiledCode, pRegister2);
				}
			}
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
			if (sizeOfDestination <= 8)
			{
				if (sizeOfDestination > 4)
				{
					x86_pop_reg(pCompiledCode, pRegister2);
				}
				x86_pop_reg(pCompiledCode, pRegister1);
			}
			switch (sizeOfDestination)
			{
				case 1:
				case 2:
				case 3:
				case 4:
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, sizeOfDestination);
					break;
				default:
				{
					uint32_t count = sizeOfDestination >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_ESP, (index << gPointerDivideShift), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, pRegister3, (index << gPointerDivideShift), pRegister2, gSizeOfPointerInBytes);
					}
					uint32_t remainder = sizeOfDestination & (gSizeOfPointerInBytes - 1);
					if (remainder)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_ESP, (count << gPointerDivideShift), remainder);
						x86_mov_membase_reg(pCompiledCode, pRegister3, (count << gPointerDivideShift), pRegister2, remainder);
					}
					x86_adjust_stack(pCompiledCode, ((int32_t)JIT_StackAlign(sizeOfDestination)));
					break;
				}
			}
			break;
		}
		case SourceType_ArrayElement:
		{
			sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
			if (sizeOfDestination <= 8)
			{
				x86_push_reg(pCompiledCode, pRegister1);
				if (sizeOfDestination > 4)
				{
					x86_push_reg(pCompiledCode, pRegister2);
				}
			}
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
			if (sizeOfDestination <= 8)
			{
				if (sizeOfDestination > 4)
				{
					x86_pop_reg(pCompiledCode, pRegister2);
				}
				x86_pop_reg(pCompiledCode, pRegister1);
			}
			if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
			{
				x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
			}
			else
			{
				if (sizeOfDestination <= 8)
				{
					x86_push_reg(pCompiledCode, pRegister1);
					if (sizeOfDestination > 4)
					{
						x86_push_reg(pCompiledCode, pRegister2);
					}
				}
				x86_push_reg(pCompiledCode, pRegister3);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
				x86_pop_reg(pCompiledCode, pRegister3);

				if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
				x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);

				if (sizeOfDestination <= 8)
				{
					if (sizeOfDestination > 4)
					{
						x86_pop_reg(pCompiledCode, pRegister2);
					}
					x86_pop_reg(pCompiledCode, pRegister1);
				}
			}
			switch (sizeOfDestination)
			{
				case 1:
				case 2:
				case 3:
				case 4:
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, sizeOfDestination);
					break;
				default:
				{
					uint32_t count = sizeOfDestination >> gPointerDivideShift;
					for (uint32_t index = 0; index < count; index++)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_ESP, (index << gPointerDivideShift), gSizeOfPointerInBytes);
						x86_mov_membase_reg(pCompiledCode, pRegister3, (index << gPointerDivideShift), pRegister2, gSizeOfPointerInBytes);
					}
					uint32_t remainder = sizeOfDestination & (gSizeOfPointerInBytes - 1);
					if (remainder)
					{
						x86_mov_reg_membase(pCompiledCode, pRegister2, X86_ESP, (count << gPointerDivideShift), remainder);
						x86_mov_membase_reg(pCompiledCode, pRegister3, (count << gPointerDivideShift), pRegister2, remainder);
					}
					x86_adjust_stack(pCompiledCode, ((int32_t)JIT_StackAlign(sizeOfDestination)));
					break;
				}
			}
			break;
		}
		Define_Bad_Destinations();
		Destination_Default(JIT_Emit_Store);
	}
	if (pSize) *pSize = sizeOfDestination;
	return pCompiledCode;
}

char* JIT_Emit_Move(char* pCompiledCode, IRMethod* pMethod, SourceTypeData* pSource, SourceTypeData* pDestination, X86_Reg_No pRegister1, X86_Reg_No pRegister2, X86_Reg_No pRegister3, size_t* pSize)
{
	uint32_t sizeOfSource = 0;
	uint32_t sizeOfDestination = 0;
	switch (pSource->Type)
	{
		case SourceType_Null:
		{
			sizeOfSource = gSizeOfPointerInBytes;
			sizeOfDestination = gSizeOfPointerInBytes;
			switch (pDestination->Type)
			{
				// Null to Local (both aligned)
				case SourceType_Local:
				{
					x86_mov_membase_imm(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, 0, gSizeOfPointerInBytes);
					break;
				}
				// Null to Parameter (both aligned)
				case SourceType_Parameter:
				{
					x86_mov_membase_imm(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, 0, gSizeOfPointerInBytes);
					break;
				}
				// Null to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_imm(pCompiledCode, pRegister3, field->Offset, 0, gSizeOfPointerInBytes);
					break;
				}
				// Null to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, 0, gSizeOfPointerInBytes);
					break;
				}
				// Null to Indirect (both aligned)
				case SourceType_Indirect:
				{
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, 0, gSizeOfPointerInBytes);
					break;
				}
				// Null to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, 0, gSizeOfPointerInBytes);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Null);
			}
			break;
		}
		// From Local Address
		case SourceType_LocalAddress:
		{
			sizeOfSource = gSizeOfPointerInBytes;
			sizeOfDestination = gSizeOfPointerInBytes;
			x86_mov_reg_reg(pCompiledCode, pRegister1, X86_EBP, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_SUB, pRegister1, pMethod->LocalVariables[pSource->Data.LocalVariableAddress.LocalVariableIndex]->Offset);
			switch (pDestination->Type)
			{
				// Local Address to Local (both aligned)
				case SourceType_Local:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Local Address to Parameter (both aligned)
				case SourceType_Parameter:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Local Address to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, field->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Local Address to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Local Address to Indirect (both aligned)
				case SourceType_Indirect:
				{
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Local Address to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister1);
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);
						x86_pop_reg(pCompiledCode, pRegister1);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Local Address);
			}
			break;
		}
		// From Parameter Address
		case SourceType_ParameterAddress:
		{
			sizeOfSource = gSizeOfPointerInBytes;
			sizeOfDestination = gSizeOfPointerInBytes;
			x86_mov_reg_reg(pCompiledCode, pRegister1, X86_EBP, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, pMethod->Parameters[pSource->Data.ParameterAddress.ParameterIndex]->Offset);
			switch (pDestination->Type)
			{
				// Parameter Address to Local (both aligned)
				case SourceType_Local:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Parameter Address to Parameter (both aligned)
				case SourceType_Parameter:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Parameter Address to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, field->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Parameter Address to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Parameter Address to Indirect (both aligned)
				case SourceType_Indirect:
				{
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Parameter Address to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister1);
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);
						x86_pop_reg(pCompiledCode, pRegister1);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Parameter Address);
			}
			break;
		}
		// From SizeOf
		case SourceType_SizeOf:
		{
			sizeOfSource = 4;
			uint32_t sizeOfType = JIT_GetSizeOfType(pSource->Data.SizeOf.Type);
			switch (pDestination->Type)
			{
				// SizeOf to Local (both aligned)
				case SourceType_Local:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->VariableType));
					x86_mov_membase_imm(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, sizeOfType, 4);
					break;
				}
				// SizeOf to Parameter (both aligned)
				case SourceType_Parameter:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Type));
					x86_mov_membase_imm(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, sizeOfType, 4);
					break;
				}
				// SizeOf to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_imm(pCompiledCode, pRegister3, field->Offset, sizeOfType, 4);
					break;
				}
				// SizeOf to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, sizeOfType, 4);
					break;
				}
				// SizeOf to Indirect (both aligned)
				case SourceType_Indirect:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.Indirect.Type);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, sizeOfType, 4);
					break;
				}
				// SizeOf to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, sizeOfType, 4);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, SizeOf);
			}
			break;
		}
		// From ConstantI4
		case SourceType_ConstantI4:
		{
			sizeOfSource = 4;
			switch (pDestination->Type)
			{
				// ConstantI4 to Local (both aligned)
				case SourceType_Local:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->VariableType));
					x86_mov_membase_imm(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pSource->Data.ConstantI4.Value, 4);
					break;
				}
				// ConstantI4 to Parameter (both aligned)
				case SourceType_Parameter:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Type));
					x86_mov_membase_imm(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pSource->Data.ConstantI4.Value, 4);
					break;
				}
				// ConstantI4 to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_imm(pCompiledCode, pRegister3, field->Offset, pSource->Data.ConstantI4.Value, 4);
					break;
				}
				// ConstantI4 to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, pSource->Data.ConstantI4.Value, 4);
					break;
				}
				// ConstantI4 to Indirect (both aligned)
				case SourceType_Indirect:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.Indirect.Type);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, pSource->Data.ConstantI4.Value, 4);
					break;
				}
				// ConstantI4 to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, pSource->Data.ConstantI4.Value, 4);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, ConstantI4);
			}
			break;
		}
		// From ConstantI8
		case SourceType_ConstantI8:
		{
			sizeOfSource = 8;
			sizeOfDestination = 8;
			switch (pDestination->Type)
			{
				// ConstantI8 to Local (both aligned)
				case SourceType_Local:
				{
					x86_mov_membase_imm(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pSource->Data.ConstantI8.Value, 4);
					x86_mov_membase_imm(pCompiledCode, X86_EBP, -(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset - 4), pSource->Data.ConstantI8.Value >> 32, 4);
					break;
				}
				// ConstantI8 to Parameter (both aligned)
				case SourceType_Parameter:
				{
					x86_mov_membase_imm(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pSource->Data.ConstantI8.Value, 4);
					x86_mov_membase_imm(pCompiledCode, X86_EBP, (pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset + 4), pSource->Data.ConstantI8.Value >> 32, 4);
					break;
				}
				// ConstantI8 to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_imm(pCompiledCode, pRegister3, field->Offset, pSource->Data.ConstantI8.Value, 4);
					x86_mov_membase_imm(pCompiledCode, pRegister3, field->Offset + 4, pSource->Data.ConstantI8.Value >> 32, 4);
					break;
				}
				// ConstantI8 to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, pSource->Data.ConstantI8.Value, 4);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 4, pSource->Data.ConstantI8.Value >> 32, 4);
					break;
				}
				// ConstantI8 to Indirect (both aligned)
				case SourceType_Indirect:
				{
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, pSource->Data.ConstantI8.Value, 4);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 4, pSource->Data.ConstantI8.Value >> 32, 4);
					break;
				}
				// ConstantI8 to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, pSource->Data.ConstantI8.Value, 4);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 4, pSource->Data.ConstantI8.Value >> 32, 4);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, ConstantI8);
			}
			break;
		}
		// From ConstantR4
		case SourceType_ConstantR4:
		{
			sizeOfSource = 4;
			switch (pDestination->Type)
			{
				// ConstantR4 to Local (both aligned)
				case SourceType_Local:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->VariableType));
					x86_mov_membase_imm(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pSource->Data.ConstantR4.Value, 4);
					break;
				}
				// ConstantR4 to Parameter (both aligned)
				case SourceType_Parameter:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Type));
					x86_mov_membase_imm(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pSource->Data.ConstantR4.Value, 4);
					break;
				}
				// ConstantR4 to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_imm(pCompiledCode, pRegister3, field->Offset, pSource->Data.ConstantR4.Value, 4);
					break;
				}
				// ConstantR4 to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, pSource->Data.ConstantR4.Value, 4);
					break;
				}
				// ConstantR4 to Indirect (both aligned)
				case SourceType_Indirect:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.Indirect.Type);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, pSource->Data.ConstantR4.Value, 4);
					break;
				}
				// ConstantR4 to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, pSource->Data.ConstantR4.Value, 4);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, ConstantR4);
			}
			break;
		}
		// From ConstantR8
		case SourceType_ConstantR8:
		{
			sizeOfSource = 8;
			sizeOfDestination = 8;
			switch (pDestination->Type)
			{
				// ConstantR8 to Local (both aligned)
				case SourceType_Local:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pSource->Data.ConstantR8.Value, 4);
					x86_mov_membase_reg(pCompiledCode, X86_EBP, -(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset - 4), pSource->Data.ConstantR8.Value >> 32, 4);
					break;
				}
				// ConstantR8 to Parameter (both aligned)
				case SourceType_Parameter:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pSource->Data.ConstantR8.Value, 4);
					x86_mov_membase_reg(pCompiledCode, X86_EBP, (pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset + 4), pSource->Data.ConstantR8.Value >> 32, 4);
					break;
				}
				// ConstantR8 to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_reg(pCompiledCode, pRegister3, field->Offset, pSource->Data.ConstantR8.Value, 4);
					x86_mov_membase_reg(pCompiledCode, pRegister3, field->Offset + 4, pSource->Data.ConstantR8.Value >> 32, 4);
					break;
				}
				// ConstantR8 to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pSource->Data.ConstantR8.Value, 4);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 4, pSource->Data.ConstantR8.Value >> 32, 4);
					break;
				}
				// ConstantR8 to Indirect (both aligned)
				case SourceType_Indirect:
				{
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pSource->Data.ConstantR8.Value, 4);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 4, pSource->Data.ConstantR8.Value >> 32, 4);
					break;
				}
				// ConstantR8 to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_imm(pCompiledCode, pRegister3, 0, pSource->Data.ConstantR8.Value, 4);
					x86_mov_membase_imm(pCompiledCode, pRegister3, 4, pSource->Data.ConstantR8.Value >> 32, 4);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, ConstantR8);
			}
			break;
		}
		// From Field Address
		case SourceType_FieldAddress:
		{
			JIT_CalculateFieldLayout(pSource->Data.FieldAddress.ParentType);
			sizeOfSource = gSizeOfPointerInBytes;
			sizeOfDestination = gSizeOfPointerInBytes;
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.FieldAddress.FieldSource, pRegister1, pRegister2, pRegister3, NULL);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, pSource->Data.FieldAddress.ParentType->Fields[pSource->Data.FieldAddress.FieldIndex]->Offset);
			switch (pDestination->Type)
			{
				// Field Address to Local (both aligned)
				case SourceType_Local:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Field Address to Parameter (both aligned)
				case SourceType_Parameter:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Field Address to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, field->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Field Address to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Field Address to Indirect (both aligned)
				case SourceType_Indirect:
				{
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Field Address to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister1);
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);
						x86_pop_reg(pCompiledCode, pRegister1);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Field Address);
			}
			break;
		}
		// From Static Field Address
		case SourceType_StaticFieldAddress:
		{
			IRField* field = pSource->Data.StaticFieldAddress.Field;
			sizeOfSource = gSizeOfPointerInBytes;
			sizeOfDestination = gSizeOfPointerInBytes;
			x86_mov_reg_membase(pCompiledCode, pRegister1, X86_EBP, gSizeOfPointerInBytes << 1, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
			x86_mov_reg_membase(pCompiledCode, pRegister1, pRegister1, 0, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, field->Offset);
			switch (pDestination->Type)
			{
				// Static Field Address to Local (both aligned)
				case SourceType_Local:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Static Field Address to Parameter (both aligned)
				case SourceType_Parameter:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Static Field Address to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, field->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Static Field Address to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Static Field Address to Indirect (both aligned)
				case SourceType_Indirect:
				{
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Static Field Address to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister1);
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);
						x86_pop_reg(pCompiledCode, pRegister1);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Static Field Address);
			}
			break;
		}
		// From Array Element Address
		case SourceType_ArrayElementAddress:
		{
			uint32_t sizeOfType = JIT_GetStackSizeOfType(pSource->Data.ArrayElementAddress.ElementType);
			sizeOfSource = gSizeOfPointerInBytes;
			sizeOfDestination = gSizeOfPointerInBytes;
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.ArrayElementAddress.ArraySource, pRegister1, pRegister2, pRegister3, NULL);
			if (pSource->Data.ArrayElementAddress.IndexSource->Type == SourceType_ConstantI4)
			{
				x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, sizeOfType * pSource->Data.ArrayElementAddress.IndexSource->Data.ConstantI4.Value);
			}
			else
			{
				x86_push_reg(pCompiledCode, pRegister1);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.ArrayElementAddress.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
				x86_pop_reg(pCompiledCode, pRegister1);

				if (sizeOfType != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfType);
				x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister1, pRegister2);
			}
			switch (pDestination->Type)
			{
				// Array Element Address to Local (both aligned)
				case SourceType_Local:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Array Element Address to Parameter (both aligned)
				case SourceType_Parameter:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Array Element Address to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, field->Offset, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Array Element Address to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Array Element Address to Indirect (both aligned)
				case SourceType_Indirect:
				{
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				// Array Element Address to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister1);
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);
						x86_pop_reg(pCompiledCode, pRegister1);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, gSizeOfPointerInBytes);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Array Element Address);
			}
			break;
		}
		// From Array Length
		case SourceType_ArrayLength:
		{
			sizeOfSource = 4;
			sizeOfDestination = 4;
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.ArrayLength.ArraySource, pRegister1, pRegister2, pRegister3, NULL);
			x86_mov_reg_membase(pCompiledCode, pRegister1, pRegister1, -gSizeOfPointerInBytes, gSizeOfPointerInBytes);
			x86_mov_reg_membase(pCompiledCode, pRegister1, pRegister1, offsetof(GCObjectHeader, Array.Length), 4);
			switch (pDestination->Type)
			{
				// Array Length to Local (both aligned)
				case SourceType_Local:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister1, 4);
					break;
				}
				// Array Length to Parameter (both aligned)
				case SourceType_Parameter:
				{
					x86_mov_membase_reg(pCompiledCode, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister1, 4);
					break;
				}
				// Array Length to Field (both aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, field->Offset, pRegister1, 4);
					break;
				}
				// Array Length to Static Field (both aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, 4);
					break;
				}
				// Array Length to Indirect (both aligned)
				case SourceType_Indirect:
				{
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, 4);
					break;
				}
				// Array Length to ArrayElement (both aligned)
				case SourceType_ArrayElement:
				{
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister1);
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);
						x86_pop_reg(pCompiledCode, pRegister1);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					x86_mov_membase_reg(pCompiledCode, pRegister3, 0, pRegister1, 4);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Array Length);
			}
			break;
		}
		// From Local
		case SourceType_Local:
		{
			sizeOfSource = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pSource->Data.LocalVariable.LocalVariableIndex]->VariableType));
			switch (pDestination->Type)
			{
				// Local to Local (both aligned)
				case SourceType_Local:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->VariableType));
					Define_Move_To_Destination(X86_EBP, -pMethod->LocalVariables[pSource->Data.LocalVariable.LocalVariableIndex]->Offset, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister2, TRUE, TRUE);
					break;
				}
				// Local to Parameter (both aligned)
				case SourceType_Parameter:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Type));
					Define_Move_To_Destination(X86_EBP, -pMethod->LocalVariables[pSource->Data.LocalVariable.LocalVariableIndex]->Offset, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister2, TRUE, TRUE);
					break;
				}
				// Local to Field (source aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(X86_EBP, -pMethod->LocalVariables[pSource->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister3, 0, pRegister2, TRUE, FALSE);
					break;
				}
				// Local to Static Field (source aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(X86_EBP, -pMethod->LocalVariables[pSource->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister3, 0, pRegister2, TRUE, FALSE);
					break;
				}
				// Local to Indirect (source aligned)
				case SourceType_Indirect:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.Indirect.Type);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					Define_Move_To_Destination(X86_EBP, -pMethod->LocalVariables[pSource->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister3, 0, pRegister2, TRUE, FALSE);
					break;
				}
				// Local to ArrayElement (source aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					Define_Move_To_Destination(X86_EBP, -pMethod->LocalVariables[pSource->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister3, 0, pRegister2, TRUE, FALSE);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Local);
			}
			break;
		}
		// From Parameter
		case SourceType_Parameter:
		{
			sizeOfSource = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pSource->Data.Parameter.ParameterIndex]->Type));
			switch (pDestination->Type)
			{
				// Parameter to Local (both aligned)
				case SourceType_Local:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->VariableType));
					Define_Move_To_Destination(X86_EBP, pMethod->Parameters[pSource->Data.Parameter.ParameterIndex]->Offset, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister2, TRUE, TRUE);
					break;
				}
				// Parameter to Parameter (both aligned)
				case SourceType_Parameter:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Type));
					Define_Move_To_Destination(X86_EBP, pMethod->Parameters[pSource->Data.Parameter.ParameterIndex]->Offset, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister2, TRUE, TRUE);
					break;
				}
				// Parameter to Field (source aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(X86_EBP, pMethod->Parameters[pSource->Data.Parameter.ParameterIndex]->Offset, pRegister3, 0, pRegister2, TRUE, FALSE);
					break;
				}
				// Parameter to Static Field (source aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(X86_EBP, pMethod->Parameters[pSource->Data.Parameter.ParameterIndex]->Offset, pRegister3, 0, pRegister2, TRUE, FALSE);
					break;
				}
				// Parameter to Indirect (source aligned)
				case SourceType_Indirect:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.Indirect.Type);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					Define_Move_To_Destination(X86_EBP, pMethod->Parameters[pSource->Data.Parameter.ParameterIndex]->Offset, pRegister3, 0, pRegister2, TRUE, FALSE);
					break;
				}
				// Parameter to ArrayElement (source aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					Define_Move_To_Destination(X86_EBP, pMethod->Parameters[pSource->Data.Parameter.ParameterIndex]->Offset, pRegister3, 0, pRegister2, TRUE, FALSE);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Parameter);
			}
			break;
		}
		// From Field
		case SourceType_Field:
		{
			JIT_CalculateFieldLayout(pSource->Data.Field.ParentType);
			IRField* sourceField = pSource->Data.Field.ParentType->Fields[pSource->Data.Field.FieldIndex];
			sizeOfSource = JIT_GetStackSizeOfType(sourceField->FieldType);
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.Field.FieldSource, pRegister1, pRegister2, pRegister3, NULL);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, sourceField->Offset);
			switch (pDestination->Type)
			{
				// Field to Local (destination aligned)
				case SourceType_Local:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->VariableType));
					Define_Move_To_Destination(pRegister1, 0, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister2, FALSE, TRUE);
					break;
				}
				// Field to Parameter (destination aligned)
				case SourceType_Parameter:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Type));
					Define_Move_To_Destination(pRegister1, 0, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister2, FALSE, TRUE);
					break;
				}
				// Field to Field (neither aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Field to Static Field (neither aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Field to Indirect (neither aligned)
				case SourceType_Indirect:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.Indirect.Type);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Field to ArrayElement (neither aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister1);
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);
						x86_pop_reg(pCompiledCode, pRegister1);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Field);
			}
			break;
		}
		// From Static Field
		case SourceType_StaticField:
		{
			IRField* field = pSource->Data.StaticField.Field;
			sizeOfSource = JIT_GetStackSizeOfType(field->FieldType);
			x86_mov_reg_reg(pCompiledCode, pRegister1, APP_DOMAIN_REG, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
			x86_mov_reg_membase(pCompiledCode, pRegister1, pRegister1, 0, gSizeOfPointerInBytes);
			x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, field->Offset);
			switch (pDestination->Type)
			{
				// Static Field to Local (destination aligned)
				case SourceType_Local:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->VariableType));
					Define_Move_To_Destination(pRegister1, 0, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister2, FALSE, TRUE);
					break;
				}
				// Static Field to Parameter (destination aligned)
				case SourceType_Parameter:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Type));
					Define_Move_To_Destination(pRegister1, 0, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister2, FALSE, TRUE);
					break;
				}
				// Static Field to Field (neither aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Static Field to Static Field (neither aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Static Field to Indirect (neither aligned)
				case SourceType_Indirect:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.Indirect.Type);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Static Field to ArrayElement (neither aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister1);
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);
						x86_pop_reg(pCompiledCode, pRegister1);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Static Field);
			}
			break;
		}
		// From Indirect
		case SourceType_Indirect:
		{
			sizeOfSource = JIT_GetStackSizeOfType(pSource->Data.Indirect.Type);
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.Indirect.AddressSource, pRegister1, pRegister2, pRegister3, NULL);
			switch (pDestination->Type)
			{
				// Indirect to Local (destination aligned)
				case SourceType_Local:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->VariableType));
					Define_Move_To_Destination(pRegister1, 0, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister2, FALSE, TRUE);
					break;
				}
				// Indirect to Parameter (destination aligned)
				case SourceType_Parameter:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Type));
					Define_Move_To_Destination(pRegister1, 0, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister2, FALSE, TRUE);
					break;
				}
				// Indirect to Field (neither aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Indirect to Static Field (neither aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Indirect to Indirect (neither aligned)
				case SourceType_Indirect:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.Indirect.Type);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Indirect to ArrayElement (neither aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister1);
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);
						x86_pop_reg(pCompiledCode, pRegister1);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Indirect);
			}			
			break;
		}
		// From Array Element
		case SourceType_ArrayElement:
		{
			sizeOfSource = JIT_GetStackSizeOfType(pSource->Data.ArrayElement.ElementType);
			pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.ArrayElement.ArraySource, pRegister1, pRegister2, pRegister3, NULL);
			if (pSource->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
			{
				x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister1, sizeOfSource * pSource->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
			}
			else
			{
				x86_push_reg(pCompiledCode, pRegister1);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pSource->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
				x86_pop_reg(pCompiledCode, pRegister1);

				if (sizeOfSource != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfSource);
				x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister1, pRegister2);
			}
			switch (pDestination->Type)
			{
				// Array Element to Local (destination aligned)
				case SourceType_Local:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->VariableType));
					Define_Move_To_Destination(pRegister1, 0, X86_EBP, -pMethod->LocalVariables[pDestination->Data.LocalVariable.LocalVariableIndex]->Offset, pRegister2, FALSE, TRUE);
					break;
				}
				// Array Element to Parameter (destination aligned)
				case SourceType_Parameter:
				{
					sizeOfDestination = JIT_StackAlign(JIT_GetStackSizeOfType(pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Type));
					Define_Move_To_Destination(pRegister1, 0, X86_EBP, pMethod->Parameters[pDestination->Data.Parameter.ParameterIndex]->Offset, pRegister2, FALSE, TRUE);
					break;
				}
				// Array Element to Field (neither aligned)
				case SourceType_Field:
				{
					JIT_CalculateFieldLayout(pDestination->Data.Field.ParentType);
					IRField* field = pDestination->Data.Field.ParentType->Fields[pDestination->Data.Field.FieldIndex];
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Field.FieldSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Array Element to Static Field (neither aligned)
				case SourceType_StaticField:
				{
					IRField* field = pDestination->Data.StaticField.Field;
					sizeOfDestination = JIT_GetStackSizeOfType(field->FieldType);
					x86_mov_reg_reg(pCompiledCode, pRegister3, APP_DOMAIN_REG, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, offsetof(AppDomain, StaticValues) + (field->ParentAssembly->AssemblyIndex << gPointerDivideShift));
					x86_mov_reg_membase(pCompiledCode, pRegister3, pRegister3, 0, gSizeOfPointerInBytes);
					x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, field->Offset);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Array Element to Indirect (neither aligned)
				case SourceType_Indirect:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.Indirect.Type);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.Indirect.AddressSource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				// Array Element to ArrayElement (neither aligned)
				case SourceType_ArrayElement:
				{
					sizeOfDestination = JIT_GetStackSizeOfType(pDestination->Data.ArrayElement.ElementType);
					x86_push_reg(pCompiledCode, pRegister1);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.ArraySource, pRegister3, pRegister2, pRegister1, NULL);
					x86_pop_reg(pCompiledCode, pRegister1);
					if (pDestination->Data.ArrayElement.IndexSource->Type == SourceType_ConstantI4)
					{
						x86_alu_reg_imm(pCompiledCode, X86_ADD, pRegister3, sizeOfDestination * pDestination->Data.ArrayElement.IndexSource->Data.ConstantI4.Value);
					}
					else
					{
						x86_push_reg(pCompiledCode, pRegister1);
						x86_push_reg(pCompiledCode, pRegister3);
						pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, pDestination->Data.ArrayElement.IndexSource, pRegister2, pRegister1, pRegister3, NULL);
						x86_pop_reg(pCompiledCode, pRegister3);
						x86_pop_reg(pCompiledCode, pRegister1);

						if (sizeOfDestination != 1) x86_imul_reg_reg_imm(pCompiledCode, pRegister2, pRegister2, sizeOfDestination);
						x86_alu_reg_reg(pCompiledCode, X86_ADD, pRegister3, pRegister2);
					}
					Define_Move_To_Destination(pRegister1, 0, pRegister3, 0, pRegister2, FALSE, FALSE);
					break;
				}
				Define_Bad_Destinations();
				Move_Destination_Default(JIT_Emit_Move, Array Element);
			}
			break;
		}
		Source_Default(JIT_Emit_Move);
	}
	if (pSize) *pSize = sizeOfDestination;
	return pCompiledCode;
}


void JIT_BranchLinker(BranchRegistry* pBranchRegistry)
{
	for (uint32_t index = 0; index < pBranchRegistry->InstructionCount; ++index)
	{
		if (pBranchRegistry->BranchLocations[index])
		{
			Log_WriteLine(LOGLEVEL__JIT_Link, "Linking %sBranch from 0x%x to 0x%x", pBranchRegistry->SpecialBranches[index] ? "Special " : "", (unsigned int)pBranchRegistry->BranchLocations[index], (unsigned int)pBranchRegistry->InstructionLocations[pBranchRegistry->TargetLocations[index]]);
			if (pBranchRegistry->SpecialBranches[index])
			{
				*(size_t*)pBranchRegistry->BranchLocations[index] = pBranchRegistry->InstructionLocations[pBranchRegistry->TargetLocations[index]];
			}
			else
			{
				x86_patch((unsigned char*)pBranchRegistry->BranchLocations[index], (unsigned char*)pBranchRegistry->InstructionLocations[pBranchRegistry->TargetLocations[index]]);
			}
		}
	}
}

char* JIT_Emit_Prologue(char* pCompiledCode, IRMethod* pMethod)
{
	// Save old stack frame
	x86_push_reg(pCompiledCode, X86_EBP);
	// Load new stack frame
	x86_mov_reg_reg(pCompiledCode, X86_EBP, X86_ESP, gSizeOfPointerInBytes);

	uint32_t localsSize = 0;
	if (pMethod->LocalVariableCount)
	{
		localsSize = pMethod->LocalVariables[pMethod->LocalVariableCount - 1]->Offset;
	}

	// Create stack space for locals
	x86_adjust_stack(pCompiledCode, -((int32_t)localsSize));

	if (localsSize > 0 &&
		pMethod->MethodDefinition->Body.Flags & MethodDefinitionBody_Fat_Flags_InitializeLocals)
	{
		for (uint32_t offset = 0; offset < localsSize; offset += 4)
		{
			// Initialize memory to zero for all of the local space
			x86_mov_membase_imm(pCompiledCode, X86_ESP, offset, 0, 4);
		}
	}

	return pCompiledCode;
}

char* JIT_Emit_Epilogue(char* pCompiledCode, IRMethod* pMethod)
{
	if (pMethod->Returns)
	{
		// Pop the value into EAX, as per cdecl call convention
		x86_pop_reg(pCompiledCode, X86_EAX);
		uint32_t sizeOfReturnType = JIT_GetStackSizeOfType(pMethod->ReturnType);
		if (sizeOfReturnType > 4 && sizeOfReturnType <= 8)
		{
			// If the size of the return type is 5-8 bytes, pop the rest into EDX as per cdecl call convention
			x86_pop_reg(pCompiledCode, X86_EDX);
		}
		else
		{
			// TODO: Deal with return parameters larger than 8 bytes
			Panic("Don't know how to support large return types yet, space is allocated before parameters for it");
		}
	}
	// Adjust the stack for local variables, and restore the previous stack frame
	x86_leave(pCompiledCode);
	// Pop EIP and return back to caller method
	x86_ret(pCompiledCode);
	return pCompiledCode;
}

char* JIT_Emit_Nop(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	if ((uint32_t)pInstruction->Arg1)
	{
		// Emit a nop if it came from IL (probably for debugging)
		x86_nop(pCompiledCode);
	}
	return pCompiledCode;
}

char* JIT_Emit_Break(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	x86_breakpoint(pCompiledCode);
	return pCompiledCode;
}

char* JIT_Emit_Move_OpCode(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	pCompiledCode = JIT_Emit_Move(pCompiledCode, pMethod, &pInstruction->Source1, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
	return pCompiledCode;
}

char* JIT_Emit_Return(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	// Register a branch target for return to immediately jump into epilogue
	BranchRegistry_RegisterBranch(pBranchRegistry, pInstruction->ILLocation, pBranchRegistry->InstructionCount + 1, pCompiledCode);
	x86_jump32(pCompiledCode, 0);
	return pCompiledCode;
}

char* JIT_Emit_Load_String(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Dup(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	pCompiledCode = JIT_Emit_Move(pCompiledCode, pMethod, &pInstruction->Source1, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
	pCompiledCode = JIT_Emit_Move(pCompiledCode, pMethod, &pInstruction->Source1, &pInstruction->Source3, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
	return pCompiledCode;
}

char* JIT_Emit_Add(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	switch((OverflowType)(uint32_t)pInstruction->Arg1)
	{
		case OverflowType_Signed:
		case OverflowType_Unsigned:
			Panic("Unsupported overflow type!");
			break;
		case OverflowType_None: break;
	}
	ElementType sAType = (ElementType)(uint32_t)pInstruction->Arg2;
	ElementType sBType = (ElementType)(uint32_t)pInstruction->Arg3;
	ElementType destType = (ElementType)0;
	switch(sAType)
	{
		case ElementType_I:
		case ElementType_U:
		case ElementType_I1:
		case ElementType_I2:
		case ElementType_I4:
		case ElementType_U1:
		case ElementType_U2:
		case ElementType_U4:
			switch(sBType)
			{
				case ElementType_I:
				case ElementType_U:
				case ElementType_I1:
				case ElementType_I2:
				case ElementType_I4:
				case ElementType_U1:
				case ElementType_U2:
				case ElementType_U4:
					destType = ElementType_I4;
					break;
				case ElementType_I8:
				case ElementType_U8:
					destType = ElementType_I8;
					break;
				case ElementType_R4:
				case ElementType_R8:
					Panic("Invalid parameter!");
					break;
			}
			break;
		case ElementType_I8:
		case ElementType_U8:
			destType = ElementType_I8;
			break;
		case ElementType_R4:
		case ElementType_R8:
			destType = ElementType_R8;
			break;
	}

	if (SourceTypeData_Equal(pInstruction->Source1, pInstruction->Destination))
	{
		if (pInstruction->Source2.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source1.Type == SourceType_Local)
			{
				x86_alu_membase_imm(pCompiledCode, X86_ADD, X86_EBP, -pMethod->LocalVariables[pInstruction->Source1.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
			}
			else if (pInstruction->Source1.Type == SourceType_Parameter)
			{
				x86_alu_membase_imm(pCompiledCode, X86_ADD, X86_EBP, pMethod->Parameters[pInstruction->Source1.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_imm(pCompiledCode, X86_ADD, PRIMARY_REG, pInstruction->Source2.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}
	else if (SourceTypeData_Equal(pInstruction->Source2, pInstruction->Destination))
	{
		if (pInstruction->Source1.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source2.Type == SourceType_Local)
			{
				x86_alu_membase_imm(pCompiledCode, X86_ADD, X86_EBP, -pMethod->LocalVariables[pInstruction->Source2.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
			}
			else if (pInstruction->Source2.Type == SourceType_Parameter)
			{
				x86_alu_membase_imm(pCompiledCode, X86_ADD, X86_EBP, pMethod->Parameters[pInstruction->Source2.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_imm(pCompiledCode, X86_ADD, PRIMARY_REG, pInstruction->Source1.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}

	switch(destType)
	{
		case ElementType_I4:
		{
			if (pInstruction->Destination.Type == SourceType_Local)
			{
				pCompiledCode = JIT_Emit_Move(pCompiledCode, pMethod, &pInstruction->Source2, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_membase_reg(pCompiledCode, X86_ADD, X86_EBP, -pMethod->LocalVariables[pInstruction->Destination.Data.LocalVariable.LocalVariableIndex]->Offset, PRIMARY_REG);
			}
			else if (pInstruction->Destination.Type == SourceType_Parameter)
			{
				pCompiledCode = JIT_Emit_Move(pCompiledCode, pMethod, &pInstruction->Source2, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_membase_reg(pCompiledCode, X86_ADD, X86_EBP, pMethod->Parameters[pInstruction->Destination.Data.Parameter.ParameterIndex]->Offset, PRIMARY_REG);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, FOURTH_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_reg(pCompiledCode, X86_ADD, FOURTH_REG, PRIMARY_REG);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, FOURTH_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			break;
		}
		case ElementType_I8:
		{
			switch(sAType)
			{
				case ElementType_I:
				case ElementType_U:
				case ElementType_I1:
				case ElementType_I2:
				case ElementType_I4:
				case ElementType_U1:
				case ElementType_U2:
				case ElementType_U4:
					// This means that operand 2 is 64-bit.
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					x86_fild_membase(pCompiledCode, X86_ESP, 0, TRUE);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
					x86_fp_int_op_membase(pCompiledCode, X86_FADD, X86_ESP, 0, TRUE);
					x86_fist_pop_membase(pCompiledCode, X86_ESP, 0, TRUE);
					pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					break;
				case ElementType_I8:
				case ElementType_U8:
				{
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					x86_fild_membase(pCompiledCode, X86_ESP, 0, TRUE);
					size_t arg2Size = 0;
					x86_adjust_stack(pCompiledCode, 8);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, &arg2Size);
					if (arg2Size <= 4)
					{
						x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
						x86_fp_int_op_membase(pCompiledCode, X86_FADD, X86_ESP, 0, TRUE);
						x86_adjust_stack(pCompiledCode, -4);
					}
					else
					{
						x86_fild_membase(pCompiledCode, X86_ESP, 0, TRUE);
						x86_fp_op(pCompiledCode, X86_FADD, 1);
					}
					x86_fist_pop_membase(pCompiledCode, X86_ESP, 0, TRUE);
					pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					break;
				}
				case ElementType_R4:
				case ElementType_R8:
					Panic("Invalid operand type!");
					break;
			}

			break;
		}
		case ElementType_R8:
		{
			switch(sAType)
			{
				case ElementType_I:
				case ElementType_U:
				case ElementType_I1:
				case ElementType_I2:
				case ElementType_I4:
				case ElementType_U1:
				case ElementType_U2:
				case ElementType_U4:
				case ElementType_I8:
				case ElementType_U8:
					Panic("Invalid operand type!");
					break;
				case ElementType_R4:
				case ElementType_R8:
				{
					size_t argSize = 0;
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, &argSize);
					if (argSize <= 4)
					{
						x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
						x86_fld_membase(pCompiledCode, X86_ESP, 0, FALSE);
						x86_adjust_stack(pCompiledCode, 4);
					}
					else
					{
						x86_fld_membase(pCompiledCode, X86_ESP, 0, TRUE);
						x86_adjust_stack(pCompiledCode, 8);
					}
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, &argSize);
					if (argSize <= 4)
					{
						x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
						x86_fld_membase(pCompiledCode, X86_ESP, 0, FALSE);
						x86_adjust_stack(pCompiledCode, -4);
					}
					else
					{
						x86_fld_membase(pCompiledCode, X86_ESP, 0, TRUE);
					}
					x86_fp_op(pCompiledCode, X86_FADD, 1);
					x86_fst_membase(pCompiledCode, X86_ESP, 0, TRUE, TRUE);
					pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					break;
				}
			}
			break;
		}
		default:
			Panic("Invalid destination type!");
			break;
	}

EmitFinished:
	return pCompiledCode;
}

char* JIT_Emit_Sub(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	switch((OverflowType)(uint32_t)pInstruction->Arg1)
	{
		case OverflowType_Signed:
		case OverflowType_Unsigned:
			Panic("Unsupported overflow type!");
			break;
		case OverflowType_None: break;
	}
	ElementType sAType = (ElementType)(uint32_t)pInstruction->Arg2;
	ElementType sBType = (ElementType)(uint32_t)pInstruction->Arg3;
	ElementType destType = (ElementType)0;
	switch(sAType)
	{
		case ElementType_I:
		case ElementType_U:
		case ElementType_I1:
		case ElementType_I2:
		case ElementType_I4:
		case ElementType_U1:
		case ElementType_U2:
		case ElementType_U4:
			switch(sBType)
			{
				case ElementType_I:
				case ElementType_U:
				case ElementType_I1:
				case ElementType_I2:
				case ElementType_I4:
				case ElementType_U1:
				case ElementType_U2:
				case ElementType_U4:
					destType = ElementType_I4;
					break;
				case ElementType_I8:
				case ElementType_U8:
					destType = ElementType_I8;
					break;
				case ElementType_R4:
				case ElementType_R8:
					Panic("Invalid parameter!");
					break;
			}
			break;
		case ElementType_I8:
		case ElementType_U8:
			destType = ElementType_I8;
			break;
		case ElementType_R4:
		case ElementType_R8:
			destType = ElementType_R8;
			break;
	}

	if (SourceTypeData_Equal(pInstruction->Source1, pInstruction->Destination))
	{
		if (pInstruction->Source2.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source1.Type == SourceType_Local)
			{
				x86_alu_membase_imm(pCompiledCode, X86_SUB, X86_EBP, -pMethod->LocalVariables[pInstruction->Source1.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
			}
			else if (pInstruction->Source1.Type == SourceType_Parameter)
			{
				x86_alu_membase_imm(pCompiledCode, X86_SUB, X86_EBP, pMethod->Parameters[pInstruction->Source1.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_imm(pCompiledCode, X86_SUB, PRIMARY_REG, pInstruction->Source2.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}
	else if (SourceTypeData_Equal(pInstruction->Source2, pInstruction->Destination))
	{
		if (pInstruction->Source1.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source2.Type == SourceType_Local)
			{
				x86_alu_membase_imm(pCompiledCode, X86_SUB, X86_EBP, -pMethod->LocalVariables[pInstruction->Source2.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
			}
			else if (pInstruction->Source2.Type == SourceType_Parameter)
			{
				x86_alu_membase_imm(pCompiledCode, X86_SUB, X86_EBP, pMethod->Parameters[pInstruction->Source2.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_imm(pCompiledCode, X86_SUB, PRIMARY_REG, pInstruction->Source1.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}

	switch(destType)
	{
		case ElementType_I4:
		{
			if (pInstruction->Destination.Type == SourceType_Local)
			{
				pCompiledCode = JIT_Emit_Move(pCompiledCode, pMethod, &pInstruction->Source2, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_membase_reg(pCompiledCode, X86_SUB, X86_EBP, -pMethod->LocalVariables[pInstruction->Destination.Data.LocalVariable.LocalVariableIndex]->Offset, PRIMARY_REG);
			}
			else if (pInstruction->Destination.Type == SourceType_Parameter)
			{
				pCompiledCode = JIT_Emit_Move(pCompiledCode, pMethod, &pInstruction->Source2, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_membase_reg(pCompiledCode, X86_SUB, X86_EBP, pMethod->Parameters[pInstruction->Destination.Data.Parameter.ParameterIndex]->Offset, PRIMARY_REG);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, FOURTH_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_reg(pCompiledCode, X86_SUB, FOURTH_REG, PRIMARY_REG);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, FOURTH_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			break;
		}
		case ElementType_I8:
		{
			switch(sAType)
			{
				case ElementType_I:
				case ElementType_U:
				case ElementType_I1:
				case ElementType_I2:
				case ElementType_I4:
				case ElementType_U1:
				case ElementType_U2:
				case ElementType_U4:
					// This means that operand 2 is 64-bit.
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					x86_fild_membase(pCompiledCode, X86_ESP, 0, TRUE);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
					x86_fp_int_op_membase(pCompiledCode, X86_FSUB, X86_ESP, 0, TRUE);
					x86_fist_pop_membase(pCompiledCode, X86_ESP, 0, TRUE);
					pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					break;
				case ElementType_I8:
				case ElementType_U8:
				{
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					x86_fild_membase(pCompiledCode, X86_ESP, 0, TRUE);
					size_t arg2Size = 0;
					x86_adjust_stack(pCompiledCode, 8);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, &arg2Size);
					if (arg2Size <= 4)
					{
						x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
						x86_fp_int_op_membase(pCompiledCode, X86_FSUB, X86_ESP, 0, TRUE);
						x86_adjust_stack(pCompiledCode, -4);
					}
					else
					{
						x86_fild_membase(pCompiledCode, X86_ESP, 0, TRUE);
						x86_fp_op(pCompiledCode, X86_FADD, 1);
					}
					x86_fist_pop_membase(pCompiledCode, X86_ESP, 0, TRUE);
					pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					break;
				}
				case ElementType_R4:
				case ElementType_R8:
					Panic("Invalid operand type!");
					break;
			}

			break;
		}
		case ElementType_R8:
		{
			switch(sAType)
			{
				case ElementType_I:
				case ElementType_U:
				case ElementType_I1:
				case ElementType_I2:
				case ElementType_I4:
				case ElementType_U1:
				case ElementType_U2:
				case ElementType_U4:
				case ElementType_I8:
				case ElementType_U8:
					Panic("Invalid operand type!");
					break;
				case ElementType_R4:
				case ElementType_R8:
				{
					size_t argSize = 0;
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, &argSize);
					if (argSize <= 4)
					{
						x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
						x86_fld_membase(pCompiledCode, X86_ESP, 0, FALSE);
						x86_adjust_stack(pCompiledCode, 4);
					}
					else
					{
						x86_fld_membase(pCompiledCode, X86_ESP, 0, TRUE);
						x86_adjust_stack(pCompiledCode, 8);
					}
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, &argSize);
					if (argSize <= 4)
					{
						x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
						x86_fld_membase(pCompiledCode, X86_ESP, 0, FALSE);
						x86_adjust_stack(pCompiledCode, -4);
					}
					else
					{
						x86_fld_membase(pCompiledCode, X86_ESP, 0, TRUE);
					}
					x86_fp_op(pCompiledCode, X86_FSUB, 1);
					x86_fst_membase(pCompiledCode, X86_ESP, 0, TRUE, TRUE);
					pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					break;
				}
			}
			break;
		}
		default:
			Panic("Invalid destination type!");
			break;
	}

EmitFinished:
	return pCompiledCode;
}

char* JIT_Emit_Mul(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	switch((OverflowType)(uint32_t)pInstruction->Arg1)
	{
		case OverflowType_Signed:
		case OverflowType_Unsigned:
			Panic("Unsupported overflow type!");
			break;
		case OverflowType_None: break;
	}
	ElementType sAType = (ElementType)(uint32_t)pInstruction->Arg2;
	ElementType sBType = (ElementType)(uint32_t)pInstruction->Arg3;
	ElementType destType = (ElementType)0;
	switch(sAType)
	{
		case ElementType_I:
		case ElementType_U:
		case ElementType_I1:
		case ElementType_I2:
		case ElementType_I4:
		case ElementType_U1:
		case ElementType_U2:
		case ElementType_U4:
			switch(sBType)
			{
				case ElementType_I:
				case ElementType_U:
				case ElementType_I1:
				case ElementType_I2:
				case ElementType_I4:
				case ElementType_U1:
				case ElementType_U2:
				case ElementType_U4:
					destType = ElementType_I4;
					break;
				case ElementType_I8:
				case ElementType_U8:
					destType = ElementType_I8;
					break;
				case ElementType_R4:
				case ElementType_R8:
					Panic("Invalid parameter!");
					break;
			}
			break;
		case ElementType_I8:
		case ElementType_U8:
			destType = ElementType_I8;
			break;
		case ElementType_R4:
		case ElementType_R8:
			destType = ElementType_R8;
			break;
	}

	if (SourceTypeData_Equal(pInstruction->Source1, pInstruction->Destination))
	{
		if (pInstruction->Source2.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source1.Type == SourceType_Local)
			{
				x86_imul_reg_membase_imm(pCompiledCode, PRIMARY_REG, X86_EBP, -pMethod->LocalVariables[pInstruction->Source1.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
				x86_mov_membase_reg(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pInstruction->Source1.Data.LocalVariable.LocalVariableIndex]->Offset, PRIMARY_REG, 4);
			}
			else if (pInstruction->Source1.Type == SourceType_Parameter)
			{
				x86_imul_reg_membase_imm(pCompiledCode, PRIMARY_REG, X86_EBP, pMethod->Parameters[pInstruction->Source1.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
				x86_mov_membase_reg(pCompiledCode, X86_EBP, pMethod->Parameters[pInstruction->Source1.Data.Parameter.ParameterIndex]->Offset, PRIMARY_REG, 4);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_imul_reg_reg_imm(pCompiledCode, PRIMARY_REG, PRIMARY_REG, pInstruction->Source2.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}
	else if (SourceTypeData_Equal(pInstruction->Source2, pInstruction->Destination))
	{
		if (pInstruction->Source1.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source2.Type == SourceType_Local)
			{
				x86_imul_reg_membase_imm(pCompiledCode, PRIMARY_REG, X86_EBP, -pMethod->LocalVariables[pInstruction->Source2.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
				x86_mov_membase_reg(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pInstruction->Source2.Data.LocalVariable.LocalVariableIndex]->Offset, PRIMARY_REG, 4);
			}
			else if (pInstruction->Source2.Type == SourceType_Parameter)
			{
				x86_imul_reg_membase_imm(pCompiledCode, PRIMARY_REG, X86_EBP, pMethod->Parameters[pInstruction->Source2.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
				x86_mov_membase_reg(pCompiledCode, X86_EBP, pMethod->Parameters[pInstruction->Source2.Data.Parameter.ParameterIndex]->Offset, PRIMARY_REG, 4);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_imul_reg_reg_imm(pCompiledCode, PRIMARY_REG, PRIMARY_REG, pInstruction->Source1.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}

	switch(destType)
	{
		case ElementType_I4:
		{
			if (pInstruction->Destination.Type == SourceType_Local)
			{
				pCompiledCode = JIT_Emit_Move(pCompiledCode, pMethod, &pInstruction->Source2, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_imul_reg_membase(pCompiledCode, PRIMARY_REG, X86_EBP, -pMethod->LocalVariables[pInstruction->Destination.Data.LocalVariable.LocalVariableIndex]->Offset);
				x86_mov_membase_reg(pCompiledCode, X86_EBP, -pMethod->LocalVariables[pInstruction->Destination.Data.LocalVariable.LocalVariableIndex]->Offset, PRIMARY_REG, 4);
			}
			else if (pInstruction->Destination.Type == SourceType_Parameter)
			{
				pCompiledCode = JIT_Emit_Move(pCompiledCode, pMethod, &pInstruction->Source2, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_imul_reg_membase(pCompiledCode, PRIMARY_REG, X86_EBP, pMethod->Parameters[pInstruction->Destination.Data.Parameter.ParameterIndex]->Offset);
				x86_mov_membase_reg(pCompiledCode, X86_EBP, pMethod->Parameters[pInstruction->Destination.Data.Parameter.ParameterIndex]->Offset, PRIMARY_REG, 4);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, FOURTH_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_imul_reg_reg(pCompiledCode, FOURTH_REG, PRIMARY_REG);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, FOURTH_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			break;
		}
		case ElementType_I8:
		{
			switch(sAType)
			{
				case ElementType_I:
				case ElementType_U:
				case ElementType_I1:
				case ElementType_I2:
				case ElementType_I4:
				case ElementType_U1:
				case ElementType_U2:
				case ElementType_U4:
					// This means that operand 2 is 64-bit.
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					x86_fild_membase(pCompiledCode, X86_ESP, 0, TRUE);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
					x86_fp_int_op_membase(pCompiledCode, X86_FMUL, X86_ESP, 0, TRUE);
					x86_fist_pop_membase(pCompiledCode, X86_ESP, 0, TRUE);
					pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					break;
				case ElementType_I8:
				case ElementType_U8:
				{
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					x86_fild_membase(pCompiledCode, X86_ESP, 0, TRUE);
					size_t arg2Size = 0;
					x86_adjust_stack(pCompiledCode, 8);
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, &arg2Size);
					if (arg2Size <= 4)
					{
						x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
						x86_fp_int_op_membase(pCompiledCode, X86_FMUL, X86_ESP, 0, TRUE);
						x86_adjust_stack(pCompiledCode, -4);
					}
					else
					{
						x86_fild_membase(pCompiledCode, X86_ESP, 0, TRUE);
						x86_fp_op(pCompiledCode, X86_FMUL, 1);
					}
					x86_fist_pop_membase(pCompiledCode, X86_ESP, 0, TRUE);
					pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					break;
				}
				case ElementType_R4:
				case ElementType_R8:
					Panic("Invalid operand type!");
					break;
			}

			break;
		}
		case ElementType_R8:
		{
			switch(sAType)
			{
				case ElementType_I:
				case ElementType_U:
				case ElementType_I1:
				case ElementType_I2:
				case ElementType_I4:
				case ElementType_U1:
				case ElementType_U2:
				case ElementType_U4:
				case ElementType_I8:
				case ElementType_U8:
					Panic("Invalid operand type!");
					break;
				case ElementType_R4:
				case ElementType_R8:
				{
					size_t argSize = 0;
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, &argSize);
					if (argSize <= 4)
					{
						x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
						x86_fld_membase(pCompiledCode, X86_ESP, 0, FALSE);
						x86_adjust_stack(pCompiledCode, 4);
					}
					else
					{
						x86_fld_membase(pCompiledCode, X86_ESP, 0, TRUE);
						x86_adjust_stack(pCompiledCode, 8);
					}
					pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, &argSize);
					if (argSize <= 4)
					{
						x86_mov_membase_reg(pCompiledCode, X86_ESP, 0, PRIMARY_REG, 4);
						x86_fld_membase(pCompiledCode, X86_ESP, 0, FALSE);
						x86_adjust_stack(pCompiledCode, -4);
					}
					else
					{
						x86_fld_membase(pCompiledCode, X86_ESP, 0, TRUE);
					}
					x86_fp_op(pCompiledCode, X86_FMUL, 1);
					x86_fst_membase(pCompiledCode, X86_ESP, 0, TRUE, TRUE);
					pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
					break;
				}
			}
			break;
		}
		default:
			Panic("Invalid destination type!");
			break;
	}

EmitFinished:
	return pCompiledCode;
}

char* JIT_Emit_Div(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Rem(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_And(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	if (SourceTypeData_Equal(pInstruction->Source1, pInstruction->Destination))
	{
		if (pInstruction->Source2.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source1.Type == SourceType_Local)
			{
				x86_alu_membase_imm(pCompiledCode, X86_AND, X86_EBP, -pMethod->LocalVariables[pInstruction->Source1.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
			}
			else if (pInstruction->Source1.Type == SourceType_Parameter)
			{
				x86_alu_membase_imm(pCompiledCode, X86_AND, X86_EBP, pMethod->Parameters[pInstruction->Source1.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_imm(pCompiledCode, X86_AND, PRIMARY_REG, pInstruction->Source2.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}
	else if (SourceTypeData_Equal(pInstruction->Source2, pInstruction->Destination))
	{
		if (pInstruction->Source1.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source2.Type == SourceType_Local)
			{
				x86_alu_membase_imm(pCompiledCode, X86_AND, X86_EBP, -pMethod->LocalVariables[pInstruction->Source2.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
			}
			else if (pInstruction->Source2.Type == SourceType_Parameter)
			{
				x86_alu_membase_imm(pCompiledCode, X86_AND, X86_EBP, pMethod->Parameters[pInstruction->Source2.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_imm(pCompiledCode, X86_AND, PRIMARY_REG, pInstruction->Source1.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}

	uint32_t aAS = 0;
	pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, (size_t*)&aAS);
	pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, FOURTH_REG, SECONDARY_REG, THIRD_REG, NULL);
	switch(aAS)
	{
		case 4:
			x86_alu_reg_reg(pCompiledCode, X86_AND, PRIMARY_REG, FOURTH_REG);
			break;
		case 8:
			x86_mov_reg_membase(pCompiledCode, SECONDARY_REG, X86_ESP, 0, 4);
			x86_alu_membase_reg(pCompiledCode, X86_AND, X86_ESP, 8, SECONDARY_REG);
			x86_mov_reg_membase(pCompiledCode, SECONDARY_REG, X86_ESP, 4, 4);
			x86_alu_membase_reg(pCompiledCode, X86_AND, X86_ESP, 12, SECONDARY_REG);
			x86_adjust_stack(pCompiledCode, 8);
			break;
	}
	pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
EmitFinished:
	return pCompiledCode;
}

char* JIT_Emit_Or(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	if (SourceTypeData_Equal(pInstruction->Source1, pInstruction->Destination))
	{
		if (pInstruction->Source2.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source1.Type == SourceType_Local)
			{
				x86_alu_membase_imm(pCompiledCode, X86_OR, X86_EBP, -pMethod->LocalVariables[pInstruction->Source1.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
			}
			else if (pInstruction->Source1.Type == SourceType_Parameter)
			{
				x86_alu_membase_imm(pCompiledCode, X86_OR, X86_EBP, pMethod->Parameters[pInstruction->Source1.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_imm(pCompiledCode, X86_OR, PRIMARY_REG, pInstruction->Source2.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}
	else if (SourceTypeData_Equal(pInstruction->Source2, pInstruction->Destination))
	{
		if (pInstruction->Source1.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source2.Type == SourceType_Local)
			{
				x86_alu_membase_imm(pCompiledCode, X86_OR, X86_EBP, -pMethod->LocalVariables[pInstruction->Source2.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
			}
			else if (pInstruction->Source2.Type == SourceType_Parameter)
			{
				x86_alu_membase_imm(pCompiledCode, X86_OR, X86_EBP, pMethod->Parameters[pInstruction->Source2.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_imm(pCompiledCode, X86_OR, PRIMARY_REG, pInstruction->Source1.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}

	uint32_t aAS = 0;
	pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, (size_t*)&aAS);
	pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, FOURTH_REG, SECONDARY_REG, THIRD_REG, NULL);
	switch(aAS)
	{
		case 4:
			x86_alu_reg_reg(pCompiledCode, X86_OR, PRIMARY_REG, FOURTH_REG);
			break;
		case 8:
			x86_mov_reg_membase(pCompiledCode, SECONDARY_REG, X86_ESP, 0, 4);
			x86_alu_membase_reg(pCompiledCode, X86_OR, X86_ESP, 8, SECONDARY_REG);
			x86_mov_reg_membase(pCompiledCode, SECONDARY_REG, X86_ESP, 4, 4);
			x86_alu_membase_reg(pCompiledCode, X86_OR, X86_ESP, 12, SECONDARY_REG);
			x86_adjust_stack(pCompiledCode, 8);
			break;
	}
	pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
EmitFinished:
	return pCompiledCode;
}

char* JIT_Emit_Xor(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	if (SourceTypeData_Equal(pInstruction->Source1, pInstruction->Destination))
	{
		if (pInstruction->Source2.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source1.Type == SourceType_Local)
			{
				x86_alu_membase_imm(pCompiledCode, X86_XOR, X86_EBP, -pMethod->LocalVariables[pInstruction->Source1.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
			}
			else if (pInstruction->Source1.Type == SourceType_Parameter)
			{
				x86_alu_membase_imm(pCompiledCode, X86_XOR, X86_EBP, pMethod->Parameters[pInstruction->Source1.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source2.Data.ConstantI4.Value);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_imm(pCompiledCode, X86_XOR, PRIMARY_REG, pInstruction->Source2.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}
	else if (SourceTypeData_Equal(pInstruction->Source2, pInstruction->Destination))
	{
		if (pInstruction->Source1.Type == SourceType_ConstantI4)
		{
			if (pInstruction->Source2.Type == SourceType_Local)
			{
				x86_alu_membase_imm(pCompiledCode, X86_XOR, X86_EBP, -pMethod->LocalVariables[pInstruction->Source2.Data.LocalVariable.LocalVariableIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
			}
			else if (pInstruction->Source2.Type == SourceType_Parameter)
			{
				x86_alu_membase_imm(pCompiledCode, X86_XOR, X86_EBP, pMethod->Parameters[pInstruction->Source2.Data.Parameter.ParameterIndex]->Offset, pInstruction->Source1.Data.ConstantI4.Value);
			}
			else
			{
				pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
				x86_alu_reg_imm(pCompiledCode, X86_XOR, PRIMARY_REG, pInstruction->Source1.Data.ConstantI4.Value);
				pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
			}
			goto EmitFinished;
		}
	}

	uint32_t aAS = 0;
	pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, (size_t*)&aAS);
	pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source2, FOURTH_REG, SECONDARY_REG, THIRD_REG, NULL);
	switch(aAS)
	{
		case 4:
			x86_alu_reg_reg(pCompiledCode, X86_XOR, PRIMARY_REG, FOURTH_REG);
			break;
		case 8:
			x86_mov_reg_membase(pCompiledCode, SECONDARY_REG, X86_ESP, 0, 4);
			x86_alu_membase_reg(pCompiledCode, X86_XOR, X86_ESP, 8, SECONDARY_REG);
			x86_mov_reg_membase(pCompiledCode, SECONDARY_REG, X86_ESP, 4, 4);
			x86_alu_membase_reg(pCompiledCode, X86_XOR, X86_ESP, 12, SECONDARY_REG);
			x86_adjust_stack(pCompiledCode, 8);
			break;
	}
	pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
EmitFinished:
	return pCompiledCode;
}

char* JIT_Emit_Neg(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	if (SourceTypeData_Equal(pInstruction->Source1, pInstruction->Destination))
	{
		if (pInstruction->Source1.Type == SourceType_Local)
		{
			IRLocalVariable* local = pMethod->LocalVariables[pInstruction->Source1.Data.LocalVariable.LocalVariableIndex];
			switch(local->Size)
			{
				case 1:
				case 2:
				case 3:
				case 4:
					x86_neg_membase(pCompiledCode, X86_EBP, -local->Offset);
					break;
				case 5:
				case 6:
				case 7:
				case 8:
					switch((ElementType)(int)pInstruction->Arg1)
					{
						case ElementType_I8:
						case ElementType_U8:
							x86_fild_membase(pCompiledCode, X86_EBP, -local->Offset, TRUE);
							x86_fchs(pCompiledCode);
							x86_fist_pop_membase(pCompiledCode, X86_EBP, -local->Offset, TRUE);
							break;
						case ElementType_R4:
							x86_fld_membase(pCompiledCode, X86_EBP, -local->Offset, FALSE);
							x86_fchs(pCompiledCode);
							x86_fst_membase(pCompiledCode, X86_EBP, -local->Offset, FALSE, TRUE);
							break;
						case ElementType_R8:
							x86_fld_membase(pCompiledCode, X86_EBP, -local->Offset, TRUE);
							x86_fchs(pCompiledCode);
							x86_fst_membase(pCompiledCode, X86_EBP, -local->Offset, TRUE, TRUE);
							break;
						default:
							Panic("Invalid large operand for Negate!");
							break;
					}
					break;
			}
			goto EmitFinished;
		}
		else if (pInstruction->Source1.Type == SourceType_Parameter)
		{
			IRParameter* param = pMethod->Parameters[pInstruction->Source1.Data.Parameter.ParameterIndex];
			switch(param->Size)
			{
				case 1:
				case 2:
				case 3:
				case 4:
					x86_neg_membase(pCompiledCode, X86_EBP, param->Offset);
					break;
				case 5:
				case 6:
				case 7:
				case 8:
					switch((ElementType)(int)pInstruction->Arg1)
					{
						case ElementType_I8:
						case ElementType_U8:
							x86_fild_membase(pCompiledCode, X86_EBP, param->Offset, TRUE);
							x86_fchs(pCompiledCode);
							x86_fist_pop_membase(pCompiledCode, X86_EBP, param->Offset, TRUE);
							break;
						case ElementType_R4:
							x86_fld_membase(pCompiledCode, X86_EBP, param->Offset, FALSE);
							x86_fchs(pCompiledCode);
							x86_fst_membase(pCompiledCode, X86_EBP, param->Offset, FALSE, TRUE);
							break;
						case ElementType_R8:
							x86_fld_membase(pCompiledCode, X86_EBP, param->Offset, TRUE);
							x86_fchs(pCompiledCode);
							x86_fst_membase(pCompiledCode, X86_EBP, param->Offset, TRUE, TRUE);
							break;
						default:
							Panic("Invalid large operand for Negate!");
							break;
					}
					break;
			}
			goto EmitFinished;
		}
	}
	
	pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
	switch((ElementType)(int)pInstruction->Arg1)
	{
		case ElementType_I:
		case ElementType_I1:
		case ElementType_I2:
		case ElementType_I4:
		case ElementType_U:
		case ElementType_U1:
		case ElementType_U2:
		case ElementType_U4:
			x86_neg_reg(pCompiledCode, PRIMARY_REG);
			break;
		case ElementType_I8:
		case ElementType_U8:
			x86_fild_membase(pCompiledCode, X86_ESP, 0, TRUE);
			x86_fchs(pCompiledCode);
			x86_fist_pop_membase(pCompiledCode, X86_ESP, 0, TRUE);
			break;
		case ElementType_R4:
			x86_fld_membase(pCompiledCode, X86_ESP, 0, FALSE);
			x86_fchs(pCompiledCode);
			x86_fst_membase(pCompiledCode, X86_ESP, 0, FALSE, TRUE);
			break;
		case ElementType_R8:
			x86_fld_membase(pCompiledCode, X86_ESP, 0, TRUE);
			x86_fchs(pCompiledCode);
			x86_fst_membase(pCompiledCode, X86_ESP, 0, TRUE, TRUE);
			break;
	}
	pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
EmitFinished:
	return pCompiledCode;
}

char* JIT_Emit_Not(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	if (SourceTypeData_Equal(pInstruction->Source1, pInstruction->Destination))
	{
		if (pInstruction->Source1.Type == SourceType_Local)
		{
			IRLocalVariable* local = pMethod->LocalVariables[pInstruction->Source1.Data.LocalVariable.LocalVariableIndex];
			switch(local->Size)
			{
				case 1:
				case 2:
				case 3:
				case 4:
					x86_not_membase(pCompiledCode, X86_EBP, -local->Offset);
					break;
				case 5:
				case 6:
				case 7:
				case 8:
					x86_not_membase(pCompiledCode, X86_EBP, -local->Offset);
					x86_not_membase(pCompiledCode, X86_EBP, -local->Offset + 4);
					break;
			}
			goto EmitFinished;
		}
		else if (pInstruction->Source1.Type == SourceType_Parameter)
		{
			IRParameter* param = pMethod->Parameters[pInstruction->Source1.Data.Parameter.ParameterIndex];
			switch(param->Size)
			{
				case 1:
				case 2:
				case 3:
				case 4:
					x86_not_membase(pCompiledCode, X86_EBP, param->Offset);
					break;
				case 5:
				case 6:
				case 7:
				case 8:
					x86_not_membase(pCompiledCode, X86_EBP, param->Offset);
					x86_not_membase(pCompiledCode, X86_EBP, param->Offset + 4);
					break;
			}
			goto EmitFinished;
		}
	}
	
	pCompiledCode = JIT_Emit_Load(pCompiledCode, pMethod, &pInstruction->Source1, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
	switch((ElementType)(int)pInstruction->Arg1)
	{
		case ElementType_I:
		case ElementType_I1:
		case ElementType_I2:
		case ElementType_I4:
		case ElementType_U:
		case ElementType_U1:
		case ElementType_U2:
		case ElementType_U4:
			x86_not_reg(pCompiledCode, PRIMARY_REG);
			break;
		case ElementType_I8:
		case ElementType_U8:
			x86_not_membase(pCompiledCode, X86_ESP, 0);
			x86_not_membase(pCompiledCode, X86_ESP, 4);
			break;
		default:
			Panic("Invalid type for Not instruction!");
			break;
	}
	pCompiledCode = JIT_Emit_Store(pCompiledCode, pMethod, &pInstruction->Destination, PRIMARY_REG, SECONDARY_REG, THIRD_REG, NULL);
EmitFinished:
	return pCompiledCode;
}

char* JIT_Emit_Shift(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Convert_Unchecked(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Convert_Checked(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_CastClass(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_IsInst(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Unbox(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_UnboxAny(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Box(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Throw(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Copy_Object(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_New_Object(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_New_Array(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_CheckFinite(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Allocate_Local(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Initialize_Object(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Copy_Block(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Initialize_Block(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Jump(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Call_Virtual(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Call_Constrained(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Call_Absolute(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Call_Internal(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Branch(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Switch(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Load_Function(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Load_VirtualFunction(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Compare(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_Load_Token(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_MkRefAny(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_RefAnyVal(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

char* JIT_Emit_RefAnyType(char* pCompiledCode, IRMethod* pMethod, IRInstruction* pInstruction, BranchRegistry* pBranchRegistry)
{
	return pCompiledCode;
}

