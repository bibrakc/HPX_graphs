#PBS -N bfs-hpx-32
#PBS -l walltime=00:40:00
#PBS -l nodes=1:ppn=32
#PBS -l mem=15GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/benchmarks


# Launch your parallel program

echo "n = 32"

echo "-----------"

aprun -n 32 ./bfs

echo "-----------"
aprun -n 32 ./bfs
echo "-----------"
aprun -n 32 ./bfs
echo "-----------"
aprun -n 32 ./bfs
echo "-----------"
aprun -n 32 ./bfs
echo "-----------"
aprun -n 32 ./bfs
echo "-----------"

echo "n = 16"

echo "-----------"

aprun -n 16 ./bfs

echo "-----------"
aprun -n 16 ./bfs
echo "-----------"
aprun -n 16 ./bfs
echo "-----------"
aprun -n 16 ./bfs
echo "-----------"
aprun -n 16 ./bfs
echo "-----------"
aprun -n 16 ./bfs
echo "-----------"




