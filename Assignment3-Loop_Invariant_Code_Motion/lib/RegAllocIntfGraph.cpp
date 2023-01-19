/**
 * @file Interference Graph Register Allocator
 */
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/CodeGen/LiveIntervals.h>
#include <llvm/CodeGen/LiveRangeEdit.h>
#include <llvm/CodeGen/LiveRegMatrix.h>
#include <llvm/CodeGen/LiveStacks.h>
#include <llvm/CodeGen/MachineBlockFrequencyInfo.h>
#include <llvm/CodeGen/MachineDominators.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/MachineLoopInfo.h>
#include <llvm/CodeGen/MachineRegisterInfo.h>
#include <llvm/CodeGen/RegAllocRegistry.h>
#include <llvm/CodeGen/RegisterClassInfo.h>
#include <llvm/CodeGen/Spiller.h>
#include <llvm/CodeGen/TargetRegisterInfo.h>
#include <llvm/CodeGen/VirtRegMap.h>
#include <llvm/InitializePasses.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>

#include <cmath>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

using namespace llvm;

namespace llvm {

void initializeRAIntfGraphPass(PassRegistry &Registry);

} // namespace llvm

namespace std {

template <> //
struct hash<Register> {
  size_t operator()(const Register &Reg) const {
    return DenseMapInfo<Register>::getHashValue(Reg);
  }
};

template <> //
struct greater<LiveInterval *> {
  bool operator()(LiveInterval *const &LHS, LiveInterval *const &RHS) {
    /**
     * @TODO(cscd70) Please finish the implementation of this function that is
     *               used for determining whether one live interval has spill
     *               cost greater than the other.
     */
    return LHS->weight() > RHS->weight();
  }
};

} // namespace std

