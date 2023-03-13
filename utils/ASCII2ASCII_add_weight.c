/*
 * Author:  Bibrak Qamar
 * File:    ASCII2Bin.c
 *
 * Basic C code to read an ASCII graph file and
 * write the edges to a binay file
 *
*/

/*
 * compile: gcc ASCII2ASCII_add_weight.c
 * run: ./a.out ./scale16.mm
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char* argv[]){

  char *filename = argv[1];
  //printf("file is: %s\n", filename);
  // is the file graph500 generated from Octave?
  // 0 for no, 1 for yes
  int is_graph500 = atoi(argv[2]);



  FILE *f = NULL;

  if ((f = fopen(filename, "r")) == NULL)
    return -1;

  int N = 0;
  int N_edges = 0;

  fscanf(f, "%d\t%d", &N, &N);
  fscanf(f, "%d", &N_edges);

  //printf("The graph vertex count is %d with %d egdes.\n", N, N_edges);

  //printf("%d\t%d\n", N, N);
  //printf("%d\n", N_edges);

  FILE *f_wtr;
  char filename_w[256];
  strcpy(filename_w, filename);
  strcat(filename_w, ".w");
  if((f_wtr = fopen(filename_w,"w")) == NULL)  // w for write, b for binary we not doing binary
    return -1;



  int vert_from, vert_to;
  int  weight;
  vert_from = -1;
  vert_to = -1;

  fprintf(f_wtr, "%d\t%d\n", N, N);
  fprintf(f_wtr, "%d\n", N_edges);
  //int is_EOF;
  //while((is_EOF = fscanf(f, "%d %d", &vert_from, &vert_to)) != EOF){
  for(int i=0; i< N_edges; i++){
    // printf("i = %d\n", i);
    fscanf(f, "%d\t%d", &vert_from, &vert_to);
    if(is_graph500){
      vert_from --; vert_to --;
    }
    weight = (rand()%9) +1;
    //printf("read %d --> %d, w = %d\n", vert_from, vert_to, weight);
    fprintf(f_wtr, "%d\t%d\t%d\n", vert_from, vert_to, weight);
    //fwrite(&vert_from, sizeof(vert_from), 1, f_wtr);
    //fwrite(&vert_to, sizeof(vert_from), 1, f_wtr);

    //fwrite(&weight, sizeof(vert_from), 1, f_wtr);
  }

  fclose(f);
  fclose(f_wtr);

  return 0;
}
