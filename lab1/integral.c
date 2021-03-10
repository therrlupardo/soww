#include <stdio.h>
#include <mpi.h>
#include <math.h>

#define NUMBER_OF_SUBINTERVALS 10000000
#define INTERVAL_START (-M_PI)
#define INTERVAL_END M_PI

double calculateTrapezoidIntegral(int currentProcessRanking, int numberOfProcesses);

double getTrapezoidalIntegral(double lambda, double x1, double x2);
double getRectangularIntegral(double lambda, double x1);
double function(double x);

double calculateRectangularIntegral(int ranking, int processes);

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

    double trapezoidalCurrentIntegral = calculateTrapezoidIntegral(currentProcessRanking, numberOfProcesses);
    double rectangularCurrentIntegral = calculateRectangularIntegral(currentProcessRanking, numberOfProcesses);

    double finalTrapezoidIntegral, finalRectangularIntegral;
    MPI_Reduce(&trapezoidalCurrentIntegral, &finalTrapezoidIntegral, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&rectangularCurrentIntegral, &finalRectangularIntegral, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (!currentProcessRanking) {
        printf("trapezoid integral   = %.30f\n", finalTrapezoidIntegral);
        printf("rectangular integral = %.30f\n", finalRectangularIntegral);
        if (finalTrapezoidIntegral > finalRectangularIntegral) {
            printf("Trapezoid is %.2f%% bigger then rectangular", (double)finalTrapezoidIntegral/finalRectangularIntegral * 100);
        } else {
            printf("Rectangular is %.2f%% bigger then trapezoid", (double)finalRectangularIntegral/finalTrapezoidIntegral * 100);
        }
    }

    MPI_Finalize();
    return 0;
}

double calculateRectangularIntegral(int currentProcessRanking, int numberOfProcesses) {
    double lambda = (double)1 / NUMBER_OF_SUBINTERVALS;
    double x1 = INTERVAL_START + currentProcessRanking * lambda;
    double x2 = INTERVAL_START + (currentProcessRanking + 1) * lambda;
    double integral = 0;
    while(x1 < INTERVAL_END) {
        integral += getRectangularIntegral(lambda, x1);
        x1 += (double)numberOfProcesses / NUMBER_OF_SUBINTERVALS;
        x2 += (double)numberOfProcesses / NUMBER_OF_SUBINTERVALS;
    }
    return integral;
}

double calculateTrapezoidIntegral(int currentProcessRanking, int numberOfProcesses) {
    double lambda = (double)1 / NUMBER_OF_SUBINTERVALS;
    double x1 = INTERVAL_START + currentProcessRanking * lambda;
    double x2 = INTERVAL_START + (currentProcessRanking + 1) * lambda;
    double integral = 0;
    while(x1 < INTERVAL_END) {
        integral += getTrapezoidalIntegral(lambda, x1, x2);
        x1 += (double)numberOfProcesses / NUMBER_OF_SUBINTERVALS;
        x2 += (double)numberOfProcesses / NUMBER_OF_SUBINTERVALS;
    }
    return integral;
}

double getTrapezoidalIntegral(double lambda, double x1, double x2) { return (function(x1) + function(x2)) * lambda / 2; }
double getRectangularIntegral(double lambda, double x1) { return function(x1) * lambda; }
double function(double x) {
    return sin(x);
}
