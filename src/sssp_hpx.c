/*
2018-2023
Asnychronous Single Source Shortest Path (SSSP) using HPX-5.

Author: Bibrak Qamar Chandio
Copyright

Work performed at Indiana University Bloomington for 
PhD in Intelligent Systems Engineering

*/

#include <hpx/hpx.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

// for sleep
#include <unistd.h>

#define EDGES_BUFF_SIZE 5000
#define UNKNOWN -3
#define ROOT -1

char* filename_;
int root_vertex_;
int graph500_gen;
int iterations = 0;

int N = 0;
int N_ = 0;
int N_edges = 0;

// stores the actions invoked by the vertices of a HPX_LOCALITY (or rank)
// later will be used in the reduction to rank 0
// This is just for the performance analysis
static int accumulate_actions;

typedef struct
{
    int node_id;
    int weight;

} edge_t;

struct edge_bucket
{

    int count;
    edge_t edges[EDGES_BUFF_SIZE];
    struct edge_bucket* next;

}; // the node

typedef struct edge_bucket edge_bucket;

typedef struct
{

    int buckets_count;
    edge_bucket* head; // head of the linked list
    edge_bucket* tail;

} edge_list;

typedef struct
{

    // int edges[EDGES_BUFF_SIZE];

    edge_list edges;

    int vertex_id;

    // the result in this case parent for SSSP
    int parent;
    int distance_from_start;

    // Dijkstra–Scholten termination detection
    int first_edge;
    int deficit;

    // control related
    hpx_addr_t vertex_local_mutex;

    // for performance statistics
    int actions_invoked;
    int reduced_int_buffer;
    double total_time_in_actions;
    double reduced_time_buffer;

} Vertex;

// Get the global address of vertex at ith location
hpx_addr_t
vertex_element_at(hpx_addr_t vert, int i)
{
    return hpx_addr_add(vert, sizeof(Vertex) * i, sizeof(Vertex));
}

HPX_ACTION_DECL(_free_vertices);
int
_free_vertices_handler(Vertex* v_, int id)
{

    hpx_addr_t local = hpx_thread_current_target();
    Vertex* v = NULL;
    if (!hpx_gas_try_pin(local, (void**)&v))
        return HPX_RESEND;

    // clean the edges
    edge_bucket* temp;

    for (int i = 0; i < v->edges.buckets_count; i++) {
        temp = v->edges.head;
        v->edges.head = v->edges.head->next;
        free(temp);
    }

    hpx_gas_unpin(local);
    return HPX_SUCCESS;
}

// Print result for a single vertex for sanity checks and validation
HPX_ACTION_DECL(_print_distance_at);
int
__print_distance_at_handler(Vertex* v_)
{
    hpx_addr_t local = hpx_thread_current_target();
    Vertex* v = NULL;
    if (!hpx_gas_try_pin(local, (void**)&v))
        return HPX_RESEND;

    printf(
        "v->vertex_id = %d, v->distance_from_start = %d\n", v->vertex_id, v->distance_from_start);
    fflush(stdout);

    hpx_gas_unpin(local);
    return HPX_SUCCESS;
}

// Dispatch this to underlying ranks to speedup
HPX_ACTION_DECL(_reset_vertices_dispatch);
int
_reset_vertices_dispatch_handler(int* hook, hpx_addr_t vertices, int N)
{

    int start_vertex = hpx_get_my_rank();
    for (int i = start_vertex; i < N; i += hpx_get_num_ranks()) {
        hpx_addr_t vertex_ = vertex_element_at(vertices, i);
        hpx_addr_t local = vertex_; // hpx_thread_current_target();
        Vertex* v = NULL;
        if (!hpx_gas_try_pin(local, (void**)&v)) {
            printf("hpx_gas_try_pin failed! vertex = %d\n", i);
            continue;
        }

        v->parent = UNKNOWN;
        v->distance_from_start = 0;
        v->first_edge = UNKNOWN; // for Dijkstra–Scholten termination detection
        v->deficit = 0;          // for Dijkstra–Scholten termination detection

        // for performance statistics
        v->actions_invoked = 0;
        v->reduced_int_buffer = 0;
        v->total_time_in_actions = 0;
        v->reduced_time_buffer = 0;

        hpx_gas_unpin(local);
    }
    // printf("rank: %d | accumulate_actions: %d\n", hpx_get_my_rank(), accumulate_actions);
    return HPX_SUCCESS;
}

