#pragma once // NOLINT(llvm-header-guard)

#include <vector>

namespace dfa {

template <typename TDomainElemRepr> //
class MeetOp {
protected:
  using DomainVal_t = std::vector<TDomainElemRepr>;

public:
  virtual DomainVal_t operator()(const DomainVal_t &LHS,
                                 const DomainVal_t &RHS) const = 0;
  virtual DomainVal_t top(const size_t DomainSize) const = 0;
};

/**
 * @brief Intersection Meet Operator
 *
 * TODO (cscd70) Please complete the definition of the intersection meet
 *               operator, and modify the existing definition if necessary.
 */
 // Simpler view: LHS and RHS are bitvectors!
class Intersect final : public MeetOp<bool> {
public:
  virtual DomainVal_t operator()(const DomainVal_t &LHS,
                                 const DomainVal_t &RHS) const override {

	//assert(LHS.size() == RHS.size());
    DomainVal_t result = DomainVal_t(LHS.size());
    for (size_t i = 0; i < LHS.size(); i++) {
    	result[i] = LHS[i] && RHS[i];
    }
    return result;
  }
  
  // top element T s.t. x intersect T = x
  // so T is union set 
  virtual DomainVal_t top(const size_t DomainSize) const override {
    return DomainVal_t(DomainSize, true);
  }
};

class Union final : public MeetOp<bool> {
public:
  virtual DomainVal_t operator()(const DomainVal_t &LHS,
								 const DomainVal_t &RHS) const override {
	//assert(LHS.size() == RHS.size());
    DomainVal_t result = DomainVal_t(LHS.size());
    for (size_t i = 0; i < LHS.size(); i++) {
      result[i] = LHS[i] || RHS[i];
   	}
    return result;							   
  }
  // top element T s.t. x union T = x
  // so T is empty set 
  virtual DomainVal_t top(const size_t DomainSize) const override {
    return DomainVal_t(DomainSize, false);
  }
};

} // namespace dfa
