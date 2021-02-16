#PBS -N bfs-hpx-1
#PBS -l walltime=03:50:00
#PBS -l nodes=1:ppn=32
#PBS -l mem=20GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/benchmarks


# Launch your parallel program

echo "n = 2"

echo "-----------"

aprun -n 2 ./bfs

echo "-----------"
aprun -n 2 ./bfs
echo "-----------"
aprun -n 2 ./bfs
echo "-----------"
aprun -n 2 ./bfs
echo "-----------"
aprun -n 2 ./bfs
echo "-----------"
aprun -n 2 ./bfs
echo "-----------"

echo "n = 1"

echo "-----------"

aprun -n 1 ./bfs

echo "-----------"
aprun -n 1 ./bfs
echo "-----------"
aprun -n 1 ./bfs
echo "-----------"
aprun -n 1 ./bfs
echo "-----------"
aprun -n 1 ./bfs
echo "-----------"
aprun -n 1 ./bfs
echo "-----------"