void
reset_vertices(hpx_addr_t hook, hpx_addr_t vertices, int N)
{
    hpx_addr_t done = hpx_lco_and_new(hpx_get_num_ranks());
    for (int i = 0; i < hpx_get_num_ranks(); i++) {
        hpx_addr_t hpx_process_ = hpx_addr_add(hook, sizeof(int) * i, sizeof(int));
        hpx_call(hpx_process_, _reset_vertices_dispatch, done, &vertices, &N);
    }

    hpx_lco_wait(done);
    hpx_lco_delete(done, HPX_NULL);

    return;
}

// Dispatch this to underlying ranks to speedup
HPX_ACTION_DECL(_reduce_actions_v2_dispatch);
int
_reduce_actions_v2_dispatch_handler(int* hook, hpx_addr_t vertices, int N)
{

    int start_vertex = hpx_get_my_rank();
    accumulate_actions = 0;
    for (int i = start_vertex; i < N; i += hpx_get_num_ranks()) {
        hpx_addr_t vertex_ = vertex_element_at(vertices, i);
        hpx_addr_t local = vertex_; // hpx_thread_current_target();
        Vertex* v = NULL;
        if (!hpx_gas_try_pin(local, (void**)&v)) {
            printf("hpx_gas_try_pin failed! vertex = %d\n", i);
            continue;
        }
        // printf("rank: %d | v[%d]: ID = %d\n", hpx_get_my_rank(), i, v->vertex_id);
        // printf("rank: %d | v->vertex_id = %d, v->distance_from_start = %d\n", hpx_get_my_rank(),
        // v->vertex_id, v->distance_from_start);
        accumulate_actions += v->actions_invoked;
        hpx_gas_unpin(local);
    }
    // printf("rank: %d | accumulate_actions: %d\n", hpx_get_my_rank(), accumulate_actions);
    return HPX_SUCCESS;
}

static int
_sum(int count, int values[count])
{
    int total = 0;
    for (int i = 0; i < count; ++i) {
        total += values[i];
    }
    return total;
}

HPX_ACTION_DECL(_get_value);
static int
_get_value_handler(void)
{
    return HPX_THREAD_CONTINUE(accumulate_actions);
}

int
reduce_actions_v2(hpx_addr_t hook, hpx_addr_t vertices, int N)
{
    hpx_addr_t done = hpx_lco_and_new(hpx_get_num_ranks());
    for (int i = 0; i < hpx_get_num_ranks(); i++) {
        hpx_addr_t hpx_process_ = hpx_addr_add(hook, sizeof(int) * i, sizeof(int));
        hpx_call(hpx_process_, _reduce_actions_v2_dispatch, done, &vertices, &N);
    }

    hpx_lco_wait(done);
    hpx_lco_delete(done, HPX_NULL);

    int values[hpx_get_num_ranks()];
    void* addrs[hpx_get_num_ranks()];
    size_t sizes[hpx_get_num_ranks()];
    hpx_addr_t futures[hpx_get_num_ranks()];

    for (int i = 0; i < hpx_get_num_ranks(); ++i) {
        addrs[i] = &values[i];
        sizes[i] = sizeof(int);
        futures[i] = hpx_lco_future_new(sizeof(int));
        hpx_call(HPX_THERE(i), _get_value, futures[i]);
    }

    hpx_lco_get_all(hpx_get_num_ranks(), futures, sizes, addrs, NULL);

    int actions_invoked_total = _sum(hpx_get_num_ranks(), values);
    // printf("Actions Invoked_total: %d\n", actions_invoked_total);
    // fflush(stdout);

    for (int i = 0; i < hpx_get_num_ranks(); ++i) {
        hpx_lco_delete(futures[i], HPX_NULL);
    }

    return actions_invoked_total;
}

