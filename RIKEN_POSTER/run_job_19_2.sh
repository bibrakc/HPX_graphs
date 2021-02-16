#PBS -N sssp-hpx-256
#PBS -l walltime=02:00:00
#PBS -l nodes=8:ppn=32
#PBS -l mem=15GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/RIKEN_POSTER


# Launch your parallel program




echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ./_Powerlaw-clustered.edgelist.w 0 0

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ./_Powerlaw-clustered.edgelist.w 0 0


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ./_Powerlaw-clustered.edgelist.w 0 0


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx ./_Powerlaw-clustered.edgelist.w 0 0



echo "Power Law Ends"


echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ./_Scale-free.edgelist.w 0 0

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ./_Scale-free.edgelist.w 0 0


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ./_Scale-free.edgelist.w 0 0


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx ./_Scale-free.edgelist.w 0 0




echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ./_Scale-free.edgelist.w 0 0

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ./_Scale-free.edgelist.w 0 0


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ./_Scale-free.edgelist.w 0 0


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx ./_Scale-free.edgelist.w 0 0


echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ./_Scale-free.edgelist.w 0 0

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ./_Scale-free.edgelist.w 0 0


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ./_Scale-free.edgelist.w 0 0


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx ./_Scale-free.edgelist.w 0 0




echo "Scale Free ends"



echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ./_Small-world.edgelist.w 0 0

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ./_Small-world.edgelist.w 0 0


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ./_Small-world.edgelist.w 0 0


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx ./_Small-world.edgelist.w 0 0



echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ./_Small-world.edgelist.w 0 0

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ./_Small-world.edgelist.w 0 0


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ./_Small-world.edgelist.w 0 0


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx ./_Small-world.edgelist.w 0 0



echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ./_Small-world.edgelist.w 0 0

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ./_Small-world.edgelist.w 0 0


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ./_Small-world.edgelist.w 0 0


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx ./_Small-world.edgelist.w 0 0


echo "Small World ends"



echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ./scale19_s.mm 0 1

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ./scale19_s.mm 0 1


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ./scale19_s.mm 0 1


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx ./scale19_s.mm 0 1




echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ./scale19_s.mm 0 1

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ./scale19_s.mm 0 1


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ./scale19_s.mm 0 1


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx ./scale19_s.mm 0 1


echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ./scale19_s.mm 0 1

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ./scale19_s.mm 0 1


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ./scale19_s.mm 0 1


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx ./scale19_s.mm 0 1

