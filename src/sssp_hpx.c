/*
2018-2023
Asnychronous Single Source Shortest Path (SSSP) using HPX-5

Author: Bibrak Qamar Chandio

Work performed at Indiana University Bloomington for
PhD in Intelligent Systems Engineering

*/

#include <stdio.h>
#include <stdlib.h>
#include <hpx/hpx.h>

#include <sys/time.h>

#define EDGES_BUFF_SIZE 5000
#define UNKNOWN -3
#define ROOT -1

char *filename_;
int root_vertex_;
int graph500_gen;

typedef struct
{
  int node_id;
  int weight;

} edge_t;

struct edge_bucket
{

  int count;
  edge_t edges[EDGES_BUFF_SIZE];
  struct edge_bucket *next;

}; // the node

typedef struct edge_bucket edge_bucket;

typedef struct
{

  int buckets_count;
  edge_bucket *head; // head of the linked list
  edge_bucket *tail;

} edge_list;

typedef struct
{

  // int edges[EDGES_BUFF_SIZE];

  edge_list edges;
  // int count;

  // TODO: although seems redundant but lets put it here
  int vertex_id;

  // the result in this case parent for SSSP
  int parent;
  int distance_from_start;

  // Dijkstra–Scholten termination detection
  int first_edge;
  int deficit;

  // TODO:
  // edges_list *next;

  // control related
  hpx_addr_t vertex_local_mutex;

  // for performance statistics
  int actions_invoked;
  int reduced_int_buffer;
  double total_time_in_actions;
  double reduced_time_buffer;

} Vertex;

// Get the global address of vertex at ith location
hpx_addr_t vertex_element_at(hpx_addr_t vert, int i)
{
  return hpx_addr_add(vert, sizeof(Vertex) * i, sizeof(Vertex));
}

/*
// For checking to see if HPX works fine
static HPX_ACTION_DECL(_hello);
static int _hello_action(void)
{
  printf("Hello World from %u.\n", hpx_get_my_rank());
  hpx_exit(0, NULL);
}
*/

static HPX_ACTION_DECL(_free_vertices);
static int _free_vertices_handler(Vertex *v_, int id)
{

  hpx_addr_t local = hpx_thread_current_target();
  Vertex *v = NULL;
  if (!hpx_gas_try_pin(local, (void **)&v))
    return HPX_RESEND;

  // clean the edges
  edge_bucket *temp;

  for (int i = 0; i < v->edges.buckets_count; i++)
  {
    // printf("vertex = %d, deleting i = %d\n", v->vertex_id, i);
    temp = v->edges.head;
    v->edges.head = v->edges.head->next;
    free(temp);
  }

  hpx_gas_unpin(local);
  return HPX_SUCCESS;
}

// Print result for a single vertex for sanity checks and validation
static HPX_ACTION_DECL(_print_distance_at);
static int __print_distance_at_handler(Vertex *v_, int id)
{
  hpx_addr_t local = hpx_thread_current_target();
  Vertex *v = NULL;
  if (!hpx_gas_try_pin(local, (void **)&v))
    return HPX_RESEND;

  printf("v->vertex_id = %d, v->distance_from_start = %d\n", v->vertex_id, v->distance_from_start);

  hpx_gas_unpin(local);
  return HPX_SUCCESS;
}