HPX_ACTION_DECL(_init_count_vertices);
int
_init_count_vertices_handler(Vertex* v_, int id)
{

    hpx_addr_t local = hpx_thread_current_target();
    Vertex* v = NULL;
    if (!hpx_gas_try_pin(local, (void**)&v))
        return HPX_RESEND;

    v->vertex_id = id;
    // v->count = 0;
    v->parent = UNKNOWN;
    v->distance_from_start = 0;
    v->first_edge = UNKNOWN; // for Dijkstra–Scholten termination detection
    v->deficit = 0;          // for Dijkstra–Scholten termination detection

    // initialize the edge_list

    edge_bucket* temp = malloc(sizeof(edge_bucket));
    if (temp == NULL) {
        printf("Vertex: %d, Error: can't allocate edge_bucket\n", v->vertex_id);
        hpx_gas_unpin(local);
        return HPX_ERROR;
    }

    // the edge_list
    v->edges.buckets_count = 1;
    v->edges.head = temp;
    v->edges.tail = temp;

    v->edges.head->count = 0;
    v->edges.head->next = NULL;

    v->vertex_local_mutex = hpx_lco_sema_new(1);

    // for performance statistics
    v->actions_invoked = 0;
    v->reduced_int_buffer = 0;
    v->total_time_in_actions = 0;
    v->reduced_time_buffer = 0;

    hpx_gas_unpin(local);

    return HPX_SUCCESS;
}

