#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define WAIT 0
#define IN_CARRIAGE 1

int passengerCount;
int carriageCount;
int carriageCapacity;
int tourCount;

int actualCarriageID = 0;

//passenger queue
int *passengerQueue;
int endOfPassengerQueue = 0;

//passengers
int *passengerState;
pthread_cond_t *passengersWaitCond;

//carriages
int *carriageSeatsState;
pthread_mutex_t carriagesWaitMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t *carriagesWaitCond;

//entry process action
int waitForEntry;
pthread_cond_t entryProcessCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t entryProcessMutex = PTHREAD_MUTEX_INITIALIZER;

//button action
int waitForButtonPress;
pthread_cond_t waitForButtonPressCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t waitForButtonPressMutex = PTHREAD_MUTEX_INITIALIZER;
int buttonPresser;

//release process action
int waitForRelease;
pthread_cond_t releaseProcessCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t releaseProcessMutex = PTHREAD_MUTEX_INITIALIZER;

//ended counter
int endedCarriage = 0;

long getTimestamp() {
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return timestamp.tv_nsec;
}

void addPassengerToQueue(int id) {
    if (endOfPassengerQueue < passengerCount) {
        passengerQueue[endOfPassengerQueue++] = id;
    }
}

int getPassengerFromQueue() {
    int passengerId = passengerQueue[0];
    for (int i = 1; i < passengerCount; i++) {
        passengerQueue[i - 1] = passengerQueue[i];
    }
    passengerQueue[endOfPassengerQueue - 1] = -1;
    endOfPassengerQueue--;

    return passengerId;
}

void setPassengerState(int id, int state) {
    passengerState[id] = state;
    pthread_cond_broadcast(&passengersWaitCond[id]);
}


void *threadCarriage(void *data) {
    int id = *((int *) data);
    int *passengers = calloc(carriageCapacity, sizeof(int));
    int isFree = 1;

    printf("[%ld] Create new carriage (%d)\n",
           getTimestamp(),
           id);

    for (int i = 0; i < tourCount; i++) {

        //waiting for platform
        pthread_mutex_lock(&carriagesWaitMutex);
        while (actualCarriageID != id) {
            pthread_cond_wait(&carriagesWaitCond[id], &carriagesWaitMutex);
        }
        pthread_mutex_unlock(&carriagesWaitMutex);

        //is next to the platform
        buttonPresser = -1;
        printf("[%ld] Carriage %d open door.\n",
               getTimestamp(),
               id);

        //release passengers from carriage
        if (!isFree) {
            for (int k = 0; k < carriageCapacity; k++) {
                pthread_mutex_lock(&releaseProcessMutex);

                int passengerId = passengers[k];
                carriageSeatsState[actualCarriageID]--;
                addPassengerToQueue(passengerId);
                setPassengerState(passengerId, WAIT);
                waitForRelease = 1;
                while (waitForRelease) {
                    pthread_cond_wait(&releaseProcessCond, &releaseProcessMutex);
                }

                pthread_mutex_unlock(&releaseProcessMutex);
            }
        }

        //get passengers from queue
        for (int k = 0; k < carriageCapacity; k++) {
            pthread_mutex_lock(&entryProcessMutex);

            int passenger = getPassengerFromQueue();
            passengers[k] = passenger;
            carriageSeatsState[actualCarriageID]++;
            waitForEntry = 1;
            setPassengerState(passenger, IN_CARRIAGE);
            while (waitForEntry) {
                pthread_cond_wait(&entryProcessCond, &entryProcessMutex);
            }

            pthread_mutex_unlock(&entryProcessMutex);
        }
        isFree = 0;
        printf("[%ld] Carriage %d close door.\n",
               getTimestamp(),
               id);

        //wait for press button
        pthread_mutex_lock(&waitForButtonPressMutex);
        buttonPresser = passengers[rand() % carriageCapacity];
        waitForButtonPress = 1;

        pthread_cond_broadcast(&waitForButtonPressCond);
        while (waitForButtonPress) {
            pthread_cond_wait(&waitForButtonPressCond, &waitForButtonPressMutex);
        }

        printf("[%ld] Carriage %d start ride.\n",
               getTimestamp(),
               id);

        if (i != tourCount - 1) {
            if (actualCarriageID == carriageCount - 1) {
                actualCarriageID = endedCarriage;
            } else {
                actualCarriageID++;
            }
            pthread_cond_broadcast(&carriagesWaitCond[actualCarriageID]);
        }

        pthread_mutex_unlock(&waitForButtonPressMutex);

    }

    //it was last run but i must release passengers from carriage :/
    for (int k = 0; k < carriageCapacity; k++) {
        pthread_mutex_lock(&releaseProcessMutex);

        int passengerId = passengers[k];
        carriageSeatsState[actualCarriageID]--;
        addPassengerToQueue(passengerId);
        setPassengerState(passengerId, WAIT);
        waitForRelease = 1;
        while (waitForRelease) {
            pthread_cond_wait(&releaseProcessCond, &releaseProcessMutex);
        }

        pthread_mutex_unlock(&releaseProcessMutex);
    }

    endedCarriage++;
    if (actualCarriageID == carriageCount - 1) {
        actualCarriageID = endedCarriage;
    } else {
        actualCarriageID++;
    }
    if (actualCarriageID != carriageCount) {
        pthread_cond_broadcast(&carriagesWaitCond[actualCarriageID]);
    }

    for (int i = 0; i < passengerCount; i++) {
        if (passengerState[i] == WAIT) {
            pthread_cond_broadcast(&passengersWaitCond[i]);
        }
    }

    free(passengers);

    return NULL;
}

