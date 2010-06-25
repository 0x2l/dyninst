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
#ifndef _PARSING_H_
#define _PARSING_H_

#include "parseAPI/h/CFGFactory.h"
#include "parseAPI/h/CodeObject.h"
#include "parseAPI/h/CodeSource.h"
#include "parseAPI/h/InstructionSource.h"
#include "parseAPI/h/CFG.h"
#include "parseAPI/h/ParseCallback.h"

using namespace Dyninst;

// some useful types
using ParseAPI::EdgeTypeEnum;
using ParseAPI::FuncReturnStatus;
using ParseAPI::FuncSource;

/*** The image_* object factory ***/
class image;

class DynCFGFactory : public ParseAPI::CFGFactory {
  public:
    DynCFGFactory(image * im);
    ~DynCFGFactory();
    
    ParseAPI::Function * mkfunc(Address addr, FuncSource src, std::string name,
            ParseAPI::CodeObject * obj, ParseAPI::CodeRegion * reg,
            InstructionSource * isrc);
    ParseAPI::Block * mkblock(ParseAPI::Function * f, ParseAPI::CodeRegion * r,
            Address addr);
    ParseAPI::Edge * mkedge(ParseAPI::Block * src, ParseAPI::Block * trg, 
            EdgeTypeEnum type);

    ParseAPI::Block * mksink(ParseAPI::CodeObject *obj, ParseAPI::CodeRegion*r);

    // leaving default atm    
    //void free_func(ParseAPI::Function * f);
    //void free_block(ParseAPI::Block * b);
    //void free_edge(ParseAPI::Edge * e);

    //void free_all();
  private:
    image * _img;     
#if defined(VERBOSE_CFG_FACTORY)
#warning "VERBOSE_CFG_FACTORY enabled ; may impact performance"
    vector<int> __func_allocs(__funcsource_end__);
    vector<int> __edge_allocs(__edgetype_end__);
    int __block_allocs;
    int __sink_block_allocs;
    //int __sink_edge_allocs; FIXME can't determine

    void __record_func_alloc(FuncSource fs)
    {
        assert(fs < __funsource_end__);
        ++__func_allocs[fs];
    }
    void __record_edge_alloc(EdgeTypeEnum et,bool sink)
    {
        assert(et < __edgetype_end__);
        ++__edge_allocs[et];

        //if(sink)
            //++__sink_block_allocs;
    }
    void __record_block_alloc(bool sink)
    {
        ++__block_allocs;
        if(sink)
            ++__sink_block_allocs;
    }
    void dump_stats();
#endif
};

class image;
class DynParseCallback : public ParseAPI::ParseCallback {
 public:
  DynParseCallback(image * img) : _img(img) { }
  ~DynParseCallback() { }

  void unresolved_cf(ParseAPI::Function*,Address,default_details*);
  void interproc_cf(ParseAPI::Function*,Address,interproc_details*);

  void overlapping_blocks(ParseAPI::Block*,ParseAPI::Block*);

#if defined(arch_power) || defined(arch_sparc)
  void instruction_cb(ParseAPI::Function*,Address,insn_details*);
#endif
 private:
    image * _img;
};


#endif
