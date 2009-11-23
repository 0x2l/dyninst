/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: addressSpace.h,v 1.9 2008/06/20 22:00:04 legendre Exp $

#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H

#include "infHeap.h"
#include "codeRange.h"
#include "InstructionSource.h"
#include "ast.h"
#include "symtabAPI/h/Symtab.h"
#include "dyninstAPI/src/trapMappings.h"

class codeRange;
class multiTramp;
class replacedFunctionCall;
class functionReplacement;

class int_function;
class int_variable;
class mapped_module;
class mapped_object;
class instPoint;

class BPatch_process;
class BPatch_function;
class BPatch_point;

class Emitter;
class generatedCodeObject;
class fileDescriptor;

using namespace Dyninst;
using namespace SymtabAPI;

class int_function;
class int_symbol;

class Dyn_Symbol;
class BinaryEdit;
class trampTrapMappings;

// This file serves to define an "address space", a set of routines that 
// code generation and instrumentation rely on to perform their duties. 
// This was derived from the process class and serves as a parent to that
// class and the static_space class for the rewriter. 

// The methods in this class were determined by what the code currently
// uses, not a particular design. As such, I expect this to change wildly
// by the time the refactoring is complete. 
//
// bernat, 5SEP07

// Note: this is a pure virtual class; it serves as an interface
// specification.

class AddressSpace : public InstructionSource {
 public:
    
    // Down-conversion functions
    process *proc();
    BinaryEdit *edit();

    // Read/write

    // We have read/write for both "text" and "data". This comes in handy,
    // somewhere, I'm sure
    virtual bool readDataSpace(const void *inOther, 
                               u_int amount, 
                               void *inSelf, 
                               bool showError) = 0;
    virtual bool readTextSpace(const void *inOther, 
                               u_int amount, 
                               const void *inSelf) = 0;
    

    virtual bool writeDataSpace(void *inOther,
                                u_int amount,
                                const void *inSelf) = 0;
    virtual bool writeTextSpace(void *inOther,
                                u_int amount,
                                const void *inSelf) = 0;

    // Memory allocation
    // We don't specify how it should be done, only that it is. The model is
    // that you ask for an allocation "near" a point, where "near" has an
    // internal, platform-specific definition. The allocation mechanism does its
    // best to give you what you want, but there are no promises - check the
    // address of the returned buffer to be sure.

    virtual Address inferiorMalloc(unsigned size, inferiorHeapType type=anyHeap,
                                   Address near = 0, bool *err = NULL) = 0;
    virtual void inferiorFree(Address item) = 0;
    void inferiorFreeInternal(Address item);
    // And a "constrain" call to free unused memory. This is useful because our
    // instrumentation is incredibly wasteful.
    virtual bool inferiorRealloc(Address item, unsigned newSize) = 0;
    bool inferiorReallocInternal(Address item, unsigned newSize);

    bool isInferiorAllocated(Address block);

    // Get the pointer size of the app we're modifying
    virtual unsigned getAddressWidth() const = 0;

    // We need a mechanism to track what exists at particular addresses in the
    // address space - both for lookup and to ensure that there are no collisions.
    // We have a multitude of ways to "muck with" the application (function replacement,
    // instrumentation, function relocation, ...) and they can all stomp on each
    // other. 

    void addOrigRange(codeRange *range);
    void addModifiedRange(codeRange *range);

    void removeOrigRange(codeRange *range);
    void removeModifiedRange(codeRange *range);

    codeRange *findOrigByAddr(Address addr);
    codeRange *findModByAddr(Address addr);

    bool getDyninstRTLibName();

    virtual void *getPtrToInstruction(Address) const;
    virtual bool isValidAddress(const Address &) const;
    virtual bool isExecutableAddress(const Address &) const;

    // Trap address to base tramp address (for trap instrumentation)
    trampTrapMappings trapMapping;
    
    // Should return iterators
    bool getOrigRanges(pdvector<codeRange *> &);
    bool getModifiedRanges(pdvector<codeRange *> &);

    // Multitramp convenience functions
    multiTramp *findMultiTrampByAddr(Address addr);
    multiTramp *findMultiTrampById(unsigned int id);
    void addMultiTramp(multiTramp *multi);
    void removeMultiTramp(multiTramp *multi);

    // Function replacement (or relocated, actually) convenience functions
    functionReplacement *findFuncReplacement(Address addr);
    void addFuncReplacement(functionReplacement *funcrep);
    void removeFuncReplacement(functionReplacement *funcrep);

