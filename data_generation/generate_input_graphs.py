#Bibrak Qamar Chandio
#Generate Input Graphs

import time
import networkx as nx
import random

# For statistics
import pandas as pd

# For argument parsing
import sys 

# nme of the graph
A=""

Output_filename=""

def write_to_file(filename, content):
    with open(filename, 'a') as f:
        f.write(content+"\n")

# Input: A graph
# Output: find degrees and plot their distribution
def Degree_Distribution(G):
    #start = time.time()

    degree = G.degree()
    degree = [ deg for (v,deg) in degree ]
    avg_degree = sum(degree)/len(degree)

    #end = time.time()
    #print("Avg Degree :", avg_degree, "\n")

    plot_distribution(degree, xlabel='Degree ($k$)',
                  ylabel='Number of nodes with degree $k$ ($N_k$)', title='Degree distributions '+A)

    plt.savefig ( "Degree_Distribution_"+A+".png" )

    print("Degree Distribution Statistics")
    s = pd.Series(degree)
    print(s.describe())
    write_to_file(Output_filename, "Degree Distribution Statistics")
    write_to_file(Output_filename, str(s.describe()))


# Input: A graph
# Find the sizes of all connected components and plot the distribution
def CC_Distribution(G):
    cc_sorted = sorted(nx.connected_components(G), key=len, reverse=True)
    # print statistics of the top 5 components (if exist)
    topcc = min(len(cc_sorted), 5)
    for i in  range(topcc):
        cc = cc_sorted[i]
        cc_graph = G.subgraph(cc)
        n = cc_graph.number_of_nodes()
        m = cc_graph.number_of_edges()
        n_percent = (n/G.number_of_nodes()) * 100
        print("Largest component #", i+1)
        print("Number of vertices:", n, " (", n_percent, ")", "\nNumber of edges: ", m, "\n")

    cc_sizes = [len(c) for c in cc_sorted]
    plot_distribution(cc_sizes, xlabel='Weakly connected component size',
                  ylabel='Count', title='Connected component size distributions '+A)

    plt.savefig ( "CC_Distribution_"+A+".png" )


# Input: A graph
# Find the sizes of all connected components diameter
def Diameter_CC(G):
    cc_sorted = sorted(nx.connected_components(G), key=len, reverse=True)
    # print statistics of the top 5 components (if exist)
    topcc = min(len(cc_sorted), 5)
    sum = 0

    for i in  range(topcc):
        cc = cc_sorted[i]
        cc_graph = G.subgraph(cc)
        n = cc_graph.number_of_nodes()
        m = cc_graph.number_of_edges()

        diam = nx.diameter(cc_graph)
        sum +=diam

        print("component #",i+1," diameter: ", diam, "n = ", n, "m= ", m, "\n")


    print("Avg Diam: ",sum/topcc, "\n")



# Input: A graph
# Find the local clustering coefficient of all vertices and plot distribution
def Clustering_Analysis(G):

    clust = nx.clustering(G)
    local_clust_coefficient = [ v for v in clust.values() ]
    #print("local_clust_coefficient = " , local_clust_coefficient)
    avg_clust_coefficient = sum(local_clust_coefficient)/G.number_of_nodes()
    #end = time.time()

    #print("Average clustering coefficient: ", avg_clust_coefficient,"\n")
    #plot the distribution of clustering coefficient
    plot_distribution(local_clust_coefficient, xlabel='Clustering coefficient',
                  ylabel='Number of vertices', title='Clustering coefficient distributions '+A,
                      xlog=False, ylog=True, showLine=False)
    plt.savefig ( "Clustering_Analysis_"+A+".png" )

    print("Clustering Statistics")
    s = pd.Series(local_clust_coefficient)
    print(s.describe())
    write_to_file(Output_filename, "Clustering Analysis")
    write_to_file(Output_filename,str(s.describe()));


# Input: A graph
# Find shortest paths in the largest 5 componets and plot distribution

