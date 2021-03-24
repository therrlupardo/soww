#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>

#define PRECISION 0.000001
#define RANGE_SIZE 1
#define DATA 0
#define RESULT 1
#define FINISH 2

#define INTERVAL_START 1
#define INTERVAL_END 1000

//#define DEBUG

double calculatedFunction(double x);

double simpleIntegration(double start, double end);

bool isEnoughSubranges(int numberOfProcesses);

void distributeRangesToSlaves(int numberOfProcesses, double *range);

double receiveResults(int numberOfProcesses, MPI_Status *status, double result, double *temporaryResult);

void shutDownSlaves(int numberOfProcesses);

void receiveTemporaryResult(MPI_Status *status, double *result, double *temporaryResult);

bool isMasterProcess(int currentProcessRanking);

void calculate(MPI_Status *status, double *temporaryResult, double *range);

void manageSlaves(int numberOfProcesses, MPI_Status *status, double *temporaryResult, double *range);

void updateEndOfRange(double *range);

void distributeRemainingSubrange(MPI_Status *status, double *temporaryResult, double *range, double *result);

void distributeCalculationsOverSlaves(int numberOfProcesses, MPI_Status *status, double *temporaryResult, double *range,
                                      double *result);
