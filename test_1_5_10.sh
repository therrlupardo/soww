mpicc $1
for n in 1 5 10
do
  echo "Test dla" $n "procesów"
  time mpirun --hostfile hostfile -np $n ./a.out
done
