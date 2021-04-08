#include "non-blocking.h"

int main(int argc, char **argv) {
    int rankingOfCurrentProcess;
    int numberOfProcesses;
    double *ranges;
    double range[2];
    double *temporaryResults;
    MPI_Status status;
    MPI_Request *requests;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rankingOfCurrentProcess);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

    if (numberOfProcessesInvalid(numberOfProcesses)) {
        MPI_Finalize();
        return -1;
    }

    if (isMasterProcess(rankingOfCurrentProcess)) {
        int amountOfCompletedRequests;
        int amountOfReceivedResults = 0;
        if (!distributeDataBetweenSlaves(numberOfProcesses, ranges, range, temporaryResults,
                                         &status, requests, &amountOfCompletedRequests, amountOfReceivedResults)) {
            return -1;
        }
    } else {
        if (!calculateIntegralOfRange(ranges, range, temporaryResults, &status, requests))
            return -1;
    }

    // Shut down MPI
    MPI_Finalize();
    return 0;
}

bool initializeSlaveMemory(double **ranges, double **temporaryResults, MPI_Request **requests) {
    (*requests) = (MPI_Request *) malloc(2 * sizeof(MPI_Request));

    if (!(*requests)) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }

    (*requests)[0] = (*requests)[1] = MPI_REQUEST_NULL;
    (*ranges) = (double *) malloc(2 * sizeof(double));

    if (!(*ranges)) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }

    (*temporaryResults) = (double *) malloc(2 * sizeof(double));

    if (!(*temporaryResults)) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }
    return true;
}

bool calculateIntegralOfRange(double *ranges, double *range, double *temporaryResults, MPI_Status *status,
                         MPI_Request *requests) {
    if(!initializeSlaveMemory(&ranges, &temporaryResults, &requests))
        return false;

    // first receive the initial data
    MPI_Recv(range, 2, MPI_DOUBLE, 0, DATA, MPI_COMM_WORLD, status);
    while (range[0] < range[1]) {
        // if there is some data to process
        // before computing the next part start receiving range_start new data part
        MPI_Irecv(ranges, 2, MPI_DOUBLE, 0, DATA, MPI_COMM_WORLD,
                  &(requests[0]));

        // compute my part
        temporaryResults[1] = SimpleIntegration(range[0], range[1]);
        // now finish receiving the new part
        // and finish sending the previous results back to the master
        MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
        range[0] = ranges[0];
        range[1] = ranges[1];
        temporaryResults[0] = temporaryResults[1];

        // and start sending the results back
        MPI_Isend(&temporaryResults[0], 1, MPI_DOUBLE, 0, RESULT,
                  MPI_COMM_WORLD, &(requests[1]));
    }

    // now finish sending the last results to the master
    MPI_Wait(&(requests[1]), MPI_STATUS_IGNORE);
    return true;
}

bool initializeMasterMemory(int numberOfProcesses, double **ranges, double **temporaryResults, MPI_Request **requests) {
    (*requests) = getMemoryForRequests(numberOfProcesses);
    if (!(*requests)) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }
    initializeRequests(numberOfProcesses, (*requests));

    (*ranges) = getMemoryForRanges(numberOfProcesses);
    if (!(*ranges)) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }

    (*temporaryResults) = getMemoryForTemporaryResults(numberOfProcesses);
    if (!(*temporaryResults)) {
        printf("\nNot enough memory");
        MPI_Finalize();
        return false;
    }
    return true;
}

bool distributeDataBetweenSlaves(int numberOfProcesses, double *ranges, double *range,
                                 double *temporaryResults,
                                 MPI_Status *status, MPI_Request *requests,
                                 int *amountOfCompletedRequests,
                                 int amountOfReceivedResults) {
    if (!initializeMasterMemory(numberOfProcesses, &ranges, &temporaryResults, &requests))
        return false;

    range[0] = RANGE_START;

    int amountOfRanges = 0;
    amountOfRanges = distributeInitialRangesToSlaves(numberOfProcesses, range, amountOfRanges);

    receiveResultsFromSlaves(numberOfProcesses, temporaryResults, requests);

    double result = 0;
    amountOfRanges = startSendingDataPartsToSlaves(numberOfProcesses, ranges, range, amountOfRanges, requests);
    while (!rangeEndReached(range)) {
        // wait for completion of any of the requests
        MPI_Waitany(2 * numberOfProcesses - 2, requests, amountOfCompletedRequests,
                    MPI_STATUS_IGNORE);

        // if it is range_start result then send new data to the process and add the result
        if ((*amountOfCompletedRequests) < (numberOfProcesses - 1)) {
            result += temporaryResults[(*amountOfCompletedRequests)];
            amountOfReceivedResults++;
            // first check if the send has terminated
            MPI_Wait(&(requests[numberOfProcesses - 1 + (*amountOfCompletedRequests)]),
                     MPI_STATUS_IGNORE);

            // now send some new data portion to this process
            sendMoreDataToSlave(numberOfProcesses, ranges, range, temporaryResults, amountOfRanges, requests,
                                (*amountOfCompletedRequests));
        }
    }
    range[0] = range[1];
    sendFinishingRangesToSlaves(numberOfProcesses, ranges, range, requests);
    // now receive results from the processes - that is finalize the pending requests
    MPI_Waitall(3 * numberOfProcesses - 3, requests, MPI_STATUSES_IGNORE);

    result = sumTemporaryResults(numberOfProcesses, result, temporaryResults, status, amountOfReceivedResults);
    printf("\nHi, I am process 0, the result is %.30f\n", result);
    return true;
}

