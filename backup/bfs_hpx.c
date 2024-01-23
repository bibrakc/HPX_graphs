#include <hpx/hpx.h>
#include <stdio.h>

// the h file

#define EDGES_BUFF_SIZE 16
#define UNKNOWN -3
#define ROOT -1
typedef struct
{

    int edges[EDGES_BUFF_SIZE];
    int count;

    // TODO: although seems redundant but lets put it here
    int vertex_id;

    // the result in this case parent for BFS
    int parent;
    int distance_from_start;

    // Dijkstra–Scholten termination detection
    int first_edge;
    int deficit;

    // TODO:
    // edges_list *next;

    // control related
    hpx_addr_t vertex_local_mutex;
} Vertex;

hpx_addr_t
vertex_element_at(hpx_addr_t vert, int i)
{
    return hpx_addr_add(vert, sizeof(Vertex) * i, sizeof(Vertex));
}

// h file ends

static HPX_ACTION_DECL(_hello);
static int
_hello_action(void)
{
    printf("Hello World from %u.\n", hpx_get_my_rank());
    hpx_exit(0, NULL);
}

// bfs

static HPX_ACTION_DECL(_init_count_vertices);
static int
_init_count_vertices_handler(Vertex* v, int id)
{

    // this pin thingy will be need in distributed setting
    // printf("pop handler neigh = %d\n", zero);
    /*
      hpx_addr_t local = hpx_thread_current_target();
      Vertex *vert = NULL;
      if (!hpx_gas_try_pin(local, (void**) &vert))
        return HPX_RESEND;

    //  printf("pop handler neigh = %d\n", zero);

      vert->count = zero;

      hpx_gas_unpin(local);
    */
    v->vertex_id = id;
    v->count = 0;
    v->parent = UNKNOWN;
    v->distance_from_start = 0;
    v->first_edge = UNKNOWN; // for Dijkstra–Scholten termination detection
    v->deficit = 0;          // for Dijkstra–Scholten termination detection
    v->vertex_local_mutex = hpx_lco_sema_new(1);

    return HPX_SUCCESS;
}

// TODO write a cleanup handler too that frees vertex internal data like:
//      vertex_local_mutex and the edges linklist

static HPX_ACTION_DECL(_send_edge);
static int
_send_edge_handler(Vertex* v, int edge)
{

    // TODO: put pining when runing in distributed setting

    hpx_lco_sema_p(v->vertex_local_mutex);
    if (v->count >= EDGES_BUFF_SIZE) {
        printf("EDGES_BUFF_SIZE exeeded, implement linklist! Not adding the edge");
        return HPX_SUCCESS;
    }

    v->edges[v->count] = edge;
    v->count++;
    // printf("vertex: %d, edge %d inserted as %d\n", v->vertex_id, edge, v->edges[v->count - 1]);
    hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
    return HPX_SUCCESS;
}