HPX_ACTION_DECL(_send_edge);
int
_send_edge_handler(Vertex* v_, int edge, int weight)
{

    hpx_addr_t local = hpx_thread_current_target();
    Vertex* v = NULL;
    if (!hpx_gas_try_pin(local, (void**)&v))
        return HPX_RESEND;

    hpx_lco_sema_p(v->vertex_local_mutex);

    if (v->edges.tail->count == EDGES_BUFF_SIZE) { // if this bucket is full

        // allocate a new one
        edge_bucket* temp = malloc(sizeof(edge_bucket));
        if (temp == NULL) {
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

    v->edges.tail->edges[v->edges.tail->count].node_id = edge;
    v->edges.tail->edges[v->edges.tail->count].weight = weight;
    v->edges.tail->count++;
    hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);

    hpx_gas_unpin(local);
    return HPX_SUCCESS;
}

// Dispatch this to underlying ranks to speedup
HPX_ACTION_DECL(_init_count_vertices_dispatch);
int
_init_count_vertices_dispatch_handler(int* hook, hpx_addr_t vertices, int N)
{
    int chunk = N / hpx_get_num_ranks();
    int my_chunk = chunk;
    if (hpx_get_num_ranks() == (hpx_get_my_rank() + 1)) {
        my_chunk += N % hpx_get_num_ranks();
    }

    int start_vertex = chunk * hpx_get_my_rank();

    hpx_addr_t done = hpx_lco_and_new(my_chunk);

    for (int i = start_vertex; i < start_vertex + my_chunk; i++) {
        hpx_addr_t vertex_ = vertex_element_at(vertices, i);
        hpx_call(vertex_, _init_count_vertices, done, &i);
    }

    hpx_lco_wait(done);
    hpx_lco_delete(done, HPX_NULL);

    return HPX_SUCCESS;
}

void
init_count_vertices(hpx_addr_t hook, hpx_addr_t vertices, int N)
{
    hpx_addr_t done = hpx_lco_and_new(hpx_get_num_ranks());
    for (int i = 0; i < hpx_get_num_ranks(); i++) {
        hpx_addr_t hpx_process_ = hpx_addr_add(hook, sizeof(int) * i, sizeof(int));
        hpx_call(hpx_process_, _init_count_vertices_dispatch, done, &vertices, &N);
    }

    hpx_lco_wait(done);
    hpx_lco_delete(done, HPX_NULL);

    return;
}

void
print_vertex_info_all(hpx_addr_t vertices)
{
    hpx_addr_t done = hpx_lco_and_new(N);

    for (int i = 0; i < N; i++) {
        hpx_addr_t vertex_ = vertex_element_at(vertices, i);
        hpx_call(vertex_, _print_distance_at, done);
    }
    hpx_lco_wait(done);
    hpx_lco_delete(done, HPX_NULL);

    return;
}

void
print_distance_at(hpx_addr_t vertices, int target_vertex)
{
    hpx_addr_t done = hpx_lco_and_new(1);

    hpx_addr_t vertex_ = vertex_element_at(vertices, target_vertex);
    hpx_call(vertex_, _print_distance_at, done, &target_vertex);

    hpx_lco_wait(done);
    hpx_lco_delete(done, HPX_NULL);

    return;
}

void
FREE_vertices_internal(hpx_addr_t vertices, int N)
{
    hpx_addr_t done = hpx_lco_and_new(N);
    for (int i = 0; i < N; i++) {
        hpx_addr_t vertex_ = vertex_element_at(vertices, i);
        hpx_call(vertex_, _free_vertices, done, &i);
    }

    hpx_lco_wait(done);
    hpx_lco_delete(done, HPX_NULL);

    return;
}

// Read the input graph from file and populate the vertices
HPX_ACTION_DECL(_populate_graph_dispatch);
int
_populate_graph_dispatch_handler(int* hook, hpx_addr_t vertices)
{

    int vert_from, vert_to, weight;
    vert_from = -1;
    vert_to = -1;
    weight = -1;

    const int edges_chunk_size = 300;

    // open file
    FILE* f = NULL;
    if ((f = fopen(filename_, "rb")) == NULL) {
        printf("File reading failed!\n");
        return -1;
    }

    fread(&N, sizeof(int), 1, f);
    fread(&N_, sizeof(int), 1, f);
    fread(&N_edges, sizeof(int), 1, f);

    int edges_chunk = N_edges / hpx_get_num_ranks();
    int my_edges_chunk = edges_chunk;
    if (hpx_get_num_ranks() == (hpx_get_my_rank() + 1)) {
        my_edges_chunk += N_edges % hpx_get_num_ranks();
    }
    int my_starting_edge = edges_chunk * hpx_get_my_rank();

    // seek to my edge chunk
    int edge_size_in_bytes = sizeof(int) * 3; // ints of: to, from, weight
    fseek(f, edge_size_in_bytes * my_starting_edge, SEEK_CUR);

    // staging the sending in chunks to not let the network saturate and cause faults
    int total_sending_chunks = my_edges_chunk / edges_chunk_size;
    int left_over_sending_chunks = my_edges_chunk % edges_chunk_size;
    if (left_over_sending_chunks != 0) {
        total_sending_chunks++;
    }

    for (int i = 0; i < total_sending_chunks; i++) {
        int LCO_size = edges_chunk_size;
        if ((i == total_sending_chunks - 1) && left_over_sending_chunks) {
            LCO_size = left_over_sending_chunks;
        }
        hpx_addr_t done = hpx_lco_and_new(LCO_size);
        for (int j = 0; j < LCO_size; j++) {
            fread(&vert_from, sizeof(int), 1, f);
            fread(&vert_to, sizeof(int), 1, f);
            fread(&weight, sizeof(int), 1, f);

            if (graph500_gen) {
                vert_from--;
                vert_to--; // because Graph500 octave generator indexes from 1
            }

            if (vert_from == vert_to) {
                printf("Cycle Detected! %d\n", vert_from);
                fflush(stdout);
            }

            hpx_addr_t vertex_ = vertex_element_at(vertices, vert_from);
            hpx_call(vertex_, _send_edge, done, &vert_to, &weight);
        }
        hpx_lco_wait(done);
        hpx_lco_delete(done, HPX_NULL);
    }

    // close the file
    fclose(f);
    return HPX_SUCCESS;
}

void
populate_graph(hpx_addr_t hook, hpx_addr_t vertices)
{
    hpx_addr_t done = hpx_lco_and_new(hpx_get_num_ranks());
    for (int i = 0; i < hpx_get_num_ranks(); i++) {
        hpx_addr_t hpx_process_ = hpx_addr_add(hook, sizeof(int) * i, sizeof(int));
        hpx_call(hpx_process_, _populate_graph_dispatch, done, &vertices);
    }

    hpx_lco_wait(done);
    hpx_lco_delete(done, HPX_NULL);

    return;
}

HPX_ACTION_DECL(_ack);
int
_ack_handler(Vertex* v_, hpx_addr_t vertices, hpx_addr_t sssp_halting_LCO)
{

    hpx_addr_t local = hpx_thread_current_target();
    Vertex* v = NULL;
    if (!hpx_gas_try_pin(local, (void**)&v))
        return HPX_RESEND;

    hpx_lco_sema_p(v->vertex_local_mutex);

    v->deficit--;

    // Debug
    if (0 && v->vertex_id == root_vertex_)
        printf("_ack_handler: vertex %d deficit = %d\n", v->vertex_id, v->deficit);

    if (v->deficit == 0 && v->parent == ROOT) {
        hpx_lco_and_set(sssp_halting_LCO, HPX_NULL);
    } else if (v->deficit == 0) {

        if (0 && v->first_edge == root_vertex_)
            printf("_ack_handler: vertex %d, HALTED: parent is %d, distance = %d, sending to %d\n",
                   v->vertex_id,
                   v->parent,
                   v->distance_from_start,
                   v->first_edge);

        hpx_addr_t vertex_ = vertex_element_at(vertices, v->first_edge);
        v->first_edge = UNKNOWN;
        hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &sssp_halting_LCO);

        // this is just for performance
        v->actions_invoked++;
    }

    hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);

    hpx_gas_unpin(local);
    return HPX_SUCCESS;
}

