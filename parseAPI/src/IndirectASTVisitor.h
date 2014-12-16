#ifndef INDIRECT_AST_VISITOR_H
#define INDIRECT_AST_VISITOR_H

#include <set>

#include "DynAST.h"
#include "SymEval.h"

#include "BoundFactData.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::DataflowAPI;

AST::Ptr SimplifyRoot(AST::Ptr ast, uint64_t size);
AST::Ptr SimplifyAnAST(AST::Ptr ast, uint64_t size);


class SimplifyVisitor: public ASTVisitor {
    uint64_t size;
public:
    using ASTVisitor::visit;
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);
    SimplifyVisitor(uint64_t s): size(s) {}
};



class BoundCalcVisitor: public ASTVisitor {
	ParseAPI::Block *block;
     
public:
    using ASTVisitor::visit;
    map<AST*, BoundValue*> bound;
    BoundFact &boundFact;
    BoundCalcVisitor(BoundFact &bf, ParseAPI::Block* b): boundFact(bf), block(b) {}
    ~BoundCalcVisitor();
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);
    virtual ASTPtr visit(DataflowAPI::ConstantAST *ast);
    virtual ASTPtr visit(DataflowAPI::VariableAST *ast);
    bool IsResultBounded(AST::Ptr ast) {
        return bound.find(ast.get()) != bound.end();
    }
    BoundValue* GetResultBound(AST::Ptr ast); 
};

class JumpCondVisitor: public ASTVisitor {

public:
    using ASTVisitor::visit;
    bool invertFlag;
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);
    JumpCondVisitor() : invertFlag(false) {}
};

class ComparisonVisitor: public ASTVisitor {

public:
    using ASTVisitor::visit;
    AST::Ptr subtrahend, minuend;
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);

    ComparisonVisitor(): subtrahend(AST::Ptr()), minuend(AST::Ptr()) {} 
};

#endif