// TODO: make this return int
// call/fork the _init_count_vertices
void
init_count_vertices(hpx_addr_t vertices, int N)
{
    hpx_addr_t done = hpx_lco_and_new(N);
    for (int i = 0; i < N; i++) {
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

// TODO: make this return int
// call/fork the _init_count_vertices
void
populate_graph(hpx_addr_t vertices, int N, int N_edges, FILE* f)
{
    hpx_addr_t done = hpx_lco_and_new(N_edges);

    int vert_from, vert_to;
    vert_from = -1;
    vert_to = -1;
    // int is_EOF;
    // while((is_EOF = fscanf(f, "%d %d", &vert_from, &vert_to)) != EOF){
    for (int i = 0; i < N_edges; i++) {
        // printf("i = %d\n", i);
        fscanf(f, "%d %d", &vert_from, &vert_to);
        printf("read %d --> %d\n", vert_from, vert_to);

        hpx_addr_t vertex_ = vertex_element_at(vertices, vert_from);
        // printf("vertex_ = %ld\n", vertex_);
        hpx_call(vertex_, _send_edge, done, &vert_to);
        // printf("i = %d done\n", i);
    }

    hpx_lco_wait(done);
    hpx_lco_delete(done, HPX_NULL);

    return;
}

static HPX_ACTION_DECL(_ack);
static int
_ack_handler(Vertex* v, hpx_addr_t vertices, hpx_addr_t bfs_halting_LCO)
{

    hpx_lco_sema_p(v->vertex_local_mutex);

    v->deficit--;

    // printf("ack: vertex %d deficit = %d\n", v->vertex_id, v->deficit);

    if (v->deficit == 0 && v->parent == ROOT) {

        hpx_lco_and_set(bfs_halting_LCO, HPX_NULL);
    } else if (v->deficit == 0) {

        // DEBUGGING: Printing the result, later write a reducer TODO:
        printf("vertex %d parent is %d\n", v->vertex_id, v->parent);

        hpx_addr_t vertex_ = vertex_element_at(vertices, v->first_edge);
        v->first_edge = UNKNOWN;
        hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &bfs_halting_LCO);
    }

    hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
    return HPX_SUCCESS;
}

static HPX_ACTION_DECL(_bfs_hpx_async);
static int
_bfs_hpx_async_handler(Vertex* v,
                       hpx_addr_t vertices,
                       int sender_vertex,
                       int incoming_distance,
                       hpx_addr_t bfs_halting_LCO)
{
    // there needs to be a vertex lco too as this is critical section
    // printf("BFS: in _bfs_hpx_async_handler, vertex: = %d\n", v->vertex_id);
    hpx_lco_sema_p(v->vertex_local_mutex);

    // this is for the first activation of the vertex
    // or in case the vertex deficit becomes zero and it deactivates and reactivates
    if (v->parent == UNKNOWN) {

        v->first_edge = sender_vertex; // this creates an implicit spanning tree for
                                       // Dijkstra–Scholten termination detection

        v->parent = sender_vertex;
        v->distance_from_start = incoming_distance;

        // printf("BFS: vertex: %d set it's parent as %d\n", v->vertex_id, v->parent);
        int distance = v->distance_from_start + 1;
        for (int i = 0; i < v->count; i++) {
            // printf("BFS: vertex: %d preparing edge: %d\n", v->vertex_id, v->edges[i]);
            hpx_addr_t vertex_ = vertex_element_at(vertices, v->edges[i]);
            hpx_call_async(vertex_,
                           _bfs_hpx_async,
                           HPX_NULL,
                           HPX_NULL,
                           &vertices,
                           &v->vertex_id,
                           &distance,
                           &bfs_halting_LCO);
            v->deficit++;
        }
        // printf("BFS: vertex: %d setting bfs_halting_LCO\n", v->vertex_id);
        // hpx_lco_and_set(bfs_halting_LCO, HPX_NULL);

        // what is the graph root is dirty such that the root has no neighbours
        //  take care of that case TODO: later
        if (v->count == 0) {

            // erase the first_edge and
            // send ack back
            hpx_addr_t vertex_ = vertex_element_at(vertices, v->first_edge);
            v->first_edge = UNKNOWN;
            hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &bfs_halting_LCO);
        }

        hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
        // printf("BFS: exiting _bfs_hpx_async_handler, vertex: = %d\n", v->vertex_id);
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

            if ((v->first_edge == UNKNOWN && v->count == 0) ||
                (v->first_edge != UNKNOWN && v->count > 0)) {

                // normal operation
                hpx_addr_t vertex_ = vertex_element_at(vertices, sender_vertex);
                hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &bfs_halting_LCO);
            }

            if (v->first_edge == UNKNOWN) {
                // TODO check if deficit is zero, it should be
                v->first_edge = sender_vertex;
            }

            int distance = v->distance_from_start + 1;
            // diffuse
            for (int i = 0; i < v->count; i++) {
                // printf("BFS: vertex: %d preparing edge: %d\n", v->vertex_id, v->edges[i]);
                hpx_addr_t vertex_ = vertex_element_at(vertices, v->edges[i]);
                hpx_call_async(vertex_,
                               _bfs_hpx_async,
                               HPX_NULL,
                               HPX_NULL,
                               &vertices,
                               &v->vertex_id,
                               &distance,
                               &bfs_halting_LCO);
                v->deficit++;
            }

        } else {

            // just ack to the sender
            hpx_addr_t vertex_ = vertex_element_at(vertices, sender_vertex);
            hpx_call_async(vertex_, _ack, HPX_NULL, HPX_NULL, &vertices, &bfs_halting_LCO);
        }
    }

    hpx_lco_sema_v(v->vertex_local_mutex, HPX_NULL);
    // printf("BFS: exiting _bfs_hpx_async_handler, vertex: = %d\n", v->vertex_id);
    return HPX_SUCCESS;
}