double sumTemporaryResults(int numberOfProcesses, double result, double *temporaryResults, MPI_Status *status,
                           int amountOfReceivedResults) {
    for (int i = 0; i < (numberOfProcesses - 1); i++) {
        result += temporaryResults[i];
    }
    // now receive results for the initial sends
    for (int i = 0; i < (numberOfProcesses - 1); i++) {
        MPI_Recv(&(temporaryResults[i]), 1, MPI_DOUBLE, i + 1, RESULT,
                 MPI_COMM_WORLD, status);
        result += temporaryResults[i];
        amountOfReceivedResults++;
    }
    return result;
}

void sendFinishingRangesToSlaves(int numberOfProcesses, double *ranges, double *range, MPI_Request *requests) {
    for (int i = 1; i < numberOfProcesses; i++) {
        ranges[2 * i - 4 + 2 * numberOfProcesses] = range[0];
        ranges[2 * i - 3 + 2 * numberOfProcesses] = range[1];
        MPI_Isend(range, 2, MPI_DOUBLE, i, DATA, MPI_COMM_WORLD,
                  &(requests[2 * numberOfProcesses - 3 + i]));
    }
}

bool rangeEndReached(const double *range) { return range[1] >= RANGE_END; }

int startSendingDataPartsToSlaves(int numberOfProcesses, double *ranges, double *range, int amountOfRanges,
                                  MPI_Request *requests) {
    for (int i = 1; i < numberOfProcesses; i++) {
        range[1] = range[0] + RANGESIZE;
        ranges[2 * i - 2] = range[0];
        ranges[2 * i - 1] = range[1];

        // send it to process i
        MPI_Isend(&(ranges[2 * i - 2]), 2, MPI_DOUBLE, i, DATA,
                  MPI_COMM_WORLD, &(requests[numberOfProcesses - 2 + i]));

        amountOfRanges++;
        range[0] = range[1];
    }
    return amountOfRanges;
}

void receiveResultsFromSlaves(int numberOfProcesses, double *temporaryResults, MPI_Request *requests) {
    for (int i = 1; i < numberOfProcesses; i++)
        MPI_Irecv(&(temporaryResults[i - 1]), 1, MPI_DOUBLE, i, RESULT,
                  MPI_COMM_WORLD, &(requests[i - 1]));
}

void sendMoreDataToSlave(int numberOfProcesses, double *ranges, double *range, double *temporaryResults,
                         int amountOfRanges, MPI_Request *requests, int amountOfCompletedRequests) {
    range[1] = range[0] + RANGESIZE;

    if (range[1] > RANGE_END)
        range[1] = RANGE_END;
    ranges[2 * amountOfCompletedRequests] = range[0];
    ranges[2 * amountOfCompletedRequests + 1] = range[1];
    MPI_Isend(&(ranges[2 * amountOfCompletedRequests]), 2, MPI_DOUBLE,
              amountOfCompletedRequests + 1, DATA, MPI_COMM_WORLD,
              &(requests[numberOfProcesses - 1 + amountOfCompletedRequests]));
    amountOfRanges++;
    range[0] = range[1];

    // now issue range_start corresponding recv
    MPI_Irecv(&(temporaryResults[amountOfCompletedRequests]), 1,
              MPI_DOUBLE, amountOfCompletedRequests + 1, RESULT,
              MPI_COMM_WORLD,
              &(requests[amountOfCompletedRequests]));
}

void initializeRequests(int numberOfProcesses, MPI_Request *requests) {
    for (int i = 0; i < 2 * (numberOfProcesses - 1); i++)
        requests[i] = MPI_REQUEST_NULL;    // none active at this point

}

bool isMasterProcess(int rankingOfCurrentProcess) { return rankingOfCurrentProcess == 0; }

int distributeInitialRangesToSlaves(int numberOfProcesses, double *range, int sentcount) {
    for (int i = 1; i < numberOfProcesses; i++) {
        range[1] = range[0] + RANGESIZE;
        // send it to process i
        MPI_Send(range, 2, MPI_DOUBLE, i, DATA, MPI_COMM_WORLD);
        sentcount++;
        range[0] = range[1];
    }
    return sentcount;
}

double *getMemoryForTemporaryResults(int numberOfProcesses) {
    return (double *) malloc((numberOfProcesses - 1) * sizeof(double));
}

double *getMemoryForRanges(int numberOfProcesses) {
    return (double *) malloc(4 * (numberOfProcesses - 1) * sizeof(double));
}

MPI_Request *getMemoryForRequests(int numberOfProcesses) {
    return (MPI_Request *) malloc(3 * (numberOfProcesses - 1) *
                                  sizeof(MPI_Request));
}

bool numberOfProcessesInvalid(double numberOfProcesses) {
    if (numberOfProcesses < 2) {
        printf("Task should be run with at least 2 processes");
        return true;
    }

    if (((RANGE_END - RANGE_START) / (double) RANGESIZE) < 2 * (numberOfProcesses - 1)) {
        printf("Not enough subranges to perform calculations");
        return true;
    }
    return false;
}

double f(double const x) {
    return sin(x);
}

double SimpleIntegration(double const range_start, double const range_end) {
    double sum = 0;
    for (double i = range_start; i < range_end; i += PRECISION)
        sum += (f(i) + f(i + PRECISION)) * PRECISION / 2;
    return sum;
}