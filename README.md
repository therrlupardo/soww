# Compilation
`mpicc file_name.c`

## Parameters:
* `-lm` - in case of math operations like `sin(x)`
* `-o output_file_name.out` - to give name to output file (by default `a.out`)

# Running
`time mpirun -np X output_file.out`

Where `X` is number of processes

To run in lab it's recomended to use command below:
`time mpirun -v --hostfile machinefile -np 4 --mca btl_tcp_if_exclude docker0 ./a.out`

Where machinefile contains addresses of machines on which code should be running. Should look like this:
```
172.20.83.201
172.20.83.203
# 172.20.83.216
# 172.20.83.217
# 172.20.83.218
```