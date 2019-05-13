#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include "data.h"

struct prod_line *line;
struct sembuf SEM_P = {0, -1, SEM_UNDO}; // semwait
struct sembuf SEM_V = {0, 1, SEM_UNDO}; // semsignal

int shm_id;
int sem_id;

long truck_cap;
long line_cap;
long line_weight_cap;
long workers_cycles;
long workers_count;
pid_t *workers;

void assert_positive(char *varname, long value) {
    if (value <= 0) {
        printf("%s must be positive\n", varname);
        exit(1);
    }
}

void shm_unmap(void *ptr) {
    if (shmdt(ptr) == -1) {
        printf("Error while detaching shared memory segment\n");
        exit(1);
    }
}

void shm_remove(int shm_id) {
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        printf("Unable to remove shared memory segment\n");
        exit(1);
    }
}

void sem_remove(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        printf("Unable to remove semaphore\n");
        exit(1);
    }
}


void exit_handler() {
    for (int i = 0; i < workers_count; i++) {
        kill(workers[i], SIGINT);
    }

    shm_unmap(line);
    shm_remove(shm_id);
    sem_remove(sem_id);
    exit(0);
}

int shm_create() {
    key_t shm_key = ftok(FTOK_SHM_PATH, FTOK_SHM_SEED);
    int shm_id = shmget(shm_key, sizeof(struct prod_line), SHM_PERM | IPC_CREAT | IPC_EXCL);
    if (shm_id == -1) {
        printf("Unable to create shared memory segment\n");
        exit(1);
    }
    return shm_id;
}

int sem_create() {
    key_t sem_key = ftok(FTOK_SEM_PATH, FTOK_SEM_SEED);
    int sem_id = semget(sem_key, 1, SEM_PERM | IPC_CREAT | IPC_EXCL);
    if (sem_id == -1) {
        printf("Unable to create semaphore\n");
        exit(1);
    }

    union semun {
        int val;
        struct semid_ds *buf;
        ushort array[1];
    } sem_attr;

    sem_attr.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_attr) == -1) {
        printf("Unable to set initial value of semaphore\n");
        exit(1);
    }

    return sem_id;
}

void *shm_map(int shm_id) {
    void *ptr = shmat(shm_id, NULL, 0);
    if (ptr == ((void *) -1)) {
        printf("Error while mapping to shared memory segment\n");
        exit(1);
    }
    return ptr;
}


struct prod_line *shm_shared_line(int shm_id, int cap, int weight) {
    struct prod_line *line = line_new(cap, weight);
    void *ptr = shm_map(shm_id);
    memcpy(ptr, line, sizeof(struct prod_line));
    line_dispose(line);
    return (struct prod_line *) ptr;
}

void sem_lock(int sem_id) {
    if (semop(sem_id, &SEM_P, 1) < 0) {
        printf("Negative value when waiting for semaphore\n");
        exit(1);
    }
}

__time_t get_timestamp() {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    return current_time.tv_sec * (int) 1e6 + current_time.tv_usec;
}

void print_timestamped(char *str) {
    printf("[%ld]: %s\n", get_timestamp(), str);
}

void sem_free(int sem_id) {
    if (semop(sem_id, &SEM_V, 1) < 0) {
        printf("Negative value when freeing semaphore\n");
        exit(1);
    }
}


void truck_logic() {
    char message[256];
    long loaded = 0;
    while (1) {
        sem_lock(sem_id);
        {
            if (loaded == truck_cap) {
                sprintf(message, "TRUCK: Truck full! Sending new one...");
                print_timestamped(message);
                loaded = 0;
                sprintf(message, "TRUCK: New truck ready. Capacity: %ld/%ld.", loaded, truck_cap);
                print_timestamped(message);
            }

            if (line->size > 0) {
                struct prod_node *package = line_oldest(line);
                unsigned long time_diff = get_timestamp() - package->timestamp;
                loaded++;
                sprintf(message, "TRUCK: Loading package %d/%ld. Weight: %ld. Truck: %ld/%ld. Time diff: %ld",
                        package->producer, package->ordnum, package->weight, loaded, truck_cap, time_diff);
                print_timestamped(message);
            }
        }
        sem_free(sem_id);
    }
}

void start_workers(int count) {
    workers = calloc((size_t) count, sizeof(pid_t));
    for (int i = 0; i < count; i++) {
        int N = rand() % (line_weight_cap - 1) + 1;
        int C = workers_cycles;

        pid_t pid = fork();
        if (pid == 0) {
            char N_str[10];
            char C_str[10];
            sprintf(N_str, "%d", N);
            sprintf(C_str, "%d", C);

            // For workes in infite loops, replace with:
            // execl(LOADER_EXECUTABLE, LOADER_EXECUTABLE,  N_str, NULL)
            if (execl(LOADER_EXECUTABLE, LOADER_EXECUTABLE, N_str, C_str, NULL) == -1) {
                printf("Unable to exec %s. PID %d\n", LOADER_EXECUTABLE, getpid());
                exit(1);
            }
        }
        workers[i] = pid;
    }
}


int main(int argc, char **argv) {
    if (argc != 6) {
        printf("Invalid number of arguments.\n");
        printf("Usage: %s <W> <WC> <X> <K> <M>\nW  - workers count\nWC - worker cycles \nX  - truck capacity\nK  - production line capcity\nM  - production line maximum weight\n",
               argv[0]);
        exit(1);
    }

    workers_count = strtol(argv[1], NULL, 10);
    workers_cycles = strtol(argv[2], NULL, 10);
    truck_cap = strtol(argv[3], NULL, 10);
    line_cap = strtol(argv[4], NULL, 10);
    line_weight_cap = strtol(argv[5], NULL, 10);

    assert_positive("Workers count", workers_count);
    assert_positive("Workers cycles", workers_cycles);
    assert_positive("Truck capacity", truck_cap);
    assert_positive("Line capacity", line_cap);
    assert_positive("Line weight capacity", line_weight_cap);
    if (line_cap > MAX_LINE_CAPACITY || truck_cap > MAX_LINE_CAPACITY) {
        printf("Maximum allowed line/truck capacity is %d\n", MAX_LINE_CAPACITY);
        exit(1);
    }

    srand(time(NULL));
    signal(SIGINT, exit_handler);

    shm_id = shm_create();
    sem_id = sem_create();
    printf("SHM ID: %d\n", shm_id);
    printf("SEM ID: %d\n", sem_id);

    line = shm_shared_line(shm_id, (int) line_cap, (int) line_weight_cap);

    if (fork() == 0) {
        truck_logic();
        exit(0);
    }

    start_workers((int) workers_count);

    // Wait for user to SIGINT
    while (1) {}
}
