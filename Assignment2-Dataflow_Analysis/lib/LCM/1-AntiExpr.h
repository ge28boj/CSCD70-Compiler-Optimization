#pragma once // NOLINT(llvm-header-guard)

/**
 * @file Anticipated Expression Dataflow Analysis
 */
#include <llvm/Pass.h>
#include <llvm/Transforms/Utils.h>

#include <dfa/Framework.h>
#include <dfa/MeetOp.h>

#include "../Expression.h"

using namespace dfa;

class AntiExprWrapperPass;

/**
 * @todo(cscd70) Provide an implementation for the anticipated expressions, by
 *               inheriting from the DFA framework.
 */
using AntiExprFrameworkBase =
	Framework<Expression, bool, Direction::kBackward, Intersect>;

class AntiExprImpl final : AntiExprFrameworkBase {
private:
  friend class AntiExprWrapperPass;
  virtual void initializeDomainFromInst(const Instruction &Inst) override {
  	//TODO
  	if (const BinaryOperator *const BinaryOp =
  			dyn_cast<BinaryOperator>(&Inst)) {
  		if (getPos(Expression(*BinaryOp)) == -1)
  			Domain.push_back(Expression(*BinaryOp));		
  	}
  }
  virtual bool transferFunc(const Instruction &Inst, const DomainVal_t &IBV,
  							DomainVal_t &OBV) override {
  	//TODO
  	DomainVal_t tmpBV = IBV;
  	if (!isa<BinaryOperator>(Inst) && !isa<PHINode>(Inst)) {
  		OBV = IBV;
  		return false;
  	}
  	
  	//errs() << Inst << "\n";
  	const Value *I = &Inst;
  	for (size_t i = 0; i < Domain.size(); i++) {
  		if (I == Domain[i].LHS || I == Domain[i].RHS)
  			tmpBV[i] = false;
  	}
  	
  	if (auto *BinaryOp = dyn_cast<BinaryOperator>(&Inst))
  		tmpBV[getPos(*BinaryOp)] = true;
  	bool change = (OBV != tmpBV);
  	if (change)
  		OBV = tmpBV;
  	return change;							
  }
public:
  bool runOnFunction(Function &F) {
  	return AntiExprFrameworkBase::runOnFunction(F);
  }
};

class AntiExprWrapperPass : public FunctionPass {
private:
  /**
   * @todo(cscd70) Uncomment the following line after the completion of
   *               @c AntiExprImpl .
   */
  AntiExprImpl AntiExpr;

public:
  static char ID;

  AntiExprWrapperPass() : FunctionPass(ID) {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    /**
     * @todo(cscd70) Request that the LLVM built-in pass @c BreakCriticalEdges
     *               to run first, by leveraging its ID.
     */
    AU.addRequiredID(BreakCriticalEdgesID);
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    errs() << "* Anticipated Expression *"
           << "\n";

    /**
     * @todo(cscd70) Invoke the method @c runOnFunction from @c AntiExprImpl .
     */
    return AntiExpr.runOnFunction(F);
  }

  /**
   * @todo(cscd70) Obtain the @c Domain and @c InstDomainValMap from
   *               @c AntiExprImpl .
   */
  std::vector<Expression> getDomain() const {
    return AntiExpr.Domain;
  }
  std::unordered_map<const Instruction *, std::vector<bool>>
  getInstDomainValMap() const {
    return AntiExpr.InstDomainValMap;
  }
};