namespace {

class RAIntfGraph;

class AllocationHints {
private:
  SmallVector<MCPhysReg, 16> Hints;

public:
  AllocationHints(RAIntfGraph *const RA, const LiveInterval *const LI);
  SmallVectorImpl<MCPhysReg>::iterator begin() { return Hints.begin(); }
  SmallVectorImpl<MCPhysReg>::iterator end() { return Hints.end(); }
};

class RAIntfGraph final : public MachineFunctionPass,
                          private LiveRangeEdit::Delegate {
private:
  MachineFunction *MF;

  SlotIndexes *SI;
  VirtRegMap *VRM;
  const TargetRegisterInfo *TRI;
  MachineRegisterInfo *MRI;
  RegisterClassInfo RCI;
  LiveRegMatrix *LRM;
  MachineLoopInfo *MLI;
  LiveIntervals *LIS;

  /**
   * @brief Interference Graph
   */
  class IntfGraph {
  private:
    RAIntfGraph *RA;
	
	std::unordered_map<Register, float> weightMap;
	std::unordered_set<MCPhysReg> assignedPhysReg;

    /// Interference Relations
    std::multimap<LiveInterval *, std::unordered_set<Register>,
                  std::greater<LiveInterval *>>
        IntfRels;

    /**
     * @brief  Try to materialize all the virtual registers (internal).
     *
     * @return (nullptr, VirtPhysRegMap) in the case when a successful
     *         materialization is made, (LI, *) in the case when unsuccessful
     *         (and LI is the live interval to spill)
     *
     * @sa tryMaterializeAll
     */
    using MaterializeResult_t =
        std::tuple<LiveInterval *,
                   std::unordered_map<LiveInterval *, MCPhysReg>>;
    MaterializeResult_t tryMaterializeAllInternal();

  public:
    explicit IntfGraph(RAIntfGraph *const RA) : RA(RA) {}
    /**
     * @brief Insert a virtual register @c Reg into the interference graph.
     */
    void insert(const Register &Reg);
    /**
     * @brief Erase a virtual register @c Reg from the interference graph.
     *
     * @sa RAIntfGraph::LRE_CanEraseVirtReg
     */
    void erase(const Register &Reg);

	/*
	 * @brief initialize cost to be (#defs & uses) * 10**loop_nest_depth 
 	 */
	void initializeWeight(const Register &Reg);
	
	inline void updateWeight(LiveInterval &node);

	/**
	 * @brief Allocate a physical register for @c LI, or have the spiller splits
	 * 		  it into a list of virtual registers
	 */
	MCRegister selectOrSplit(LiveInterval *const LI,
							 SmallVectorImpl<Register> *const SplitVirtRegs);
	
	bool spillInterferences(LiveInterval *const LI, MCRegister PhysReg,
									SmallVectorImpl<Register> *const SplitVirtRegs);
    /**
     * @brief Build the whole graph.
     */
    void build();
    /**
     * @brief Try to materialize all the virtual registers.
     */
    void tryMaterializeAll();
    void clear() { IntfRels.clear(); }
  } G;

  SmallPtrSet<MachineInstr *, 32> DeadRemats;
  std::unique_ptr<Spiller> SpillerInst;

  void postOptimization() {
    SpillerInst->postOptimization();
    for (MachineInstr *const DeadInst : DeadRemats) {
      LIS->RemoveMachineInstrFromMaps(*DeadInst);
      DeadInst->eraseFromParent();
    }
    DeadRemats.clear();
    G.clear();
  }

  friend class AllocationHints;
  friend class IntfGraph;

  /// The following two methods are inherited from @c LiveRangeEdit::Delegate
  /// and implicitly used by the spiller to edit the live ranges.
  bool LRE_CanEraseVirtReg(Register Reg) override {
    /**
     * @TODO(cscd70) Please implement this method.
     */
    // If the virtual register has been materialized, undo its physical
    // assignment and erase it from the interference graph.
	LiveInterval &LI = LIS->getInterval(Reg);
	if (!VRM->hasPhys(Reg)) {
		LI.clear();
		return false;	
	}
	LRM->unassign(LI);
	G.erase(Reg);
    return true;
  }
  void LRE_WillShrinkVirtReg(Register Reg) override {
    /**
     * @TODO(cscd70) Please implement this method.
     */
    // If the virtual register has been materialized, undo its physical
    // assignment and re-insert it into the interference graph.
	if (!VRM->hasPhys(Reg))
		return;
	LiveInterval &LI = LIS->getInterval(Reg);
	LRM->unassign(LI);
	G.insert(Reg);
  }

public:
  static char ID;

  StringRef getPassName() const override {
    return "Interference Graph Register Allocator";
  }

  RAIntfGraph() : MachineFunctionPass(ID), G(this) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    MachineFunctionPass::getAnalysisUsage(AU);
    AU.setPreservesCFG();
#define REQUIRE_AND_PRESERVE_PASS(PassName)                                    \
  AU.addRequired<PassName>();                                                  \
  AU.addPreserved<PassName>()

    REQUIRE_AND_PRESERVE_PASS(SlotIndexes);
    REQUIRE_AND_PRESERVE_PASS(VirtRegMap);
    REQUIRE_AND_PRESERVE_PASS(LiveIntervals);
    REQUIRE_AND_PRESERVE_PASS(LiveRegMatrix);
    REQUIRE_AND_PRESERVE_PASS(LiveStacks);
    REQUIRE_AND_PRESERVE_PASS(AAResultsWrapperPass);
    REQUIRE_AND_PRESERVE_PASS(MachineDominatorTree);
    REQUIRE_AND_PRESERVE_PASS(MachineLoopInfo);
    REQUIRE_AND_PRESERVE_PASS(MachineBlockFrequencyInfo);
  }

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::NoPHIs);
  }
  MachineFunctionProperties getClearedProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::IsSSA);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
}; // class RAIntfGraph

