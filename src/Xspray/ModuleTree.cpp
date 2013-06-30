//
//  ModuleTree.cpp
//  Xspray
//
//  Created by Sébastien Métrot on 6/15/13.
//
//

#include "Xspray.h"

using namespace Xspray;

ModuleTree::ModuleTree(const lldb::SBTarget& rTarget)
: nuiTreeNode(NULL, false, false, true, false), mTarget(rTarget), mType(eTarget)
{
  //SetTrace(true);
}

ModuleTree::ModuleTree(const lldb::SBModule& rModule, Type type)
: nuiTreeNode(NULL, false, false, true, false), mModule(rModule), mType(type)
{
  //SetTrace(true);
  nuiLabel* pLabel = NULL;
  switch (mType)
  {
    case eCompileUnitFolder:
      pLabel = new nuiLabel("Files");
      break;
    case eSymbolFolder:
      pLabel = new nuiLabel("Symbols");
      break;
    default:
      break;
  }
  if (pLabel)
    SetElement(pLabel);
}

ModuleTree::ModuleTree(const lldb::SBCompileUnit& rCompileUnit)
: nuiTreeNode(NULL, false, false, false, false), mCompileUnit(rCompileUnit), mType(eCompileUnit)
{
  Update();
}

ModuleTree::ModuleTree(const lldb::SBSymbol& rSymbol)
: nuiTreeNode(NULL, false, false, false, false), mSymbol(rSymbol), mType(eSymbol)
{
  Update();
}

ModuleTree::~ModuleTree()
{

}

bool ModuleTree::IsEmpty() const
{
  if (mType == eSymbol || mType == eCompileUnit)
    return true;
  if (mType == eCompileUnitFolder)
    return mModule.GetNumCompileUnits() == 0;
  if (mType == eSymbolFolder)
    return mModule.GetNumSymbols() == 0;

  return false;
}

void ModuleTree::Open(bool Opened)
{
  if (Opened && !mOpened)
  {
    Update();
  }
  else
  {
    Clear();
  }
  nuiTreeNode::Open(Opened);
}

const lldb::SBTarget& ModuleTree::GetTarget() const
{
  return mTarget;
}

const lldb::SBModule& ModuleTree::GetModule() const
{
  return mModule;
}

void ModuleTree::Update()
{
  switch (mType)
  {
    case eTarget:
      UpdateTarget();
      break;
    case eModule:
      UpdateModule();
      break;
    case eCompileUnit:
      UpdateCompileUnit();
      break;
    case eSymbol:
      UpdateSymbol();
      break;
    case eCompileUnitFolder:
      UpdateCompileUnitFolder();
      break;
    case eSymbolFolder:
      UpdateSymbolFolder();
      break;
    default:
      NGL_ASSERT(0);
  }
}

void ModuleTree::UpdateTarget()
{
  lldb::SBFileSpec f = mTarget.GetExecutable();
  nglString str;
  str.CFormat("Target %s", f.GetFilename());
  //  NGL_OUT("%s\n", str.GetChars());
  nglPath p(f.GetDirectory());
  p += nglString(f.GetFilename());
  nuiLabel* pLabel = new nuiLabel(str);
  pLabel->SetToolTip(p.GetChars());
  SetElement(pLabel);

  int modules = mTarget.GetNumModules();
  for (int i = 0; i < modules; i++)
  {
    ModuleTree* pPT = new ModuleTree(mTarget.GetModuleAtIndex(i), eModule);
    AddChild(pPT);
    pPT->Open(true);
  }
}

void ModuleTree::UpdateModule()
{
 lldb::SBFileSpec f = mModule.GetFileSpec();
  nglString str(f.GetFilename());
  NGL_OUT("module: %s\n", str.GetChars());
  nglPath p(f.GetDirectory());
  p += nglString(f.GetFilename());
  nuiLabel* pLabel = new nuiLabel(str);
  pLabel->SetToolTip(p.GetChars());
  SetElement(pLabel);

  if (mModule.GetNumCompileUnits() != 0)
  {
    ModuleTree* pFiles = new ModuleTree(mModule, eCompileUnitFolder);
    AddChild(pFiles);
  }

  if (mModule.GetNumSymbols() != 0)
  {
    ModuleTree* pSymbols = new ModuleTree(mModule, eSymbolFolder);
    AddChild(pSymbols);
  }
}