    // Function call replacement convenience functions
    replacedFunctionCall *findReplacedCall(Address addr);
    void addReplacedCall(replacedFunctionCall *rep);
    void removeReplacedCall(replacedFunctionCall *rep);

    //////////////////////////////////////////////////////////////
    // Function/variable lookup code
    // Turns out that instrumentation needs this... so the 
    // AddressSpace keeps growing. 
    //////////////////////////////////////////////////////////////

    // findFuncByName: returns function associated with "func_name"
    // This routine checks both the a.out image and any shared object images 
    // for this function
    //int_function *findFuncByName(const std::string &func_name);
    
    bool findFuncsByAll(const std::string &funcname,
                        pdvector<int_function *> &res,
                        const std::string &libname = "");
    
    // Specific versions...
    bool findFuncsByPretty(const std::string &funcname,
                           pdvector<int_function *> &res,
                           const std::string &libname = "");
    bool findFuncsByMangled(const std::string &funcname, 
                            pdvector<int_function *> &res,
                            const std::string &libname = "");
    
    bool findVarsByAll(const std::string &varname,
                       pdvector<int_variable *> &res,
                       const std::string &libname = "");
    
    // And we often internally want to wrap the above to return one
    // and only one func...
    virtual int_function *findOnlyOneFunction(const std::string &name,
                                              const std::string &libname = "",
                                              bool search_rt_lib = true);


    // This will find the named symbol in the image or in a shared object
    // Necessary since some things don't show up as a function or variable.
    //    bool getSymbolInfo( const std::string &name, Dyn_Symbol &ret );
    // This gets wrapped with an int_symbol and returned.
    bool getSymbolInfo( const std::string &name, int_symbol &ret );

    // getAllFunctions: returns a vector of all functions defined in the
    // a.out and in the shared objects
    void getAllFunctions(pdvector<int_function *> &);
    
    // Find the code sequence containing an address
    // Note: fix the name....
    int_function *findFuncByAddr(Address addr);
    bool findFuncsByAddr(Address addr, std::vector<int_function *> &funcs);

    int_basicBlock *findBasicBlockByAddr(Address addr);
    
    // And a lookup by "internal" function to find clones during fork...
    int_function *findFuncByInternalFunc(image_func *ifunc);
    
    //findJumpTargetFuncByAddr Acts like findFunc, but if it fails,
    // checks if 'addr' is a jump to a function.
    int_function *findJumpTargetFuncByAddr(Address addr);
    
    // findModule: returns the module associated with "mod_name" 
    // this routine checks both the a.out image and any shared object 
    // images for this module
    // if check_excluded is true it checks to see if the module is excluded
    // and if it is it returns 0.  If check_excluded is false it doesn't check
    //  if substring_match is true, the first module whose name contains
    //  the provided string is returned.
    // Wildcard: handles "*" and "?"
    mapped_module *findModule(const std::string &mod_name, bool wildcard = false);
    // And the same for objects
    // Wildcard: handles "*" and "?"
    mapped_object *findObject(const std::string &obj_name, bool wildcard = false);
    mapped_object *findObject(Address addr);
    mapped_object *findObject(fileDescriptor desc);

    mapped_object *getAOut() { assert(mapped_objects.size()); return mapped_objects[0];}
    
    // getAllModules: returns a vector of all modules defined in the
    // a.out and in the shared objects
    void getAllModules(pdvector<mapped_module *> &);

    // return the list of dynamically linked libs
    const pdvector<mapped_object *> &mappedObjects() { return mapped_objects;  } 

    // And a shortcut pointer
    mapped_object *runtime_lib;
    // ... and keep the name around
    std::string dyninstRT_name;
    
    // If true is passed for ignore_if_mt_not_set, then an error won't be
    // initiated if we're unable to determine if the program is multi-threaded.
    // We are unable to determine this if the daemon hasn't yet figured out
    // what libraries are linked against the application.  Currently, we
    // identify an application as being multi-threaded if it is linked against
    // a thread library (eg. libpthreads.a on AIX).  There are cases where we
    // are querying whether the app is multi-threaded, but it can't be
    // determined yet but it also isn't necessary to know.
    virtual bool multithread_capable(bool ignore_if_mt_not_set = false) = 0;
    
    // Do we have the RT-side multithread functions available
    virtual bool multithread_ready(bool ignore_if_mt_not_set = false) = 0;

    //////////////////////////////////////////////////////
    // Process-level instrumentation (?)
    /////////////////////////////////////////////////////

