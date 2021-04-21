# Compilation
`mpicc file_name.c`

## Parameters:
* `-lm` - in case of math operations like `sin(x)`
* `-o output_file_name.out` - to give name to output file (by default `RANGE_START.out`)

# Running
`time mpirun -np X output_file.out`

Where `X` is number of processes

To run in lab it's recomended to use command below:
`time mpirun -v --hostfile machinefile -np 4 --mca btl_tcp_if_exclude docker0 ./RANGE_START.out`

Where machinefile contains addresses of machines on which code should be running. Should look like this:
```
172.20.83.201
172.20.83.203
# 172.20.83.216
# 172.20.83.217
# 172.20.83.218
```

# Instructions
## Lab1
Using given example (on enauczanie):
1. calculate PI using formula below:
```
pi/4 = 1/1 - 1/3 + 1/5 - 1/7 ...
```
2. calculate integral of sin(x) from -pi to pi using trapezoid and rectange methods

## Lab2
Look at given example (on enauczanie). 
1. Convert it to non-blocking communication
2. Compare time of execution on both parts.

## Lab3
Silna hipoteza Goldbacha - dowolna liczba parzysta może być przedstawiona jako suma liczb pierwszych

algorytmicznie, pomysł jak sprawdzić
parametry wejściowe to zakres liczb
wewnątrz przedziału sprawdzić czy hipoteza się sprawdza
wewnątrz problemu znajdowanie liczb pierwszych