def ShortestPaths_Analysis(G):
    cc_sorted = sorted(nx.connected_components(G), key=len, reverse=True)

    # find shortest paths in top 1 components
    topcc = min(len(cc_sorted), 1)
    for i in  range(topcc) :
        cc = cc_sorted[i]
        cc_graph = G.subgraph(cc)
        
        if(len(cc)>200):
            print("This component is too large. Using fifteen single-source shortest paths.")
            cc = list(cc)
            cc_graph = G.subgraph(cc)
            shortest_path_lens = []
            for i in range(15):
                #print("cc[i] = ", cc[i])
                length = nx.single_source_shortest_path_length(cc_graph, cc[i])
                shortest_path_lens += [ v for v in length.values() ]
        else:
            all_shortest_path_dict = dict(nx.all_pairs_shortest_path_length(cc_graph))
            shortest_path_lens = []
            for val1 in all_shortest_path_dict.values():
                for val in val1.values():
                    shortest_path_lens.append(val)


        #print(shortest_path_lens)
        avg_shortest_path_lens = sum(shortest_path_lens)/len(shortest_path_lens)
        #print("Average shortest_path_lens: ", avg_shortest_path_lens)
        plot_distribution(shortest_path_lens, xlabel='Shortest path lengths (hops)',
                  ylabel='Number of paths', title='Shortest path lengths distributions '+A,
                      xlog=False, ylog=False, showLine=True, intAxis=True)

        plt.savefig ( "ShortestPaths_Analysis"+A+".png" )

        print("Shortest Path Lengths Statistics")
        s = pd.Series(shortest_path_lens)
        print(s.describe())
        write_to_file(Output_filename, "ShortestPaths_Analysis")
        write_to_file(Output_filename, str(s.describe()));


import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
plt.style.use('ggplot')
#plt.style.use('seaborn-ticks')
#plt.style.use('seaborn-notebook')
plt.rcParams['lines.linewidth']=3
plt.rcParams['xtick.labelsize']=12
plt.rcParams['ytick.labelsize']=12
plt.rcParams['axes.labelsize']=14


def plot_distribution (data, xlabel='', ylabel='', title='', xlog = True, ylog= True, showLine=False, intAxis=False) :
    counts = {}
    for item in data :
        if item not in counts :
            counts [ item ] = 0
        counts [ item ] += 1
    counts = sorted ( counts.items () )
    fig = plt.figure ()
    ax = fig.add_subplot (111)
    ax.scatter ([ k for (k , v ) in counts ] , [ v for (k , v ) in counts ])
    if(len(counts)<20):  # for tiny graph
        showLine=True
    if showLine==True:
        ax.plot ([ k for (k , v ) in counts ] , [ v for (k , v ) in counts ])
    if xlog == True:
        ax.set_xscale ( 'log')
    if ylog == True:
        ax.set_yscale ( 'log')
    if intAxis == True:
        gca = fig.gca()
        gca.xaxis.set_major_locator(MaxNLocator(integer=True))
    ax.set_xlabel ( xlabel)
    ax.set_ylabel ( ylabel )
    plt.title ( title )
    #fig.savefig ( "degree_distribution.png" )

def plot_degree_bar (G) :
    degs = {}
    for n in G.nodes () :
        deg = G.degree ( n )
        if deg not in degs :
            degs [ deg ] = 0
        degs [ deg ] += 1
    items = sorted ( degs.items () )
    fig = plt.figure ()
    ax = fig.add_subplot (111)
    print(items)
    ax.bar([ k for (k , v ) in items ] , [ v for (k , v ) in items ])
    ax.set_xlabel ( 'Degree ($k$)')
    ax.set_ylabel ( 'Number of nodes with degree $k$ ($N_k$)')


# Main


args = sys.argv 

#graph_types = ["Powerlaw", "Scalefree", "Smallworld", "Erdos"]
graph = args[1]

edge_factor = int(args[2])
scale_factor = int(args[3])
vertices_needed = 2**scale_factor
edges_needed = vertices_needed * edge_factor



print(graph)
Output_filename=graph+"_ef_"+str(edge_factor)+"_v_"+str(scale_factor)+"_output.txt"
write_to_file(Output_filename, "\n\n"+graph)

if graph == "Powerlaw":
    G_gen = nx.powerlaw_cluster_graph(vertices_needed, edge_factor, .4)
    while (nx.is_connected(G_gen) == False):
        print(graph+" was not connected trying again")
        G_gen = nx.powerlaw_cluster_graph(vertices_needed, edge_factor, .4)
    A = "Powerlaw-clustered_ef_"+str(edge_factor)+"_v_"+str(scale_factor)