void ModuleTree::UpdateCompileUnitFolder()
{
  uint32_t compileunits = mModule.GetNumCompileUnits();
  for (uint32_t i = 0; i < compileunits; i++)
  {
    ModuleTree* pPT = new ModuleTree(mModule.GetCompileUnitAtIndex(i));
    AddChild(pPT);
  }
}

void ModuleTree::UpdateSymbolFolder()
{
  {
    uint32_t symbols = mModule.GetNumSymbols();
    int skipped = 0;
    for (uint32_t i = 0; i < symbols; i++)
    {
      lldb::SBSymbol symbol = mModule.GetSymbolAtIndex(i);
    }
  }

  lldb::SBTypeList types = mModule.GetTypes(lldb::eTypeClassClass);
  for (int i = 0; i < types.GetSize(); i++)
  {
    lldb::SBType t = types.GetTypeAtIndex(i);

    printf("Type %d: %s\n", i, t.GetName());
  }

  return;
  uint32_t symbols = mModule.GetNumSymbols();
  int skipped = 0;
  for (uint32_t i = 0; i < symbols; i++)
  {
    lldb::SBSymbol symbol = mModule.GetSymbolAtIndex(i);

    bool skip = false;
    switch (symbol.GetType())
    {
      case lldb::eSymbolTypeCode:
      case lldb::eSymbolTypeAbsolute:
      case lldb::eSymbolTypeException:
      case lldb::eSymbolTypeVariable:
      case lldb::eSymbolTypeObjCClass:
      case lldb::eSymbolTypeObjCMetaClass:
      case lldb::eSymbolTypeObjCIVar:
        skip = false;
        break;
      case lldb::eSymbolTypeTrampoline:
      case lldb::eSymbolTypeData:
      case lldb::eSymbolTypeVariableType:
      case lldb::eSymbolTypeLocal:
      case lldb::eSymbolTypeAdditional: // When symbols take more than one entry: the extra entries get this type
      case lldb::eSymbolTypeParam:
      case lldb::eSymbolTypeRuntime:
      case lldb::eSymbolTypeResolver:
      case lldb::eSymbolTypeCompiler:
      case lldb::eSymbolTypeInstrumentation:
      case lldb::eSymbolTypeUndefined:
      case lldb::eSymbolTypeInvalid:
      case lldb::eSymbolTypeSourceFile:
      case lldb::eSymbolTypeHeaderFile:
      case lldb::eSymbolTypeObjectFile:
      case lldb::eSymbolTypeCommonBlock:
      case lldb::eSymbolTypeBlock:
      case lldb::eSymbolTypeLineEntry:
      case lldb::eSymbolTypeLineHeader:
      case lldb::eSymbolTypeScopeBegin:
      case lldb::eSymbolTypeScopeEnd:
        skip = true;
        skipped++;
        break;
    };

    if (mSymbol.IsSynthetic())
    {
      skip = true;
      skipped++;
    }

    if (!skip)
    {
      ModuleTree* pPT = new ModuleTree(symbol);
      AddChild(pPT);
    }
  }
  NGL_OUT("Symbols %d (%d skipped)\n", symbols, skipped);
}

void ModuleTree::UpdateCompileUnit()
{
  lldb::SBFileSpec f = mCompileUnit.GetFileSpec();
  nglString str(f.GetFilename());
  NGL_OUT("  compile unit: %s\n", str.GetChars());
  nglPath p(f.GetDirectory());
  p += nglString(f.GetFilename());
  nuiLabel* pLabel = new nuiLabel(str);
  pLabel->SetToolTip(p.GetChars());
  SetElement(pLabel);

  uint32_t count = mCompileUnit.GetNumLineEntries();

  for (uint32 i = 0; i < count; i++)
  {
    lldb::SBLineEntry entry = mCompileUnit.GetLineEntryAtIndex(i);
    int32 line = entry.GetLine();
    int32 column = entry.GetColumn();
    NGL_OUT("    entry %d:%d\n", line, column);
  }

}

