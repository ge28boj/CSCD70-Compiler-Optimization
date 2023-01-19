#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>
using namespace llvm;

namespace {

class MultiInstOpt final : public FunctionPass {
private:
  
  bool isLhsInstrRhsConst(const Instruction &I) {
	return isa<Instruction>(I.getOperand(0)) && isa<ConstantInt>(I.getOperand(1));
  }
  bool isLhsConstRhsInstr(const Instruction &I) {
	return isa<ConstantInt>(I.getOperand(0)) && isa<Instruction>(I.getOperand(1));
  }
  void runOnBB(BasicBlock &BB) {
	std::vector<Instruction*> DeleteInst;
	for(auto &I : BB) {
		if(I.isBinaryOp()) {
			auto *lhs = I.getOperand(0);
			auto *rhs = I.getOperand(1);
			
			Instruction *instrToBeExpanded = nullptr;
			ConstantInt *constCur = nullptr;
			int curInstrPos = -1;
			if(isLhsInstrRhsConst(I)) {
				instrToBeExpanded = dyn_cast<Instruction>(lhs);
				constCur = dyn_cast<ConstantInt>(rhs);
				curInstrPos = 0;
			} else if(isLhsConstRhsInstr(I)) {
				constCur = dyn_cast<ConstantInt>(lhs);
				curInstrPos = 1;
				instrToBeExpanded = dyn_cast<Instruction>(rhs);
			} else {
				continue;
			}

			//Instruction *instrPrev = nullptr;
			ConstantInt *constPrev = nullptr;
			int prevInstrPos = -1;
			if(instrToBeExpanded->isBinaryOp()) { 
				if (isLhsInstrRhsConst(*instrToBeExpanded)) {
					//instrPrev = dyn_cast<Instruction>(instrToBeExpanded->getOperand(0));
					constPrev = dyn_cast<ConstantInt>(instrToBeExpanded->getOperand(1));
					prevInstrPos = 0;
				} else if (isLhsConstRhsInstr(*instrToBeExpanded)) {
					constPrev = dyn_cast<ConstantInt>(instrToBeExpanded->getOperand(0));
					prevInstrPos = 1;
					//instrPrev = dyn_cast<Instruction>(instrToBeExpanded->getOperand(1));
				} else {
					continue;
				}
			}

			outs() << I.getOpcodeName() << "\n";
			outs() << "  " << curInstrPos << "\n";
			outs() << "  " << instrToBeExpanded->getOpcode() << "\n";
			// case 1
			// %1 = sub %2, 1
			// %3 = add %1, 1
			switch(I.getOpcode()) {
			case Instruction::Add: {
				outs() << "Add" << "\n";
				if(instrToBeExpanded->getOpcode() == Instruction::Sub) {
					if(curInstrPos == prevInstrPos && constCur->getSExtValue() == constPrev->getSExtValue()) {
						I.replaceAllUsesWith(instrToBeExpanded->getOperand(prevInstrPos));
						DeleteInst.push_back(&I);
					}
				}
				break;
			}
			case Instruction::Sub: {
				outs() << "Sub" << "\n";
				if(instrToBeExpanded->getOpcode() == Instruction::Add) {
					if(curInstrPos == prevInstrPos && constCur->getSExtValue() == constPrev->getSExtValue()) {
						I.replaceAllUsesWith(instrToBeExpanded->getOperand(prevInstrPos));
						DeleteInst.push_back(&I);
					}
				}
				break;
			}
			case Instruction::Mul: {
				outs() << "Mul" << "\n";
				if(instrToBeExpanded->getOpcode() == Instruction::SDiv) {
					// %1 = SDiv %0, 7
					// %2 = Mul 7, %1 => %2 = %0
					if(prevInstrPos == 0 && constCur->getSExtValue() == constPrev->getSExtValue()) {
						I.replaceAllUsesWith(instrToBeExpanded->getOperand(prevInstrPos));
						DeleteInst.push_back(&I);
					}
				}
				break;
			}
			case Instruction::SDiv: {
				if(instrToBeExpanded->getOpcode() == Instruction::Mul) {
					// %1 = Mul 54, %0
					// %2 = SDiv %1, 54 => %2 = %0
					if(curInstrPos == 0 && constCur->getSExtValue() == constPrev->getSExtValue()) {
						I.replaceAllUsesWith(instrToBeExpanded->getOperand(prevInstrPos));
						DeleteInst.push_back(&I);
					}
				}
				break;
			}
			}			
		}
	}
	for(auto &Inst : DeleteInst) {
		Inst->eraseFromParent();
	}
  }
public:
  static char ID;

  MultiInstOpt() : FunctionPass(ID) {}

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
}; // class MultiInstOpt

char MultiInstOpt::ID = 0;
RegisterPass<MultiInstOpt> X("multi-inst-opt",
                             "CSCD70: Multi-Instruction Optimization");

} // anonymous namespace
