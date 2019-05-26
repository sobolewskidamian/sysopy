#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define MUTEX_ENTER 0
#define MUTEX_BUTTON 1
#define MUTEX_LEAVE 2
#define MUTEX_COUNT 3

#define STATE_NOT_ACCEPTING 0
#define STATE_ACCEPTING 1
#define STATE_EXITING 2
#define STATE_RIDING 3
#define STATE_WAITING_FOR_START 4
#define STATE_STARTED 5

struct Cart {
    int cart_id;
    int cart_state;
    int rides_done;
    int passengers_inside;
    struct Cart *next;
    struct Cart *prev;
};

struct Passenger {
    int passenger_id;
    int passenger_state;
    struct Passenger *next;
    struct Passenger *prev;
};

struct T_Arg {
    int identifier;
    int rides_count;
    int cart_capacity;
    struct Cart **cart;
    struct Cart *cart_head;
    struct Passenger **passenger;
    struct Passenger *passenger_head;
    pthread_mutex_t *mutex[MUTEX_COUNT];
    pthread_cond_t *cond[MUTEX_COUNT];
};

struct Cart *init_roller_coaster(int size) {
    struct Cart *head = malloc(sizeof(struct Cart) * size);
    struct Cart *current = head;

    for (int i = 0; i < size; i++) {
        current->cart_id = i;
        current->cart_state = STATE_NOT_ACCEPTING;
        current->rides_done = 0;
        current->passengers_inside = 0;

        if (i > 0)
            current->prev = current - 1;
        if (i < size - 1)
            current->next = current + 1;

        current++;
    }

    current--;
    current->next = head;
    head->prev = current;

    return head;
}

struct Passenger *init_passenger_queue(int size) {
    struct Passenger *head = malloc(sizeof(struct Passenger) * size);
    struct Passenger *current = head;

    for (int i = 0; i < size; i++) {
        current->passenger_id = i;
        current->passenger_state = STATE_NOT_ACCEPTING;

        if (i > 0)
            current->prev = current - 1;
        if (i < size - 1)
            current->next = current + 1;

        current++;
    }

    current--;
    current->next = head;
    head->prev = current;

    return head;
}

void spawn_threads(pthread_t *tids, int size, void *(*routine)(void *), struct T_Arg *arg) {
    for (int i = 0; i < size; i++) {
        struct T_Arg *targ = malloc(sizeof(struct T_Arg));
        memcpy(targ, arg, sizeof(struct T_Arg));
        targ->identifier = i;
        pthread_create(&tids[i], NULL, routine, targ);
    }
}

void join_threads(pthread_t *tids, int size) {
    for (int i = 0; i < size; i++) {
        void *ret;
        pthread_join(tids[i], &ret);
    }
}

void print_timestamped(char *str) {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    __time_t timestamp = current_time.tv_sec * (int) 1e6 + current_time.tv_usec;
    printf("[%ld]: %s", timestamp, str);
}

void cart_test_stop_cond(struct T_Arg *targ) {
    if (targ->cart_head->prev->rides_done < targ->rides_count)
        return;

    char message[256];
    sprintf(message, "C%d: ending execution\n", targ->identifier);
    print_timestamped(message);
    free(targ);
    pthread_exit(NULL);
}

void passenger_test_stop_cond(struct T_Arg *targ) {
    if (targ->cart_head->prev->rides_done < targ->rides_count)
        return;

    char message[256];
    sprintf(message, "P%d: ending execution\n", targ->identifier);
    print_timestamped(message);
    free(targ);
    pthread_exit(NULL);
}

void *cart_logic(void *arg) {
    struct T_Arg *targ = (struct T_Arg *) arg;
    int this_id = targ->identifier;
    struct Cart *cart_head = targ->cart_head;
    struct Cart *this_cart = cart_head + this_id;
    char message[256];

    // While last cart haven't done specific number of rides
    while (cart_head->prev->rides_done < targ->rides_count) {
        // While not in place for accepting passegners
        while (*targ->cart != this_cart) {}

        cart_test_stop_cond(targ);

        sprintf(message, "C%d: opening doors\n", this_id);
        print_timestamped(message);

        this_cart->cart_state = STATE_ACCEPTING;

        // While not full
        pthread_mutex_lock(targ->mutex[MUTEX_ENTER]);
        while (this_cart->passengers_inside < targ->cart_capacity) {
            pthread_cond_wait(targ->cond[MUTEX_ENTER], targ->mutex[MUTEX_ENTER]);
            pthread_mutex_unlock(targ->mutex[MUTEX_ENTER]);
        }

        this_cart->cart_state = STATE_WAITING_FOR_START;

        while (this_cart->cart_state != STATE_STARTED) {}

        sprintf(message, "C%d: closing doors\n", this_id);
        print_timestamped(message);
        sprintf(message, "C%d: starting ride\n", this_id);
        print_timestamped(message);

        this_cart->cart_state = STATE_RIDING;
        *targ->cart = (*targ->cart)->next;

        sleep(1);

        while (*targ->cart != this_cart)
            cart_test_stop_cond(targ);

        this_cart->cart_state = STATE_EXITING;

        sprintf(message, "C%d: ride done. waiting for passengers to exit\n", this_id);
        print_timestamped(message);
        sprintf(message, "C%d: opening doors\n", this_id);
        print_timestamped(message);

        pthread_mutex_lock(targ->mutex[MUTEX_LEAVE]);
        while (this_cart->passengers_inside > 0) {
            pthread_cond_wait(targ->cond[MUTEX_LEAVE], targ->mutex[MUTEX_LEAVE]);
            pthread_mutex_unlock(targ->mutex[MUTEX_LEAVE]);
        }

        sprintf(message, "C%d: closing doors\n", this_id);
        print_timestamped(message);

        this_cart->cart_state = STATE_NOT_ACCEPTING;
        this_cart->rides_done++;
    }

    sprintf(message, "C%d: ending execution\n", this_id);
    print_timestamped(message);

    free(arg);
    return NULL;
}

