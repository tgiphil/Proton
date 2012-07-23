#include "Metadata.h"

CLR::AssemblyDefinitionData::AssemblyDefinitionData()
{
	TableIndex = 0;
	HashAlgorithmID = 0;
	MajorVersion = 0;
	MinorVersion = 0;
	Build = 0;
	Revision = 0;
	Flags = 0;
	PublicKeyLength = 0;
	PublicKey = nullptr;
	Name = nullptr;
	Culture = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
	DeclSecurity = nullptr;
}

CLR::AssemblyOperatingSystemData::AssemblyOperatingSystemData()
{
	TableIndex = 0;
	PlatformID = 0;
	MajorVersion = 0;
	MinorVersion = 0;
}

CLR::AssemblyProcessorData::AssemblyProcessorData()
{
	TableIndex = 0;
	Processor = 0;
}

CLR::AssemblyReferenceData::AssemblyReferenceData()
{
	TableIndex = 0;
	MajorVersion = 0;
	MinorVersion = 0;
	Build = 0;
	Revision = 0;
	Flags = 0;
	PublicKeyOrTokenLength = 0;
	PublicKeyOrToken = nullptr;
	Name = nullptr;
	Culture = nullptr;
	HashValueLength = 0;
	HashValue = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
}

CLR::AssemblyReferenceOperatingSystemData::AssemblyReferenceOperatingSystemData()
{
	TableIndex = 0;
	PlatformID = 0;
	MajorVersion = 0;
	MinorVersion = 0;
	AssemblyReference = nullptr;
}

CLR::AssemblyReferenceProcessorData::AssemblyReferenceProcessorData()
{
	TableIndex = 0;
	Processor = 0;
	AssemblyReference = nullptr;
}

CLR::ClassLayoutData::ClassLayoutData()
{
	TableIndex = 0;
	PackingSize = 0;
	ClassSize = 0;
	Parent = nullptr;
}

CLR::ConstantData::ConstantData()
{
	TableIndex = 0;
	Type = 0;
	Parent.Field = nullptr;
	ValueLength = 0;
	Value = nullptr;
}

CLR::CustomAttributeData::CustomAttributeData()
{
	TableIndex = 0;
	Parent.AssemblyDefinition = nullptr;
	Type.MethodDefinition = nullptr;
	ValueLength = 0;
	Value = nullptr;
}

CLR::DeclSecurityData::DeclSecurityData()
{
	TableIndex = 0;
	Action = 0;
	Parent.TypeDefinition = nullptr;
	PermissionSetLength = 0;
	PermissionSet = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
}

CLR::EventData::EventData()
{
	TableIndex = 0;
	Flags = 0;
	Name = nullptr;
	EventType.TypeDefinition = nullptr;
}

CLR::EventMapData::EventMapData()
{
	TableIndex = 0;
	Parent = nullptr;
	EventList = nullptr;
	EventListCount = 0;
}

CLR::ExportedTypeData::ExportedTypeData()
{
	TableIndex = 0;
	Flags = 0;
	TypeDefinitionID = nullptr;
	Name = nullptr;
	Namespace = nullptr;
	Implementation.File = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
}

CLR::FieldData::FieldData()
{
	File = nullptr;
	TableIndex = 0;
	Flags = 0;
	Name = nullptr;
	SignatureLength = 0;
	Signature = nullptr;
	SignatureCache = nullptr;

	Constant = nullptr;
	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
	FieldMarshal = nullptr;
	ImplementationMap = nullptr;
	TypeDefinition = nullptr;
}

CLR::FieldLayoutData::FieldLayoutData()
{
	TableIndex = 0;
	Offset = 0;
	Field = nullptr;
}

CLR::FieldMarshalData::FieldMarshalData()
{
	TableIndex = 0;
	Parent.Field = nullptr;
	NativeTypeLength = 0;
	NativeType = nullptr;
}

CLR::FieldRVAData::FieldRVAData()
{
	TableIndex = 0;
	InitialValue = nullptr;
	Field = nullptr;
}

CLR::FileData::FileData()
{
	TableIndex = 0;
	Flags = 0;
	Name = nullptr;
	HashValueLength = 0;
	HashValue = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
}

CLR::GenericParameterData::GenericParameterData()
{
	TableIndex = 0;
	Index = 0;
	Flags = 0;
	Owner.TypeDefinition = nullptr;
	Name = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
	GenericParameterConstraintCount = 0;
	GenericParameterConstraints = nullptr;
}

