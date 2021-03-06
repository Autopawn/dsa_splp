#include "reduce.h"

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// DISSIMILITUDE PAIRS HEAP
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

// NOTE: Is possible to replace lint=>float and int=>ushort to use half of the memory saving pair data.
typedef struct{
    lint dissim;
    int indx_a, indx_b;
} dissimpair;

int dissimpair_cmp(dissimpair a, dissimpair b){
    lint delta = a.dissim - b.dissim;
    if(delta!=0) return delta;
    delta = a.indx_a-b.indx_a;
    if(delta!=0) return delta;
    delta = a.indx_b-b.indx_b;
    return delta;
}

// It's a regular heap but the main array is partitioned in arrays of HEAP_PARTITION size,
// to avoid mallocating huge blocks.
typedef struct{
    dissimpair **pairs;
    int n_pairs;
    int n_partitions;
} pairheap;

pairheap *pairheap_init(int size){
    pairheap *heap = safe_malloc(sizeof(pairheap));
    heap->n_pairs = 0;
    heap->n_partitions = (size+HEAP_PARTITION-1)/HEAP_PARTITION;
    heap->pairs = safe_malloc(sizeof(dissimpair*)*heap->n_partitions);
    for(int i=0;i<heap->n_partitions;i++){
        int part_size = i==heap->n_partitions-1? size-HEAP_PARTITION*(heap->n_partitions-1): HEAP_PARTITION;
        heap->pairs[i] = safe_malloc(sizeof(dissimpair)*part_size);
    }
    return heap;
}

void pairheap_free(pairheap *heap){
    for(int i=0;i<heap->n_partitions;i++){
        free(heap->pairs[i]);
    }
    free(heap->pairs);
    free(heap);
}

dissimpair pairheap_get(pairheap *heap, int i){
    assert(i<heap->n_pairs);
    int part = i/HEAP_PARTITION;
    int comp = i-part*HEAP_PARTITION;
    return heap->pairs[part][comp];
}
void pairheap_set(pairheap *heap, int i, dissimpair v){
    assert(i<heap->n_pairs);
    int part = i/HEAP_PARTITION;
    int comp = i-part*HEAP_PARTITION;
    heap->pairs[part][comp] = v;
}

dissimpair pairheap_poll(pairheap *heap){
    dissimpair retp = pairheap_get(heap,0);
    pairheap_set(heap,0,pairheap_get(heap,heap->n_pairs-1));
    heap->n_pairs -= 1;
    // Heapify down:
    unsigned int i = 0;
    unsigned int c = 2*i+1;
    while(c<heap->n_pairs){
        if(c+1<heap->n_pairs && dissimpair_cmp(pairheap_get(heap,c+1),pairheap_get(heap,c))<0) c = c+1;
        if(dissimpair_cmp(pairheap_get(heap,i),pairheap_get(heap,c))<0) break;
        dissimpair aux = pairheap_get(heap,i);
        pairheap_set(heap,i,pairheap_get(heap,c));
        pairheap_set(heap,c,aux);
        i = c;
        c = 2*i+1;
    }
    //
    return retp;
}

