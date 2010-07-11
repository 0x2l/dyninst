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

#include "dynutil/h/ProcReader.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/h/Handler.h"
#include "common/h/addrtranslate.h"

using namespace Dyninst;
using namespace ProcControlAPI;

class PCProcReader : public ProcessReader
{
private:
   int_process *proc;
   
   long word_cache;
   Dyninst::Address word_cache_addr;
   bool word_cache_valid;
public:
   PCProcReader(int_process *proc_);
   virtual ~PCProcReader();
   virtual bool start();
   virtual bool ReadMem(Address addr, void *buffer, unsigned size);
   virtual bool GetReg(MachRegister reg, MachRegisterVal &val);
   virtual bool done();
};


class sysv_process : public int_process
{
 public:
   sysv_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::map<int,int> f);
   sysv_process(Dyninst::PID pid_, int_process *p);
   virtual ~sysv_process();
   virtual bool refresh_libraries(std::set<int_library *> &added_libs,
                                  std::set<int_library *> &rmd_libs);
   virtual bool initLibraryMechanism();

   Dyninst::Address getLibBreakpointAddr() const;

   bool isLibraryTrap(Dyninst::Address trap_addr);
   virtual bool plat_execed();
   virtual int_library *getExecutableLib();
   
 protected:
   static int_breakpoint *lib_trap;

 private:
   std::set<LoadedLib*, LoadedLibCmp> loaded_libs;
   Address breakpoint_addr;
   AddressTranslate *translator;
   bool lib_initialized;
   PCProcReader *procreader;
   int_library *aout;
   static SymbolReaderFactory *symreader_factory;   
};