HPX_ACTION_DECL(_sssp_hpx_async);
int
_sssp_hpx_async_handler(Vertex* v_,
                        hpx_addr_t vertices,
                        int sender_vertex,
                        int incoming_distance,
                        hpx_addr_t sssp_halting_LCO)
{

    struct timeval end, begin;

    hpx_addr_t local = hpx_thread_current_target();
    Vertex* v = NULL;
    if (!hpx_gas_try_pin(local, (void**)&v))
        return HPX_RESEND;

    gettimeofday(&begin, 0);

    // there needs to be a vertex lco too as this is critical section
    // printf("SSSP: in _sssp_hpx_async_handler, vertex: = %d\n", v->vertex_id);
    hpx_lco_sema_p(v->vertex_local_mutex);

    // this is for the first activation of the vertex
    // or in case the vertex deficit becomes zero and it deactivates
    //  and tries to reactivate
    if (v->parent == UNKNOWN) {

        v->first_edge = sender_vertex; // this creates an implicit spanning tree for
                                       // Dijkstra–Scholten termination detection

        v->parent = sender_vertex;
        v->distance_from_start = incoming_distance;

        int distance = v->distance_from_start;

        int new_distance = 0;
        edge_bucket* cursor = v->edges.head;
        // diffuse
        for (int i = 0; i < v->edges.buckets_count; i++) {
            for (int j = 0; j < cursor->count; j++) {
                new_distance = distance + cursor->edges[j].weight;
                hpx_addr_t vertex_ = vertex_element_at(vertices, cursor->edges[j].node_id);

                // Debug
                if (0 && v->vertex_id == root_vertex_)
                    printf("vertex %d: sending to vertex %d\n",
                           v->vertex_id,
                           cursor->edges[j].node_id);

                hpx_call_async(vertex_,
                               _sssp_hpx_async,
                               HPX_NULL,
                               HPX_NULL,
                               &vertices,
                               &v->vertex_id,
                               &new_distance,
                               &sssp_halting_LCO);

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
        if (cursor->count == 0) {
            // printf("root vertex has no neighbours \n");
            //  erase the first_edge and
            //  send ack back

            if (sender_vertex == ROOT) {
                hpx_lco_and_set(sssp_halting_LCO, HPX_NULL);

                gettimeofday(&end, 0);
                v->total_time_in_actions +=
                    (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec) / 1000000.0);

                hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
                hpx_gas_unpin(local);
                return HPX_SUCCESS;
            } else {
                hpx_addr_t vertex_ = vertex_element_at(vertices, v->first_edge);

                // if(sender_vertex == root_vertex_){
                // printf("Vertex: %d: Has no neighbours so just leaving: v->first_edge %d\n",
                // v->vertex_id, v->first_edge);
                // }
                v->first_edge = UNKNOWN;
                hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &sssp_halting_LCO);

                // this is just for performance
                v->actions_invoked++;
            }
        }

        gettimeofday(&end, 0);
        v->total_time_in_actions +=
            (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec) / 1000000.0);

        hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
        // printf("SSSP: exiting _sssp_hpx_async_handler, vertex: = %d\n", v->vertex_id);
        hpx_gas_unpin(local);
        return HPX_SUCCESS;

        // note here that we are not ackoledging the message recipt right here
        // this will be done when v->deficit reaches zero and then send recipt to the
        // v->first_edge
    } else {

        if (v->distance_from_start > incoming_distance) {

            // update and diffuse/spread the update
            v->parent = sender_vertex;
            v->distance_from_start = incoming_distance;

            // send ack to the sender, if v->first_edge != UNKNOWN && v->count != 0

            if ((v->first_edge == UNKNOWN && v->edges.head->count == 0) ||
                (v->first_edge != UNKNOWN && v->edges.head->count > 0)) {

                // normal operation
                hpx_addr_t vertex_ = vertex_element_at(vertices, sender_vertex);

                if (0 && sender_vertex == root_vertex_) {
                    printf("Normal ack: sending ack to %d from %d\n", sender_vertex, v->vertex_id);
                }

                hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &sssp_halting_LCO);

                // this is just for performance
                v->actions_invoked++;
            }

            if ((v->first_edge == UNKNOWN) && (v->edges.head->count > 0)) {
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
            edge_bucket* cursor = v->edges.head;
            // diffuse
            for (int i = 0; i < v->edges.buckets_count; i++) {
                for (int j = 0; j < cursor->count; j++) {
                    new_distance = distance + cursor->edges[j].weight;
                    hpx_addr_t vertex_ = vertex_element_at(vertices, cursor->edges[j].node_id);
                    hpx_call_async(vertex_,
                                   _sssp_hpx_async,
                                   HPX_NULL,
                                   HPX_NULL,
                                   &vertices,
                                   &v->vertex_id,
                                   &new_distance,
                                   &sssp_halting_LCO);

                    v->deficit++;

                    // this is just for performance
                    v->actions_invoked++;
                }
                cursor = cursor->next;
            }
        } else {

            if (0 && sender_vertex == root_vertex_) {
                printf("Stable ack: to: %d from Vertex: %d: Distance: %d \n",
                       sender_vertex,
                       v->vertex_id,
                       v->distance_from_start);
            }
            // just ack to the sender
            hpx_addr_t vertex_ = vertex_element_at(vertices, sender_vertex);
            hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &sssp_halting_LCO);

            // this is just for performance statistics
            v->actions_invoked++;
        }
    }

    gettimeofday(&end, 0);
    v->total_time_in_actions +=
        (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec) / 1000000.0);

    hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
    // printf("SSSP: exiting _sssp_hpx_async_handler, vertex: = %d\n", v->vertex_id);
    hpx_gas_unpin(local);
    return HPX_SUCCESS;
}