void pairheap_add(pairheap *heap, dissimpair val){
    heap->n_pairs += 1;
    pairheap_set(heap,heap->n_pairs-1,val);
    // Heapify up:
    unsigned int i = heap->n_pairs-1;
    unsigned int p = (i-1)/2;
    while(i>0 && dissimpair_cmp(pairheap_get(heap,i),pairheap_get(heap,p))<0){
        dissimpair aux = pairheap_get(heap,i);
        pairheap_set(heap,i,pairheap_get(heap,p));
        pairheap_set(heap,p,aux);
        i = p;
        p = (i-1)/2;
    }
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// MULTITHREAD DISSIMILITUDE COMPUTATION
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

#if defined(REDUCTION_DISPERSE) && THREADS>0

typedef struct {
    // Thread id:
    int thread_id;
    // Concurrent access to heap.
    pthread_mutex_t *heap_mutex;
    pairheap *heap;
    // Concurrent access to prev and next solution lists to build reposition pairs.
    sem_t *thread_sem;
    sem_t *complete_sem; // Inform that thread has terminated with its reposition pairs.
    int *terminated;
    const int *prev_sols;
    const int *next_sols;
    // Solutions (read only!).
    const problem *prob;
    const solution **sols;
    int n_sols;
    int vision_range;
} reduce_thread_args;

void *reduce_thread_execution(void *arg){
    reduce_thread_args *args = (reduce_thread_args *) arg;

    // Help building initial set of dissimilitude pairs
    dissimpair *pairs = safe_malloc(sizeof(dissimpair)*args->vision_range);
    int n_pairs = 0;
    for(int i=args->thread_id;i<args->n_sols;i+=THREADS){
        for(int j=1;j<=args->vision_range;j++){
            if(i+j>=args->n_sols) break;
            // REPLACE
            pairs[n_pairs].indx_a = i;
            pairs[n_pairs].indx_b = i+j;
            pairs[n_pairs].dissim = solution_dissimilitude(args->prob,
                args->sols[i],args->sols[i+j]);
            n_pairs += 1;
        }
        // Save pairs in the heap
        pthread_mutex_lock(args->heap_mutex);
            for(int k=0;k<n_pairs;k++){
                pairheap_add(args->heap,pairs[k]);
            }
        pthread_mutex_unlock(args->heap_mutex);
        // Delete pairs
        n_pairs = 0;
    }
    free(pairs);

    // Wake when is required to compute new dissimilitudes
    dissimpair *pair_buffer = safe_malloc(sizeof(dissimpair)*(args->vision_range/THREADS+1));
    int pair_buffer_len = 0;
    while(1){
        sem_post(args->complete_sem);
        // ---@> Main thread works here.
        sem_wait(args->thread_sem);
        assert(args->prev_sols!=NULL);
        assert(args->next_sols!=NULL);
        int terminated = *args->terminated;
        // If the job is done, terminate.
        if(terminated) break;
        // Create new pairs
        for(int i=args->thread_id;i<args->vision_range;i+=THREADS){
            int pair_a = args->prev_sols[args->vision_range-1-i];
            int pair_b = args->next_sols[i];
            if(pair_a!=-1 && pair_b!=-1){
                // Create the replace node:
                dissimpair pair;
                pair.indx_a = pair_a;
                pair.indx_b = pair_b;
                pair.dissim = solution_dissimilitude(args->prob,
                    args->sols[pair_a],args->sols[pair_b]);
                // Add to pair buffer:
                pair_buffer[pair_buffer_len] = pair;
                pair_buffer_len += 1;
                // Add pairs in buffer to to the heap if mutex is free
                if(pthread_mutex_trylock(args->heap_mutex)==0){
                    for(int k=0;k<pair_buffer_len;k++){
                        pairheap_add(args->heap,pair_buffer[k]);
                    }
                    pthread_mutex_unlock(args->heap_mutex);
                    pair_buffer_len = 0;
                }
            }
        }
        // If pairs remaining, wait for mutex
        if(pair_buffer_len>0){
            pthread_mutex_lock(args->heap_mutex);
            for(int i=0;i<pair_buffer_len;i++){
                pairheap_add(args->heap,pair_buffer[i]);
            }
            pthread_mutex_unlock(args->heap_mutex);
            pair_buffer_len = 0;
        }
    }
    free(pair_buffer);
    return NULL;
}

#endif

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// REDUCTION PROCESS
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

// Reduces an array (sols) with pointers to a set of solutions to target_n size, freeing memory of the discarted ones. *n_sols is modified.
// If  macro is defined, only pick the best solutions
#ifdef REDUCTION_BESTS

void reduce_solutions(const problem *prob,
        solution **sols, int *n_sols, int target_n, int vision_range){
    assert(vision_range==0);
    // Sort solution pointers from larger to smaller value of the solution.
    qsort(sols,*n_sols,sizeof(solution*),solution_value_cmp_inv);
    // Free other solutions:
    for(int i=target_n;i<*n_sols;i++){
        free(sols[i]);
    }
    // Set the amount of solutions right.
    if(*n_sols>target_n) *n_sols = target_n;
}

#endif

#ifdef REDUCTION_RANDOM

void reduce_solutions(const problem *prob,
        solution **sols, int *n_sols, int target_n, int vision_range){
    assert(vision_range==0);
    if(*n_sols>target_n){
        // Put target_n randomly selected solutions first on the array:
        for(int i=0;i<target_n;i++){
            int choice = i+rand()%(*n_sols-i);
            solution *aux = sols[i];
            sols[i] = sols[choice];
            sols[choice] = aux;
        }
    }
    // Free other solutions:
    for(int i=target_n;i<*n_sols;i++){
        free(sols[i]);
    }
    // Set the amount of solutions right.
    if(*n_sols>target_n) *n_sols = target_n;
}

#endif

#ifdef REDUCTION_DISPERSE

void reduce_solutions(const problem *prob,
        solution **sols, int *n_sols, int target_n, int vision_range){
    assert(vision_range>0);
    // Ensure that the vision_range isn't larger than the number of solutions.
    if(vision_range>*n_sols) vision_range = *n_sols;
    // Sort solution pointers from larger to smaller value of the solution.
    qsort(sols,*n_sols,sizeof(solution*),solution_value_cmp_inv);
    #ifdef VERBOSE_BASESORTED
        printf("#BASESORTED\n");
        print_solsets(sols,*n_sols);
    #endif
    // Return if there is no need of reduction.
    if(*n_sols<=target_n) return;
    // Double linked structure to know solutions that haven't yet been discarted:
    int *discarted = safe_malloc((*n_sols)*sizeof(int));
    int *nexts = safe_malloc((*n_sols)*sizeof(int));
    int *prevs = safe_malloc((*n_sols)*sizeof(int));
    for(int i=0;i<*n_sols;i++){
        discarted[i] = 0;
        prevs[i] = i-1;
        nexts[i] = i+1;
    }
    prevs[0] = -1;
    nexts[*n_sols-1] = -1;

    int *prev_sols = safe_malloc(sizeof(int)*vision_range);
    int *next_sols = safe_malloc(sizeof(int)*vision_range);

    #if THREADS>0
        // Heap of dissimilitude pairs
        pairheap *heap = pairheap_init(2*(*n_sols)*vision_range);
        pthread_mutex_t heap_mutex;
        pthread_mutex_init(&heap_mutex,NULL);
        //
        // Create threads:
        pthread_t *threads = safe_malloc(sizeof(pthread_t)*THREADS);
        sem_t *t_sems = safe_malloc(sizeof(pthread_mutex_t)*THREADS);
        sem_t *c_sems = safe_malloc(sizeof(pthread_mutex_t)*THREADS);
        reduce_thread_args *targs = safe_malloc(sizeof(reduce_thread_args)*THREADS);
        int terminated = 0; // NOTE: Only access before threads are waken.
        //
        for(int i=0;i<THREADS;i++){
            sem_init(&t_sems[i],0,0);
            sem_init(&c_sems[i],0,0);
            //
            targs[i].thread_id = i;
            targs[i].heap_mutex = &heap_mutex;
            targs[i].heap = heap;
            //
            targs[i].thread_sem = &t_sems[i];
            targs[i].complete_sem = &c_sems[i];
            targs[i].terminated = &terminated;
            targs[i].prev_sols = prev_sols;
            targs[i].next_sols = next_sols;
            //
            targs[i].prob = prob;
            targs[i].sols = (const solution **) sols; // NOTE: incompatible pointer?
            targs[i].n_sols = *n_sols;
            targs[i].vision_range = vision_range;
            //
            int rc = pthread_create(&threads[i],NULL,reduce_thread_execution,&targs[i]);
            if(rc){
                printf("Error %d on thread creation\n",rc);
                exit(1);
            }
        }
        // Wait for all threads to terminate the last job
        for(int i=0;i<THREADS;i++){
            sem_wait(&c_sems[i]);
        }
        // ---@> At this point, all initial dissimilitude pairs are complete.
    #else
        // Heap of dissimilitude pairs
        pairheap *heap = pairheap_init(2*(*n_sols)*vision_range);
        // Initial set of dissimilitude pairs
        for(int i=0;i<*n_sols;i++){
            for(int j=1;j<=vision_range;j++){
                if(i+j>=*n_sols) break;
                dissimpair pair;
                pair.indx_a = i;
                pair.indx_b = i+j;
                pair.dissim = solution_dissimilitude(prob,
                    sols[pair.indx_a],sols[pair.indx_b]);
                pairheap_add(heap,pair);
            }
        }
    #endif

    // Eliminate as many solutions as required:
    int n_eliminate = *n_sols-target_n;
    int elims = 0;
    while(elims<n_eliminate){
        // Eliminate worst solution of most similar pair
        if(heap->n_pairs==0) break;
        dissimpair pair = pairheap_poll(heap);
        if(!discarted[pair.indx_a] && !discarted[pair.indx_b]){
            // Delete the second solution on the pair.
            int to_delete = pair.indx_b;
            discarted[to_delete] = 1;
            free(sols[to_delete]);
            elims += 1;
            // Update double linked list:
            if(nexts[to_delete]!=-1) prevs[nexts[to_delete]] = prevs[to_delete];
            if(prevs[to_delete]!=-1) nexts[prevs[to_delete]] = nexts[to_delete];
            // Add new pairs to replace those that will be deleted on the destroyed solution.
            int iter;
            // Get solutions after
            iter = to_delete;
            for(int i=0;i<vision_range;i++){
                if(nexts[iter]==-1){
                    next_sols[i] = -1;
                }else{
                    iter = nexts[iter];
                    next_sols[i] = iter;
                }
            }
            // Get solutions before
            iter = to_delete;
            for(int i=0;i<vision_range;i++){
                if(prevs[iter]==-1){
                    prev_sols[i] = -1;
                }else{
                    iter = prevs[iter];
                    prev_sols[i] = iter;
                }
            }
            #if THREADS>0
                // Wake threads to create new pairs
                for(int i=0;i<THREADS;i++){
                    sem_post(&t_sems[i]);
                }
                // Wait for all threads to terminate their job
                for(int i=0;i<THREADS;i++){
                    sem_wait(&c_sems[i]);
                }
            #else
                // Create new pairs
                for(int i=0;i<vision_range;i++){
                    int pair_a = prev_sols[vision_range-1-i];
                    int pair_b = next_sols[i];
                    if(pair_a!=-1 && pair_b!=-1){
                        // Create the replace node:
                        dissimpair pair;
                        pair.indx_a = pair_a;
                        pair.indx_b = pair_b;
                        pair.dissim = solution_dissimilitude(prob,
                            sols[pair.indx_a],sols[pair.indx_b]);
                        assert(heap->n_pairs<2*(*n_sols)*vision_range);
                        pairheap_add(heap,pair);
                    }
                }
            #endif
        }
    }

    #if THREADS>0
        // Wake threads for termination
        terminated = 1;
        for(int i=0;i<THREADS;i++){
            sem_post(&t_sems[i]);
        }
        // Join threads
        for(int i=0;i<THREADS;i++){
            pthread_join(threads[i],NULL);
        }
        // Destroy semaphores
        for(int i=0;i<THREADS;i++){
            sem_destroy(&t_sems[i]);
            sem_destroy(&c_sems[i]);
        }
        // Free double linked list
        free(threads);
        free(t_sems);
        free(c_sems);
        free(targs);
    #endif

    // Free prev and next sols arrays
    free(prev_sols);
    free(next_sols);
    // Free all the pairs:
    pairheap_free(heap);
    // Set output final array:
    int new_nsols=0;
    for(int i=0;i<*n_sols;i++){
        if(discarted[i]==0){
            sols[new_nsols] = sols[i];
            new_nsols += 1;
        }
    }
    *n_sols = new_nsols;
    // Free arrays
    free(discarted);
    free(nexts);
    free(prevs);

}

#endif
