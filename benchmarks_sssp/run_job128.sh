#PBS -N sssp17-hpx-128
#PBS -l walltime=00:40:00
#PBS -l nodes=4:ppn=32
#PBS -l mem=10GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/benchmarks_sssp


# Launch your parallel program

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp17

echo "-----------"
aprun -n 128 ./sssp17
echo "-----------"
aprun -n 128 ./sssp17
echo "-----------"
aprun -n 128 ./sssp17
echo "-----------"
aprun -n 128 ./sssp17
echo "-----------"
aprun -n 128 ./sssp17
echo "-----------"


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp17

echo "-----------"
aprun -n 64 ./sssp17
echo "-----------"
aprun -n 64 ./sssp17
echo "-----------"
aprun -n 64 ./sssp17
echo "-----------"
aprun -n 64 ./sssp17
echo "-----------"
aprun -n 64 ./sssp17
echo "-----------"


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp17

echo "-----------"
aprun -n 32 ./sssp17
echo "-----------"
aprun -n 32 ./sssp17
echo "-----------"
aprun -n 32 ./sssp17
echo "-----------"
aprun -n 32 ./sssp17
echo "-----------"
aprun -n 32 ./sssp17
echo "-----------"




