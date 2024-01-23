#PBS -N sssp-hpx-64
#PBS -l walltime=01:00:00
#PBS -l nodes=2:ppn=32
#PBS -l mem=8GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/RIKEN_POSTER


# Launch your parallel program


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx _Erdos-Renyi.edgelist


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx _Erdos-Renyi.edgelist








