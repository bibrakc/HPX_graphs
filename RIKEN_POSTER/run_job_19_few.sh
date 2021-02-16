#PBS -N sssp-hpx-1024-few
#PBS -l walltime=00:58:00
#PBS -l nodes=32:ppn=32
#PBS -l mem=15GB
#PBS -S /bin/bash


# Change to the directory you submitted the job from
cd /N/u/bchandio/BigRed2/work/Research/graphs/mydev/HPX_Graph/RIKEN_POSTER


# Launch your parallel program




echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0



echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0

echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0


echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0




echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0



echo "n = 1024"

echo "-----------"

aprun -n 1024 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0



echo "n = 512"

echo "-----------"

aprun -n 512 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0

echo "n = 256"

echo "-----------"

aprun -n 256 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0


echo "n = 128"

echo "-----------"

aprun -n 128 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0




echo "n = 64"

echo "-----------"

aprun -n 64 ./sssp-hpx ../../../graph_database_networkx/_Powerlaw-clustered.edgelist.w 0 0


