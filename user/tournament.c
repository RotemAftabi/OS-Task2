typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
#include "user.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: tournament N\n");
        exit(1);
    }
    int n = atoi(argv[1]);
    int idx = tournament_create(n);
    if (idx < 0) {
        printf("tournament_create failed\n");
        exit(1);
    }
    sleep(10); // Let all processes start
    tournament_acquire();
    printf("PID %d: tournament index %d entering critical section\n", getpid(), idx);
    sleep(20); // Simulate work
    printf("PID %d: tournament index %d leaving critical section\n", getpid(), idx);
    tournament_release();
    exit(0);
}