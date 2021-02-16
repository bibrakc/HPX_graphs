#PBS -N sssp-hpx-128
#PBS -l walltime=30:00:00
#PBS -l nodes=4:ppn=32
#PBS -l mem=15GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/RIKEN_POSTER


# Launch your parallel program


echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ./_Erdos-Renyi.edgelist.w 0 0

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ./_Erdos-Renyi.edgelist.w 0 0


echo "n = 64"
 
echo "-----------"

aprun -n 64 ./sssp-hpx ./_Erdos-Renyi.edgelist.w 0 0


