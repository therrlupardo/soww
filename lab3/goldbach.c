//
// Created by mateusz on 21.04.2021.
//
#include "goldbach.h"

int rankingOfCurrentProcess;
int numberOfProcesses;
int numberOfPrimes = 0;

int* primes;


void printExecutionTime(struct timeval *start, struct timeval *stop) {
    long time=1000000*(stop->tv_sec-start->tv_sec)+stop->tv_usec-start->tv_usec;

    printf("\nMPI execution time=%ld microseconds\n",time);

}

int main(int argc, char **argv) {

    struct timeval start,stop;
    // Initialize MPI
    MPI_Init(&argc, &argv);

    // find out my rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rankingOfCurrentProcess);

    // find out the number of processes in MPI_COMM_WORLD
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

    if (numberOfProcesses < 2) {
        printf("Run with at least 2 processes");
        MPI_Finalize();
        return -1;
    }

    if (((RANGE_END - RANGE_START)) < 2 * (numberOfProcesses - 1)) {
        printf("Range to small");
        MPI_Finalize();
        return -1;
    }

    primes = (int *) malloc((RANGE_END - RANGE_START) * sizeof(int));

    bool result;
    if (isCurrentProcessMaster()){
        gettimeofday(&start,NULL);
        result = masterCheckPrime();
    }
    else
        result = slaveCheckPrime();

    if (!result)
        return -1;

    if (isCurrentProcessMaster())
        qsort(primes, numberOfPrimes, sizeof(int), compare);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&numberOfPrimes, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(primes, numberOfPrimes, MPI_INT, 0, MPI_COMM_WORLD);

    if (isCurrentProcessMaster())
        masterGoldbach();
    else
        slaveGoldbach();


    if (isCurrentProcessMaster()) {
        gettimeofday(&stop,NULL);
        printExecutionTime(&start, &stop);
    }
    // Shut down MPI
    MPI_Finalize();

    return 0;
}

bool isCurrentProcessMaster() { return rankingOfCurrentProcess == MASTER_PROCESS_ID; }

int compare(const void *a, const void *b) {
    int _a = *(int *) a;
    int _b = *(int *) b;
    if (_a < _b)
        return -1;
    else if (_a == _b)
        return 0;
    else
        return 1;
}

/**
 * Checks if value is in array using binary search method
 * @param array array in which value is being searched for
 * @param leftIndex index of left border of searched range
 * @param rightIndex index of right border of searched range
 * @param value value which is being searched
 */
bool isValueInArray(int *array, int leftIndex, int rightIndex, int value) {
    if (rightIndex >= leftIndex) {
        int midIndex = leftIndex + (rightIndex - leftIndex) / 2;

        if (array[midIndex] == value)
            return true;

        return array[midIndex] > value
            ? isValueInArray(array, leftIndex, midIndex - 1, value)
            : isValueInArray(array, midIndex + 1, rightIndex, value);
    }

    return false;
}

/**
 * Checks if n is valid value in Goldbach's conjecture
 * @param n checked value
 */
bool checkGoldbachsConjecture(int n) {
    if (n <= 2 || n % 2 != 0) {
//        printf(CHECKING_GOLDBACH_OKAY, n);
        return true;
    }

    for (int i = 0; primes[i] <= n / 2; i++) {
        int diff = n - primes[i];

        if (isValueInArray(primes, 0, numberOfPrimes - 1, diff)) {
//            printf(CHECKING_GOLDBACH_OKAY, n);
            return true;
        }
    }
//    printf(CHECKING_GOLDBACH_NOT_OKAY, n);
    return false;
}

bool isPrime(unsigned int n) {
    if (n == 1)
        return true;

    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

/**
 * Method run by slave process which checks if received number is prime.
 */
bool slaveCheckPrime() {
    MPI_Request *requests;
    MPI_Status status;
    int result_to_send[2];
    int number = 0;

    requests = (MPI_Request *) malloc(2 * sizeof(MPI_Request));

    if (!requests) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }

    requests[0] = MPI_REQUEST_NULL;
    requests[1] = MPI_REQUEST_NULL;

    MPI_Recv(&number, 1, MPI_INT, 0, DATA, MPI_COMM_WORLD, &status);

    while (number != -1) {

        int receivedNumber = 0;
        MPI_Irecv(&receivedNumber, 1, MPI_INT, 0, DATA, MPI_COMM_WORLD, &(requests[0]));

        bool result = isPrime(number);

        fflush(stdout);

        MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);

        result_to_send[0] = number;
        result_to_send[1] = result;

        number = receivedNumber;

        MPI_Isend(result_to_send, 2, MPI_INT, 0, RESULT, MPI_COMM_WORLD, &(requests[1]));

    }

    MPI_Wait(&(requests[1]), MPI_STATUS_IGNORE);

    free(requests);

    return true;
}

/**
 * Method run by slave process which checks if received number if valid number for Goldbach's Conjecture
 */
