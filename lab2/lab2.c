#include "lab2.h"

int main(int argc, char **argv) {
    int rankingOfCurrentProcess, numberOfProcesses;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rankingOfCurrentProcess);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

    if (numberOfProcesses < 2) {
        printf("Run with at least 2 processes");
        MPI_Finalize();
        return -1;
    }

    if (isEnoughSubranges(numberOfProcesses)) {
        printf("More subranges needed");
        MPI_Finalize();
        return -1;
    }
    double temporaryResult, range[2];

    if (isMasterProcess(rankingOfCurrentProcess)) {
        manageSlaves(numberOfProcesses, &status, &temporaryResult, range);
    } else {
        calculate(&status, &temporaryResult, range);
    }

    MPI_Finalize();
    return 0;
}

void manageSlaves(int numberOfProcesses, MPI_Status *status, double *temporaryResult, double *range) {
    double result = 0;
    range[0] = INTERVAL_START;
    distributeCalculationsOverSlaves(numberOfProcesses, status, temporaryResult, range, &result);
    result = receiveResults(numberOfProcesses, status, result, temporaryResult);
    shutDownSlaves(numberOfProcesses);
    printf("\nHi, I am process 0, the result is %.30f\n", result);
}

void distributeCalculationsOverSlaves(int numberOfProcesses, MPI_Status *status, double *temporaryResult, double *range,
                                 double *result) {
    distributeRangesToSlaves(numberOfProcesses, range);
    do {
        distributeRemainingSubrange(status, temporaryResult, range, result);
    } while (range[1] < INTERVAL_END);
}

void distributeRemainingSubrange(MPI_Status *status, double *temporaryResult, double *range, double *result) {
    receiveTemporaryResult(status, result, temporaryResult);
    updateEndOfRange(range);
    MPI_Send(range, 2, MPI_DOUBLE, (*status).MPI_SOURCE, DATA, MPI_COMM_WORLD);
    range[0] = range[1];
}

void updateEndOfRange(double *range) {
    range[1] = range[0] + RANGE_SIZE;
    if (range[1] > INTERVAL_END) { range[1] = INTERVAL_END; }
}

void calculate(MPI_Status *status, double *temporaryResult, double *range) {
    do {
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, status);
        if ((*status).MPI_TAG == DATA) {
            MPI_Recv(range, 2, MPI_DOUBLE, 0, DATA, MPI_COMM_WORLD, status);
            (*temporaryResult) = simpleIntegration(range[0], range[1]);
            MPI_Send(temporaryResult, 1, MPI_DOUBLE, 0, RESULT, MPI_COMM_WORLD);
        }
    } while ((*status).MPI_TAG != FINISH);
}

bool isMasterProcess(int currentProcessRanking) { return currentProcessRanking == 0; }

void receiveTemporaryResult(MPI_Status *status, double *result, double *temporaryResult) {
    MPI_Recv(temporaryResult, 1, MPI_DOUBLE, MPI_ANY_SOURCE, RESULT, MPI_COMM_WORLD, status);
    (*result) += (*temporaryResult);
}

void shutDownSlaves(int numberOfProcesses) {
    for (int i = 1; i < numberOfProcesses; i++) {
        MPI_Send(NULL, 0, MPI_DOUBLE, i, FINISH, MPI_COMM_WORLD);
    }
}

double receiveResults(int numberOfProcesses, MPI_Status *status, double result, double *temporaryResult) {
    for (int i = 0; i < (numberOfProcesses - 1); i++) {
        MPI_Recv(temporaryResult, 1, MPI_DOUBLE, MPI_ANY_SOURCE, RESULT, MPI_COMM_WORLD, status);
#ifdef DEBUG
        printf ("\nMaster received result %f from process %d", temporaryResult, status.MPI_SOURCE);
        fflush (stdout);
#endif
        result += (*temporaryResult);
    }
    return result;
}

void distributeRangesToSlaves(int numberOfProcesses, double *range) {
    for (int i = 1; i < numberOfProcesses; i++) {
        range[1] = range[0] + RANGE_SIZE;
#ifdef DEBUG
        printf ("\nMaster sending range %f,%f to process %d", range[0], range[1], i);
        fflush (stdout);
#endif
        // send it to process i
        MPI_Send(range, 2, MPI_DOUBLE, i, DATA, MPI_COMM_WORLD);
        range[0] = range[1];
    }
}

bool isEnoughSubranges(int numberOfProcesses) {
    return ((INTERVAL_END - INTERVAL_START) / RANGE_SIZE) < 2 * (numberOfProcesses - 1);
}

double calculatedFunction(double x) {
    return sin(x);
}

double simpleIntegration(double start, double end) {
    double sum = 0;
    for (double i = start; i < end; i += PRECISION) {
        sum += (calculatedFunction(i) + calculatedFunction(i + PRECISION)) * PRECISION / 2;
    }
    return sum;
}