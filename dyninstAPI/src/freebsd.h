/*
 * Copyright (c) 1996-2010 Barton P. Miller
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

#if !defined(os_freebsd)
#error "invalid architecture-os inclusion"
#endif

#ifndef FREEBSD_PD_HDR
#define FREEBSD_PD_HDR

class process;

#include <sys/param.h>
#include <pthread.h>

#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"

struct dyn_saved_regs {
    int placeholder;
};

/* More than the number of bundles necessary for loadDYNINSTlib()'s code. */
#define CODE_BUFFER_SIZE	512
#define BYTES_TO_SAVE		(CODE_BUFFER_SIZE * 16)

#define SIGNAL_HANDLER "no_signal_handler"

#if defined(arch_x86) || defined(arch_x86_64)
//Constant values used for the registers in the vsyscall page.
#define DW_CFA  0
#define DW_ESP 4
#define DW_EBP 5
#define DW_PC  8

#define MAX_DW_VALUE 17

Address getRegValueAtFrame(void *ehf, Address pc, int reg, 
                           Address *reg_map,
                           process *p, bool *error);
#endif

typedef int handleT; // a /proc file descriptor

#if defined(i386_unknown_freebsd8_0) || defined(i386_unknown_freebsd7_0) || defined(amd64_unknown_freebsd7_0)
#include "freebsd-x86.h"
#else
#error Invalid or unknown architecture-os inclusion
#endif

#include "unix.h"

//  no /proc, dummy function
typedef int procProcStatus_t;

#define INDEPENDENT_LWP_CONTROL false

#endif