const char* GetSymbolTypeName(lldb::SymbolType t)
{
  switch (t)
  {
    case lldb::eSymbolTypeAbsolute:
      return "absolute";
    case lldb::eSymbolTypeTrampoline:
      return "trampoline";
    case lldb::eSymbolTypeException:
      return "exception";
    case lldb::eSymbolTypeVariable:
      return "variable";
    case lldb::eSymbolTypeObjCClass:
      return "ObjC class";
    case lldb::eSymbolTypeObjCMetaClass:
      return "ObjC meta class";
    case lldb::eSymbolTypeObjCIVar:
      return "ObjC IVar";
    case lldb::eSymbolTypeData:
      return "data";
    case lldb::eSymbolTypeVariableType:
      return "variable type";
    case lldb::eSymbolTypeLocal:
      return "local";
    case lldb::eSymbolTypeAdditional: // When symbols take more than one entry: the extra entries get this type
      return "additionnal";
    case lldb::eSymbolTypeParam:
      return "param";
    case lldb::eSymbolTypeRuntime:
      return "runtime";
    case lldb::eSymbolTypeCode:
      return "code";
    case lldb::eSymbolTypeResolver:
      return "resolver";
    case lldb::eSymbolTypeCompiler:
      return "compiler";
    case lldb::eSymbolTypeInstrumentation:
      return "instrumentation";
    case lldb::eSymbolTypeUndefined:
      return "undefined";
    case lldb::eSymbolTypeInvalid:
      return "invalid";
    case lldb::eSymbolTypeSourceFile:
      return "source file";
    case lldb::eSymbolTypeHeaderFile:
      return "header file";
    case lldb::eSymbolTypeObjectFile:
      return "object file";
    case lldb::eSymbolTypeCommonBlock:
      return "common block";
    case lldb::eSymbolTypeBlock:
      return "block";
    case lldb::eSymbolTypeLineEntry:
      return "line entry";
    case lldb::eSymbolTypeLineHeader:
      return "line header";
    case lldb::eSymbolTypeScopeBegin:
      return "scope begin";
    case lldb::eSymbolTypeScopeEnd:
      return "scope end";
  }

  return "???";
}

const char* GetTypeClassName(lldb::TypeClass cls)
{
  switch (cls)
  {
    case lldb::eTypeClassInvalid:
      return "invalid";
    case lldb::eTypeClassArray:
      return "array";
    case lldb::eTypeClassBlockPointer:
      return "pointer";
    case lldb::eTypeClassBuiltin:
      return "builtin";
    case lldb::eTypeClassClass:
      return "class";
    case lldb::eTypeClassComplexFloat:
      return "complex float";
    case lldb::eTypeClassComplexInteger:
      return "complex integer";
    case lldb::eTypeClassEnumeration:
      return "enumeration";
    case lldb::eTypeClassFunction:
      return "function";
    case lldb::eTypeClassMemberPointer:
      return "member pointer";
    case lldb::eTypeClassObjCObject:
      return "ObjC object";
    case lldb::eTypeClassObjCInterface:
      return "ObjC interface";
    case lldb::eTypeClassObjCObjectPointer:
      return "ObjC object pointer";
    case lldb::eTypeClassPointer:
      return "pointer";
    case lldb::eTypeClassReference:
      return "reference";
    case lldb::eTypeClassStruct:
      return "struct";
    case lldb::eTypeClassTypedef:
      return "typedef";
    case lldb::eTypeClassUnion:
      return "union";
    case lldb::eTypeClassVector:
      return "vector";
      // Define the last type class as the MSBit of a 32 bit value
    case lldb::eTypeClassOther:
      return "other";
      // Define a mask that can be used for any type when finding types
    case lldb::eTypeClassAny:
      return "anat";

  }
}

void ModuleTree::UpdateSymbol()
{
#if 0
  lldb::TypeClass cls = mSymbol.GetTypeClass();

  nglString str;
  str.Add("(").Add(GetSymbolTypeName(mSymbol.GetType())).Add("/").Add(GetTypeClassName(cls)).Add(") - ").Add(mSymbol.GetName());
  //NGL_OUT("  symbol: %s\n", str.GetChars());
  nuiLabel* pLabel = new nuiLabel(str);
  SetElement(pLabel);
#else
  nglString str;
  str.Add("(").Add(GetSymbolTypeName(mSymbol.GetType())).Add(") - ").Add(mSymbol.GetName());
  //NGL_OUT("  symbol: %s\n", str.GetChars());
  nuiLabel* pLabel = new nuiLabel(str);
  SetElement(pLabel);
#endif
}

ModuleTree::Type ModuleTree::GetType() const
{
  return mType;
}

nglPath ModuleTree::GetSourcePath() const
{
  NGL_ASSERT(mType == eCompileUnit);
  lldb::SBFileSpec f = mCompileUnit.GetFileSpec();
  nglPath p(f.GetDirectory());
  p += f.GetFilename();

  return p;
}