static HPX_ACTION_DECL(_init_count_vertices);
static int _init_count_vertices_handler(Vertex *v_, int id)
{

  // this pin thingy will be need in distributed setting
  // printf("pop handler neigh = %d\n", zero);

  hpx_addr_t local = hpx_thread_current_target();
  Vertex *v = NULL;
  if (!hpx_gas_try_pin(local, (void **)&v))
    return HPX_RESEND;

  //  printf("pop handler neigh = %d\n", zero);

  //  printf("v->count = %d\n", v->count);

  v->vertex_id = id;
  // v->count = 0;
  v->parent = UNKNOWN;
  v->distance_from_start = 0;
  v->first_edge = UNKNOWN; // for Dijkstra–Scholten termination detection
  v->deficit = 0;          // for Dijkstra–Scholten termination detection

  // initialize the edge_list

  edge_bucket *temp = malloc(sizeof(edge_bucket));
  if (temp == NULL)
  {
    printf("Vertex: %d, Error: can't allocate edge_bucket\n", v->vertex_id);
    hpx_gas_unpin(local);
    return HPX_ERROR;
  }

  // temp->count = 0;
  // printf("initializing vertex: %d\n", v->vertex_id);
  // the edge_list
  v->edges.buckets_count = 1;
  v->edges.head = temp;
  v->edges.tail = temp;

  v->edges.head->count = 0;
  v->edges.head->next = NULL;

  // printf("initializing vertex: %d : done with pointers\n", v->vertex_id);

  v->vertex_local_mutex = hpx_lco_sema_new(1);

  // for performance statistics
  v->actions_invoked = 0;
  v->reduced_int_buffer = 0;
  v->total_time_in_actions = 0;
  v->reduced_time_buffer = 0;

  hpx_gas_unpin(local);

  return HPX_SUCCESS;
}

// TODO write a cleanup handler too that frees vertex internal data like:
//      vertex_local_mutex and the edges linklist

static HPX_ACTION_DECL(_send_edge);
static int _send_edge_handler(Vertex *v_, int edge, int weight)
{

  hpx_addr_t local = hpx_thread_current_target();
  Vertex *v = NULL;
  if (!hpx_gas_try_pin(local, (void **)&v))
    return HPX_RESEND;

  hpx_lco_sema_p(v->vertex_local_mutex);

  // printf("vertex: %d, inserting %d v->edges.tail->count = %d \n", v->vertex_id, edge, v->edges.tail->count);
  if (v->edges.tail->count == EDGES_BUFF_SIZE)
  { // if this bucket is full

    // allocate a new one
    edge_bucket *temp = malloc(sizeof(edge_bucket));
    if (temp == NULL)
    {
      printf("Vertex: %d, Error: can't allocate edge_bucket\n", v->vertex_id);
      hpx_gas_unpin(local);
      return HPX_ERROR;
    }

    temp->count = 0;
    temp->next = NULL;

    v->edges.tail->next = temp;
    v->edges.tail = temp;
    v->edges.buckets_count++;
  }

  /*
    if(v->count >= EDGES_BUFF_SIZE){
      printf("Vertex ID: %d, EDGES_BUFF_SIZE exeeded, implement linklist! Not adding edge %d, count = %d\n",v->vertex_id, edge, v->count);
      hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
      unpin here
      return HPX_SUCCESS;
    }

  */

  v->edges.tail->edges[v->edges.tail->count].node_id = edge;
  v->edges.tail->edges[v->edges.tail->count].weight = weight;
  v->edges.tail->count++;
  // printf("vertex: %d, edge %d inserted as %d\n", v->vertex_id, edge, v->edges.tail->edges[v->edges.tail->count - 1]);
  hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);

  hpx_gas_unpin(local);
  return HPX_SUCCESS;
}

// TODO: make this return int
// call/fork the _init_count_vertices
void init_count_vertices(hpx_addr_t vertices, int N)
{
  hpx_addr_t done = hpx_lco_and_new(N);
  for (int i = 0; i < N; i++)
  {
    // printf("i = %d\n", i);
    hpx_addr_t vertex_ = vertex_element_at(vertices, i);
    // printf("vertex_ = %ld\n", vertex_);
    hpx_call(vertex_, _init_count_vertices, done, &i);
    // printf("i = %d done\n", i);
  }

  hpx_lco_wait(done);
  hpx_lco_delete(done, HPX_NULL);

  return;
}

void print_distance_at(hpx_addr_t vertices, int target_vertex)
{
  hpx_addr_t done = hpx_lco_and_new(1);

  hpx_addr_t vertex_ = vertex_element_at(vertices, target_vertex);
  // printf("vertex_ = %ld\n", vertex_);
  hpx_call(vertex_, _print_distance_at, done, &target_vertex);

  hpx_lco_wait(done);
  hpx_lco_delete(done, HPX_NULL);

  return;
}

