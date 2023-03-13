/*
 * Author:  Bibrak Qamar
 * File:    Bin2ASCII.c
 *
 * Basic C code to read an ASCII graph file and 
 * write the edges to a binay file
 *
*/

/*
 * compile: gcc Bin2ASCII.c
 * run: ./a.out ./scale16_s.mm
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char* argv[]){
   
  char *filename = argv[1];
  //printf("file is: %s\n", filename);
  int is_zero_indexed = atoi(argv[2]);

  FILE *f = NULL;

  if ((f = fopen(filename, "r")) == NULL)
    return -1; 
  
  int N = 0;
  int N_edges = 0;
 
  fscanf(f, "%d\t%d", &N, &N);  
  fscanf(f, "%d", &N_edges);

  //printf("The graph vertex count is %d with %d egdes.\n", N, N_edges);

  printf("%d\t%d\n", N, N);
  printf("%d\n", N_edges);

  FILE *f_wtr;
  char filename_w[256];
  strcpy(filename_w, filename);
  strcat(filename_w, ".bin");
  if((f_wtr = fopen(filename_w,"wb")) == NULL)  // w for write, b for binary
    return -1;



  int vert_from, vert_to;
  float  weight;
  vert_from = -1;
  vert_to = -1;
  
  //int is_EOF;
  //while((is_EOF = fscanf(f, "%d %d", &vert_from, &vert_to)) != EOF){
  for(int i=0; i< N_edges; i++){
    // printf("i = %d\n", i);
    fscanf(f, "%d\t%d\t%d", &vert_from, &vert_to, &weight);
    if(is_zero_indexed){
      vert_from --; vert_to --;
    }
    //printf("read %d --> %d, w = %d\n", vert_from, vert_to, weight);
    //printf("%d\t%d\t%d\n", vert_from, vert_to, weight);
    fwrite(&vert_from, sizeof(vert_from), 1, f_wtr); 
    fwrite(&vert_to, sizeof(vert_from), 1, f_wtr);
    fwrite(&weight, sizeof(vert_from), 1, f_wtr);
  }

  fclose(f);
  fclose(f_wtr);

  return 0;
}