CLR::GenericParameterConstraintData::GenericParameterConstraintData()
{
	TableIndex = 0;
	Owner = nullptr;
	Constraint.TypeDefinition = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
}

CLR::ImplementationMapData::ImplementationMapData()
{
	TableIndex = 0;
	MappingFlags = 0;
	MemberForwarded.Field = nullptr;
	ImportName = nullptr;
	ImportScope = nullptr;
}

CLR::InterfaceImplementationData::InterfaceImplementationData()
{
	TableIndex = 0;
	Implementor = nullptr;
	Interface.TypeDefinition = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
}

CLR::ManifestResourceData::ManifestResourceData()
{
	TableIndex = 0;
	Offset = 0;
	Flags = 0;
	Name = nullptr;
	Implementation.File = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
}

CLR::MemberReferenceData::MemberReferenceData()
{
	TableIndex = 0;
	Parent.TypeDefinition = nullptr;
	Name = nullptr;
	SignatureLength = 0;
	Signature = nullptr;
	MethodSignatureCache = nullptr;
	FieldSignatureCache = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;

	TypeOfResolved = 0;
	Resolved.MethodDefinition = nullptr;
}

CLR::MethodDefinitionBody::MethodDefinitionBody()
{
	Flags = 0;
	IsFat = false;
	MaxStack = 0;
	CodeSize = 0;
	LocalVariableSignatureToken = 0;
	Code = nullptr;
}

CLR::MethodDefinitionException::MethodDefinitionException()
{
	Flags = 0;
	TryOffset = 0;
	TryLength = 0;
	HandlerOffset = 0;
	HandlerLength = 0;
	ClassTokenOrFilterOffset = 0;
}

CLR::MethodDefinitionData::MethodDefinitionData()
{
	File = nullptr;
	TableIndex = 0;
	ExceptionCount = 0;
	Exceptions = nullptr;
	ImplFlags = 0;
	Flags = 0;
	Name = nullptr;
	SignatureLength = 0;
	Signature = nullptr;
	SignatureCache = nullptr;
	ParameterList = nullptr;
	ParameterListCount = 0;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
	DeclSecurity = nullptr;
	GenericParameterCount = 0;
	GenericParameters = nullptr;
	ImplementationMap = nullptr;
	MemberReferenceCount = 0;
	MemberReferences = nullptr;
	TypeDefinition = nullptr;

	InternalCall = nullptr;
}

CLR::MethodImplementationData::MethodImplementationData()
{
	TableIndex = 0;
	Parent = nullptr;
	MethodBody.MethodDefinition = nullptr;
	MethodDeclaration.MethodDefinition = nullptr;
}

CLR::MethodSemanticsData::MethodSemanticsData()
{
	TableIndex = 0;
	Semantics = 0;
	Method = nullptr;
	Association.Event = nullptr;
}

CLR::MethodSpecificationData::MethodSpecificationData()
{
	TableIndex = 0;
	Method.MethodDefinition = nullptr;
	InstantiationLength = 0;
	Instantiation = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
}

CLR::ModuleDefinitionData::ModuleDefinitionData()
{
	TableIndex = 0;
	Generation = 0;
	Name = nullptr;
	ModuleVersionID = nullptr;
	EncID = nullptr;
	EncBaseID = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
}

CLR::ModuleReferenceData::ModuleReferenceData()
{
	TableIndex = 0;
	Name = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
	MemberReferenceCount = 0;
	MemberReferences = nullptr;
}

CLR::NestedClassData::NestedClassData()
{
	TableIndex = 0;
	Nested = nullptr;
	Enclosing = nullptr;
}

CLR::ParameterData::ParameterData()
{
	TableIndex = 0;
	Flags = 0;
	Sequence = 0;
	Name = nullptr;

	Constant = nullptr;
	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
	FieldMarshal = nullptr;
}

CLR::PropertyData::PropertyData()
{
	TableIndex = 0;
	Flags = 0;
	Name = nullptr;
	SignatureLength = 0;
	Signature = nullptr;

	Constant = nullptr;
	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
	MethodSemantics = nullptr;
}

CLR::PropertyMapData::PropertyMapData()
{
	TableIndex = 0;
	Parent = nullptr;
	PropertyList = nullptr;
	PropertyListCount = 0;
}