void FREE_vertices_internal(hpx_addr_t vertices, int N)
{
  hpx_addr_t done = hpx_lco_and_new(N);
  for (int i = 0; i < N; i++)
  {
    hpx_addr_t vertex_ = vertex_element_at(vertices, i);
    hpx_call(vertex_, _free_vertices, done, &i);
  }

  hpx_lco_wait(done);
  hpx_lco_delete(done, HPX_NULL);

  return;
}


// Read the input graph from file and populate the vertices
void populate_graph(hpx_addr_t vertices, int N, int N_edges, FILE *f)
{

  int vert_from, vert_to, weight;
  vert_from = -1;
  vert_to = -1;
  weight = -1;

  const int edges_chunk_size = 4000;

  int total_sending_chunks = N_edges / edges_chunk_size;
  int left_over_sending_chunks = N_edges % edges_chunk_size;
  if (left_over_sending_chunks != 0)
  {
    total_sending_chunks++;
  }
 printf("edges_chunk_size: %d, total_sending_chunks: %d, left_over_sending_chunks: %d\n", edges_chunk_size, total_sending_chunks, left_over_sending_chunks);
  for (int i = 0; i < total_sending_chunks; i++)
  {
    int LCO_size = edges_chunk_size;
    if ((i == total_sending_chunks - 1) && left_over_sending_chunks)
    {
      LCO_size = left_over_sending_chunks;
    }
    printf("LCO_size: %d\n", LCO_size);
    hpx_addr_t done = hpx_lco_and_new(LCO_size);
    for (int j = 0; j < LCO_size; j++)
    {

      fscanf(f, "%d\t%d\t%d", &vert_from, &vert_to, &weight);

      if (graph500_gen)
      {
        vert_from--;
        vert_to--; // because Graph500 octave generator indexes from 1
      }

      if (vert_from == vert_to)
      {
        printf("Cycle Detected! %d\n", vert_from);
      }
      if (0 && vert_from == root_vertex_)
        printf("read %d --> %d\n", vert_from, vert_to);

      hpx_addr_t vertex_ = vertex_element_at(vertices, vert_from);
      hpx_call(vertex_, _send_edge, done, &vert_to, &weight);
    }
    hpx_lco_wait(done);
    hpx_lco_delete(done, HPX_NULL);
  }

  return;
}

static HPX_ACTION_DECL(_ack);
static int _ack_handler(Vertex *v_, hpx_addr_t vertices, hpx_addr_t sssp_halting_LCO)
{

  hpx_addr_t local = hpx_thread_current_target();
  Vertex *v = NULL;
  if (!hpx_gas_try_pin(local, (void **)&v))
    return HPX_RESEND;

  hpx_lco_sema_p(v->vertex_local_mutex);

  v->deficit--;

  // Debug
  if (0 && v->vertex_id == root_vertex_)
    printf("_ack_handler: vertex %d deficit = %d\n", v->vertex_id, v->deficit);

  if (v->deficit == 0 && v->parent == ROOT)
  {
    hpx_lco_and_set(sssp_halting_LCO, HPX_NULL);
  }
  else if (v->deficit == 0)
  {

    if (0 && v->first_edge == root_vertex_)
      printf("_ack_handler: vertex %d, HALTED: parent is %d, distance = %d, sending to %d\n",
             v->vertex_id, v->parent, v->distance_from_start, v->first_edge);

    // printf("ack: v->first_edge %d\n",v->first_edge);
    hpx_addr_t vertex_ = vertex_element_at(vertices, v->first_edge);
    v->first_edge = UNKNOWN;
    hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &sssp_halting_LCO);

    // this is just for performance
    v->actions_invoked++;
    // DEBUGGING: Printing the result, later write a reducer TODO:
    // printf("vertex %d actions invoked is %d\n",v->vertex_id, v->actions_invoked);
  }

  hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);

  hpx_gas_unpin(local);
  return HPX_SUCCESS;
}