void *threadPassenger(void *data) {
    int id = *((int *) data);
    printf("[%ld] Create new passenger (%d) thread.\n", getTimestamp(), id);


    while (endedCarriage < carriageCount) {

        //default passenger state is WAIT
        pthread_mutex_lock(&entryProcessMutex);
        while (passengerState[id] == WAIT && endedCarriage < carriageCount) {
            pthread_cond_wait(&passengersWaitCond[id], &entryProcessMutex);
        }

        if (endedCarriage >= carriageCount) {
            pthread_mutex_unlock(&entryProcessMutex);
            return NULL;
        }

        //entry to carriage
        printf("[%ld] Passenger %d entry to carriage (%d) %d/%d.\n",
               getTimestamp(),
               id,
               actualCarriageID,
               carriageSeatsState[actualCarriageID],
               carriageCapacity);

        //notify carriage
        waitForEntry = 0;
        pthread_cond_broadcast(&entryProcessCond);
        pthread_mutex_unlock(&entryProcessMutex);

        //press button
        pthread_mutex_lock(&waitForButtonPressMutex);
        while (buttonPresser == -1) {
            pthread_cond_wait(&waitForButtonPressCond, &waitForButtonPressMutex);
        }

        if (buttonPresser == id) {
            waitForButtonPress = 0;
            printf("[%ld] Passenger %d press button start in carriage (%d).\n",
                   getTimestamp(),
                   id,
                   actualCarriageID);
            pthread_cond_broadcast(&waitForButtonPressCond);
        }
        pthread_mutex_unlock(&waitForButtonPressMutex);

        //release carriage
        pthread_mutex_lock(&releaseProcessMutex);
        while (passengerState[id] == IN_CARRIAGE) {
            pthread_cond_wait(&passengersWaitCond[id], &releaseProcessMutex);
        }

        printf("[%ld] Passenger %d released carriage (%d) %d/%d.\n",
               getTimestamp(),
               id,
               actualCarriageID,
               carriageSeatsState[actualCarriageID],
               carriageCapacity);

        waitForRelease = 0;
        pthread_cond_broadcast(&releaseProcessCond);
        pthread_mutex_unlock(&releaseProcessMutex);
    }

    return NULL;
}

