#include "kernel/types.h"
#include "user.h"

int main(int argc, char *argv[]) {
    int n_processes;
    if (argc != 2) {
        printf("Usage: tournament N\n");
        exit(1);
    }
    n_processes = atoi(argv[1]);
    int idx = tournament_create(n_processes);

    if (idx < 0) {
        printf("tournament_create failed\n");
        exit(1);
    }
    if (tournament_acquire() < 0) {
        printf("Process %d (tid %d): failed to acquire lock\n", getpid(), idx);
        exit(1);
    }
    //Critical Section
    printf("Process %d, Tournament ID: %d in critical section\n", getpid(), idx);
    if (tournament_release() < 0) {
        printf("Process %d (tid %d): failed to release lock\n", getpid(), idx);
        exit(1);
    }
    exit(0);
}