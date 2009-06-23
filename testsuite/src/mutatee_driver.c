/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/* Test application (Mutatee) */

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#if defined(os_windows_test)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define snprintf _snprintf
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#ifdef __cplusplus
int mutateeCplusplus = 1;
#else
int mutateeCplusplus = 0;
#endif

#ifndef COMPILER
#define COMPILER ""
#endif
const char *Builder_id=COMPILER; /* defined on compile line */

#ifdef __cplusplus
extern "C" {
#endif
#include "mutatee_util.h"
#ifdef __cplusplus
}
#endif

volatile int isAttached = 0;

#include "mutatee_call_info.h"
/* #include "mutatee_glue.h" */

#if defined(USE_GROUP_MUTATEE)
#include "group_driver.gen.h"
#else
#include "solo_driver.h"
#endif

#ifdef SOLO_MUTATEE
#include "solo_driver.h"
#endif

/* Pointer size variable, for multiple-ABI platforms */
int pointerSize = sizeof (void *);

/* Global copies of argc and argv, for tests that require them */
int gargc;
char **gargv;

int debugPrint = 0;

#define USAGE "Usage: mutatee [-attach <fd>] [-verbose] -run <num> .."

void setRunTest(const char *testname) {
  unsigned int i;
  if (NULL == testname) { /* Sanity check */
    return;
  }

  for (i = 0; i < max_tests; i++) {
    if (!strcmp(testname, mutatee_funcs[i].testname)) {
      runTest[i] = TRUE;
      break;
    }
  }
  if (i >= max_tests) 
  {
    output->log(STDERR, "%s is not a valid test for this mutatee: %s, max_tests = %d\n", testname, gargv[0], max_tests);
  }
} /* setRunTest() */

/*
 * Check to see if the mutator has attached to us.
 */
int checkIfAttached()
{
  return isAttached;
}

/* Note which test we're running so we can detect crashes */
void updateResumeLog(const char *resumelogname, const char *testname) {
  FILE *resumelog;

  resumelog = fopen(resumelogname, "w");
  if (resumelog != NULL) {
    fprintf(resumelog, "%s\n", testname);
    fclose(resumelog);
  } else {
    output->log(STDERR, "Error opening mutatee resumelog: %s\n",
	    strerror(errno));
  }
}

void updateResumeLogCompleted(const char *resumelogname) {
  FILE *resumelog;
  resumelog = fopen(resumelogname, "a");
  if (resumelog != NULL) {
    fprintf(resumelog, "+\n");
    fclose(resumelog);
  } else {
    output->log(STDERR, "Error opening mutatee resumelog: %s\n",
	    strerror(errno));
  }
}

void setLabel(unsigned int label_ndx, char *label) {
  /* TODO Fix me so group mutatees work */
  if (max_tests > 1) {
    output->log(STDERR, "Group mutatees not enabled yet\n");
    return;
  }

  mutatee_funcs[0].testlabel = label;
}

