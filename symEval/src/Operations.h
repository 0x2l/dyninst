/*
 * Copyright (c) 2007-2008 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if !defined(OPERATION_H)
#define OPERATION_H

#include "Absloc.h"

namespace Dyninst {

using namespace InstructionAPI;

// NOTE: this is NOT the instructionAPI operation. This is a
// single-definition component of an instruction.

class Operation {
 public:
    typedef dyn_detail::boost::shared_ptr<Operation> Ptr;
    typedef std::set<Operation::Ptr> OperationSet;

    SYMEVAL_EXPORT static decompose(Instruction::Ptr insn,
                     Address addr,
                     OperationSet &ops);

    SYMEVAL_EXPORT std::string format() const;
    SYMEVAL_EXPORT Address addr() const { return addr_; }
    SYMEVAL_EXPORT Instruction::Ptr insn() const { return insn_; }
    SYMEVAL_EXPORT Absloc::Ptr absloc() const { return absloc_; }

    SYMEVAL_EXPORT Operation(Instruction::Ptr insn,
              Address addr,
              Absloc::Ptr absloc) :
        insn_(insn), addr_(addr), absloc_(absloc) {};

 private:
    Instruction::Ptr insn_;
    Address addr_;
    Absloc::Ptr absloc_;
};
};

#endif