void
SSSP_hpx_async(hpx_addr_t vertices, int start_vertex, hpx_addr_t sssp_done_lco)
{

    hpx_addr_t vertex_ = vertex_element_at(vertices, start_vertex);
    int root_parent = ROOT;
    int distance = 0;
    hpx_call_async(vertex_,
                   _sssp_hpx_async,
                   HPX_NULL,
                   HPX_NULL,
                   &vertices,
                   &root_parent,
                   &distance,
                   &sssp_done_lco);
}

// For checking to see if HPX works fine
HPX_ACTION_DECL(_hello_action);
int
_hello_action_handler(int* hook)
{
    /*
    hpx_addr_t local = hpx_thread_current_target();
    Vertex *v = NULL;
    if (!hpx_gas_try_pin(local, (void **)&v))
      return HPX_RESEND;

    printf("v->vertex_id = %d, v->distance_from_start = %d\n", v->vertex_id,
    v->distance_from_start);

    hpx_gas_unpin(local);
    return HPX_SUCCESS;
  */
    printf("Hello World from %d.\n", hpx_get_my_rank());
    return HPX_SUCCESS;
}

void
call_hello(hpx_addr_t hook)
{

    hpx_addr_t done = hpx_lco_and_new(hpx_get_num_ranks());

    for (int i = 0; i < hpx_get_num_ranks(); i++) {

        hpx_addr_t hpx_process_ = hpx_addr_add(hook, sizeof(int) * i, sizeof(int));
        // printf("vertex_ = %ld\n", vertex_);
        hpx_call(hpx_process_, _hello_action, done);
    }

    hpx_lco_wait(done);
    hpx_lco_delete(done, HPX_NULL);
    return;
}