AllocationHints::AllocationHints(RAIntfGraph *const RA,
                                 const LiveInterval *const LI) {
  //const TargetRegisterClass *const RC = RA->MRI->getRegClass(LI->reg());
  const TargetRegisterClass *const RC = RA->MF->getRegInfo().getRegClass(LI->reg());

  /**
   * @TODO(cscd70) Please complete this part by constructing the allocation
   *               hints, similar to the tutorial example.
   */
  ArrayRef<MCPhysReg> Order = RA->RCI.getOrder(RC);
  bool isHardHint =
	RA->TRI->getRegAllocationHints(LI->reg(), Order, Hints, *RA->MF, RA->VRM, RA->LRM);
  if (!isHardHint) {
	outs() << "noHardHint\n";
	for (const MCPhysReg &PhysReg : Order) {
		Hints.push_back(PhysReg);
	}
  }
  outs() << "Hint Registers for Class " << RA->TRI->getRegClassName(RC)
         << ": [";
  for (const MCPhysReg &PhysReg : Hints) {
    outs() << RA->TRI->getRegAsmName(PhysReg) << ", ";
  }
  outs() << "]\n";
}

bool RAIntfGraph::runOnMachineFunction(MachineFunction &MF) {
  outs() << "************************************************\n"
         << "* Machine Function\n"
         << "************************************************\n";
  SI = &getAnalysis<SlotIndexes>();
  for (const MachineBasicBlock &MBB : MF) {
    MBB.print(outs(), SI);
    outs() << "\n";
  }
  outs() << "\n\n";

  this->MF = &MF;

  VRM = &getAnalysis<VirtRegMap>();
  TRI = &VRM->getTargetRegInfo();
  MRI = &VRM->getRegInfo();
  MRI->freezeReservedRegs(MF);
  LIS = &getAnalysis<LiveIntervals>();
  LRM = &getAnalysis<LiveRegMatrix>();
  RCI.runOnMachineFunction(MF);
  MLI = &getAnalysis<MachineLoopInfo>();

  SpillerInst.reset(createInlineSpiller(*this, MF, *VRM));

  G.build();
  G.tryMaterializeAll();

  postOptimization();
  return true;
}

void RAIntfGraph::IntfGraph::insert(const Register &Reg) {
  /**
   * @TODO(cscd70) Please implement this method.
   */
  auto *liveInterval = &(RA->LIS->getInterval(Reg));
  IntfRels.insert({liveInterval, {}});
  auto it = IntfRels.find(liveInterval);

  // 1. Collect all VIRTUAL registers that interfere with 'Reg'.
  for (unsigned virtRegIdx = 0; virtRegIdx < RA->MRI->getNumVirtRegs(); virtRegIdx++) {
	Register curReg = Register::index2VirtReg(virtRegIdx);
	if (curReg == Reg || RA->MRI->reg_nodbg_empty(curReg))
	  continue;
	auto *curLiveInterval = &(RA->LIS->getInterval(curReg));

	if (curLiveInterval->overlaps(liveInterval)) {
		outs() << "\tOverlapping {Reg=}" << *curLiveInterval << "\n";
		// add curReg into the mapping of the input liveInterval
		it->second.insert(curReg);
		// add Reg into the mapping of curLiveInterval, if curLiveInterval already exists in IntfRels
		auto it_curReg = IntfRels.find(curLiveInterval);
		if (it_curReg != IntfRels.end())
			it_curReg->second.insert(Reg);
	}
  }
  updateWeight(*liveInterval);
  outs() << "Inserting {Reg=}" << *liveInterval << "}\n";
  // 2. Collect all PHYSICAL registers that interfere with 'Reg'.
  // 3. Update the weights of Reg (and its interfering neighbors), using the
  //    formula on "Lecture 6 Register Allocation Page 23".
  // 4. Insert 'Reg' into the graph.
}

void RAIntfGraph::IntfGraph::erase(const Register &Reg) {
  /**
   * @TODO(cscd70) Please implement this method.
   */
  // 1. ∀n ∈ neighbors(Reg), erase 'Reg' from n's interfering set and update its
  //    weights accordingly.
  // 2. Erase 'Reg' from the interference graph.

  // IntfRels: std::multimap<LiveInterval*, std::unordered_set<Register>>
  auto *liveInterval = &(RA->LIS->getInterval(Reg));
  auto it = IntfRels.find(liveInterval);
  if (it != IntfRels.end()) {
	outs() << "Popping {Reg=" << *liveInterval << "}\n";
	IntfRels.erase(liveInterval);
	liveInterval->setWeight(-1);
  }
  for (auto &node : IntfRels) {
	auto &regSet = node.second; 
	if (regSet.count(Reg)) {
	  regSet.erase(Reg);
	  updateWeight(*(node.first));
	}
  }
}

