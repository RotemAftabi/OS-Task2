#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
#define O_TRUNC   0x400

#include "kernel/types.h"
#include "user.h"

#define FNAME "tournament_test.txt"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: tournament_filetest N\n");
        exit(1);
    }
    int n = atoi(argv[1]);
    int idx = tournament_create(n);
    if (idx < 0) {
        printf("tournament_create failed\n");
        exit(1);
    }
    sleep(10); // Let all processes start

    for (int i = 0; i < 3; i++) {
        tournament_acquire();

        int fd = open(FNAME, O_RDWR | O_CREATE);
        if (fd < 0) {
            printf("PID %d: failed to open file\n", getpid());
            tournament_release();
            exit(1);
        }

        // Read the current value
        char buf[16];
        int val = 0;
        int nread = read(fd, buf, sizeof(buf)-1);
        buf[nread > 0 ? nread : 0] = 0;
        if (nread > 0)
            val = atoi(buf);

        // Simulate work
        sleep(5);

        val++;
        // Rewind to start: close and reopen with O_TRUNC to overwrite
        close(fd);
        fd = open(FNAME, O_WRONLY | O_CREATE | O_TRUNC);
        if (fd < 0) {
            printf("PID %d: failed to reopen file\n", getpid());
            tournament_release();
            exit(1);
        }
        // Write new value
        int len = 0;
        int tmp = val;
        do {
            tmp /= 10;
            len++;
        } while (tmp);
        // Write as decimal string
        buf[0] = 0;
        int v = val, p = len - 1;
        while (p >= 0) {
            buf[p--] = '0' + (v % 10);
            v /= 10;
        }
        buf[len++] = '\n';
        write(fd, buf, len);
        close(fd);

        printf("PID %d: tournament idx %d wrote value %d\n", getpid(), idx, val);

        tournament_release();
        sleep(10); // Let others go
    }
    exit(0);
}