HPX_ACTION_DECL(_sssp);
int
_sssp_handler(void)
{

    FILE* f = NULL;
    if ((f = fopen(filename_, "rb")) == NULL) {
        printf("File reading failed!\n");
        return -1;
    }

    fread(&N, sizeof(int), 1, f);
    fread(&N_, sizeof(int), 1, f);
    fread(&N_edges, sizeof(int), 1, f);

    printf("Graph: %s, graph vertex count is %d with %d egdes.\n", filename_, N, N_edges);
    fclose(f);
    fflush(stdout);

    hpx_addr_t vertices = hpx_gas_calloc_cyclic(N, sizeof(Vertex), 0);
    // this is the main hpx sssp code
    assert(vertices != HPX_NULL);
    printf("Vertices allocation is done\n");
    fflush(stdout);

    hpx_addr_t hook_to_HPX_processes = hpx_gas_calloc_cyclic(N, sizeof(int), 0);
    // this is the main hpx sssp code
    assert(hook_to_HPX_processes != HPX_NULL);
    printf("hook_to_HPX_processes done\n");
    fflush(stdout);
    // call_hello(hook_to_HPX_processes);

    // Populate the graph
    // intialize the counts of Vertex struct
    init_count_vertices(hook_to_HPX_processes, vertices, N);
    printf("init_count_vertices done\n");
    fflush(stdout);

    populate_graph(hook_to_HPX_processes, vertices);
    printf("Graph has been initialied from the input file\n");
    fflush(stdout);

    printf("Iteration\tSSSP_Time(secs)\tActions_Invoked\treset_time(secs)\treduce_time(secs)\n");
    for (int iterations_runs = 0; iterations_runs < iterations; iterations_runs++) {
        // printf("Sleep 3 sec to cool down\n");
        // fflush(stdout);
        sleep(3);

        // Call the Asynchronous SSSP here
        hpx_addr_t sssp_done_lco = hpx_lco_and_new(1);

        struct timeval end, begin;
        gettimeofday(&begin, 0);

        int start_vertex = root_vertex_;
        SSSP_hpx_async(vertices, start_vertex, sssp_done_lco);

        hpx_lco_wait(sssp_done_lco);
        hpx_lco_delete(sssp_done_lco, HPX_NULL);

        gettimeofday(&end, 0);
        // printf("Asynchronous SSSP using HPX-5 finished\n");

        // timing
        double elapsed_sssp =
            (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec) / 1000000.0);

        // printf("Time taken in Asynchronous SSSP (secs): %lf\n", elapsed);

        /* Validation:
        Print the sssp value at a known target_vertex for the input graph
        Manually check whether it aligns with the result using python networkx
        Yes, it is working, so commenting it out.
        */

        //    int target_vertex = root_vertex_;
        //    print_distance_at(vertices, target_vertex);
        // int target_vertex = 3;
        // print_distance_at(vertices, target_vertex);
        //  print_vertex_info_all(vertices);
        // fflush(stdout);

        gettimeofday(&begin, 0);
        // reduce the results, in this case the number of actions invoked
        int actions_invoked_total = reduce_actions_v2(hook_to_HPX_processes, vertices, N);
        fflush(stdout);

        gettimeofday(&end, 0);

        double elapsed_reduce_actions =
            (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec) / 1000000.0);

        gettimeofday(&begin, 0);
        // reset vertices for next run (iteration)
        reset_vertices(hook_to_HPX_processes, vertices, N);

        gettimeofday(&end, 0);

        // timing
        double elapsed_reset =
            (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec) / 1000000.0);

        printf("%d\t%lf\t%d\t%lf\t%lf\n",
               iterations_runs,
               elapsed_sssp,
               actions_invoked_total,
               elapsed_reset,
               elapsed_reduce_actions);
        fflush(stdout);
    }

    printf("Input Graph: %s\n", filename_);
    printf("HPX Processes: %d\n", hpx_get_num_ranks());

    printf("Freeing memory, vertices\n");
    FREE_vertices_internal(vertices, N);
    hpx_gas_free(vertices, HPX_NULL);

    printf("All done! Goodbye.\n");

    fflush(stdout);
    hpx_exit(0, NULL);
}