void
BFS_hpx_async(hpx_addr_t vertices, int start_vertex, hpx_addr_t bfs_done_lco)
{

    hpx_addr_t vertex_ = vertex_element_at(vertices, start_vertex);
    int root_parent = ROOT;
    int distance = 0;
    hpx_call_async(vertex_,
                   _bfs_hpx_async,
                   HPX_NULL,
                   HPX_NULL,
                   &vertices,
                   &root_parent,
                   &distance,
                   &bfs_done_lco);
}

static HPX_ACTION_DECL(_bfs);
static int
_bfs_handler(void)
{
    FILE* f = NULL;

    if ((f = fopen("graph.mm", "r")) == NULL)
        return -1;

    int N = 0;
    int N_edges = 0;

    fscanf(f, "%d", &N);
    fscanf(f, "%d", &N_edges);

    printf("Hello World in _bfs_handler from %u the graph vertex count is %d with %d egdes.\n",
           hpx_get_my_rank(),
           N,
           N_edges);

    hpx_addr_t vertices = hpx_gas_calloc_cyclic(N, sizeof(Vertex), 0);
    // this is the main hpx bfs code
    assert(vertices != HPX_NULL);
    printf("allocation is done\n");

    printf("vertices = %ld\n", vertices);
    // populate the graph
    // intialize the counts of Vertex struct
    init_count_vertices(vertices, N);

    populate_graph(vertices, N, N_edges, f);
    fclose(f);
    // TODO 1
    // call the BFS algo here
    hpx_addr_t bfs_done_lco = hpx_lco_and_new(1);
    printf("bfs_done_lco = %ld\n", bfs_done_lco);

    int start_vertex = 3;
    BFS_hpx_async(vertices, start_vertex, bfs_done_lco);

    hpx_lco_wait(bfs_done_lco);
    hpx_lco_delete(bfs_done_lco, HPX_NULL);
    printf("BFS_hpx_async and LCOs done waited and deleted\n");

    // TODO 2
    // print the results

    hpx_gas_free(vertices, HPX_NULL);
    hpx_exit(0, NULL);
    // return 0;
}

static HPX_ACTION(HPX_DEFAULT, 0, _hello, _hello_action);
static HPX_ACTION(HPX_DEFAULT,
                  HPX_PINNED,
                  _init_count_vertices,
                  _init_count_vertices_handler,
                  HPX_POINTER,
                  HPX_INT);
static HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _send_edge, _send_edge_handler, HPX_POINTER, HPX_INT);
static HPX_ACTION(HPX_DEFAULT, HPX_PINNED, _ack, _ack_handler, HPX_POINTER, HPX_ADDR, HPX_ADDR);
static HPX_ACTION(HPX_DEFAULT,
                  HPX_PINNED,
                  _bfs_hpx_async,
                  _bfs_hpx_async_handler,
                  HPX_POINTER,
                  HPX_ADDR,
                  HPX_INT,
                  HPX_INT,
                  HPX_ADDR);

// main action
static HPX_ACTION(HPX_DEFAULT, 0, _bfs, _bfs_handler);

int
main(int argc, char* argv[argc])
{
    if (hpx_init(&argc, &argv) != 0)
        return -1;
    int e = hpx_run(&_bfs, NULL);
    hpx_finalize();
    return e;
}
