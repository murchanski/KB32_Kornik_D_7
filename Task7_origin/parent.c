#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define SHM_SIZE 1024

int shm_id;          // mem id
void *shm_ptr;
pid_t child_pid;     // PID child

void signal_handler(int sig) {
    printf("Signal handler called with signal: %d\n", sig);
    if (sig == SIGUSR1) {
        int sum;
        // read sum
        memcpy(&sum, shm_ptr, sizeof(int));
        printf("Sum is: %d\n", sum);
    }
}

int main(int argc, char *argv[]) {
    // create general mem
    shm_id = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    shm_ptr = shmat(shm_id, NULL, 0);

    if (shm_ptr == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    // signal reg
    signal(SIGUSR1, signal_handler);

    // create child
    child_pid = fork();
    if (child_pid == 0) {
        // run child
        char shm_id_str[10];
        sprintf(shm_id_str, "%d", shm_id);
        execlp("./child", "child", shm_id_str, NULL);
        perror("execlp");
        exit(1);
    }
    while (1) {
        int n, i, input;
        printf("Enter quantity of nums to sum (0 - exit): ");
        scanf("%d", &n);

        if (n <= 0) {
            break;
        }

        for (i = 0; i < n; i++) {
            printf("Input num: ");
            scanf("%d", &input);
            memcpy(shm_ptr + i * sizeof(int), &input, sizeof(int));
        }
        int end_marker = 0;
        memcpy(shm_ptr + n * sizeof(int), &end_marker, sizeof(int));

        // send signal
        kill(child_pid, SIGUSR1);
        pause();
    }

    // exit
    kill(child_pid, SIGTERM);
    waitpid(child_pid, NULL, 0);
    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