static HPX_ACTION_DECL(_reduce_actions_ack);
static int _reduce_actions_ack_handler(Vertex *v_, hpx_addr_t vertices,
                                       int partial_reduce, double partial_time, hpx_addr_t reduce_halting_LCO)
{

  hpx_addr_t local = hpx_thread_current_target();
  Vertex *v = NULL;
  if (!hpx_gas_try_pin(local, (void **)&v))
    return HPX_RESEND;

  hpx_lco_sema_p(v->vertex_local_mutex);

  v->deficit--;
  v->reduced_int_buffer += partial_reduce;
  v->reduced_time_buffer += partial_time;

  // printf("ack: vertex %d deficit = %d\n", v->vertex_id, v->deficit);

  if (v->deficit == 0 && v->parent == ROOT)
  {
    // v->reduce_int_buffer += v->actions_invoked;
    v->reduced_int_buffer += v->actions_invoked;
    // v->reduced_time_buffer += (v->total_time_in_actions / v->actions_invoked) * 2.0;
    v->total_time_in_actions = 1; // hack
    v->reduced_time_buffer += v->total_time_in_actions;
    printf("vertex %d reduced actions invoked is %d, sum of action time is %lf\n",
           v->vertex_id, v->reduced_int_buffer, v->reduced_time_buffer);

    v->reduced_int_buffer = 0;
    v->actions_invoked = 0;
    v->reduced_time_buffer = 0;
    v->total_time_in_actions = 0;

    hpx_lco_and_set(reduce_halting_LCO, HPX_NULL);
  }
  else if (v->deficit == 0)
  {

    // DEBUGGING: Printing the result, later write a reducer TODO:
    // printf("vertex %d parent is %d\n",v->vertex_id, v->parent);

    v->reduced_int_buffer += v->actions_invoked;
    // v->reduced_time_buffer += (v->total_time_in_actions / v->actions_invoked) * 2;
    v->total_time_in_actions = 1; // hack
    v->reduced_time_buffer += v->total_time_in_actions;
    hpx_addr_t vertex_ = vertex_element_at(vertices, v->first_edge);
    v->first_edge = UNKNOWN;
    hpx_call_async(vertex_, _reduce_actions_ack, HPX_NULL, HPX_NULL, &vertices,
                   &v->reduced_int_buffer, &v->reduced_time_buffer, &reduce_halting_LCO);

    /*
    if(v->actions_invoked != 0)
    printf("vertex %d actions invoked is %d, v->reduced_int_buffer = %d\n",v->vertex_id, v->actions_invoked, v->reduced_int_buffer);
    */
    v->reduced_int_buffer = 0;
    v->actions_invoked = 0;
    v->reduced_time_buffer = 0;
    v->total_time_in_actions = 0;
  }

  hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);

  hpx_gas_unpin(local);
  return HPX_SUCCESS;
}