HPX_ACTION(HPX_DEFAULT, 0, _get_value, _get_value_handler);

HPX_ACTION(HPX_DEFAULT,
           HPX_PINNED,
           _populate_graph_dispatch,
           _populate_graph_dispatch_handler,
           HPX_POINTER,
           HPX_ADDR);

HPX_ACTION(HPX_DEFAULT,
           HPX_PINNED,
           _reduce_actions_v2_dispatch,
           _reduce_actions_v2_dispatch_handler,
           HPX_POINTER,
           HPX_ADDR,
           HPX_INT);

HPX_ACTION(HPX_DEFAULT,
           HPX_PINNED,
           _reset_vertices_dispatch,
           _reset_vertices_dispatch_handler,
           HPX_POINTER,
           HPX_ADDR,
           HPX_INT);

HPX_ACTION(HPX_DEFAULT,
           HPX_PINNED,
           _init_count_vertices,
           _init_count_vertices_handler,
           HPX_POINTER,
           HPX_INT);

HPX_ACTION(HPX_DEFAULT,
           HPX_PINNED,
           _init_count_vertices_dispatch,
           _init_count_vertices_dispatch_handler,
           HPX_POINTER,
           HPX_ADDR,
           HPX_INT);

HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _hello_action, _hello_action_handler, HPX_POINTER);

HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _print_distance_at, __print_distance_at_handler, HPX_POINTER);

HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _free_vertices, _free_vertices_handler, HPX_POINTER, HPX_INT);

HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _send_edge, _send_edge_handler, HPX_POINTER, HPX_INT, HPX_INT);

HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _ack, _ack_handler, HPX_POINTER, HPX_ADDR, HPX_ADDR);

HPX_ACTION(HPX_DEFAULT,
           HPX_PINNED,
           _sssp_hpx_async,
           _sssp_hpx_async_handler,
           HPX_POINTER,
           HPX_ADDR,
           HPX_INT,
           HPX_INT,
           HPX_ADDR);

// main action
HPX_ACTION(HPX_DEFAULT, 0, _sssp, _sssp_handler);

int
main(int argc, char* argv[argc])
{

    filename_ = argv[1];
    root_vertex_ = atoi(argv[2]);
    graph500_gen = atoi(argv[3]);
    iterations = atoi(argv[4]);
    // strcpy(filename_, argv[1]);
    // printf("%s -- filename\n", filename_);
    if (hpx_init(&argc, &argv) != 0)
        return -1;
    int e = hpx_run(&_sssp, NULL);
    hpx_finalize();
    return e;
}