bool slaveGoldbach() {
    MPI_Request* requests;
    MPI_Status status;
    int recv_number = 0;
    int number = 0;
    int result_to_send[2];

    requests = (MPI_Request *) malloc(2 * sizeof(MPI_Request));

    if (!requests) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }

    requests[0] = MPI_REQUEST_NULL;
    requests[1] = MPI_REQUEST_NULL;

    MPI_Recv(&number, 1, MPI_INT, 0, DATA, MPI_COMM_WORLD, &status);

    while (number <= RANGE_END) {

        MPI_Irecv(&recv_number, 1, MPI_INT, 0, DATA, MPI_COMM_WORLD, &(requests[0]));
        bool result = checkGoldbachsConjecture(number);

        MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);

        result_to_send[0] = number;
        result_to_send[1] = result;

        number = recv_number;

        MPI_Isend(result_to_send, 2, MPI_INT, 0, RESULT, MPI_COMM_WORLD, &(requests[1]));

    }

    MPI_Wait(&(requests[1]), MPI_STATUS_IGNORE);

    free(requests);

    return true;
}

/**
 * Method run by master process which checks if received number is prime.
 */
void sendInitialPrimeCheckData(MPI_Request *requests, int *temporaryResults, int *counter, int *value) {
    // Synchronously send initial data to slaves
    for (int i = 1; i < numberOfProcesses; i++) {
        MPI_Send(value, 1, MPI_INT, i, DATA, MPI_COMM_WORLD);
        (*counter)++;
        (*value)++;
    }

    // Start asynchronously receiving data from slaves
    for (int i = 1; i < numberOfProcesses; i++) {
        MPI_Irecv(&(temporaryResults[2 * (i - 1)]), 2, MPI_INT, i, RESULT, MPI_COMM_WORLD, &(requests[i - 1]));
    }

    // Start asynchronously sending data to slaves, so they have always something to calculate
    for (int i = 1; i < numberOfProcesses; i++) {
        MPI_Isend(value, 1, MPI_INT, i, DATA, MPI_COMM_WORLD, &(requests[numberOfProcesses - 2 + i]));
        (*counter)++;
        (*value)++;
    }
}

/**
 * Master's method for building array of prime numbers by distributing it between slaves
 */
bool masterCheckPrime() {
    MPI_Request *requests;
    MPI_Status status;


    // Counter storing balance between sent tasks and received results (should be 0 at the enf of the program)
    int counter = 0;

    int *temporaryResults;

    // 1 handle for receive and 2 for sending
    requests = (MPI_Request *) malloc(3 * (numberOfProcesses - 1) * sizeof(MPI_Request));

    if (!requests) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }

    for (int i = 0; i < 3 * (numberOfProcesses - 1); i++)
        requests[i] = MPI_REQUEST_NULL;

    temporaryResults = (int *) malloc(2 * (numberOfProcesses - 1) * sizeof(int));
    if (!temporaryResults) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }

    int value = 1;
    sendInitialPrimeCheckData(requests, temporaryResults, &counter, &value);

    int numberOfCompletedRequests;
    sendRestOfPrimeCheckData(requests, &numberOfCompletedRequests, counter, temporaryResults, &value);

    value = -1;
    for (int i = 1; i < numberOfProcesses; i++) {
        MPI_Isend(&value, 1, MPI_INT, i, DATA, MPI_COMM_WORLD, &
                (requests[2 * numberOfProcesses - 3 + i]));
    }

    MPI_Waitall(3 * numberOfProcesses - 3, requests, MPI_STATUSES_IGNORE);

    for (int i = 0; i < (numberOfProcesses - 1); i++) {
        int number = temporaryResults[2 * i];
        int result = temporaryResults[2 * i + 1];
        if (result) {
            primes[numberOfPrimes++] = number;
        }
    }

    // now receive results for the initial sends
    for (int i = 0; i < (numberOfProcesses - 1); i++) {
        MPI_Recv(&(temporaryResults[2 * i]), 2, MPI_INT, i + 1, RESULT,
                 MPI_COMM_WORLD, &status);

        int number = temporaryResults[2 * i];
        int result = temporaryResults[2 * i + 1];
        if (result) {
            primes[numberOfPrimes++] = number;
        }
    }

    free(requests);
    free(temporaryResults);
    return true;
}

/**
 * Loops while end of range if not reached and sends new data every time any slave process sends something back
 */
