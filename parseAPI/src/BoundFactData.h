#ifndef BOUND_FACT_DATA_H
#define BOUND_FACT_DATA_H

#include "Absloc.h"
#include "Node.h"
#include "debug_parse.h"

#include "Instruction.h"
#include "CFG.h"
#include "entryIDs.h"
using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::DataflowAPI;

struct StridedInterval {   
    static const int64_t minValue = -0x3fffffffffffffffLL;
    static const int64_t maxValue = 0x3fffffffffffffffLL;
    static const StridedInterval top;
    static const StridedInterval bottom;
    // stride < 0: bottom (empty set)
    // stride = 0: represent a constant
    // stride > 0: represent an interval
    int stride;
    int64_t low, high;

    // Bottom: empty set
    StridedInterval(): stride(-1), low(0), high(0) {}

    // Construct a constant
    StridedInterval(int64_t x): stride(0), low(x), high(x) {} 

    // Construct an interval
    StridedInterval(unsigned s, int64_t l, int64_t h):
        stride(s), low(l), high(h) {}

    StridedInterval(const StridedInterval &si):
        stride(si.stride), low(si.low), high(si.high) {}

    // Meet is point-wise union
    void Join(const StridedInterval &rhs);

    void Neg();
    void Not();

    void Add(const StridedInterval &rhs);
    void Sub(const StridedInterval &rhs);
    void And(const StridedInterval &rhs);
    void Or(const StridedInterval &rhs);
    void Xor(const StridedInterval &rhs);
    void Mul(const StridedInterval &rhs); 
    void ShiftLeft(const StridedInterval &rhs);
    std::string format();

    StridedInterval & operator = (const StridedInterval &rhs);
    bool operator == (const StridedInterval &rhs) const;
    bool operator != (const StridedInterval &rhs) const;
    bool operator < (const StridedInterval &rhs) const;

    void Intersect(StridedInterval &rhs);
    void DeleteElement(int64_t val);

    uint64_t size() const;
    bool IsConst(int64_t v) const;
    bool IsConst() const;
};

struct BoundValue {

    static const BoundValue top;
    static const BoundValue bottom;

    StridedInterval interval;

    Address targetBase;    
    bool isTableRead;
    bool isInverted;
    bool isSubReadContent;
    BoundValue(int64_t val); 
    BoundValue(const StridedInterval& si); 
    BoundValue();
    BoundValue(const BoundValue & bv);
    BoundValue& operator = (const BoundValue &bv);
    bool operator< (const BoundValue &bv) const { return interval < bv.interval; }
    bool operator== (const BoundValue &bv) const;
    bool operator!= (const BoundValue &bv) const;
    void Print();

    void SetToBottom();
    void IntersectInterval(StridedInterval &si);
    void DeleteElementFromInterval(int64_t val);
    void Join(BoundValue &bv);
    void ClearTableCheck();
    void Add(const BoundValue &rhs);
    void And(const BoundValue &rhs);
    void Mul(const BoundValue &rhs);
    void ShiftLeft(const BoundValue &rhs);
    void Or(const BoundValue &rhs);
    void Invert();
};

struct BoundFact {

    typedef enum {
        Equal,
	NotEqual, 
	UnsignedLessThan,
	UnsignedLargerThan,
	UnsignedLessThanOrEqual,
	UnsignedLargerThanOrEqual,
	SignedLessThan,
	SignedLargerThan,
	SignedLessThanOrEqual,
	SignedLargerThanOrEqual,
    } RelationType;
    typedef map<AST::Ptr, BoundValue*> FactType;
    FactType fact;

    struct RelationShip {
        AST::Ptr left;
	AST::Ptr right;
	RelationType type;
	RelationShip(AST::Ptr l, AST::Ptr r, RelationType t):
	    left(l), right(r), type(t) {}

        bool operator != (const RelationShip &rhs) const {
	    if (type != rhs.type) return true;
	    if (!(*left == *rhs.left)) return true;
	    if (!(*right == *rhs.right)) return true;
	    return false;
	}

	RelationShip& operator = (const RelationShip &rhs) {
	    left = rhs.left;
	    right = rhs.right;
	    type = rhs.type;
	    return *this;
	}

	RelationShip(const RelationShip &r) { *this = r; }
    };

    vector<RelationShip*> relation;

    struct FlagPredicate {
        bool valid;
	entryID id;
	AST::Ptr e1;
	AST::Ptr e2;

	FlagPredicate():
	    valid(false),
	    id( _entry_ids_max_),
	    e1(),
	    e2() {}
	bool operator != (const FlagPredicate& fp) const {
	    if (!valid && !fp.valid) return false;
	    if (valid != fp.valid) return true;
	    if (id != fp.id) return true;
	    return !(*e1 == *e2);
	}

	FlagPredicate& operator = (const FlagPredicate &fp) {
	    valid = fp.valid;
	    if (valid) {
		e1 = fp.e1;
	        e2 = fp.e2;
		id = fp.id;
	    }
	    return *this;
	}
    } pred;

    bool operator< (const BoundFact &bf) const {return fact < bf.fact; }
    bool operator!= (const BoundFact &bf) const;

    BoundValue* GetBound(const AST::Ptr ast); 
    BoundValue* GetBound(const AST* ast);
    void Meet(BoundFact &bf);


    bool ConditionalJumpBound(InstructionAPI::Instruction::Ptr insn, EdgeTypeEnum type);
    void SetPredicate(Assignment::Ptr assign);
    void GenFact(const AST::Ptr ast, BoundValue* bv);
    void KillFact(const AST::Ptr ast);
    void SetToBottom();
    void Print();
    void AdjustPredicate(AST::Ptr out, AST::Ptr in);

    void IntersectInterval(const AST::Ptr ast, StridedInterval si);
    void DeleteElementFromInterval(const AST::Ptr ast, int64_t val);
    void InsertRelation(AST::Ptr left, AST::Ptr right, RelationType);

    BoundFact();
    ~BoundFact();

    BoundFact(const BoundFact& bf);
    BoundFact& operator = (const BoundFact &bf);
};

typedef map<Node::Ptr, BoundFact*> BoundFactsType;


#endif
