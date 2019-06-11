#ifndef LLVM_LIB_IRDEBUGCREATOR_H
#define LLVM_LIB_IRDEBUGCREATOR_H

#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/Support/SMLoc.h"

namespace llvm {
class SourceLocationMap;
class IRDebugCreator : public InstVisitor<IRDebugCreator> {
private:
    using LocTy = SMLoc;
    DIBuilder &Builder;
    LLVMContext &Context;
    DIFile &File;
    DIScope *FunctionScope;
    SourceLocationMap &LocationMap;
public:
    IRDebugCreator(DIFile &File,
                   DIBuilder &Builder,
                   LLVMContext &Context,
                   SourceLocationMap& LocationMap)
        : Builder(Builder), Context(Context), File(File),
          FunctionScope(nullptr), LocationMap(LocationMap)
    {
      // TODO [debug-ir] Determine reaosonable parameters
      Builder.createCompileUnit(dwarf::DW_LANG_C89,
                                &File, "LLParser IR debug",
                                false, StringRef(), 0,
                                StringRef(), DICompileUnit::FullDebug);


    };

public:
    void visitInstruction(Instruction &I);
    void visitFunction(Function &F);
private:
    std::string getTypeName(const Type &T);
    unsigned getTypeEncoding(const Type &T);
    DIBasicType* getDIType(Type const &T);
};

class SourceLocationMap {
public:
    using LocTy = SMLoc;
    using Line = unsigned;
    using Column = unsigned;

    SourceLocationMap(
            std::function<std::pair<Line, Column>(LocTy)>
            GetLineAndNumber) : GetLineAndNumber(GetLineAndNumber)
    {}

    void setInstructionLoc(const Instruction &I, const LocTy loc);
    std::pair<Line, Column> getInstructionLoc(const Instruction& I);

    void setFunctionLoc(const Function &I, const LocTy loc);
    std::pair<Line, Column> getFunctionLoc(const Function& I);

private:
    DenseMap<const Instruction*, std::pair<Line, Column>> InstructionMap;
    DenseMap<const Function*, std::pair<Line, Column>> FunctionMap;

    std::function<std::pair<Line, Column>(LocTy)> GetLineAndNumber;


};

}

#endif // LLVM_LIB_IRDEBUGCREATOR_H
