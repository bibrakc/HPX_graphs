#PBS -N sssp19-hpx-1024
#PBS -l walltime=01:10:00
#PBS -l nodes=32:ppn=32
#PBS -l mem=7GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/benchmarks_sssp


# Launch your parallel program

echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp19

echo "-----------"
aprun -n 1024 ./sssp19
echo "-----------"
aprun -n 1024 ./sssp19
echo "-----------"
aprun -n 1024 ./sssp19
echo "-----------"
aprun -n 1024 ./sssp19
echo "-----------"
aprun -n 1024 ./sssp19
echo "-----------"

echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp19

echo "-----------"
aprun -n 256 ./sssp19
echo "-----------"
aprun -n 256 ./sssp19
echo "-----------"
aprun -n 256 ./sssp19
echo "-----------"
aprun -n 256 ./sssp19
echo "-----------"
aprun -n 256 ./sssp19
echo "-----------"



