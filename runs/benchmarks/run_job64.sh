#PBS -N bfs-hpx-64
#PBS -l walltime=00:30:00
#PBS -l nodes=2:ppn=32
#PBS -l mem=15GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/benchmarks


# Launch your parallel program

echo "n = 64"

echo "-----------"

aprun -n 64 ./bfs

echo "-----------"
aprun -n 64 ./bfs
echo "-----------"
aprun -n 64 ./bfs
echo "-----------"
aprun -n 64 ./bfs
echo "-----------"
aprun -n 64 ./bfs
echo "-----------"
aprun -n 64 ./bfs
echo "-----------"



