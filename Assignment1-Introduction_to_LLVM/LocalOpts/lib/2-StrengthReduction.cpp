#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Pass.h>
#include <vector>
using namespace llvm;

namespace {

class StrengthReduction final : public FunctionPass {
private:
  void deleteInstruction(std::vector<Instruction*> Insts) {
	for (auto &Inst : Insts) {
	  if (Inst->isSafeToRemove())
		Inst->eraseFromParent();
    }
  }
  size_t get_shift(uint64_t val) {
	if(val & (val-1))
		return 0;
	size_t n = 0;
	while(val >>= 1) {
		n++;
	}
	return n;
  }
  void runOnBB(BasicBlock &BB) {
	bool DeleteFlag;
	std::vector<Instruction*> DeleteInst;
	for(auto &I : BB) {
	  	if(I.isBinaryOp()) {
			DeleteFlag = true;
			Value *lhs = I.getOperand(0);
			Value *rhs = I.getOperand(1);

			auto *lhs_const = dyn_cast<ConstantInt>(lhs);
			auto *rhs_const = dyn_cast<ConstantInt>(rhs);
			
			size_t shift = 0;
			if(lhs_const && (shift = get_shift(lhs_const->getZExtValue()))) {
				switch(I.getOpcode()) {
				case Instruction::Mul: {
					auto *shift_instr = BinaryOperator::Create(
						Instruction::Shl, rhs, ConstantInt::getSigned(I.getType(), shift), "", &I);
					I.replaceAllUsesWith(shift_instr);
					break;
				}
				default:
					DeleteFlag = false;
					break;
				}
			} else if(rhs_const && (shift = get_shift(rhs_const->getZExtValue()))) {
				switch(I.getOpcode()) {
				case Instruction::Mul: {
					auto *shift_instr = BinaryOperator::Create(
						Instruction::Shl, lhs, ConstantInt::getSigned(I.getType(), shift), "", &I);
					I.replaceAllUsesWith(shift_instr);
					break;
				}
				case Instruction::SDiv: {
					auto *shift_instr = BinaryOperator::Create(
						Instruction::LShr, lhs, ConstantInt::getSigned(I.getType(), shift), "", &I);
					I.replaceAllUsesWith(shift_instr);
					break;
				}
				default:
					DeleteFlag = false;
					break;
				}
			} else {
				DeleteFlag = false;
			}
			if (DeleteFlag)	DeleteInst.push_back(&I);
		}
	}
	deleteInstruction(DeleteInst);
  }
public:
  static char ID;

  StrengthReduction() : FunctionPass(ID) {}

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
}; // class StrengthReduction

char StrengthReduction::ID = 0;
RegisterPass<StrengthReduction> X("strength-reduction",
                                  "CSCD70: Strength Reduction");

} // anonymous namespace