// To understand this reducer you need to intuitively think of reducing the spaning tree
// Therefore this function is asynchronously recurssive
static HPX_ACTION_DECL(_reduce_acks);
static int _reduce_acks_handler(Vertex *v_, hpx_addr_t vertices,
                                int sender_vertex,
                                hpx_addr_t reduce_halting_LCO)
{

  hpx_addr_t local = hpx_thread_current_target();
  Vertex *v = NULL;
  if (!hpx_gas_try_pin(local, (void **)&v))
    return HPX_RESEND;

  hpx_lco_sema_p(v->vertex_local_mutex);

  if (sender_vertex == ROOT && v->edges.head->count > 0)
  {

    edge_bucket *cursor = v->edges.head;
    // diffuse
    for (int i = 0; i < v->edges.buckets_count; i++)
    {
      for (int j = 0; j < cursor->count; j++)
      {

        hpx_addr_t vertex_ = vertex_element_at(vertices, cursor->edges[j].node_id);
        hpx_call_async(vertex_, _reduce_acks, HPX_NULL, HPX_NULL,
                       &vertices, &v->vertex_id, &reduce_halting_LCO);
        v->deficit++;
      }
      cursor = cursor->next;
    }

    hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);

    hpx_gas_unpin(local);
    return HPX_SUCCESS;
  }

  if (sender_vertex == ROOT && v->edges.head->count == 0)
  {

    printf("vertex %d reduced actions invocked is %d\n", v->vertex_id, v->reduced_int_buffer);

    hpx_lco_and_set(reduce_halting_LCO, HPX_NULL);

    hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
    hpx_gas_unpin(local);
    return HPX_SUCCESS;
  }

  if (v->first_edge == UNKNOWN && sender_vertex != ROOT)
  {

    v->first_edge = sender_vertex;

    edge_bucket *cursor = v->edges.head;
    // diffuse
    for (int i = 0; i < v->edges.buckets_count; i++)
    {
      for (int j = 0; j < cursor->count; j++)
      {

        hpx_addr_t vertex_ = vertex_element_at(vertices, cursor->edges[j].node_id);
        hpx_call_async(vertex_, _reduce_acks, HPX_NULL, HPX_NULL,
                       &vertices, &v->vertex_id, &reduce_halting_LCO);
        v->deficit++;
      }
      cursor = cursor->next;
    }

    cursor = v->edges.head;
    if (cursor->count == 0)
    {
      v->first_edge = UNKNOWN;
      // just send the ack
      int partial_reduce = v->actions_invoked;
      // double time_per_action = (v->total_time_in_actions / partial_reduce) * 2;
      v->total_time_in_actions = 1; // hack
      double time_per_action = v->total_time_in_actions;
      hpx_addr_t vertex_ = vertex_element_at(vertices, sender_vertex);
      hpx_call_async(vertex_, _reduce_actions_ack, HPX_NULL, HPX_NULL,
                     &vertices, &partial_reduce, &time_per_action, &reduce_halting_LCO);
    }

    hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);

    hpx_gas_unpin(local);
    return HPX_SUCCESS;
  }
  else
  {

    // just send the ack
    int partial_reduce = 0;
    double time_per_action = 0;
    hpx_addr_t vertex_ = vertex_element_at(vertices, sender_vertex);
    hpx_call_async(vertex_, _reduce_actions_ack, HPX_NULL, HPX_NULL,
                   &vertices, &partial_reduce, &time_per_action, &reduce_halting_LCO);
  }

  hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);

  hpx_gas_unpin(local);
  return HPX_SUCCESS;
}

