/**
 * @file Available Expression Dataflow Analysis
 */
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Pass.h>

#include <dfa/Framework.h>
#include <dfa/MeetOp.h>

#include "Expression.h"
#include <iostream>
using namespace dfa;
using namespace llvm;

namespace {

using AvailExprFrameworkBase =
    Framework<Expression, bool, Direction::kForward, Intersect>;

class AvailExpr final : public AvailExprFrameworkBase, public FunctionPass {
private:
  virtual void initializeDomainFromInst(const Instruction &Inst) override {
	if (const BinaryOperator *const BinaryOp =
            dyn_cast<BinaryOperator>(&Inst)) {
      /**
       * @todo(cscd70) Please complete the construction of domain.
       */
	  // note: Expression(const BinaryOperator &BinaryOp)  
	  if (getPos(Expression(*BinaryOp)) == -1)
		Domain.push_back(Expression(*BinaryOp));
    }
  }
  virtual bool transferFunc(const Instruction &Inst, const DomainVal_t &IBV,
                            DomainVal_t &OBV) override {
    /**
     * @todo(cscd70) Please complete the definition of the transfer function.
     */

	// Rule out non-binary operation, 
	// since we only consider binary operations in this assignment
	// don't forget to set OBV equal to IBV !
	// otherwise the anaylsis on further instructions will fail
	// because the initial condition (implemented by MeetOperands->topup(...)) is union set, e.g. [true, true, ..., true]
	if(!isa<BinaryOperator>(Inst)) {
		DomainVal_t tmpBV = IBV;
		OBV = IBV;
		return false;
	}

	/*
	errs() << "  IBV: ";
	for (const auto& v : IBV) {
		errs() << v << "\t";
	}
	errs() << "\n";
	*/	

	DomainVal_t tmpBV = IBV;

	// kill set
	const Value *I = &Inst;
	
	for (size_t i = 0; i < Domain.size(); i++) {
		if (I == Domain[i].LHS || I == Domain[i].RHS) {
			tmpBV[i] = false;
		}
	}

	// gen set
	tmpBV[getPos(*dyn_cast<BinaryOperator>(&Inst))] = true;

	// check whether output OBV has been changed
	bool change = (OBV != tmpBV);
	if(change)
		OBV = tmpBV;	
    return change;
  }

public:
  static char ID;

  AvailExpr() : AvailExprFrameworkBase(), FunctionPass(ID) {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
  bool runOnFunction(Function &F) override {
    return AvailExprFrameworkBase::runOnFunction(F);
  }
};

char AvailExpr::ID = 0;
RegisterPass<AvailExpr> X("avail-expr", "Available Expression");

} // anonymous namespace