void sendRestOfPrimeCheckData(MPI_Request *requests, int *numberOfCompletedRequests, int counter, int *temporaryResults, int *value) {
    while ((*value) <= RANGE_END) {
        MPI_Waitany(2 * numberOfProcesses - 2, requests, numberOfCompletedRequests, MPI_STATUS_IGNORE);
        if ((*numberOfCompletedRequests) < (numberOfProcesses - 1)) {
            counter--;
            int number = temporaryResults[2 * (*numberOfCompletedRequests)];
            int result = temporaryResults[2 * (*numberOfCompletedRequests) + 1];
            if (result) {
                primes[numberOfPrimes++] = number;
            }

            // Before we can send new data we must check if existing send was finished
            MPI_Wait(&(requests[numberOfProcesses - 1 + (*numberOfCompletedRequests)]), MPI_STATUS_IGNORE);

            int rank_to_send = (*numberOfCompletedRequests) + 1;

            MPI_Isend(value, 1, MPI_INT, rank_to_send, DATA, MPI_COMM_WORLD,
                      &(requests[numberOfProcesses - 1 + (*numberOfCompletedRequests)]));
            counter++;
            (*value)++;

            MPI_Irecv(&(temporaryResults[2 * (*numberOfCompletedRequests)]), 2, MPI_INT, (*numberOfCompletedRequests) + 1, RESULT, MPI_COMM_WORLD,
                      &(requests[(*numberOfCompletedRequests)]));

        }

    }
}

/**
 * Master's task for checking which values in defined range are valid for Goldbach's Conjecture by distributing tasks
 * between slaves.
 */
bool masterGoldbach() {
    MPI_Request *requests;
    MPI_Status status;



    // 1 handle for receive and 2 for sending
    requests = (MPI_Request *) malloc(3 * (numberOfProcesses - 1) * sizeof(MPI_Request));

    for (int i = 0; i < 3 * (numberOfProcesses - 1); i++) {
        requests[i] = MPI_REQUEST_NULL;
    }

    if (!requests) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }

    int* temporaryResults = (int *) malloc(2 * (numberOfProcesses - 1) * sizeof(int));
    if (!temporaryResults) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }

    int value = RANGE_START;

    if (value <= 2) {
        value = 3;
    }
    if (value % 2 == 1) {
        value++;
    }

    // Counter storing balance between sent tasks and received results (should be 0 at the end of the program)
    int counter = 0;
    // First part of tasks
    for (int i = 1; i < numberOfProcesses; i++) {
        MPI_Send(&value, 1, MPI_INT, i, DATA, MPI_COMM_WORLD);
        counter++;
        value += 2;
    }

    // Try to receive data from slaves
    for (int i = 1; i < numberOfProcesses; i++) {
        MPI_Irecv(&(temporaryResults[2 * (i - 1)]), 2, MPI_INT, i, RESULT, MPI_COMM_WORLD, &(requests[i - 1]));
    }

    // Send data to slaves
    for (int i = 1; i < numberOfProcesses; i++) {
        MPI_Isend(&value, 1, MPI_INT, i, DATA, MPI_COMM_WORLD, &(requests[numberOfProcesses - 2 + i]));
        counter++;
        value += 2;
    }

    // Loop sending new task when we get result from another task
    while (value <= RANGE_END) {

        int request_completed;
        MPI_Waitany(2 * numberOfProcesses - 2, requests, &request_completed, MPI_STATUS_IGNORE);
        // Results are in first part of requests array. Check if finished request was result
        if (request_completed < (numberOfProcesses - 1)) {
            int number = temporaryResults[2 * request_completed];
            int result = temporaryResults[2 * request_completed + 1];
            if (!result) {
                printf("Goldbach's Conjecture is not valid for number %d\n", number);
                return false;
            }

            // Before we can send new data we must check if existing send was finished
            MPI_Wait(&(requests[numberOfProcesses - 1 + request_completed]), MPI_STATUS_IGNORE);

            int rank_to_send = request_completed + 1;

            MPI_Isend(&value, 1, MPI_INT, rank_to_send, DATA, MPI_COMM_WORLD,
                      &(requests[numberOfProcesses - 1 + request_completed]));
            value += 2;

            MPI_Irecv(&(temporaryResults[2 * request_completed]), 2, MPI_INT, request_completed + 1, RESULT, MPI_COMM_WORLD,
                      &(requests[request_completed]));

        }

    }

    for (int i = 1; i < numberOfProcesses; i++) {
        MPI_Isend(&value, 1, MPI_INT, i, DATA, MPI_COMM_WORLD, &
                (requests[2 * numberOfProcesses - 3 + i]));
    }
    MPI_Waitall(3 * numberOfProcesses - 3, requests, MPI_STATUSES_IGNORE);

    for (int i = 0; i < (numberOfProcesses - 1); i++) {
        int number = temporaryResults[2 * i];
        int result = temporaryResults[2 * i + 1];
        if (!result) {
            printf("Goldbach's Conjecture is not valid for number %d\n", number);
            return false;
        }
    }

    // now receive results for the initial sends
    for (int i = 0; i < (numberOfProcesses - 1); i++) {
        MPI_Recv(&(temporaryResults[2 * i]), 2, MPI_INT, i + 1, RESULT,
                 MPI_COMM_WORLD, &status);

        int number = temporaryResults[2 * i];
        int result = temporaryResults[2 * i + 1];
        if (!result) {
            printf(GOLDBACH_FINAL_RESULT_WRONG, number);
            return false;
        }
    }

    printf(GOLDBACH_FINAL_RESULT_RIGHT, RANGE_START, RANGE_END);

    free(requests);
    free(temporaryResults);
    return true;
}