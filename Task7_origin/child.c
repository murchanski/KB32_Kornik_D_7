#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <string.h>

int shm_id;
void *shm_ptr;

void signal_handler(int sig) {
    printf("\nChild: signal handler called with signal: %d\n", sig);
    if (sig == SIGUSR1) {
        int sum = 0, i = 0, val;
        printf("Child: Reading data from shared memory...\n");

        // get data from general mem
        do {
            memcpy(&val, shm_ptr + i * sizeof(int), sizeof(int));
            printf("Child: Read value %d\n", val);
            if (val == 0) break;  // break if 0
            sum += val;
            i++;
        } while (1);

        printf("Child: Calculated sum: %d\n", sum);

        // write sum to mem
        memcpy(shm_ptr, &sum, sizeof(int));

        // send signal
        kill(getppid(), SIGUSR1);
        printf("Child: Signal sent back to parent\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <shm_id>\n", argv[0]);
        return 1;
    }

    shm_id = atoi(argv[1]);
    printf("\nChild: Shared memory ID received: %d\n", shm_id);
    shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *) -1) {
        perror("Child: shmat");
        exit(1);
    } else {
        printf("Child: Shared memory attached at address: %p\n", shm_ptr);
    }

    // reg signal check
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigaction(SIGUSR1, &sa, NULL);

    while (1) {
        pause();
    }
    // for independence???
    shmdt(shm_ptr);
    return 0;
}