    // instPoint isn't const; it may get an updated list of
    // instances since we generate them lazily.
    // Shouldn't this be an instPoint member function?
    bool replaceFunctionCall(instPoint *point,const int_function *newFunc);
    
    // And this....
    bool getDynamicCallSiteArgs(instPoint *callSite, 
                                pdvector<AstNodePtr> &args);

    // Default to "nope"
    virtual bool hasBeenBound(const relocationEntry &, 
                              int_function *&, 
                              Address) { return false; }
    
    // Trampoline guard get/set functions
    int_variable* trampGuardBase(void) { return trampGuardBase_; }
    AstNodePtr trampGuardAST(void);

    // Get the current code generator (or emitter)
    Emitter *getEmitter();

    // Should be easy if the process isn't _executing_ where
    // we're deleting...
    virtual void deleteGeneratedCode(generatedCodeObject *del);

    //True if any reference to this address space needs PIC
    virtual bool needsPIC() = 0;
    //True if we need PIC to reference the given variable or function
    // from this addressSpace.
    bool needsPIC(int_variable *v); 
    bool needsPIC(int_function *f);
    bool needsPIC(AddressSpace *s);
    
    //////////////////////////////////////////////////////
    // BPatch-level stuff
    //////////////////////////////////////////////////////
    // Callbacks for higher level code (like BPatch) to learn about new 
    //  functions and InstPoints.
 private:
    BPatch_function *(*new_func_cb)(AddressSpace *a, int_function *f);
    BPatch_point *(*new_instp_cb)(AddressSpace *a, int_function *f, instPoint *ip, 
                                  int type);
 public:
    //Trigger the callbacks from a lower level
    BPatch_function *newFunctionCB(int_function *f) 
        { assert(new_func_cb); return new_func_cb(this, f); }
    BPatch_point *newInstPointCB(int_function *f, instPoint *pt, int type)
        { assert(new_instp_cb); return new_instp_cb(this, f, pt, type); }
    
    //Register callbacks from the higher level
    void registerFunctionCallback(BPatch_function *(*f)(AddressSpace *p, 
                                                        int_function *f))
        { new_func_cb = f; };
    void registerInstPointCallback(BPatch_point *(*f)(AddressSpace *p, int_function *f,
                                                      instPoint *ip, int type))
        { new_instp_cb = f; }
    
    
    //Anonymous up pointer to the containing process.  This is BPatch_process
    // in Dyninst.  Currently stored as an void pointer in case we do
    // anything with this during the library split.
    void *up_ptr() { return up_ptr_; }
    void set_up_ptr(void *ptr) { up_ptr_ = ptr; }
    
    //////////////////////////////////////////////////////
    // Internal and cleanup 
    //////////////////////////////////////////////////////

    // Clear things out (e.g., deleteProcess)
    void deleteAddressSpace();
    // Fork psuedo-constructor
    void copyAddressSpace(process *parent);

    // Aaand constructor/destructor
    AddressSpace();
    virtual ~AddressSpace();


    //////////////////////////////////////////////////////
    // Yuck
    //////////////////////////////////////////////////////
    Address getObservedCostAddr() const { return costAddr_; }
    void updateObservedCostAddr(Address addr) { costAddr_ = addr;}

    // Can we use traps if necessary?
    bool canUseTraps();
    void setUseTraps(bool usetraps);

 protected:

    // inferior malloc support functions
    void inferiorFreeCompact();
    int findFreeIndex(unsigned size, int type, Address lo, Address hi);
    void addHeap(heapItem *h);
    void initializeHeap();
    
    // Centralization of certain inferiorMalloc operations
    Address inferiorMallocInternal(unsigned size, Address lo, Address hi, 
                                   inferiorHeapType type);
    void inferiorMallocAlign(unsigned &size);

    bool heapInitialized_;
    bool useTraps_;
    inferiorHeap heap_;

    // Text sections (including added - instrumentation)
    codeRangeTree textRanges_;
    // Data sections
    codeRangeTree dataRanges_;
    // And address-space-wide patches that we've dropped in
    codeRangeTree modifiedRanges_;

    // We label multiTramps by ID
    dictionary_hash<int, multiTramp *> multiTrampsById_;

    // Loaded mapped objects (may be just 1)
    pdvector<mapped_object *> mapped_objects;

    int_variable* trampGuardBase_; // Tramp recursion index mapping
    AstNodePtr trampGuardAST_;

    void *up_ptr_;

    Address costAddr_;
};

extern int heapItemCmpByAddr(const heapItem **A, const heapItem **B);

#endif // ADDRESS_SPACE_H
