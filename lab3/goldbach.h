//
// Created by mateusz on 21.04.2021.
//
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

#define DATA 0
#define RESULT 1
#define MASTER_PROCESS_ID 0
#define RANGE_START 1
#define RANGE_END 10000000

#define CHECKING_GOLDBACH_OKAY "Checking Goldbach's Conjecture for %d - okay"
#define CHECKING_GOLDBACH_NOT_OKAY "Checking Goldbach's Conjecture for %d - not okay"
#define GOLDBACH_FINAL_RESULT_RIGHT "Every number from %d to %d is valid for Goldbach's Conjecture\n"
#define GOLDBACH_FINAL_RESULT_WRONG "Number %d is against Goldbach's Conjecture\n"
int compare(const void *a, const void *b);

bool isValueInArray(int *array, int leftIndex, int rightIndex, int value);

bool checkGoldbachsConjecture(int n);

bool isPrime(unsigned int n);

bool slaveCheckPrime();

bool slaveGoldbach();

bool masterCheckPrime();

bool masterGoldbach();

bool isCurrentProcessMaster();

void sendInitialPrimeCheckData(MPI_Request *requests, int *temporaryResults, int *counter, int *value);

void sendRestOfPrimeCheckData(MPI_Request *requests, int *numberOfCompletedRequests, int counter, int *temporaryResults, int *value);