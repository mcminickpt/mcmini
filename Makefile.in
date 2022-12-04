ROOT=$$PWD/

CC=gcc
CXX=g++
# The defines needed for MCSharedLibraryWrappers.c (and maybe other things???).
override CFLAGS+=-I${ROOT}/include -fPIC -DMC_SHARED_LIBRARY=1 -Dmcmini_checker_EXPORTS
override CXXFLAGS=${CFLAGS} -std=gnu++11

LIBOBJS1=src/MCObjectStore.o src/MCSharedTransition.o src/MCTransition.o src/mcmini_private.o src/MCState.o src/MCTransitionFactory.o src/MCStateStackItem.o src/MCThreadData.o src/MCClockVector.o src/signals.o

LIBOBJS2=src/misc/cond/MCCondVarWakeupPolicyImpl.o src/misc/cond/MCCondVarArbitraryWakeupPolicy.o src/misc/cond/MCCondVarOrderedWakeupPolicy.o src/misc/cond/MCCondVarGLibcSignalPolicy.o src/misc/cond/MCCondVarSingleGroupSignalPolicy.o src/misc/cond/MCWakeGroup.o

LIBOBJS3=src/objects/MCThread.o src/objects/MCVisibleObject.o src/objects/MCMutex.o src/objects/MCRWLock.o src/objects/MCRWWLock.o src/objects/MCSemaphore.o src/objects/MCGlobalVariable.o src/objects/MCBarrier.o src/objects/MCConditionVariable.o

LIBOBJS4=src/transitions/barrier/MCBarrierEnqueue.o src/transitions/barrier/MCBarrierInit.o src/transitions/barrier/MCBarrierWait.o src/transitions/cond/MCCondBroadcast.o src/transitions/cond/MCCondEnqueue.o src/transitions/cond/MCCondInit.o src/transitions/cond/MCCondSignal.o src/transitions/cond/MCCondWait.o src/transitions/MCTransitionsShared.o src/transitions/misc/MCAbortTransition.o src/transitions/misc/MCExitTransition.o src/transitions/misc/MCGlobalVariableRead.o src/transitions/misc/MCGlobalVariableWrite.o src/transitions/mutex/MCMutexInit.o src/transitions/mutex/MCMutexLock.o src/transitions/mutex/MCMutexUnlock.o src/transitions/semaphore/MCSemEnqueue.o src/transitions/semaphore/MCSemInit.o src/transitions/semaphore/MCSemPost.o src/transitions/semaphore/MCSemWait.o src/transitions/threads/MCThreadCreate.o src/transitions/threads/MCThreadFinish.o src/transitions/threads/MCThreadJoin.o src/transitions/threads/MCThreadStart.o src/transitions/rwlock/MCRWLockInit.o src/transitions/rwlock/MCRWLockReaderLock.o src/transitions/rwlock/MCRWLockWriterLock.o src/transitions/rwlock/MCRWLockUnlock.o src/transitions/rwlock/MCRWLockWriterEnqueue.o src/transitions/rwlock/MCRWLockReaderEnqueue.o src/transitions/rwwlock/MCRWWLockInit.o src/transitions/rwwlock/MCRWWLockReaderEnqueue.o src/transitions/rwwlock/MCRWWLockReaderLock.o src/transitions/rwwlock/MCRWWLockWriter1Enqueue.o src/transitions/rwwlock/MCRWWLockWriter1Lock.o src/transitions/rwwlock/MCRWWLockWriter2Enqueue.o src/transitions/rwwlock/MCRWWLockWriter2Lock.o src/transitions/rwwlock/MCRWWLockUnlock.o src/transitions/wrappers/MCBarrierWrappers.o src/transitions/wrappers/MCConditionVariableWrappers.o src/transitions/wrappers/MCGlobalVariableWrappers.o src/transitions/wrappers/MCMutexTransitionWrappers.o src/transitions/wrappers/MCSemaphoreTransitionWrappers.o src/transitions/wrappers/MCSharedLibraryWrappers.o src/transitions/wrappers/MCThreadTransitionWrappers.o src/transitions/wrappers/MCRWLockWrappers.o src/transitions/wrappers/MCRWWLockWrappers.o

LIBOBJS=${LIBOBJS1} ${LIBOBJS2} ${LIBOBJS3} ${LIBOBJS4} \
       	src/mc_shared_cv.o src/MCCommon.o src/main.o

all: mcmini libmcmini.so mcmini-demo

# If McMini was already built, first do:  'make clean'
debug:
	${MAKE} CFLAGS='-g3 -O0' CXXFLAGS='-g3 -O0' all

test/program/producer_consumer: test/program/producer_consumer.c
	cd test/program && make producer_consumer

check: all test/program/producer_consumer
	./mcmini test/program/producer_consumer --quiet

# If McMini was already built, first do:  'make clean'
check-gdb: debug test/program/producer_consumer
	@ if gdb --version | grep '\<[0-9][0-9]*\.[0-9]' | \
	 sed -e 's%^.* \([0-9][0-9]*\.[0-9]\).*$$%\1%' | grep --quiet 8; then \
	  echo GDB version is 8.x.  Version 8.1 known to have a bug; \
	  echo "  when using 'set detach-fork-mode 0' needed by McMini GDB."; \
	  echo After 15 seconds, GDB will start, but it may not work properly.;\
	  sleep 15; \
	fi
	gdb -x gdbinit --args ./mcmini test/program/producer_consumer --quiet

mcmini: src/launch.c libmcmini.so
	${CC} -g3 -O0 -Iinclude -o $@ $<

libmcrwlock_lib.a: src/export/rwwlock.o
	ar qc $@ $<
	ranlib $@

libmcmini.so: ${LIBOBJS}
	${CXX} -fPIC -shared -Wl,-soname,$@ -o $@ ${LIBOBJS} -pthread -lrt -lm -ldl

mcmini-demo: ${LIBOBJS} libmcrwlock_lib.a
	${CXX} -g3 ${LIBOBJS} -pthread -o $@  -lrt -lm -ldl libmcrwlock_lib.a

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c -o $@ $<

clean:
	rm -f ${LIBOBJS} mcmini libmcmini.so mcmini-demo libmcrwlock_lib.a
distclean: clean
	rm -f Makefile mcmini-gdb

dist: distclean
	dir=`basename $$PWD` && cd .. && tar zcvf $$dir.tar.gz ./$$dir
	dir=`basename $$PWD` && ls -l ../$$dir.tar.gz
