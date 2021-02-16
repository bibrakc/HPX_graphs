#PBS -N sssp-hpx-1024-all
#PBS -l walltime=00:58:00
#PBS -l nodes=32:ppn=32
#PBS -l mem=15GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/RIKEN_POSTER


# Launch your parallel program


echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ../../../graph_database_networkx/_Erdos-Renyi.edgelist.w 0 0

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ../../../graph_database_networkx/_Erdos-Renyi.edgelist.w 0 0


echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ../../../graph_database_networkx/_Erdos-Renyi.edgelist.w 0 0

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ../../../graph_database_networkx/_Erdos-Renyi.edgelist.w 0 0


echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ../../../graph_database_networkx/_Erdos-Renyi.edgelist.w 0 0

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ../../../graph_database_networkx/_Erdos-Renyi.edgelist.w 0 0



echo "Erdos ends"




echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0



echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0


echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0




echo "Power Law Ends"


echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ../../../graph_database_networkx/_Scale-free.edgelist.w 0 0

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ../../../graph_database_networkx/_Scale-free.edgelist.w 0 0


echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ../../../graph_database_networkx/_Scale-free.edgelist.w 0 0

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ../../../graph_database_networkx/_Scale-free.edgelist.w 0 0


echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ../../../graph_database_networkx/_Scale-free.edgelist.w 0 0


echo "n = 32"

echo "-----------"

aprun -n 32 ./sssp-hpx ../../../graph_database_networkx/_Scale-free.edgelist.w 0 0




echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ../../../graph_database_networkx/_Scale-free.edgelist.w 0 0

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ../../../graph_database_networkx/_Scale-free.edgelist.w 0 0


echo "Scale Free ends"



echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ../../../graph_database_networkx/_Small-world.edgelist.w 0 0

echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ../../../graph_database_networkx/_Small-world.edgelist.w 0 0


echo "Small World ends"


echo "n= 1024"

aprun -n 1024 ./sssp-hpx ./scale19_s.mm 0 1

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ./scale19_s.mm 0 1


echo "n= 1024"

aprun -n 1024 ./sssp-hpx ./scale19_s.mm 0 1

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ./scale19_s.mm 0 1


echo "n= 1024"

aprun -n 1024 ./sssp-hpx ./scale19_s.mm 0 1

echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ./scale19_s.mm 0 1