static HPX_ACTION_DECL(_sssp_hpx_async);
static int _sssp_hpx_async_handler(Vertex *v_, hpx_addr_t vertices,
                                   int sender_vertex, int incoming_distance,
                                   hpx_addr_t sssp_halting_LCO)
{

  struct timeval end, begin;

  hpx_addr_t local = hpx_thread_current_target();
  Vertex *v = NULL;
  if (!hpx_gas_try_pin(local, (void **)&v))
    return HPX_RESEND;

  gettimeofday(&begin, 0);

  // there needs to be a vertex lco too as this is critical section
  // printf("SSSP: in _sssp_hpx_async_handler, vertex: = %d\n", v->vertex_id);
  hpx_lco_sema_p(v->vertex_local_mutex);

  // this is for the first activation of the vertex
  // or in case the vertex deficit becomes zero and it deactivates
  //  and tries to reactivate
  if (v->parent == UNKNOWN)
  {

    v->first_edge = sender_vertex; // this creates an implicit spanning tree for
                                   // Dijkstra–Scholten termination detection

    v->parent = sender_vertex;
    v->distance_from_start = incoming_distance;

    // printf("Vertex: %d: Active, First Edge is: %d\n", v->vertex_id, v->first_edge);
    // if(sender_vertex == ROOT)
    //   printf("In ROOT: sender_vertex = %d, distance = %d\n",sender_vertex, incoming_distance);

    // printf("SSSP: vertex: %d set it's parent as %d\n", v->vertex_id, v->parent);
    int distance = v->distance_from_start;

    /*
    for(int i=0; i<v->count; i++){
      //printf("SSSP: vertex: %d preparing edge: %d\n", v->vertex_id, v->edges[i]);
      hpx_addr_t vertex_ = vertex_element_at(vertices, v->edges[i]);
      hpx_call_async(vertex_, _sssp_hpx_async, HPX_NULL, HPX_NULL,
                     &vertices, &v->vertex_id, &distance, &sssp_halting_LCO);
      v->deficit ++;

      // this is just for performance
      v->actions_invoked ++;
    }
    */
    int new_distance = 0;
    edge_bucket *cursor = v->edges.head;
    // diffuse
    for (int i = 0; i < v->edges.buckets_count; i++)
    {
      for (int j = 0; j < cursor->count; j++)
      {
        new_distance = distance + cursor->edges[j].weight;
        hpx_addr_t vertex_ = vertex_element_at(vertices, cursor->edges[j].node_id);

        // Debug
        if (0 && v->vertex_id == root_vertex_)
          printf("vertex %d: sending to vertex %d\n", v->vertex_id, cursor->edges[j].node_id);

        hpx_call_async(vertex_, _sssp_hpx_async, HPX_NULL, HPX_NULL,
                       &vertices, &v->vertex_id, &new_distance, &sssp_halting_LCO);

        v->deficit++;

        // this is just for performance
        v->actions_invoked++;
      }
      cursor = cursor->next;
    }
    // printf("send for the firsst time ver %d\n", v->vertex_id);

    // printf("SSSP: vertex: %d setting sssp_halting_LCO\n", v->vertex_id);
    // hpx_lco_and_set(sssp_halting_LCO, HPX_NULL);

    cursor = v->edges.head;
    // what is the graph root is dirty such that the root has no neighbours
    //  take care of that case TODO: later
    if (cursor->count == 0)
    {
      // printf("root vertex has no neighbours \n");
      //  erase the first_edge and
      //  send ack back

      if (sender_vertex == ROOT)
      {
        hpx_lco_and_set(sssp_halting_LCO, HPX_NULL);

        gettimeofday(&end, 0);
        v->total_time_in_actions += (end.tv_sec - begin.tv_sec) +
                                    ((end.tv_usec - begin.tv_usec) / 1000000.0);

        hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
        hpx_gas_unpin(local);
        return HPX_SUCCESS;
      }
      else
      {
        hpx_addr_t vertex_ = vertex_element_at(vertices, v->first_edge);

        // if(sender_vertex == root_vertex_){
        // printf("Vertex: %d: Has no neighbours so just leaving: v->first_edge %d\n", v->vertex_id, v->first_edge);
        // }
        v->first_edge = UNKNOWN;
        hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &sssp_halting_LCO);

        // this is just for performance
        v->actions_invoked++;
      }
    }

    gettimeofday(&end, 0);
    v->total_time_in_actions += (end.tv_sec - begin.tv_sec) +
                                ((end.tv_usec - begin.tv_usec) / 1000000.0);

    hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
    // printf("SSSP: exiting _sssp_hpx_async_handler, vertex: = %d\n", v->vertex_id);
    hpx_gas_unpin(local);
    return HPX_SUCCESS;

    // note here that we are not ackoledging the message recipt right here
    // this will be done when v->deficit reaches zero and then send recipt to the
    // v->first_edge
  }
  else
  {

    if (v->distance_from_start > incoming_distance)
    {

      // update and diffuse/spread the update
      v->parent = sender_vertex;
      v->distance_from_start = incoming_distance;

      // send ack to the sender, if v->first_edge != UNKNOWN && v->count != 0

      if ((v->first_edge == UNKNOWN && v->edges.head->count == 0) || (v->first_edge != UNKNOWN && v->edges.head->count > 0))
      {

        // normal operation
        hpx_addr_t vertex_ = vertex_element_at(vertices, sender_vertex);

        if (0 && sender_vertex == root_vertex_)
        {
          printf("Normal ack: sending ack to %d from %d\n", sender_vertex, v->vertex_id);
        }

        hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &sssp_halting_LCO);

        // this is just for performance
        v->actions_invoked++;
      }

      if ((v->first_edge == UNKNOWN) && (v->edges.head->count > 0))
      {
        // TODO check if deficit is zero, it should be
        v->first_edge = sender_vertex;
        // printf("Vertex: %d: Active, first Edge: %d\n", v->vertex_id, v->first_edge);
      }

      int distance = v->distance_from_start;
      /*
      // diffuse
      for(int i=0; i<v->count; i++){
        //printf("SSSP: vertex: %d preparing edge: %d\n", v->vertex_id, v->edges[i]);
        hpx_addr_t vertex_ = vertex_element_at(vertices, v->edges[i]);
        hpx_call_async(vertex_, _sssp_hpx_async, HPX_NULL, HPX_NULL,
                     &vertices, &v->vertex_id, &distance, &sssp_halting_LCO);
        v->deficit ++;

        // this is just for performance
        v->actions_invoked ++;

      }
      */
      int new_distance = 0;
      edge_bucket *cursor = v->edges.head;
      // diffuse
      for (int i = 0; i < v->edges.buckets_count; i++)
      {
        for (int j = 0; j < cursor->count; j++)
        {
          new_distance = distance + cursor->edges[j].weight;
          hpx_addr_t vertex_ = vertex_element_at(vertices, cursor->edges[j].node_id);
          hpx_call_async(vertex_, _sssp_hpx_async, HPX_NULL, HPX_NULL,
                         &vertices, &v->vertex_id, &new_distance, &sssp_halting_LCO);

          v->deficit++;

          // this is just for performance
          v->actions_invoked++;
        }
        cursor = cursor->next;
      }
    }
    else
    {

      if (0 && sender_vertex == root_vertex_)
      {
        printf("Stable ack: to: %d from Vertex: %d: Distance: %d \n", sender_vertex,
               v->vertex_id, v->distance_from_start);
      }
      // just ack to the sender
      hpx_addr_t vertex_ = vertex_element_at(vertices, sender_vertex);
      hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &sssp_halting_LCO);

      // this is just for performance statistics
      v->actions_invoked++;
    }
  }

  gettimeofday(&end, 0);
  v->total_time_in_actions += (end.tv_sec - begin.tv_sec) +
                              ((end.tv_usec - begin.tv_usec) / 1000000.0);

  hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
  // printf("SSSP: exiting _sssp_hpx_async_handler, vertex: = %d\n", v->vertex_id);
  hpx_gas_unpin(local);
  return HPX_SUCCESS;
}

