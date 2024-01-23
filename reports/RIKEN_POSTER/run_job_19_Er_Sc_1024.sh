#PBS -N sssp-hpx-1024-Erdos-Scale
#PBS -l walltime=01:00:00
#PBS -l nodes=32:ppn=32
#PBS -l mem=15GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/RIKEN_POSTER


# Launch your parallel program


echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ./_Erdos-Renyi_ef_32.edgelist.w 0 0

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ./_Erdos-Renyi_ef_32.edgelist.w 0 0


echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ./_Erdos-Renyi_ef_32.edgelist.w 0 0

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ./_Erdos-Renyi_ef_32.edgelist.w 0 0


echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ./_Erdos-Renyi_ef_32.edgelist.w 0 0

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ./_Erdos-Renyi_ef_32.edgelist.w 0 0



echo "Erdos ends"

echo "n = 1024"
echo "-----------"

aprun -n 1024 ./sssp-hpx ./_Scale-free_ef_32.edgelist.w 0 0

echo "n = 512"
echo "-----------"

aprun -n 512 ./sssp-hpx ./_Scale-free_ef_32.edgelist.w 0 0



echo "n = 1024"
echo "-----------"

aprun -n 1024 ./sssp-hpx ./_Scale-free_ef_32.edgelist.w 0 0

echo "n = 512"
echo "-----------"

aprun -n 512 ./sssp-hpx ./_Scale-free_ef_32.edgelist.w 0 0


echo "n = 1024"
echo "-----------"

aprun -n 1024 ./sssp-hpx ./_Scale-free_ef_32.edgelist.w 0 0

echo "n = 512"
echo "-----------"

aprun -n 512 ./sssp-hpx ./_Scale-free_ef_32.edgelist.w 0 0


