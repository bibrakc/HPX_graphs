#PBS -N bfs-hpx-128
#PBS -l walltime=00:13:00
#PBS -l nodes=4:ppn=32
#PBS -l mem=15GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/benchmarks


# Launch your parallel program

echo "n = 128"

echo "-----------"

aprun -n 128 ./bfs

echo "-----------"
aprun -n 128 ./bfs
echo "-----------"
aprun -n 128 ./bfs
echo "-----------"
aprun -n 128 ./bfs
echo "-----------"
aprun -n 128 ./bfs
echo "-----------"
aprun -n 128 ./bfs
echo "-----------"