void SSSP_hpx_async(hpx_addr_t vertices, int start_vertex,
                    hpx_addr_t sssp_done_lco)
{

  hpx_addr_t vertex_ = vertex_element_at(vertices, start_vertex);
  int root_parent = ROOT;
  int distance = 0;
  hpx_call_async(vertex_, _sssp_hpx_async, HPX_NULL, HPX_NULL,
                 &vertices, &root_parent, &distance, &sssp_done_lco);
}

void REDUCE_actions_invoked_async(hpx_addr_t vertices, int start_vertex,
                                  hpx_addr_t reduce_done_lco)
{

  hpx_addr_t vertex_ = vertex_element_at(vertices, start_vertex);
  int root_parent = ROOT;

  hpx_call_async(vertex_, _reduce_acks, HPX_NULL, HPX_NULL,
                 &vertices, &root_parent, &reduce_done_lco);
}

static HPX_ACTION_DECL(_sssp);
static int _sssp_handler(void)
{
  FILE *f = NULL;
  if ((f = fopen(filename_, "r")) == NULL)
    return -1;

  int N = 0;
  int N_ = 0;
  int N_edges = 0;

  fscanf(f, "%d\t%d", &N, &N_);
  fscanf(f, "%d", &N_edges);

  printf("Graph: %s, graph vertex count is %d with %d egdes.\n",
         filename_, N, N_edges);

  hpx_addr_t vertices = hpx_gas_calloc_cyclic(N, sizeof(Vertex), 0);
  // this is the main hpx sssp code
  assert(vertices != HPX_NULL);
  printf("Vertices allocation is done\n");

  // Populate the graph
  // intialize the counts of Vertex struct
  init_count_vertices(vertices, N);

  populate_graph(vertices, N, N_edges, f);
  fclose(f);

  printf("Graph has been initialied from the input file\n");

  // Call the Asynchronous SSSP here
  hpx_addr_t sssp_done_lco = hpx_lco_and_new(1);

  struct timeval end, begin;
  gettimeofday(&begin, 0);

  int start_vertex = root_vertex_;
  SSSP_hpx_async(vertices, start_vertex, sssp_done_lco);

  hpx_lco_wait(sssp_done_lco);
  hpx_lco_delete(sssp_done_lco, HPX_NULL);

  gettimeofday(&end, 0);
  printf("Asynchronous SSSP using HPX-5 finished\n");

  // timing
  double elapsed = (end.tv_sec - begin.tv_sec) +
                   ((end.tv_usec - begin.tv_usec) / 1000000.0);

  printf("Time taken in Asynchronous SSSP (secs): %lf\n", elapsed);

  /* Validation:
  Print the sssp value at a known target_vertex for the input graph
  Manually check whether it aligns with the result using python networkx
  Yes, it is working, so commenting it out.
  */

   int target_vertex = 47;
   print_distance_at(vertices, target_vertex);

  // reduce the results, in this case the number of actions invoked
  hpx_addr_t reduce_done_lco = hpx_lco_and_new(1);
  REDUCE_actions_invoked_async(vertices, start_vertex, reduce_done_lco);
  hpx_lco_wait(reduce_done_lco);
  hpx_lco_delete(reduce_done_lco, HPX_NULL);

  printf("Input Graph: %s\n", filename_);
  printf("HPX Processes: %d\n", hpx_get_num_ranks());


  printf("Freeing memory, vertices\n");
  FREE_vertices_internal(vertices, N);
  hpx_gas_free(vertices, HPX_NULL);

  printf("All done! Goodbye.\n");
  hpx_exit(0, NULL);
}

