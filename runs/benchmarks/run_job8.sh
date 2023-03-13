#PBS -N bfs-hpx-8
#PBS -l walltime=00:50:00
#PBS -l nodes=1:ppn=32
#PBS -l mem=15GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/benchmarks


# Launch your parallel program

echo "n = 8"

echo "-----------"

aprun -n 8 ./bfs

echo "-----------"
aprun -n 8 ./bfs
echo "-----------"
aprun -n 8 ./bfs
echo "-----------"
aprun -n 8 ./bfs
echo "-----------"
aprun -n 8 ./bfs
echo "-----------"
aprun -n 8 ./bfs
echo "-----------"

echo "n = 4"

echo "-----------"

aprun -n 4 ./bfs

echo "-----------"
aprun -n 4 ./bfs
echo "-----------"
aprun -n 4 ./bfs
echo "-----------"
aprun -n 4 ./bfs
echo "-----------"
aprun -n 4 ./bfs
echo "-----------"
aprun -n 4 ./bfs
echo "-----------"




