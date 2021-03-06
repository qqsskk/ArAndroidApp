/*
MobileRobots Advanced Robotics Interface for Applications (ARIA)
Copyright (C) 2004, 2005 ActivMedia Robotics LLC
Copyright (C) 2006, 2007, 2008, 2009 MobileRobots Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
MobileRobots Inc, 10 Columbia Drive, Amherst, NH 03031; 800-639-9481
*/
#ifndef ARMUTEX_H
#define ARMUTEX_H

#ifndef WIN32
#include <pthread.h>
#endif
#include <string>
#include "ariaTypedefs.h"

class ArTime;
class ArFunctor;

/// Mutex wrapper class
/**
   This class wraps the operating system's mutex functions. It allows mutualy
   exclusive access to a critical section. This is extremely useful for
   multiple threads which want to use the same variable. On Linux, ArMutex simply
   uses the POSIX pthread interface in an object oriented manner. It also
   applies the same concept to Windows using Windows' own abilities to restrict
   access to critical sections.

   @note ArMutex is by default a recursive mutex. This means that a 
      thread is allowed to acquire an additional lock (whether via lock() or tryLock())
      on a locked mutex if that same thread already has the lock. This allows a thread
      to lock a mutex, but not become deadlocked if any functions called while it 
      is locked also attempt to lock the mutex, while still preventing other threads
      from interrupting it.
      If you want a non-recursive mutex, so that multiple attempts by the same thread 
      to lock a mutex to block, supply an argument of 'false' to the constructor.
*/
class ArMutex
{
public:

#ifdef WIN32
  typedef HANDLE MutexType;
#else
  typedef pthread_mutex_t MutexType;
#endif

  typedef enum {
    STATUS_FAILED_INIT=1, ///< Failed to initialize
    STATUS_FAILED, ///< General failure
    STATUS_ALREADY_LOCKED ///< Mutex already locked
  } Status;

  /// Constructor
  AREXPORT ArMutex(bool recursive = true);
  /// Destructor
  AREXPORT virtual ~ArMutex();
  /// Copy constructor
  AREXPORT ArMutex(const ArMutex &mutex);

  /** Lock the mutex
   *
   *  If this is a recursive mutex (the default type), and a <i>different</i>
   *  thread has locked this mutex, then block until it is unlocked; if
   *  <i>this</i> thread has locked this mutex, then continue immediately and do
   *  not block.
   *
   *  If this is <i>not</i> a recursive mutex, then block if any thread
   *  (including this thread) has locked this mutex.
   *
   *  Call setLog(true) to enable extra logging.
   *
   *  @return 0 if the mutex was successfully locked (after blocking if it was
   *    already locked by another thread, or by any thread if this is <i>not</i> a
   *    recursive mutex).
   *  @return ArMutex::STATUS_ALREADY_LOCKED immediately if this is a recursive mutex
   *    (default) and the current thread has already locked this mutex.
   *  @return ArMutex::STATUS_FAILED on an error from the platform mutex
   *    implementation.
   *  @return ArMutex::STATUS_FAILED_INIT if the platform threading is not 
   *    enabled, initialized, etc.
   */
  AREXPORT virtual int lock();

  /** Try to lock the mutex, but do not block
   *
   *  If this is a recursive mutex (the default type), and a <i>different</i>
   *  thread has locked this mutex, then return ArMutex::STATUS_ALREADY_LOCKED; if
   *  <i>this</i> thread has locked this mutex, then return 0.
   *
   *  If this is <i>not</i> a recursive mutex, then return 0 if any thread
   *  (including this thread) has locked this mutex.
   *
   *  Returns ArMutex::STATUS_FAILED or ArMutex::STATUS_FAILED_INIT on an error (such as threading
   *  not initialized or supported).
   *
   *  Call setLog(true) to enable extra logging.
   *
   *  @return 0 If tryLock() acquires the lock, or mutex is a recursive mutex
   *    (default) and is already locked by this thread
   *  @return ArMutex::STATUS_ALREADY_LOCKED if this mutex is currently locked
   *    by another thread, or if mutex is <i>not</i> recursive, by any thread
   *    including the current thread.
   *  @return ArMutex::STATUS_FAILED on an error from the platform mutex
   *    implementation.
   *  @return ArMutex::STATUS_FAILED_INIT if the platform threading is not 
   *    enabled, initialized, etc.
   */
  AREXPORT virtual int tryLock();

  /// Unlock the mutex, allowing another thread to obtain the lock
  AREXPORT virtual int unlock();

  /// Get a human readable error message from an error code
  AREXPORT virtual const char * getError(int messageNumber) const;
  /** Sets a flag that will log out when we lock and unlock. Use setLogName() to
    set a descriptive name for this mutex, and ArThread::setThreadName() to set a
    descriptive name for a thread.
  */
  void setLog(bool log) { myLog = log; } 
  /// Sets a name we'll use to log with
  void setLogName(const char *logName) { myLogName = logName; } 
#ifndef SWIG
  /// Sets a name we'll use to log with formatting
  /** @swigomit use setLogName() */
  AREXPORT void setLogNameVar(const char *logName, ...);
#endif
  /// Get a reference to the underlying OS-specific mutex variable
  AREXPORT virtual MutexType & getMutex() {return(myMutex);}
  /** Sets the lock warning time (sec). If it takes more than @a lockWarningSeconds to perform the mutex lock in lock(), log a warning.
      @linuxonly
  */
  static void setLockWarningTime(double lockWarningSeconds) 
    { ourLockWarningMS = (unsigned int)(lockWarningSeconds*1000.0); }
  /** Gets the lock warning time (sec)
      @linuxonly
  */
  static double getLockWarningTime(void)
    { return ourLockWarningMS/1000.0; }
  /** Sets the unlock warning time (sec). If it takes more than @a unlockWarningSeconds between the mutex being locked with lock() then unlocked with unlock(), log a warning.
      @linuxonly
  */
  static void setUnlockWarningTime(double unlockWarningSeconds) 
    { ourUnlockWarningMS = (unsigned int)(unlockWarningSeconds*1000.0); }
  /** Gets the lock warning time (sec)
      @linuxonly
  */
  static double getUnlockWarningTime(void)
    { return ourUnlockWarningMS/1000.0; }
protected:
  
  bool myFailedInit;
  MutexType myMutex;
// Eliminating this from Windows in an attempt to debug a memory issue
#ifndef WIN32
  ArStrMap myStrMap;
#endif 

  bool myLog;
  std::string myLogName;

  bool myNonRecursive;
  bool myWasAlreadyLocked;

  bool myFirstLock;
  ArTime *myLockTime;
  AREXPORT static unsigned int ourLockWarningMS;
  AREXPORT static unsigned int ourUnlockWarningMS;
  ArTime *myLockStarted;
  // Intialize lock timing state. Call in ArMutex constructor.
  void initLockTiming();
  // Destroy lock timing state. Call in destructor.
  void uninitLockTiming();
  // Start timing how long it takes to perform the mutex lock.
  void startLockTimer();
  // Check time it took to lock the mutex against ourLockWarningMS and log about it
  void checkLockTime();
  // Start timing how long it takes between this mutex lock and its next unlock.
  void startUnlockTimer();
  // Check time it took between lock and unlock against ourUnlockWarningMS and log about it
  void checkUnlockTime();


  static ArFunctor *ourNonRecursiveDeadlockFunctor;
};


#endif // ARMUTEX_H
