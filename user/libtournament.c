

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
#include "user/user.h"

#define MAX_PROCS 16
#define MAX_LEVELS 4 // log2(16) = 4

// Global variables
static int num_procs = 0;
static int num_levels = 0;
static int lock_ids[(MAX_PROCS * 2) - 1]; // BFS order
static int roles[MAX_LEVELS][MAX_PROCS];  // role per level per process
static int lock_idxs[MAX_LEVELS][MAX_PROCS]; // lock index per level per process
static int proc_index = -1; // 0..N-1

// Helper: log2 of power of 2
static int log2i(int n) {
    int l = 0;
    while (n >>= 1) ++l;
    return l;
}

// Create tournament tree, fork processes, assign indices, compute lock/role arrays
int tournament_create(int processes) {
    if (processes < 2 || processes > MAX_PROCS) return -1;
    // Check power of 2
    if ((processes & (processes - 1)) != 0) return -1;

    num_procs = processes;
    num_levels = log2i(processes);

    // Create all locks in BFS order
    int num_locks = processes - 1;
    for (int i = 0; i < num_locks; i++) {
        lock_ids[i] = peterson_create();
        if (lock_ids[i] < 0) return -1;
    }

    // Precompute roles and lock indices for all processes BEFORE forking
    for (int idx = 0; idx < num_procs; idx++) {
        for (int l = 0; l < num_levels; l++) {
            int role = (idx & (1 << (num_levels - l - 1))) >> (num_levels - l - 1);
            int lock_l = idx >> (num_levels - l);
            int arr_idx = lock_l + (1 << l) - 1;
            roles[l][idx] = role;
            lock_idxs[l][idx] = arr_idx;
        }
    }

    // Fork processes and assign index
    int idx = 0; // Parent gets 0
    for (int i = 1; i < processes; i++) {
        int pid = fork();
        if (pid < 0) return -1;
        if (pid == 0) {
            idx = i;
            break;
        }
    }
    proc_index = idx;
    return idx;

}

// Acquire all locks from leaf to root
int tournament_acquire(void) {
    for (int l = 0; l < num_levels; l++) {
        int arr_idx = lock_idxs[l][proc_index];
        int role = roles[l][proc_index];
        peterson_acquire(lock_ids[arr_idx], role);
    }
    return 0;
}

// Release all locks from root to leaf (reverse order)
int tournament_release(void) {
    for (int l = num_levels - 1; l >= 0; l--) {
        int arr_idx = lock_idxs[l][proc_index];
        int role = roles[l][proc_index];
        peterson_release(lock_ids[arr_idx], role);
    }
    return 0;
}