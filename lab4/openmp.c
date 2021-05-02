#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>

#define RANGE_END 100000000

/**
 * Generates list of prime numbers from 0 to RANGE_END using Sieve of Eratosthenes
 * @return list in which list[index] shows if index is prime
 */
bool* generateListOfPrimes() {
    bool* primes = malloc(RANGE_END * sizeof(bool));

#pragma omp parallel for schedule(dynamic, 100)
    for (int i = 0; i < RANGE_END; i++) {
        primes[i] = true;
    }

#pragma omp parallel for schedule(dynamic, 100)
    for (int i = 2; i < RANGE_END; i++) {
        if (primes[i]) { // if wasn't marked as not-prime earlier
#pragma omp parallel for schedule(dynamic, 100)
            // set each multiplicity of i as not-prime
            for(int j = 2 * i; j < RANGE_END; j+=i) {
                primes[j] = false;
            }
        }
    }
    return primes;
}

void printExecutionTime(struct timeval *start, struct timeval *stop) {
    long time = 1000000 * (stop->tv_sec - start->tv_sec) + stop->tv_usec - start->tv_usec;
    printf("\nOpenMP execution time=%ld microseconds\n", time);
}

int main(int argc, char **argv) {
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    bool* primes = generateListOfPrimes();
    bool global_conjecture_check = true;

#pragma omp parallel for schedule(dynamic, 100)
    for (int i = 0 ; i < RANGE_END; i+=2) {
        bool flag = false;
        for(int j = 0; j <= i / 2; j++) {
            if (primes[j] && primes[i-j]) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            global_conjecture_check = false;
            printf("Goldbach's Conjecture is not correct for number %d\n", i);
        }
    }

    if (global_conjecture_check){
        printf("Goldbach's conjecture for numbers in range (%d, %d) found no mistakes", 0, RANGE_END);
    } else {
        printf("Goldbach's conjecture is not right");
    }

    gettimeofday(&stop, NULL);
    printExecutionTime(&start, &stop);
}
