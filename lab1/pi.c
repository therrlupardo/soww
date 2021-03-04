#include <stdio.h>
#include <mpi.h>

#define NUMBER_OF_SUBINTERVALS 1000000000

int getSign(int denominator);

double calculatePi(int currentProcessRanking, int numberOfProcesses);

int main(int argc, char **argv) {
    int currentProcessRanking, numberOfProcesses;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &currentProcessRanking);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

    if (NUMBER_OF_SUBINTERVALS < numberOfProcesses) {
        printf("Precision smaller than the number of processes - try again.");
        MPI_Finalize();
        return -1;
    }

    double currentPi = calculatePi(currentProcessRanking, numberOfProcesses);

    double pi_final;
    MPI_Reduce(&currentPi, &pi_final, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (!currentProcessRanking) {
        pi_final *= 4;
        printf("currentPi=%f", pi_final);
    }

    MPI_Finalize();
    return 0;
}

double calculatePi(int currentProcessRanking, int numberOfProcesses) {
    double currentPi = 0;
    int denominator = currentProcessRanking * 2 + 1;

    for (; denominator < NUMBER_OF_SUBINTERVALS;) {
        int sign = getSign(denominator);
        currentPi += sign / (double) denominator;
        denominator += 2 * numberOfProcesses;
    }
    return currentPi;
}

int getSign(int denominator) { return (((denominator - 1) / 2) % 2) ? -1 : 1; }