// static HPX_ACTION(HPX_DEFAULT, 0, _hello, _hello_action);

static HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _print_distance_at,
                  __print_distance_at_handler, HPX_POINTER, HPX_INT);

static HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _init_count_vertices,
                  _init_count_vertices_handler, HPX_POINTER, HPX_INT);

static HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _free_vertices,
                  _free_vertices_handler, HPX_POINTER, HPX_INT);

static HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _send_edge, _send_edge_handler,
                  HPX_POINTER, HPX_INT, HPX_INT);

static HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _ack, _ack_handler,
                  HPX_POINTER, HPX_ADDR, HPX_ADDR);

static HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _reduce_actions_ack, _reduce_actions_ack_handler,
                  HPX_POINTER, HPX_ADDR, HPX_INT, HPX_DOUBLE, HPX_ADDR);

static HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _reduce_acks, _reduce_acks_handler,
                  HPX_POINTER, HPX_ADDR, HPX_INT, HPX_ADDR);

static HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _sssp_hpx_async, _sssp_hpx_async_handler,
                  HPX_POINTER, HPX_ADDR, HPX_INT, HPX_INT, HPX_ADDR);

// main action
static HPX_ACTION(HPX_DEFAULT, 0, _sssp, _sssp_handler);

int main(int argc, char *argv[argc])
{

  filename_ = argv[1];
  root_vertex_ = atoi(argv[2]);
  graph500_gen = atoi(argv[3]);
  // strcpy(filename_, argv[1]);
  // printf("%s -- filename\n", filename_);
  if (hpx_init(&argc, &argv) != 0)
    return -1;
  int e = hpx_run(&_sssp, NULL);
  hpx_finalize();
  return e;
}