void *passenger_logic(void *arg) {
    struct T_Arg *targ = (struct T_Arg *) arg;
    int this_id = targ->identifier;
    struct Passenger *passenger_head = targ->passenger_head;
    struct Passenger *this_passenger = passenger_head + this_id;
    struct Cart *this_cart;
    char message[256];

    while (targ->cart_head->prev->rides_done < targ->rides_count) {
        while (*targ->passenger != this_passenger)
            passenger_test_stop_cond(targ);

        // While cart not accepting
        while ((*targ->cart)->cart_state != STATE_ACCEPTING || (*targ->cart)->passengers_inside >= targ->cart_capacity)
            passenger_test_stop_cond(targ);

        pthread_mutex_lock(targ->mutex[MUTEX_ENTER]);
        this_cart = *targ->cart;
        this_cart->passengers_inside++;
        *targ->passenger = (*targ->passenger)->next;
        sprintf(message, "P%d: entered C%d. State: %d\\%d\n",
                this_id, this_cart->cart_id, this_cart->passengers_inside, targ->cart_capacity);
        print_timestamped(message);

        pthread_cond_broadcast(targ->cond[MUTEX_ENTER]);
        pthread_mutex_unlock(targ->mutex[MUTEX_ENTER]);

        // Wait until cart gets full
        while (this_cart->cart_state != STATE_WAITING_FOR_START &&
               this_cart->cart_state != STATE_STARTED &&
               this_cart->cart_state != STATE_RIDING) {}

        // Press start
        pthread_mutex_lock(targ->mutex[MUTEX_BUTTON]);
        if (this_cart->cart_state == STATE_WAITING_FOR_START) {
            sprintf(message, "P%d: pressed 'start' button in C%d\n", this_id, this_cart->cart_id);
            print_timestamped(message);
            this_cart->cart_state = STATE_STARTED;
        }
        pthread_mutex_unlock(targ->mutex[MUTEX_BUTTON]);

        // Wait until ride finish
        while (this_cart->cart_state != STATE_EXITING)
            passenger_test_stop_cond(targ);

        pthread_mutex_lock(targ->mutex[MUTEX_LEAVE]);
        this_cart->passengers_inside--;
        sprintf(message, "P%d: left C%d. State: %d\\%d\n",
                this_id, this_cart->cart_id, this_cart->passengers_inside, targ->cart_capacity);
        print_timestamped(message);

        pthread_cond_broadcast(targ->cond[MUTEX_LEAVE]);
        pthread_mutex_unlock(targ->mutex[MUTEX_LEAVE]);
    }

    sprintf(message, "P%d: ending execution\n", this_id);
    print_timestamped(message);
    free(arg);
    return NULL;
}


int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Invalid number of arguments.\n");
        return 1;
    }

    int passenger_thread_count = (int) strtol(argv[1], NULL, 10);
    int cart_thread_count = (int) strtol(argv[2], NULL, 10);
    int cart_capacity = (int) strtol(argv[3], NULL, 10);
    int rides_count = (int) strtol(argv[4], NULL, 10);

    if (passenger_thread_count <= 0 || cart_thread_count <= 0 || cart_capacity <= 0 || rides_count <= 0) {
        printf("Invalid values of arguments.\n");
        return 1;
    }

    if (passenger_thread_count < cart_capacity * cart_thread_count) {
        printf("Passenger threads must be greater or equal to cart_capacity * cart_thread_count\n");
        return 1;
    }


    struct Cart *cart = init_roller_coaster(cart_thread_count);
    struct Cart *cart_head = cart;
    struct Passenger *passenger = init_passenger_queue(passenger_thread_count);
    struct Passenger *passenger_head = passenger;

    struct T_Arg *targ = malloc(sizeof(struct T_Arg));
    targ->rides_count = rides_count;
    targ->cart_capacity = cart_capacity;
    targ->cart = &cart;
    targ->cart_head = cart_head;
    targ->passenger = &passenger;
    targ->passenger_head = passenger_head;

    for (int i = 0; i < MUTEX_COUNT; i++) {
        targ->mutex[i] = malloc(sizeof(pthread_mutex_t));
        targ->cond[i] = malloc(sizeof(pthread_cond_t));
        pthread_mutex_init(targ->mutex[i], NULL);
        pthread_cond_init(targ->cond[i], NULL);
    }

    pthread_t *cart_tids = malloc(sizeof(pthread_t) * cart_thread_count);
    pthread_t *passenger_tids = malloc(sizeof(pthread_t) * passenger_thread_count);

    spawn_threads(cart_tids, cart_thread_count, cart_logic, targ);
    spawn_threads(passenger_tids, passenger_thread_count, passenger_logic, targ);

    join_threads(cart_tids, cart_thread_count);
    join_threads(passenger_tids, passenger_thread_count);

    free(passenger_tids);
    free(cart_tids);
    free(cart_head);
    free(passenger_head);
    free(targ);
    return 0;
}