int main(int argc, char *argv[], char *env[]) {

    if (argc < 5) {
        printf("Not enough arguments");
        return 0;
    }

    passengerCount = (int)strtol(argv[1], NULL, 0);
    if (passengerCount < 1) {
        printf("Passenger count must by greater than 0");
        return 0;
    }

    carriageCount = (int)strtol(argv[2], NULL, 0);
    if (carriageCount < 1) {
        printf("Carriage count must by greater than 0");
        return 0;
    }

    carriageCapacity = (int)strtol(argv[3], NULL, 0);
    if (carriageCapacity < 1) {
        printf("Carriage capacity must by greater than 0");
        return 0;
    }

    tourCount = (int)strtol(argv[4], NULL, 0);
    if (tourCount < 1) {
        printf("Tour count must by greater than 0");
        return 0;
    }

    if (passengerCount < carriageCount * carriageCapacity) {
        printf("Not enough passenger");
        return 0;
    }

    //array init
    passengerQueue = calloc(passengerCount, sizeof(int));
    for (int i = 0; i < passengerCount; i++) {
        passengerQueue[i] = -1;
    }

    passengerState = calloc(passengerCount, sizeof(int));

    passengersWaitCond = calloc(passengerCount, sizeof(pthread_cond_t));
    for (int i = 0; i < passengerCount; i++) {
        passengersWaitCond[i] = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    }

    carriageSeatsState = calloc(carriageCount, sizeof(int));

    carriagesWaitCond = calloc(carriageCount, sizeof(pthread_cond_t));
    for (int i = 0; i < carriageCount; i++) {
        carriagesWaitCond[i] = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    }

    //creating passenger threads
    pthread_t *passengerKey = calloc(passengerCount, sizeof(pthread_t));
    int *passengerID = calloc(passengerCount, sizeof(int));

    for (int i = 0; i < passengerCount; i++) {
        passengerID[i] = i;
        addPassengerToQueue(passengerID[i]);
        pthread_create(&passengerKey[i], NULL, threadPassenger, &passengerID[i]);
    }

    //creating carriage threads
    pthread_t *carriageKey = calloc(carriageCount, sizeof(pthread_t));
    int *carriageID = calloc(carriageCount, sizeof(int));

    for (int i = 0; i < carriageCount; i++) {
        carriageID[i] = i;
        pthread_create(&carriageKey[i], NULL, threadCarriage, &carriageID[i]);
    }

    //ending carriage threads
    for (int i = 0; i < carriageCount; i++) {
        pthread_join(carriageKey[i], NULL);
        printf("[%ld] Carriage's thread is end - %d\n",
               getTimestamp(),
               i
        );
    }

    //ending passenger threads
    for (int i = 0; i < passengerCount; i++) {
        pthread_join(passengerKey[i], NULL);
        printf("[%ld] Passenger's thread is end - %d\n",
               getTimestamp(),
               i
        );
    }

    free(carriageKey);
    free(carriageID);

    free(passengerKey);
    free(passengerID);

    free(passengerQueue);
    free(passengerState);
    free(carriageSeatsState);

    for (int i = 0; i < passengerCount; i++)
        pthread_cond_destroy(&passengersWaitCond[i]);

    free(passengersWaitCond);


    for (int i = 0; i < carriageCount; i++)
        pthread_cond_destroy(&carriagesWaitCond[i]);

    free(carriagesWaitCond);

    pthread_mutex_destroy(&carriagesWaitMutex);\
    pthread_mutex_destroy(&entryProcessMutex);
    pthread_mutex_destroy(&waitForButtonPressMutex);
    pthread_mutex_destroy(&releaseProcessMutex);

    pthread_cond_destroy(&entryProcessCond);
    pthread_cond_destroy(&waitForButtonPressCond);
    pthread_cond_destroy(&releaseProcessCond);

    printf("[%ld] END PROGRAM\n", getTimestamp());

    return 0;
}