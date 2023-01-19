/**
 * @file Loop Invariant Code Motion
 */
#include <llvm/Analysis/LoopPass.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <unordered_set>

using namespace llvm;

namespace {

class LoopInvariantCodeMotion final : public LoopPass {
private:
  //const LoopInfo const *loopInfo;
  DominatorTree *dominatorTree;
  std::unordered_set<Instruction*> InvarInstSet;
  inline bool contains(Instruction *I) {
	return std::find(InvarInstSet.begin(), InvarInstSet.end(), I) != InvarInstSet.end();
  }
  bool isInvariant(Loop *L, Instruction const *I) {
	//TODO
	bool isInvariant = true;
	for (auto &op : I->operands()) {
		bool isDefOutsideLoop = false;
		bool isInInvarInstSet = false;
		bool isLiteral = isa<Constant>(op) || isa<Argument>(op);
		if (auto opInst = dyn_cast<Instruction>(op)) {
			isDefOutsideLoop = !L->contains(opInst);
			isInInvarInstSet = contains(opInst);
		}
		isInvariant = isInvariant && (isInInvarInstSet || isLiteral || isDefOutsideLoop);
	}
	return isSafeToSpeculativelyExecute(I)
		&& !I->mayReadFromMemory()
		&& !isa<LandingPadInst>(I)
		&& isInvariant;
  }
  bool traverseLoop(Loop *L) {
	//TODO
	bool change = false;
	for (BasicBlock *B : L->getBlocks()) {
		for (Instruction &I : *B) {
			if (!contains(&I) && isInvariant(L, &I)) {
				InvarInstSet.insert(&I);
				change = true;
			}
		} 
	}
	return change;
  }
  void tryHoist(Loop *L, Instruction *I) {
	SmallVector<BasicBlock*> uniqueExitBlocks;
	L->getExitBlocks(uniqueExitBlocks);
	//errs() << "\t" << *I << "\n";
	for (const auto &exitBlock : uniqueExitBlocks) {
		if(dominatorTree->dominates(I, exitBlock)) {
			//errs() << "\texitBlock " << *exitBlock << "\n"; 
			I->moveBefore((L->getLoopPreheader())->getTerminator());
		}
	}
  }
public:
  static char ID;

  LoopInvariantCodeMotion() : LoopPass(ID) {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    /**
     * @todo(cscd70) Request the dominator tree and the loop simplify pass.
     */
	AU.addRequiredID(LoopSimplifyID);
	AU.addRequired<DominatorTreeWrapperPass>();
	//AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesCFG();
  }

  /**
   * @todo(cscd70) Please finish the implementation of this method.
   */
  virtual bool runOnLoop(Loop *L, LPPassManager &LPM) override { 
	//if (!loopInfo)	loopInfo = &(getAnalysis<LoopInfoWrapperPass>().getLoopInfo());
	errs() << dominatorTree << "\n";
	if (!L->getLoopPreheader()) {
		errs() << "Loop not in simplified form!\n";
		return false;
	}

	dominatorTree = &(getAnalysis<DominatorTreeWrapperPass>().getDomTree());
	// Wrong!
	//if (!dominatorTree)	dominatorTree = &(getAnalysis<DominatorTreeWrapperPass>().getDomTree());

	// find loop invariant instructions
	while (traverseLoop(L)) {

	}
	
	// hoist or sink loop invariant instructions
	for (auto &invarInst : InvarInstSet) {
		//errs() << "\t" << *invarInst << "\n";
		tryHoist(L, invarInst);
	}
	return false;
  }
};

char LoopInvariantCodeMotion::ID = 0;
RegisterPass<LoopInvariantCodeMotion> X("loop-invariant-code-motion",
                                        "Loop Invariant Code Motion");

} // anonymous namespace
