#include "kernel/types.h"
#include "user.h"

#define MAX_PROCS 16
#define MAX_LEVELS 4  // log2(MAX_PROCS)

// גלובליים לשימוש בתוך הספרייה
static int num_procs = 0;
static int process_idx = -1;
static int lock_ids[MAX_PROCS - 1];  // מנעולים בעץ
static int num_levels = 0;

// מחשב log2(n) עבור n שהוא חזקה של 2
static int log2_floor(int n) {
  int l = 0;
  while (n > 1) {
    n >>= 1;
    l++;
  }
  return l;
}

// Create tournament tree, fork processes, assign indices, compute lock/role arrays
int tournament_create(int processes) {
   if (processes < 2 || processes > MAX_PROCS)
        return -1;
    if ((processes & (processes - 1)) != 0)
        return -1;  // not power of 2

    num_procs = processes;
    num_levels = log2_floor(processes);

     // צור את המנעולים הדרושים (N-1)
    for (int i = 0; i < processes - 1; i++) {
        int lock_id = peterson_create();
        if (lock_id < 0)
            return -1;
        lock_ids[i] = lock_id;
    }

    // Fork processes and assign index
    int idx = 0; // Parent gets 0
    for (int i = 1; i < processes; i++) {
        int pid = fork();
        if (pid < 0) 
            return -1; //fork failed
        if (pid == 0) {
            idx = i;
            break;
        }
    }
    process_idx = idx;

    //wait for all children
     
    if (process_idx == 0) {
        for(int i = 1; i< processes ; i++)
            wait(0);
    }
    
    return process_idx;
}

// Acquire all locks from leaf to root
int tournament_acquire(void) {
    if (process_idx < 0)
        return -1;

    for (int level = num_levels - 1; level >= 0; level--) {
        int role = (process_idx >> (num_levels - level - 1)) & 1;
        int local_idx = process_idx >> (num_levels - level);
        int lock_index = (1 << level) - 1 + local_idx;
        if (peterson_acquire(lock_ids[lock_index], role) < 0)
            return -1;
    }
    return 0;
}

// Release all locks from root to leaf (reverse order)
int tournament_release(void) {
    if (process_idx < 0)
        return -1;
    for (int level = 0; level < num_levels; level++) {
         int role = (process_idx >> (num_levels - level - 1)) & 1;
        int local_idx = process_idx >> (num_levels - level);
        int lock_index = (1 << level) - 1 + local_idx;
        if (peterson_release(lock_ids[lock_index], role) < 0)
        return -1;
    }
    return 0;
}

    