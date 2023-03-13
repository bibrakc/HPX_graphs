#!/bin/bash

#SBATCH -J sssp-hpx
#SBATCH -p general
#SBATCH -o 4_out_%j.txt
#SBATCH -e 4_out_%j.err
#SBATCH --mail-type=ALL
#SBATCH --mail-user=bchandio@iu.edu
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=128
#SBATCH --time=00:30:00
#SBATCH --mem=145G

#Load any modules that your program needs
#module load modulename

#change directory to:
cd /N/u/bchandio/BigRed200/Research/SC23/run_scripts/march_12_2023



echo "n = 256"
procs=512
echo "-----------"

for i in {1..10}; do

  echo "Iteration: $i" 
  #Run your program
  srun -n 512 ../../code/sssp_hpx.out ../../input_graphs/data_gen/Powerlaw-clustered_ef_16_v_17.edgelist 0 0
  echo "-----------"

  #Run your program
  srun -n 512 ../../code/sssp_hpx.out ../../input_graphs/data_gen/Scale-free_ef_16_v_17.edgelist 0 0
  echo "-----------"

  #Run your program
  srun -n 512 ../../code/sssp_hpx.out ../../input_graphs/data_gen/Small-world_ef_16_v_17.edgelist 0 0
  echo "-----------"
done