if graph == "Scalefree":
    G_gen = nx.barabasi_albert_graph(vertices_needed, edge_factor)
    while (nx.is_connected(G_gen) == False):
        print(graph+" was not connected trying again")
        G_gen = nx.barabasi_albert_graph(vertices_needed, edge_factor)
    A = "Scale-free_ef_"+str(edge_factor)+"_v_"+str(scale_factor)

if graph == "Smallworld":
    G_gen = nx.watts_strogatz_graph(vertices_needed, edge_factor, .2)
    while (nx.is_connected(G_gen) == False):
        print(graph+" was not connected trying again")
        G_gen = nx.watts_strogatz_graph(vertices_needed, edge_factor, .2)
    A = "Small-world_ef_"+str(edge_factor)+"_v_"+str(scale_factor)


if graph == "Erdos":
    G_gen = nx.gnm_random_graph(vertices_needed, edges_needed, seed=133)
    while (nx.is_connected(G_gen) == False):
        print(graph+" was not connected trying again")
        G_gen = nx.gnm_random_graph(vertices_needed, edges_needed, seed=133)
    A = "Erdos-Renyi_ef_"+str(edge_factor)+"_v_"+str(scale_factor)


#print(A, ": Number of vertices:", G_gen.number_of_nodes(), ", Number of edges: ", G_gen.number_of_edges())
#write_to_file(Output_filename, A+ ": Number of vertices:"+ str(G_gen.number_of_nodes())+ ", Number of edges: "+ str(G_gen.number_of_edges()))

for (u, v) in G_gen.edges():
    G_gen.edges[u,v]['weight'] = random.randint(1,5)





# Analyze the graph that you have created

# Degree Distribution is time consuming
start = time.time()
Degree_Distribution(G_gen)
end = time.time()
print("Time in Degree_Distribution: ", end-start, "\n")


start = time.time()
Clustering_Analysis(G_gen)
end = time.time()
print("Time in Clustering_Analysis: ", end-start, "\n")


start = time.time()
ShortestPaths_Analysis(G_gen)
end = time.time()
print("Time in SSSP: ", end-start, "\n")





length, path = nx.single_source_dijkstra(G_gen, 0, 35, weight='weight')
print("SSSP Path with wieghts from Src: 0 to target: 35 = ", length, "\n")
print(path)

#sssp = nx.shortest_path(G_gen,source=4, target=47, weight=None)#, weight='weight')
#sssp = nx.shortest_path(G_gen,source=0, target=35,  weight='weight')
#sss_path = nx.single_source_shortest_path(G_gen, 47)
#print(sssp)
#print(path)


""" 
# Get edge labels
edge_labels = {(u,v):d["weight"] for u,v,d in G_gen.edges(data=True)}

# Draw the graph
pos = nx.spring_layout(G_gen)
nx.draw(G_gen, pos, with_labels=True)
nx.draw_networkx_edge_labels(G_gen, pos, edge_labels=edge_labels)

# Show the graph
plt.show() """


'''
start = time.time()
CC_Distribution(G_gen)
end = time.time()
print("Time in CC: ", end-start, "\n")
'''

# Store the graph as directed graph in file
G = G_gen.to_directed()



#nx.draw(G, with_labels=True)
# Show the graph
#plt.show()





filename_to_write = A+".edgelist"

nx.write_weighted_edgelist(G, filename_to_write, delimiter='\t')

n = G.number_of_nodes()
m = G.number_of_edges()
print(A, "Directed : Number of vertices:", n, ", Number of edges: ", m, ", Edge Factor: ", edge_factor, ", Scale: ", scale_factor)
write_to_file(Output_filename, A+" Directed: Number of vertices:"+ str(n)+", Number of edges: "+ str(m)+", Edge Factor: "+ str(edge_factor) + ", Scale: "+ str( scale_factor));


f = open(filename_to_write,'r')
temp = f.read()
f.close()

f = open(filename_to_write, 'w')

f.write(str(n)+"\t"+str(n)+"\n")
f.write(str(m)+"\n")

f.write(temp)
f.close()
