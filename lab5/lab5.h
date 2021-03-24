#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define PRECISION 0.000001
#define RANGESIZE 1
#define DATA 0
#define RESULT 1
#define FINISH 2

#define RANGE_START 1
#define RANGE_END 1000

bool numberOfProcessesInvalid(double numberOfProcesses);

double f(double x);

double SimpleIntegration(double range_start, double range_end);

MPI_Request *getMemoryForRequests(int numberOfProcesses);

double *getMemoryForRanges(int numberOfProcesses);

double *getMemoryForTemporaryResults(int numberOfProcesses);

int distributeInitialRangesToSlaves(int numberOfProcesses, double *range, int sentcount);

bool isMasterProcess(int rankingOfCurrentProcess);

void initializeRequests(int numberOfProcesses, MPI_Request *requests);

void sendMoreDataToSlave(int numberOfProcesses, double *ranges, double *range, double *temporaryResults,
                         int amountOfRanges, MPI_Request *requests, int amountOfCompletedRequests);

void receiveResultsFromSlaves(int numberOfProcesses, double *temporaryResults, MPI_Request *requests);

int startSendingDataPartsToSlaves(int numberOfProcesses, double *ranges, double *range, int amountOfRanges,
                                  MPI_Request *requests);

bool rangeEndReached(const double *range);

void sendFinishingRangesToSlaves(int numberOfProcesses, double *ranges, double *range, MPI_Request *requests);

double sumTemporaryResults(int numberOfProcesses, double result, double *temporaryResults, MPI_Status *status,
                           int amountOfReceivedResults);

bool distributeDataBetweenSlaves(int numberOfProcesses, double *ranges, double *range, double *temporaryResults,
                                 MPI_Status *status, MPI_Request *requests, int *amountOfCompletedRequests,
                                 int amountOfReceivedResults);

bool calculateIntegralOfRange(double *ranges, double *range, double *temporaryResults, MPI_Status *status,
                              MPI_Request *requests);

bool initializeMasterMemory(int numberOfProcesses, double **ranges, double **temporaryResults, MPI_Request **requests);

bool initializeSlaveMemory(double **ranges, double **temporaryResults, MPI_Request **requests);