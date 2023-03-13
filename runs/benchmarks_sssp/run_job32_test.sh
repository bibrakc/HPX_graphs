#PBS -N sssp17-hpx-32
#PBS -l walltime=01:00:00
#PBS -l nodes=1:ppn=32
#PBS -l mem=10GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/benchmarks_sssp


# Launch your parallel program

echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp17

echo "-----------"
aprun -n 32 ./sssp17