inline void RAIntfGraph::IntfGraph::updateWeight(LiveInterval &node) {
	int degree = IntfRels.find(&node)->second.size();
	outs() << "\t" << degree << "\n";
	if (degree)
		node.setWeight(weightMap.at(node.reg()) / (double)degree);
	// what if degree = 0? 
}

MCRegister RAIntfGraph::IntfGraph::selectOrSplit(LiveInterval *const LI,
							 SmallVectorImpl<Register> *const SplitVirtRegs) {
	AllocationHints AH(RA, LI);
	SmallVector<MCRegister, 8> PhysRegSpillCandidates;
	for (auto it = AH.begin(); it != AH.end(); it++) {
		MCRegister PhysReg = *it;
		// !!!
		//if (assignedPhysReg.count(PhysReg))
		//	continue;
		switch (RA->LRM->checkInterference(*LI, PhysReg)) {
		case LiveRegMatrix::IK_Free:
			outs() << "Allocating physical register " << RA->TRI->getRegAsmName(PhysReg)
				   << "\n";
			return PhysReg;
		case LiveRegMatrix::IK_VirtReg:
			PhysRegSpillCandidates.push_back(PhysReg);
			continue;
		default:
			continue;
		}	
	}
	
	for (MCRegister PhysReg : PhysRegSpillCandidates) {
		outs() << "Hi\n";
		if (!spillInterferences(LI, PhysReg, SplitVirtRegs))
			continue;
		return PhysReg;	
	}

	LiveRangeEdit LRE(LI, *SplitVirtRegs, *RA->MF, *RA->LIS, RA->VRM, RA, &RA->DeadRemats);
	RA->SpillerInst->spill(LRE);
	outs() << "Hi\n";
	return 0;	
}

bool RAIntfGraph::IntfGraph::spillInterferences(LiveInterval *const LI, MCRegister PhysReg,
						 SmallVectorImpl<Register> *const SplitVirtRegs) {
	SmallVector<LiveInterval*, 8> IntfLIs;
	for (MCRegUnitIterator Units(PhysReg, RA->TRI); Units.isValid(); ++Units) {
		LiveIntervalUnion::Query &Q = RA->LRM->query(*LI, *Units);
		Q.collectInterferingVRegs();
		for (LiveInterval *const IntfLI : Q.interferingVRegs()) {
			if (!IntfLI->isSpillable() || IntfLI->weight() > LI->weight()) {
				return false;
			}
			IntfLIs.push_back(IntfLI);
		}
	}
	for (unsigned IntfIdx = 0; IntfIdx < IntfLIs.size(); IntfIdx++) {
		LiveInterval *const LIToSpill = IntfLIs[IntfIdx];
		if (!RA->VRM->hasPhys(LIToSpill->reg())) {
			continue;
		}
		RA->LRM->unassign(*LIToSpill);
		LiveRangeEdit LRE(LIToSpill, *SplitVirtRegs, *RA->MF, *RA->LIS, RA->VRM,
						  RA, &RA->DeadRemats);
		RA->SpillerInst->spill(LRE);
	}
	return true;
}

void RAIntfGraph::IntfGraph::build() {
  /**
   * @TODO(cscd70) Please implement this method.
   */
  for (unsigned virtRegIdx = 0; virtRegIdx < RA->MRI->getNumVirtRegs();
		virtRegIdx++) {
	Register virtReg = Register::index2VirtReg(virtRegIdx);
	if (RA->MRI->reg_nodbg_empty(virtReg))	continue;
	initializeWeight(virtReg);
	insert(virtReg);	
  }
}

void RAIntfGraph::IntfGraph::initializeWeight(const Register &Reg) {
	float weight = 0;
	for (auto it = RA->MRI->reg_instr_begin(Reg);
			it != RA->MRI->reg_instr_end(); it++) {
		MachineInstr *mcInst = &(*it);
		unsigned loopDepth = RA->MLI->getLoopDepth(mcInst->getParent());
		auto rw = mcInst->readsWritesVirtualRegister(Reg);
		weight += (rw.first + rw.second) * std::pow(10, loopDepth);	
	}
	weightMap.emplace(Reg, weight); 
}


