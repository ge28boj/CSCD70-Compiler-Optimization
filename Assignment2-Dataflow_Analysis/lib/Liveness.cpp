/**
 * @file Liveness Dataflow Analysis
 */
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>

#include <dfa/Framework.h>
#include <dfa/MeetOp.h>

#include "Variable.h"
using namespace dfa;
using namespace llvm;

namespace {

using LivenessFrameworkBase =
	Framework<Variable, bool, Direction::kBackward, Union>;

/**
 * @todo(cscd70) Implement @c Liveness using the @c dfa::Framework interface.
 */
class Liveness final : public LivenessFrameworkBase, public FunctionPass {
private:
  virtual void initializeDomainFromInst(const Instruction &Inst) override {
	//TODO
	for (const auto &op : Inst.operands()) {
		if (isa<Instruction>(op) || isa<Argument>(op)) {
			if (getPos(Variable(op)) == -1) {// avoid putting same variable into Domain more than once
				Domain.push_back(Variable(op));
			}
		}
	}
  }

  virtual bool transferFunc(const Instruction &Inst, const DomainVal_t &IBV,
							DomainVal_t &OBV) override {
	DomainVal_t tmpBV = IBV;

	// Kill set (Def)
	const Value *I = &Inst;
	for (const Variable &v : Domain) {
		if (I == v.V)
			tmpBV[getPos(v)] = false;
	}

	// TODO Phi Node
	// Gen set (Use)
	for (const auto &op : Inst.operands()) {
		if (isa<Instruction>(op) || isa<Argument>(op))
			tmpBV[getPos(Variable(op))] = true;
	}

	bool change = (tmpBV != OBV);
	if (change)
		OBV = tmpBV;
	return change;
  }

public:
  static char ID;

  Liveness() : FunctionPass(ID) {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  virtual bool runOnFunction(Function &F) override {
    // clang-format off
    //errs() << "**************************************************" << "\n"
    //       << "* Instruction-Domain Value Mapping" << "\n"
    //       << "**************************************************" << "\n";
    // clang-format on
	return LivenessFrameworkBase::runOnFunction(F);
  }
};

char Liveness::ID = 0;
RegisterPass<Liveness> X("liveness", "Liveness");

} // anonymous namespace
