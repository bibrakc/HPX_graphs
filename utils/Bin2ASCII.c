/*
 * Author:  Bibrak Qamar
 * File:    Bin2ASCII.c
 * 2018-2023
 *
 * Basic C code to read an ASCII graph file and
 * write the edges to a binay file
 *
 */

/*
 * compile: gcc Bin2ASCII.c
 * run: ./a.out ./graph_edges.bin 40 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char *argv[])
{

  int N = 0;
  int N_edges = 0;

  char *filename = argv[1];
  N = atoi(argv[2]);
  N_edges = atoi(argv[3]);

  FILE *f = NULL;
  if ((f = fopen(filename, "rb")) == NULL)
  {
    printf("File reading failed!\n");
    return -1;
  }

  printf("The graph vertex count is %d with %d egdes.\n", N, N_edges);

  int vert_from, vert_to, weight;

  int edge_size_in_bytes = sizeof(int) * 3;

  // seek to my chunk
  fseek(f, edge_size_in_bytes, SEEK_CUR);

  for (int i = 0; i < N_edges; i++)
  {
    // printf("i = %d\n", i);
    fread(&vert_from, sizeof(int), 1, f);
    fread(&vert_to, sizeof(int), 1, f);
    fread(&weight, sizeof(int), 1, f);

    // fscanf(f, "%d\t%d\t%d", &vert_from, &vert_to, &weight);

    printf("%d\t%d\t%d\n",vert_from, vert_to, weight);
  }

  fclose(f);

  return 0;
}