RAIntfGraph::IntfGraph::MaterializeResult_t
RAIntfGraph::IntfGraph::tryMaterializeAllInternal() {
  std::unordered_map<LiveInterval *, MCPhysReg> PhysRegAssignment;

  //@TODO(cscd70) Please implement this method.
  // ∀r ∈ IntfRels.keys, try to materialize it. If successful, cache it in
  // PhysRegAssignment, else mark it as to be spilled.
  bool failToMaterialize = false;
  LiveInterval *toBeSpilled = nullptr;
  while (!IntfRels.empty() && !failToMaterialize) { //
	outs() << "Hi\n";
	auto it_toBePopped = IntfRels.begin(); // the node with highest weight
	LiveInterval *toBePopped = (*it_toBePopped).first;
	erase(toBePopped->reg());	

	// invalidate cached interference queries
	// after modifying virtual register live ranges
	RA->LRM->invalidateVirtRegs();	
	
	SmallVector<Register, 4> SplitVirtRegs;
	MCRegister PhysReg = selectOrSplit(toBePopped, &SplitVirtRegs);

	if (PhysReg) {
	  RA->LRM->assign(*toBePopped, PhysReg);
	  PhysRegAssignment.emplace(toBePopped, PhysReg);
	} else {
	  toBeSpilled = toBePopped;
	  failToMaterialize = true;
	}

	for (Register Reg : SplitVirtRegs) {
	  LiveInterval *LI = &RA->LIS->getInterval(Reg);
	  if (RA->MRI->reg_nodbg_empty(LI->reg())) {
		RA->LIS->removeInterval(LI->reg());
		continue;
	  }
	  insert(Reg);
	}
  }
  return std::make_tuple(toBeSpilled, PhysRegAssignment);
}

void RAIntfGraph::IntfGraph::tryMaterializeAll() {
  std::unordered_map<LiveInterval *, MCPhysReg> PhysRegAssignment;
  /**
   * @TODO(cscd70) Please implement this method.
   */
  // Keep looping until a valid assignment is made. In the case of spilling,
  // modify the interference graph accordingly. 
  bool finish = false;
  while (!finish) {
	auto internalResult = tryMaterializeAllInternal();
	LiveInterval *toBeSpilled = std::get<LiveInterval*>(internalResult);
	auto &tmpAssignment = std::get<std::unordered_map<LiveInterval*, MCPhysReg>>(internalResult);
	for (const auto &search : tmpAssignment) {
		assignedPhysReg.emplace(search.second);
		PhysRegAssignment.emplace(search.first, search.second);
	}
	if (toBeSpilled == nullptr) {
	  finish = true; 
	}
  } 

  for (auto &PhysRegAssignPair : PhysRegAssignment) {
    RA->LRM->assign(*PhysRegAssignPair.first, PhysRegAssignPair.second);
  }
}

char RAIntfGraph::ID = 0;

static RegisterRegAlloc X("intfgraph", "Interference Graph Register Allocator",
                          []() -> FunctionPass * { return new RAIntfGraph(); });

} // anonymous namespace

INITIALIZE_PASS_BEGIN(RAIntfGraph, "regallointfgraph",
                      "Interference Graph Register Allocator", false, false)
INITIALIZE_PASS_DEPENDENCY(SlotIndexes)
INITIALIZE_PASS_DEPENDENCY(VirtRegMap)
INITIALIZE_PASS_DEPENDENCY(LiveIntervals)
INITIALIZE_PASS_DEPENDENCY(LiveRegMatrix)
INITIALIZE_PASS_DEPENDENCY(LiveStacks);
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass);
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree);
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo);
INITIALIZE_PASS_DEPENDENCY(MachineBlockFrequencyInfo);
INITIALIZE_PASS_END(RAIntfGraph, "regallointfgraph",
                    "Interference Graph Register Allocator", false, false)
