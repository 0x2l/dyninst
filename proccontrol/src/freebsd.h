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
#if !defined(FREEBSD_H_)
#define FREEBSD_H_

#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Decoder.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/int_thread_db.h"
#include "common/h/dthread.h"

using namespace Dyninst;
using namespace ProcControlAPI;

class GeneratorFreeBSD : public GeneratorMT
{
public:
    GeneratorFreeBSD();
    virtual ~GeneratorFreeBSD();

    virtual bool initialize();
    virtual bool canFastHandle();
    virtual ArchEvent *getEvent(bool block);
};

class ArchEventFreeBSD : public ArchEvent
{
public:
    int status;
    pid_t pid;
    lwpid_t lwp;
    bool interrupted;
    int error;

    ArchEventFreeBSD(bool inter_);
    ArchEventFreeBSD(pid_t p, lwpid_t l, int s);
    ArchEventFreeBSD(int e);

    virtual ~ArchEventFreeBSD();
};

class DecoderFreeBSD : public Decoder
{
public:
    DecoderFreeBSD();
    virtual ~DecoderFreeBSD();
    virtual unsigned getPriority() const;
    virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events);
    Dyninst::Address adjustTrapAddr(Dyninst::Address address, Dyninst::Architecture arch);
};

class freebsd_process : public thread_db_process
{
public:
    freebsd_process(Dyninst::PID p, std::string e, std::vector<std::string> a);
    freebsd_process(Dyninst::PID pid_, int_process *p);
    virtual ~freebsd_process();

    virtual bool plat_create();
    virtual bool plat_attach();
    virtual bool plat_forked();
    virtual bool post_forked();
    virtual bool plat_execed();
    virtual bool plat_detach();
    virtual bool plat_terminate(bool &needs_sync);

    virtual bool plat_readMem(int_thread *thr, void *local,
                              Dyninst::Address remote, size_t size);
    virtual bool plat_writeMem(int_thread *thr, void *local,
                               Dyninst::Address remote, size_t size);

    virtual bool needIndividualThreadAttach();
    virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps);
    virtual Dyninst::Architecture getTargetArch();
    virtual bool plat_individualRegAccess();
    virtual bool plat_contProcess();

    /* thread_db_process methods */
    virtual string getThreadLibName(const char *symName);
    virtual bool isSupportedThreadLib(const string &libName);
    virtual bool plat_readProcMem(void *local,
            Dyninst::Address remote, size_t size);
    virtual bool plat_writeProcMem(void *local,
            Dyninst::Address remote, size_t size);
    virtual bool plat_getLWPInfo(lwpid_t lwp, void *lwpInfo);
    virtual bool plat_contThread(lwpid_t lwp);
    virtual bool plat_stopThread(lwpid_t lwp);

protected:
    string libThreadName;
};

class freebsd_thread : public thread_db_thread
{
public:
    freebsd_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);

    freebsd_thread();
    virtual ~freebsd_thread();

    virtual bool plat_cont(bool user_cont);
    virtual bool plat_stop();
    virtual bool plat_getAllRegisters(int_registerPool &reg);
    virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
    virtual bool plat_setAllRegisters(int_registerPool &reg);
    virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);
    virtual bool attach();

    /* FreeBSD-specific */
    virtual bool plat_setStep();
};

class FreeBSDSyncHandler : public Handler
{
public:
    FreeBSDSyncHandler();
    virtual ~FreeBSDSyncHandler();
    virtual bool handleEvent(Event::ptr ev);
    virtual int getPriority() const;
    void getEventTypesHandled(std::vector<EventType> &etypes);
};

#endif
