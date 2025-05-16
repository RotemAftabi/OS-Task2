#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "petersonlock.h"  

#define MAX_PETERSON_LOCKS 15
struct petersonlock peterson_locks[MAX_PETERSON_LOCKS];

// Initialize the global Peterson lock array.
// This function must be called once at system startup,
// before any Peterson lock is used.
void init_peterson_locks(void){
  for (int i = 0; i < MAX_PETERSON_LOCKS; i++) {
    peterson_locks[i].flag[0] = 0;
    peterson_locks[i].flag[1] = 0;
    peterson_locks[i].turn = 0;
    peterson_locks[i].in_use = 0;  // mark lock as available
  }
}

int peterson_create(void){
  for (int i = 0; i < MAX_PETERSON_LOCKS; i++) {
    // Try to atomically claim the lock slot
    if (__sync_lock_test_and_set(&peterson_locks[i].in_use, 1) == 0) {
      // Slot was free; we now own it
      __sync_synchronize();  // Ensure memory visibility
      peterson_locks[i].flag[0] = 0;
      peterson_locks[i].flag[1] = 0;
      peterson_locks[i].turn = 0;
      return i;
    }
  }
  return -1; // No available locks
}

int peterson_acquire(int lock_id, int role){
  if (lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS || (role != 0 && role != 1))
    return -1;
    
  struct petersonlock *lock = &peterson_locks[lock_id];
  if (lock->in_use == 0)
    return -1;
  __sync_synchronize();
  __sync_lock_test_and_set(&lock->flag[role], 1);
  __sync_synchronize();
  __sync_lock_test_and_set(&lock->turn, role);
  __sync_synchronize(); 

  while(lock->flag[1-role] && lock->turn == role) {
    yield();
    __sync_synchronize();                         
  }
  return 0;
}

int peterson_release(int lock_id, int role){
  if (lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS || (role != 0 && role != 1))
    return -1;

  struct petersonlock *lock = &peterson_locks[lock_id];
  if (lock->in_use == 0)
    return -1;

  __sync_synchronize();
  __sync_lock_release(&lock->flag[role]);
  //__sync_synchronize();

  return 0;
}

int peterson_destroy(int lock_id){
  if (lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS)
    return -1;

  struct petersonlock *lock = &peterson_locks[lock_id];
  if (lock->in_use == 0)
    return -1;

  __sync_synchronize();
  if(__sync_lock_test_and_set(&lock->in_use, 0)!= 1){
    return -1;
  }
  __sync_lock_release(&lock->flag[0]);
  __sync_lock_release(&lock->flag[1]);
  __sync_synchronize();
  
  return 0;
}
