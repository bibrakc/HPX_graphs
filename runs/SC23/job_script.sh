#!/bin/bash

sc=15
input_dir=../../input_graphs/data_gen_scale_${sc}

#Define the list of edge factors
edges_factors=(14)

# Executable
exe=../../code/sssp_hpx.out

#Define the list of input files
files=(Powerlaw-clustered)
# Scale-free Small-world)

#Define the list of number of processes
processes=($1)
export FI_OFI_RXM_TX_SIZE=4096
export FI_OFI_RXM_RX_SIZE=4096

# Run


#Loop through the processes
for procs in ${processes[@]}; do
    echo "***********"
    echo "n = $procs"
    echo "***********"
    #Loop through the input files
    for f in ${files[@]}; do
        for ef in ${edges_factors[@]}; do
            echo "Edge Factor: $ef"

            input_file=${f}_ef_${ef}_v_${sc}.edgelist.bin
            echo "Running $input_file"
            for i in {1..3}; do
                echo "Iteration: $i"
                #Run your program
                srun -n ${procs} -c 2 ${exe} ${input_dir}/${input_file} 0 0 5
                echo "-----------"
            done
        done
    done
done
