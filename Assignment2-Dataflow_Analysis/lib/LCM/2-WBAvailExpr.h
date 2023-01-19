#pragma once // NOLINT(llvm-header-guard)

/**
 * @file Will-be-Available Expression Dataflow Analysis
 */
#include "1-AntiExpr.h"

class WBAvailExprWrapperPass;

using WBAvailExprFrameworkBase =
  Framework<Expression, bool, Direction::kForward, Intersect>;

/**
 * @todo(cscd70) Provide an implementation for the will-be-available
 *               expressions, by inheriting from the DFA framework.
 */
class WBAvailExprImpl final : public WBAvailExprFrameworkBase {
private:
  friend class WBAvailExprWrapperPass;
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
  	if (!isa<BinaryOperator>(Inst)) {
  		OBV = IBV;
  		return false;
  	}
  	
  	const Value *I = &Inst;
  	for (size_t i = 0; i < Domain.size(); i++) {
  		if (I == Domain[i].LHS || I == Domain[i].RHS)
  			tmpBV[i] = false;
  	}
  	
  	//tmpBV[getPos(*dyn_cast<BinaryOperator>(&Inst))] = true;
  	bool change = (OBV != tmpBV);
  	if (change)
  		OBV = tmpBV;
  	return change;	
  }
public:
  bool runOnFunction(Function &F) {
  	return WBAvailExprFrameworkBase::runOnFunction(F);
  }
};

class WBAvailExprWrapperPass : public FunctionPass {
private:
  /**
   * @todo(cscd70) Uncomment the following line after the completion of
   *               @c WBAvailExprImpl .
   */
  WBAvailExprImpl WBAvailExpr;

public:
  static char ID;

  WBAvailExprWrapperPass() : FunctionPass(ID) {}

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    /**
     * @todo(cscd70) Request that the anticipated expressions to run first.
     */
    AU.addRequired<AntiExprWrapperPass>();
    AU.setPreservesAll();
  }
  
  bool runOnFunction(Function &F) override {
    errs() << "* Will-Be-Available Expression *"
           << "\n";
    /**
     * @todo(cscd70) Obtain the domain and analysis results from the anticipated
     *               expressions, and then invoke the method @c runOnFunction
     *               from @c WBAvailExprImpl .
     */
    // legacy method to access the pass declared in getAnalysisUsage()
    AntiExprWrapperPass &AntiExprWrapper = getAnalysis<AntiExprWrapperPass>();
    std::vector<Expression> AntiExprDomain = AntiExprWrapper.getDomain();
    std::unordered_map<const Instruction*, std::vector<bool>> AntiExprInstDomainValMap =
    	AntiExprWrapper.getInstDomainValMap();
    
    WBAvailExpr.runOnFunction(F);
    errs() << "AntiExprDomain size: " << AntiExprDomain.size() << "\n";
    errs() << "WBAvailExprDomain size: " << WBAvailExpr.Domain.size() << "\n";
    // assert(WBAvailExprImpl::diff(Domain, AntiExprDomain) == false);
    // OUT[i] = (IN[i] union Anti_IN[i]) / Kill(i)
    //		  = (IN[i] / Kill(i)) union (Anti_IN[i] / Kill(i))
    auto &InstDomainValMap = WBAvailExpr.InstDomainValMap;
    for (auto &Inst : instructions(F)) {
      	auto AntiExprCurBitVector = AntiExprInstDomainValMap.at(&Inst);
      	//const Value *I = &Inst;
      	for (size_t i = 0; i < AntiExprDomain.size(); i++) {
      		//if (I == AntiExprDomain[i].LHS || I == AntiExprDomain[i].RHS)
      		//	AntiExprCurBitVector[i] = false;
      		//errs() << i << "th AntiExprCurBitVector " << AntiExprCurBitVector[i] << "\t";      		
      		//errs() << i << "th InstDomainBitVector " << (InstDomainValMap.at(&Inst))[i] << "\n";
      		
      		(InstDomainValMap.at(&Inst))[i] = 
      			(InstDomainValMap.at(&Inst))[i] || AntiExprCurBitVector[i];
      	}
    }
    return false;
  }

  /**
   * @todo(cscd70) Obtain the @c InstDomainValMap and boundary values at each BB
   *               from @c WBAvailExprImpl .
   */
  std::unordered_map<const Instruction *, std::vector<bool>>
  getInstDomainValMap() const {
    return std::unordered_map<const Instruction *, std::vector<bool>>();
  }
  std::unordered_map<const BasicBlock *, std::vector<bool>>
  getBoundaryVals() const {
    return std::unordered_map<const BasicBlock *, std::vector<bool>>();
  }
};
