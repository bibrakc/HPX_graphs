#PBS -N sssp19-hpx-512
#PBS -l walltime=02:40:00
#PBS -l nodes=16:ppn=32
#PBS -l mem=7GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/benchmarks_sssp


# Launch your parallel program

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp19

echo "-----------"
aprun -n 512 ./sssp19
echo "-----------"
aprun -n 512 ./sssp19
echo "-----------"
aprun -n 512 ./sssp19
echo "-----------"
aprun -n 512 ./sssp19
echo "-----------"
aprun -n 512 ./sssp19
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