CLR::StandAloneSignatureData::StandAloneSignatureData()
{
	TableIndex = 0;
	SignatureLength = 0;
	Signature = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
}

CLR::TypeDefinitionData::TypeDefinitionData()
{
	File = nullptr;
	TableIndex = 0;
	Flags = 0;
	Name = nullptr;
	Namespace = nullptr;
	Extends.TypeDefinition = nullptr;
	FieldList = nullptr;
	FieldListCount = 0;
	MethodDefinitionList = nullptr;
	MethodDefinitionListCount = 0;

	ClassLayout = nullptr;
	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
	DeclSecurity = nullptr;
	EventMap = nullptr;
	GenericParameterCount = 0;
	GenericParameters = nullptr;
	InterfaceImplementationCount = 0;
	InterfaceImplementations = nullptr;
	MemberReferenceCount = 0;
	MemberReferences = nullptr;
	MethodImplementationCount = 0;
	MethodImplementations = nullptr;
	NestedClassCount = 0;
	NestedClasses = nullptr;
	PropertyMap = nullptr;
}

CLR::TypeReferenceData::TypeReferenceData()
{
	TableIndex = 0;
	ResolutionScope.ModuleDefinition = nullptr;
	ExportedType = nullptr;
	Name = nullptr;
	Namespace = nullptr;
	ResolvedType = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
	MemberReferenceCount = 0;
	MemberReferences = nullptr;
}

CLR::TypeSpecificationData::TypeSpecificationData()
{
	File = nullptr;
	TableIndex = 0;
	SignatureLength = 0;
	Signature = nullptr;

	CustomAttributeCount = 0;
	CustomAttributes = nullptr;
	MemberReferenceCount = 0;
	MemberReferences = nullptr;
}

CLR::MethodSignature::MethodSignature()
{
	HasThis = false;
	ExplicitThis = false;
	Default = false;
	VarArgs = false;
	Generic = false;
	CCall = false;
	STDCall = false;
	ThisCall = false;
	FastCall = false;
	GenericParameterCount = 0;
	ReturnType = nullptr;
	ParameterCount = 0;
	Parameters = nullptr;
	HasSentinel = false;
	SentinelIndex = 0;
}

CLR::FieldSignature::FieldSignature()
{
	CustomModifierCount = 0;
	CustomModifiers = nullptr;
	Type = nullptr;
}

CLR::PropertySignature::PropertySignature()
{
	HasThis = false;
	CustomModifierCount = 0;
	CustomModifiers = nullptr;
	Type = nullptr;
	ParameterCount = 0;
	Parameters = nullptr;
}

CLR::LocalsSignature::LocalsSignature()
{
	LocalVariableCount = 0;
	LocalVariables = nullptr;
}

CLR::SignatureReturnType::SignatureReturnType()
{
	CustomModifierCount = 0;
	CustomModifiers = nullptr;
	ByReference = false;
	Type = nullptr;
	TypedByReference = false;
	Void = false;
}

CLR::SignatureParameter::SignatureParameter()
{
	CustomModifierCount = 0;
	CustomModifiers = nullptr;
	ByReference = false;
	Type = nullptr;
	TypedByReference = false;
}

CLR::SignatureCustomModifier::SignatureCustomModifier()
{
	Optional = false;
	TypeDefOrRefOrSpecToken = 0;
}

CLR::SignatureType::SignatureType()
{
	ElementType = 0;
	GenericInstTypeDefOrRefOrSpecToken = 0;
	GenericInstGenericArgumentCount = 0;
	GenericInstGenericArguments = nullptr;
	GenericInstClass = false;
	GenericInstValue = false;
}

CLR::SignatureMethodSpecification::SignatureMethodSpecification()
{
	GenericInstGenericArgumentCount = 0;
	GenericInstGenericArguments = nullptr;
}

CLR::SignatureArrayShape::SignatureArrayShape()
{
	Rank = 0;
	SizeCount = 0;
	Sizes = nullptr;
	LowerBoundCount = 0;
	LowerBounds = nullptr;
}

CLR::SignatureLocalVariable::SignatureLocalVariable()
{
	CustomModifierCount = 0;
	CustomModifiers = nullptr;
	ByReference = false;
	Type = nullptr;
	TypedByReference = false;
	IsPinned = false;
}
