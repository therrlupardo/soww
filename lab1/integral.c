#include <stdio.h>
#include <mpi.h>
#include <math.h>

#define NUMBER_OF_SUBINTERVALS 10000000
#define INTERVAL_START (-M_PI)
#define INTERVAL_END M_PI

double calculateIntegral(int currentProcessRanking, int numberOfProcesses);

double getTrapezoidalIntegral(double lambda, double x1, double x2);
double getRectangularIntegral(double lambda, double x1);

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

    double currentIntegral = calculateIntegral(currentProcessRanking, numberOfProcesses);

    double finalIntegral;
    MPI_Reduce(&currentIntegral, &finalIntegral, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (!currentProcessRanking) {
        printf("integral=%.30f", finalIntegral);
    }

    MPI_Finalize();
    return 0;
}

double calculateIntegral(int currentProcessRanking, int numberOfProcesses) {
    double lambda = (double)1 / NUMBER_OF_SUBINTERVALS;
    double x1 = INTERVAL_START + currentProcessRanking * lambda;
    double x2 = INTERVAL_START + (currentProcessRanking + 1) * lambda;
    double integral = 0;
    for(; x1 < INTERVAL_END;) {
        integral += getTrapezoidalIntegral(lambda, x1, x2);W
//        integral += getRectangularIntegral(lambda, x1);
        x1 += (double)numberOfProcesses / NUMBER_OF_SUBINTERVALS;
        x2 += (double)numberOfProcesses / NUMBER_OF_SUBINTERVALS;
    }
    return integral;
}

double getTrapezoidalIntegral(double lambda, double x1, double x2) { return (sin(x1) + sin(x2)) * lambda / 2; }
double getRectangularIntegral(double lambda, double x1) { return sin(x1) * lambda; }

