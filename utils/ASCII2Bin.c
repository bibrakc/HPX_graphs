/*
 * Author:  Bibrak Qamar
 * File:    ASCII2Bin.c
 * 2018-2023
 * 
 * Basic C code to read an ASCII graph file and
 * write the edges to a binay file
 *
 */

/*
 * compile: gcc ASCII2Bin.c
 * run: ./a.out ./scale16_s.mm 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{

  char *filename = argv[1];
  // printf("file is: %s\n", filename);

  // is the file graph500 generated from Octave?
  // 0 for no, 1 for yes
  // Octave stores matrices from 1 not 0, therefore need to convert
  // for C representations that start index from 0.
  int is_octave_indexed = atoi(argv[2]);

  FILE *f = NULL;

  if ((f = fopen(filename, "r")) == NULL)
    return -1;

  int N = 0;
  int N_edges = 0;

  fscanf(f, "%d\t%d", &N, &N);
  fscanf(f, "%d", &N_edges);

  // printf("The graph vertex count is %d with %d egdes.\n", N, N_edges);

  printf("%d\t%d\n", N, N);
  printf("%d\n", N_edges);

  FILE *f_wtr;
  char filename_w[256];

  char num_vertices_str[256];
  char num_edges_str[256];

  sprintf(num_vertices_str, "%d", N);
  sprintf(num_edges_str, "%d", N_edges);
  //itoa(N, num_vertices_str, DECIMAL);
  //itoa(N_edges, num_edges_str, DECIMAL);

  printf("num_vertices_str: %s\n", num_vertices_str);
  printf("num_edges_str: %s\n", num_edges_str);

  strcpy(filename_w, filename);

  strcat(filename_w, "_v_");
  strcat(filename_w, num_vertices_str);
  strcat(filename_w, "_e_");
  strcat(filename_w, num_edges_str);

  strcat(filename_w, ".bin");

  if ((f_wtr = fopen(filename_w, "wb")) == NULL) // w for write, b for binary
    return -1;

  int vert_from, vert_to;
  int weight;
  vert_from = -1;
  vert_to = -1;

  // int is_EOF;
  // while((is_EOF = fscanf(f, "%d %d", &vert_from, &vert_to)) != EOF){
  for (int i = 0; i < N_edges; i++)
  {
    // printf("i = %d\n", i);
    fscanf(f, "%d\t%d\t%d", &vert_from, &vert_to, &weight);
    if (is_octave_indexed)
    {
      vert_from--;
      vert_to--;
    }
    // printf("read %d --> %d, w = %d\n", vert_from, vert_to, weight);
    // printf("%d\t%d\t%d\n", vert_from, vert_to, weight);
    fwrite(&vert_from, sizeof(vert_from), 1, f_wtr);
    fwrite(&vert_to, sizeof(vert_from), 1, f_wtr);
    fwrite(&weight, sizeof(vert_from), 1, f_wtr);
  }

  fclose(f);
  fclose(f_wtr);

  return 0;
}