int main(int iargc, char *argv[])
{                                       /* despite different conventions */
   unsigned argc = (unsigned) iargc;   /* make argc consistently unsigned */
   unsigned int i, j;
#if !defined(i386_unknown_nt4_0_test)
   int pfd;
#endif
   int useAttach = FALSE;
   char *logfilename = NULL;
   int allTestsPassed = TRUE;
   int retval;
   char *mutatee_name = NULL;
   unsigned int label_count = 0;
   int print_labels = FALSE;
   int has_pidfile = 0;
#if !defined(os_windows_test)
   struct timeval start_time;
#endif

   gargc = argc;
   gargv = argv;

   initOutputDriver();

   /* Extract the name of the mutatee binary from argv[0] */
   /* Find the last '/' in argv[0]; we want everything after that */
   mutatee_name = strrchr(argv[0], '/');
   if (!mutatee_name) {
      /* argv[0] contains just the filename for the executable */
      mutatee_name = argv[0];
      setExecutableName(argv[0]);
   } else {
      mutatee_name += 1; /* Skip past the '/' */
      setExecutableName(mutatee_name);
   }

   for (j=0; j < max_tests; j++) {
      runTest[j] = FALSE;
   }

   /* Parse command line arguments */
   for (i=1; i < argc; i++) {
      if (!strcmp(argv[i], "-verbose")) {
         debugPrint = 1;
      } else if (!strcmp(argv[i], "-log")) {
         /* Read the log file name so we can set it up later */
         if ((i + 1) >= argc) {
            output->log(STDERR, "Missing log file name\n");
            exit(-1);
         }
         i += 1;
         logfilename = argv[i];
      } else if (!strcmp(argv[i], "-attach")) {
         useAttach = TRUE;
#if !defined(os_windows_test)
         if (++i >= argc) {
            output->log(STDERR, "attach usage\n");
            output->log(STDERR, "%s\n", USAGE);
            exit(-1);
         }
         pfd = atoi(argv[i]);
#endif
      } else if (!strcmp(argv[i], "-run")) {
         char *tests;
         char *name;

         if (i + 1 >= argc) {
            output->log(STDERR, "-run must be followed by a test name\n");
            exit(-1);
         }
         i += 1;
         tests = strdup(argv[i]);
         /* FIXME I think strtok is frowned on */
         name = strtok(tests, ",");
         setRunTest(name); /* Enables the named test to run */
         while (name != NULL) {
            name = strtok(NULL, ",");
            if (name != NULL) {
               setRunTest(name);
            }
         }
         free(tests);
      } else if (!strcmp(argv[i], "-label")) {
         if (i + 1 >= argc) {
            output->log(STDERR, "-label must be followed by a label string\n");
            exit(-1);
         }
         i += 1;
         setLabel(label_count, argv[i]);
         label_count += 1;
      } else if (!strcmp(argv[i], "-print-labels")) {
         print_labels = TRUE;
      } else if (!strcmp(argv[i], "-humanlog")) {
         if (i + 1 >= argc) {
            output->log(STDERR, "-humanlog must be followed by a file name or '-'\n");
            exit(-1);
         }
         i += 1;
         setHumanLog(argv[i]);
      } else if (strcmp(argv[i], "-pidfile") == 0) {
         if (i + 1 >= argc) {
            output->log(STDERR, "-pidfile must be followed by a file name\n");
            exit(-1);
         }
         has_pidfile = 1;
         i += 1;
         setPIDFilename(argv[i]);
      } else if (!strcmp(argv[i], "-runall")) {
         for (j = 0; j < max_tests; j++) {
            runTest[j] = TRUE;
         }
      } else if (!strcmp(argv[i], "-dboutput")) {
         /* Set up database output */
         initDatabaseOutputDriver();
      } else {
         /* Let's just ignore unrecognized parameters.  They might be
          * important to a specific test.
          */
      }
   }

   if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
      /* Set up the log file */
      redirectStream(LOGINFO, logfilename);
      redirectStream(LOGERR, logfilename);
      outlog = fopen(logfilename, "a");
      if (NULL == outlog) {
         output->log(STDERR, "Error opening log file %s\n", logfilename);
         exit(-1);
      }
      errlog = outlog;
   } else {
      outlog = stdout;
      errlog = stderr;
   }

   if ((argc==1) || debugPrint)
      logstatus("Mutatee %s [%s]:\"%s\"\n", argv[0],
                mutateeCplusplus ? "C++" : "C", Builder_id);
   if (argc==1) exit(0);

   /* see if we should wait for the attach */
   if (useAttach) {
#ifndef i386_unknown_nt4_0_test
      char ch = 'T';
      if (write(pfd, &ch, sizeof(char)) != sizeof(char)) {
         output->log(STDERR, "*ERROR*: Writing to pipe\n");
         exit(-1);
      }
      close(pfd);
#else
	   char ch = 'T';
	   LPCTSTR pipeName = "\\\\.\\pipe\\mutatee_signal_pipe";
	   DWORD bytes_written = 0;
	   BOOL wrote_ok = FALSE;
	   HANDLE signalPipe = CreateFile(pipeName,
		   GENERIC_WRITE,
		   0,
		   NULL,
		   OPEN_EXISTING,
		   0,
		   NULL);
	   if(signalPipe == INVALID_HANDLE) 
	   {
		   if(GetLastError() != ERROR_PIPE_BUSY)
		   {
			   output->log(STDERR, "*ERROR*: Couldn't open pipe\n");
			   exit(-1);
		   }
		   if(!WaitNamedPipe(pipeName, 2000))
		   {
			   output->log(STDERR, "*ERROR*: Couldn't open pipe\n");
			   exit(-1);
		   }
	   }
	   wrote_ok = WriteFile(signalPipe,
		   &ch,
		   1,
		   &bytes_written,
		   NULL);
	   if(!wrote_ok ||(bytes_written != 1))
	   {
		   output->log(STDERR, "*ERROR*: Couldn't write to pipe\n");
		   exit(-1);
	   }
	   CloseHandle(signalPipe);
#endif
      setUseAttach(TRUE);
#if defined (os_windows_test)
      logstatus("mutatee: Waiting for mutator to attach...\n");
#else
      logstatus("mutatee %d: Waiting for mutator to attach...\n", getpid());
	  gettimeofday(&start_time, NULL);
#endif
      flushOutputLog();

      while (!checkIfAttached()) {
#if !defined(os_windows_test)
		  struct timeval present_time;
		  gettimeofday(&present_time, NULL);
		  if (present_time.tv_sec > (start_time.tv_sec + 30))
		  {
			  logstatus("mutatee: mutator attach problem, failing...\n");
			  exit(-1);
		  }
#endif
#if 0
		  struct timeval slp;
		  slp.tv_sec = 1;
		  slp.tv_usec = 0;
		  select(0, NULL, NULL, NULL, &slp);

		  elapsed++;
		  if (elapsed > 60) {
			  logstatus("mutatee %d: mutator attach problem, failing...\n", getpid());
			  exit(-1);
		  }
#endif
         /* Do nothing */
      }
      fflush(stderr);
      logstatus("Mutator attached.  Mutatee continuing.\n");
   } else {
      setUseAttach(FALSE);
   }

   /* 
    * Run the tests and keep track of return values in case of test failure
    */
   for (i = 0; i < max_tests; i++) {
      if (runTest[i]) {

         log_testrun(mutatee_funcs[i].testname);

         if (print_labels && (mutatee_funcs[i].testlabel != NULL)) {
            logstatus("%s\n", mutatee_funcs[i].testlabel);
         }

         output->setTestName(mutatee_funcs[i].testname);
         mutatee_funcs[i].func();
         log_testresult(passedTest[i]);
    
         if (!passedTest[i]) {
            allTestsPassed = FALSE;
         }
      }

      flushOutputLog();
      flushErrorLog();
   }

   if (allTestsPassed) {
      logstatus("All tests passed.\n");
      retval = 0;
   } else {
#if 0
	   unsigned int i;
	   for (i = 0; i < max_tests; ++i)
	   {
		   if (runTest[i])
			   logstatus("%s[%d]: %s: %s \n", passedTest[i] ? "PASSED" : "FAILED", __FILE__, __LINE__, 
					   mutatee_funcs[i].testlabel == NULL ? "bad_label" : mutatee_funcs[i].testlabel);
	   }
	   logstatus("\n");
#endif
      retval = -1;
   }

   /* Clean up after ourselves */
   if ((outlog != NULL) && (outlog != stdout)) {
      fclose(outlog);
   }

   exit( retval);
}