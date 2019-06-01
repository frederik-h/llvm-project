//===-- AsmParser/IRDebugCreator.h ------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the definition of the IRDebugCreator class which
/// can create generic debug information that allows to debug IR
/// (i.e. in contrast to debugging some source language that gets compiled
/// into IR).
///
//===----------------------------------------------------------------------===//

#include "IRDebugCreator.h"
#include <iostream>

namespace llvm {
std::string getTypeName(const Type &T) {
    std::string name;
    raw_string_ostream o(name);
    T.print(o);
    o.flush();
    return name;
}

unsigned getTypeEncoding(const Type &T) {
    if (T.isIntegerTy())
        return dwarf::DW_ATE_unsigned;
    if (T.isFloatingPointTy())
        return dwarf::DW_ATE_float;

    return dwarf::DW_ATE_signed;
}

static std::string getTypeString(Type *T) {
    std::string Result;
    raw_string_ostream Tmp(Result);
    Tmp << *T;
    return Tmp.str();
}

DIBasicType* getDIType(DIBuilder &B, Type const &T) {
    auto t = const_cast<Type*>(&T);
    return B.createBasicType(getTypeString(t),
                             T.getScalarSizeInBits(),
                             getTypeEncoding(T));
}


void IRDebugCreator::visitInstruction(Instruction &I) {
    auto lineAndCol = LocationMap.getInstructionLoc(I);
    DILocation* loc = DILocation::get(Context, lineAndCol.first,
                                      lineAndCol.second, FunctionScope);
    I.setDebugLoc(loc);
}

DISubroutineType* createFunctionType(DIBuilder &B, const Function &F)
{
    Type *returnType = F.getReturnType();
    assert(argType && "Function must have a return type.");
    StringRef returnTypeString = StringRef(getTypeString(returnType));

    DIType *diReturnType = B.createBasicType(returnTypeString,
                                             returnType->getScalarSizeInBits(),
                                             getTypeEncoding(*returnType));

    SmallVector<Metadata*, 4> types;
    types.push_back(diReturnType);
    for(const Argument& arg : F.args())
    {
        Type* argType = arg.getType();
        assert(argType && "Function argument must have a type.");

        DIType* t = getDIType(B, *argType);
        types.push_back(t);
    }

    DITypeRefArray paramTypes = B.getOrCreateTypeArray(types);

    return B.createSubroutineType(paramTypes);
}

void IRDebugCreator::visitFunction(Function &F) {
    std::pair<unsigned, unsigned> lineAndCol = LocationMap.getFunctionLoc(F);
    unsigned line = lineAndCol.first;

    StringRef functionName = F.getName();

    DISubroutineType *functionType = createFunctionType(Builder, F);
    DISubprogram *subprogram = Builder.createFunction(&File, functionName,
                                                      functionName, &File,
                                                      line, functionType,
                                                      line, DINode::FlagZero,
                                                      DISubprogram::SPFlagDefinition);
    F.setSubprogram(subprogram);
    FunctionScope = subprogram;
}


/// Attach the source buffer location to the object as a metadata
/// node. This metadata will be used to obtain line and column numbers
/// for debug metadata and pruned later if the object is visited by this
/// InstVisitor.
void IRDebugCreator::attachLocationMD(LLVMContext &context, GlobalObject &o,
                                      const IRDebugCreator::LocTy loc)
{
    /*  int64_t locPtr = reinterpret_cast<int64_t>(loc.getPointer());
        ConstantInt *ptrConst = ConstantInt::get(context,
                                                APInt(64, locPtr, true));
        ConstantAsMetadata* md = ConstantAsMetadata::get(ptrConst);
        MDNode* constInt = MDNode::get(context, ArrayRef<Metadata*>(md));
        o.setMetadata("debugir.location_ptr", constInt)*/;

}

//IRDebugCreator::LocTy IRDebugCreator::getBufferLocation(GlobalObject &o)
//{
//    /*   MDNode* md = o.getMetadata("debugir.location_ptr");
//        auto* mdTup = static_cast<MDTuple*>(md);
//        ConstantAsMetadata* constMd = mdTup->get
////        ConstantInt* constInt = constMd->getValue();
//        unsigned locPtr = constInt->getValue().getZExtValue()*/;


void
SourceLocationMap::setInstructionLoc(const Instruction &I,
                                     const LocTy loc)
{
    std::pair<Line, Column> lineAndCol = GetLineAndNumber(loc);
    InstructionMap.insert({&I, lineAndCol});
}

std::pair<unsigned, unsigned>
SourceLocationMap::getInstructionLoc(const Instruction& I) {
    assert(InstructionMap.count(&I) == 1);
    return InstructionMap.lookup(&I);
}

void
SourceLocationMap::setFunctionLoc(const Function &I,
                                  const LocTy loc)
{
    std::pair<Line, Column> lineAndCol = GetLineAndNumber(loc);
    FunctionMap.insert({&I, lineAndCol});
}

std::pair<unsigned, unsigned>
SourceLocationMap::getFunctionLoc(const Function& I) {
    assert(FunctionMap.count(&I) == 1);
    return FunctionMap.lookup(&I);
}

}
