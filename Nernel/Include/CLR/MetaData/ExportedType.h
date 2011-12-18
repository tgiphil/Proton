#pragma once

#include <CLR/MetaData/MetaData.h>

struct _ExportedType
{
    uint32_t Flags;
    TypeDefinition* TypeDefinitionID;
    const char* Name;
    const char* Namespace;
    ImplementationUnion(Implementation)
};

const uint8_t* ExportedType_Initialize(CLIFile* pFile, const uint8_t* pTableData);
void ExportedType_Cleanup(CLIFile* pFile);
const uint8_t* ExportedType_Load(CLIFile* pFile, const uint8_t* pTableData);
