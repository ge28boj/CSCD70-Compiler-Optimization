#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/Casting.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstrTypes.h>

using namespace llvm;

namespace {

class FunctionInfo final : public ModulePass {
public:
  static char ID;

  FunctionInfo() : ModulePass(ID) {}

  // We don't modify the program, so we preserve all analysis.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  virtual bool runOnModule(Module &M) override {
    //outs() << "CSCD70 Function Information Pass" << "\n";
    //outs() << "Name # Args # Calls # Blocks # Insts" << "\n";
    /**
     * @todo(cscd70) Please complete this method.
     */
    for(auto& F : M){
    	outs() << "Function Name: " << F.getName() << "\n";
       	outs() << "Number of Arguments: " << F.arg_size() << "\n";	
		outs() << "Number of Calls: " << F.getNumUses() << "\n";
		outs() << "Number of BBs: " << F.getBasicBlockList().size() << "\n";
    	outs() << "Number of Instructions: " << F.getInstructionCount() << "\n";
    }
    return false;
  }
}; // class FunctionInfo

char FunctionInfo::ID = 0;
RegisterPass<FunctionInfo> X("function-info", "CSCD70: Function Information");

} // anonymous namespace
