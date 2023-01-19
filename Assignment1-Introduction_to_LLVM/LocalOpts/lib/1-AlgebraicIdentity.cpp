#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/Pass.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>
using namespace llvm;

namespace {

class AlgebraicIdentity final : public FunctionPass {
private:
  void deleteInstruction(std::vector<Instruction *> Insts) {
    for (auto &Inst : Insts) {
      if (Inst->isSafeToRemove())
        Inst->eraseFromParent();
    }  
  }

  void runOnBB(BasicBlock &BB) {
    bool DeleteFlag;
    std::vector<Instruction *> DeleteInst;
	for(auto &I : BB) {
	  if(I.isBinaryOp()) {
		DeleteFlag = true;
		Value *lhs = I.getOperand(0);
		Value *rhs = I.getOperand(1);
		
		auto *lhs_const = dyn_cast<ConstantInt>(lhs);
		auto *rhs_const = dyn_cast<ConstantInt>(rhs);

		// e.g. %1 = add i32 0, 2 
		if(lhs_const && rhs_const) {
		  switch(I.getOpcode()) {
		  case Instruction::Add: {
			I.replaceAllUsesWith(ConstantInt::getSigned(
				I.getType(), lhs_const->getSExtValue() + rhs_const->getSExtValue()));
			break;
		  }
		  case Instruction::Sub: {
			I.replaceAllUsesWith(ConstantInt::getSigned(
				I.getType(), lhs_const->getSExtValue() - rhs_const->getSExtValue()));
			break;
		  }
		  case Instruction::Mul: {
			I.replaceAllUsesWith(ConstantInt::getSigned(
				I.getType(), lhs_const->getSExtValue() * rhs_const->getSExtValue()));
			break;
		  }
		  case Instruction::SDiv: {
			I.replaceAllUsesWith(ConstantInt::getSigned(
				I.getType(), lhs_const->getSExtValue() / rhs_const->getSExtValue()));
			break;
		  }
		  default:
			DeleteFlag = false;
			break;
		  }
		} else if(lhs_const) {
		  switch(I.getOpcode()) {
		  case Instruction::Add: {// e.g. %2 = add int32 0, %1
			if(!lhs_const->getSExtValue())	I.replaceAllUsesWith(rhs);
			else	DeleteFlag = false; // Don't forget !!!
			break;
		  }
		  case Instruction::Mul: {// e.g. %2 = mul int32 1, %1
			if(lhs_const->getSExtValue() == 1)	I.replaceAllUsesWith(rhs);
			else	DeleteFlag = false;
			break;
		  }
		  default:
			DeleteFlag = false;
			break;
		  }
		} else if(rhs_const) {
		  switch(I.getOpcode()) {
		  case Instruction::Add: {// e.g. %2 = add int32 %1, 0
		 	if(!rhs_const->getSExtValue())	I.replaceAllUsesWith(lhs);
			else	DeleteFlag = false;
			break;
		  }
		  case Instruction::Sub: {// e.g. %2 = sub int32 %1, 0
			if(!rhs_const->getSExtValue())	I.replaceAllUsesWith(lhs);
			else	DeleteFlag = false;
			break;
		  }
		  case Instruction::Mul: {// e.g. %2 = mul int32 %1, 1
			if(rhs_const->getSExtValue() == 1)	I.replaceAllUsesWith(lhs);
			else	DeleteFlag = false;
			break;
		  }
		  case Instruction::SDiv: {// e.g. %2 = sdiv int32 %1, 1
			if(rhs_const->getSExtValue() == 1)	I.replaceAllUsesWith(lhs);
			else	DeleteFlag = false;
			break;
		  }
		  default:
			DeleteFlag = false;
			break;
		  }
		} else {
		  switch(I.getOpcode()) {
		  case Instruction::Sub: {// e.g. %2 = sub i32 %1, %1
			if(lhs == rhs)	I.replaceAllUsesWith(ConstantInt::getSigned(I.getType(), 0));
			else	DeleteFlag = false;
			break;
		  }
		  case Instruction::SDiv: {// e.g. %2 = sdiv i32 %1, %1
			if(lhs == rhs)	I.replaceAllUsesWith(ConstantInt::getSigned(I.getType(), 1));
			else	DeleteFlag = false;
			break;
		  }
		  default:
			DeleteFlag = false;
			break;
		  }
		}
        if (DeleteFlag)  DeleteInst.push_back(&I);
	  }
	}
    deleteInstruction(DeleteInst);
  }

public:
  static char ID;

  AlgebraicIdentity() : FunctionPass(ID) {}

  /**
   * @todo(cscd70) Please complete the methods below.
   */
  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {}

  virtual bool runOnFunction(Function &F) override {
	for(auto &BB : F) {
		runOnBB(BB);
	}
	return false;
  }
}; // class AlgebraicIdentity

char AlgebraicIdentity::ID = 0;
RegisterPass<AlgebraicIdentity> X("algebraic-identity",
                                  "CSCD70: Algebraic Identity");

} // anonymous namespace
