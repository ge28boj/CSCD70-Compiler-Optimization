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
  using MeetOperands_t = std::vector<DomainVal_t>;

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

  // phi instructions have ramifications on how liveness analysis is implemented
  virtual DomainVal_t getBoundaryVal(const BasicBlock &BB) const override {
	MeetOperands_t Operands;
	for (const auto &pred : successors(&BB)) {
      const Instruction *inst = &(pred->front());
	  DomainVal_t original_bV = InstDomainValMap.at(inst);

	  for (const auto &phi : pred->phis()) {
		for (const auto &block : phi.blocks()) {
		  if (block != &BB) {
			Value *v = phi.getIncomingValueForBlock(block);
			if (getPos(Variable(v)) != -1)
			  original_bV[getPos(Variable(v))] = false;
		  }		  
		}		
	  }

	  Operands.push_back(original_bV); 
	}
	
	if (Operands.begin() == Operands.end()) {
	  return bc();
	}
	return meet(Operands);
	/*
    Union MeetOp;
	DomainVal_t mergedBV = MeetOp.top(Domain.size());
	for (const auto &BitVector : MeetOperands) {
	  mergedBV = MeetOp(mergedBV, BitVector);
	}
	return mergedBV;
	*/
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
