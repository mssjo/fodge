/*
 * This file contains methods used to represent diagrams as compound cycreps.
 * They are defined in cycrep.h
 * 
 * Author: Mattias Sjoe, 2019
 */


#include "cycrep.h"

/** Comparison function for use with infosort.  */
int _compare_con(const void* a, const void* b){
    return compare_comprep(*((const comprep**) a), *((const comprep**) b));
}

/** Comparison function for use with infosort. */
int _compare_rep(const void* a, const void* b){
    return compare_cycrep(*((const cycrep**) a), *((const cycrep**) b), 
            ALL_LEVELS);
}

/**
 * Creates the comprep representation of a polygon diagram. This representation 
 * is central to the generation of diagrams, since it is used for their 
 * comparison and symmetry properties.
 * 
 * @param diagr     the diagram.
 * @return  a comprep representing the diagram.
 */
comprep* represent_diagram(const diagram* diagr){

    comprep* crep = salloc(sizeof(comprep));
    
    crep->nreps  = 0;
    crep->reps   = salloc(diagr->npolys * sizeof(cycrep*));
    crep->poly_reps = scalloc(diagr->npolys, sizeof(cycrep*));
    
    /* Traverses all polygons and calls represent_part to represent the flavour
     * parts of the diagram. Since represent_part traverses exactly those 
     * polygons that are included in that part, the use of visited ensures that
     * each part is represented exactly once. */
    for(size_t p_idx = 0; p_idx < diagr->npolys; p_idx++){
        if(!crep->poly_reps[p_idx]){
            crep->reps[crep->nreps] 
                    = represent_part(diagr, crep->poly_reps, p_idx, NULL);
            
            crep->nreps++;
        }
    }
        
    /* Sorts the parts and stores equality information for faster comparisons.*/
    crep->eq_reps = salloc(crep->nreps * sizeof(size_t));
    info_sort(crep->reps, crep->nreps, sizeof(cycrep*), _compare_rep,
            NULL, NULL, crep->eq_reps, NULL);
       
    return crep;
}

/**
 * Represents a flavour part as a cycrep. It will refer to all parts connected
 * to it through flavour-split vertices, but to prevent loops, these are
 * represented as "slaves" that treat their "master" as a black box rather than
 * referring back to it. Therefore, each part is represented many times, once as
 * master and several times as slave, slave of slave, etc. This is a necessary
 * evil.
 * 
 * @param diagr     the diagram containing the part.
 * @param visited   visited[p] is marked true when polygon p is visited by this
 *                  method and its auxiliaries.
 * @param p_idx     the index of a polygon in the part.
 * @param master    master[p] is true iff polygon p is contained in a part that
 *                  is a master to the current part.
 * @return  a newly allocated cycrep representing the part.
 */
cycrep* represent_part(const diagram* diagr, cycrep** poly_reps, size_t p_idx, 
        int* master){
            
    /* If this part is not the slave of another, set up the master flag array.*/
    int is_master;
    if(!master && poly_reps){
        master = scalloc(diagr->npolys, sizeof(int));
        is_master = TRUE;
    }
    else if(master && !poly_reps){
        poly_reps = scalloc(diagr->npolys, sizeof(cycrep*));
        is_master = FALSE;
    }
    else{
        printf("Malformed call to represent_part!\n");
        exit(EXIT_FAILURE);
    }
    
    /* Sets up the slave representations. */
    cycrep* part = init_part(diagr, p_idx, poly_reps, master);
                 
    fill_part(diagr, p_idx, part, master);
       
    if(is_master)
        free(master);
    else
        free(poly_reps);
    
    /* Normalises in three stages for more sensibly oriented diagrams. */
    normalise_cycrep(part, TOP_LEVEL);
    normalise_cycrep(part, ORD_LEVEL);
    normalise_cycrep(part, FSP_LEVEL);
    
    if(part->period == 0){
        printf("Representation failed!\n");
        exit(EXIT_FAILURE);
    }
        
    return part;
}

/**
 * Creates an array of all connections to the polygons of a flavour part.
 * After this is complete, poly_cons[p] will hold the connections to polygon p
 * for all polygons in the part.
 * 
 * @param diagr     the diagram containing the part.
 * @param poly_cons a NULL-filled array of pointers to comprep's.
 * @param p_idx     the index of a polygon in the part.
 * @param master    master[p] is true iff the polygon with index p is contained
 *                  in a part that is a master of the current part.
 * @return  the number of entries needed to represent this part.
 */
cycrep* init_part(const diagram* diagr, size_t p_idx, 
        cycrep** poly_reps, int* master){
    
    cycrep* part = salloc(sizeof(cycrep));
    part->length = 0;
    part->offset = 0;
    part->period = 0;
    
    part->n_cons = diagr->npolys;
    part->fsp_cons = scalloc(diagr->npolys, sizeof(comprep*));
    
    size_t g_idx = 0;
    size_t g_mark = g_idx, p_mark = p_idx;
    polygon* poly = diagr->polys[p_idx];
    gon_t gon;
    
    poly_reps[p_idx] = part;
    part->fsp_cons[p_idx] = represent_fsp_con(diagr, p_idx, master);
    
    /* Traverses the part - flavour splits and singlet propagators count as
     * external sides. */
    for(;;){
        switch(poly->edges[g_idx].type){
            case SINGLET:
            case EXT_LEG:
                part->length++;
                /* Intentional fall-through */
            case FLSPLIT:
                g_idx = (g_idx + 1) % poly->ngons;
                break;
                
            case PROPGTR:
                /* This construction crosses the propagator correctly. */
                gon = poly->gons[g_idx];
                p_idx = poly->edges[g_idx].idx;
                poly = diagr->polys[p_idx];

                for(g_idx = 0; poly->gons[g_idx] != gon; g_idx++);
                
                /* Represents hitherto untraversed polygons */
                if(!poly_reps[p_idx]){
                    poly_reps[p_idx] = part;
                    part->fsp_cons[p_idx] 
                            = represent_fsp_con(diagr, p_idx, master);
                }
                
                break;
        }
        
        /* Breaks when we are back at our point of origin */
        if(p_idx == p_mark && g_idx == g_mark)
            break;
    }
    
    part->array = scalloc(part->length, sizeof(gonrep));
        
    return part;
}

/**
 * Represents the connections to a polygon as a comprep.
 * 
 * @param diagr     the diagram containing the polygon.
 * @param p_idx     the index of the polygon.
 * @param master    master[p] is true iff polygon p is contained in a part that
 *                  is a master of the part containing polygon p_idx.
 * @return  a newly allocated comprep representing the connections to the 
 *          polygon, or NULL if it is not connected to anything.
 */
comprep* represent_fsp_con(const diagram* diagr, size_t p_idx, int* master){
    polygon* poly = diagr->polys[p_idx];
    gon_t gon;
    
    int* visited = scalloc(diagr->npolys, sizeof(int));
    visited[p_idx] = TRUE;
    
    size_t g_idx = 0;
    size_t g_mark = g_idx, p_mark = p_idx;
    
    /* Sets this polygon as master to its connections */
    master[p_idx] = TRUE;
    int* m = malloc(diagr->npolys * sizeof(int));
    
    comprep* con = salloc(sizeof(comprep));
    con->nreps = 0;
    con->reps = salloc(poly->order * sizeof(cycrep*));
        
    /* Traverses the vertex - (singlet) propagators count as external, but
     * flavour splits do not. */
    for(;;){
        if(!visited[p_idx]){
            /* A connection to the master is represented as NULL 
             * - a black box, not a recursive structure. */
            if(master[p_idx]){
                con->reps[con->nreps] = NULL;
                con->nreps++;
            }
            /* Otherwise, recurse back to represent_part with this polygon
             * as master.*/
            else{
                memcpy(m, master, diagr->npolys * sizeof(int));
                con->reps[con->nreps] = represent_part(diagr, NULL, p_idx, m);
                con->nreps++;
            }
            visited[p_idx] = TRUE;
        }
        
        switch(poly->edges[g_idx].type){    
            case FLSPLIT:
                /* Crosses flavour splits just like propagators normally are */
                gon = poly->gons[g_idx];
                p_idx = poly->edges[g_idx].idx;
                poly = diagr->polys[p_idx];

                for(g_idx = 0; poly->gons[g_idx] != gon; g_idx++);

                break;
                
            case PROPGTR:
            case SINGLET:
            case EXT_LEG:
                g_idx = (g_idx + 1) % poly->ngons;
                break;
            
        }
        
        if(p_idx == p_mark && g_idx == g_mark)
            break;
    }
    
    free(m);
    free(visited);
    
    /* If there are no connections, return NULL rather than an empty one. */
    if(con->nreps == 0){
        free(con->reps);
        free(con);
        return NULL;
    }
    
    /* Sorts the representations and stores equality information. */
    con->eq_reps = salloc(con->nreps * sizeof(size_t));
    info_sort(con->reps, con->nreps, sizeof(cycrep*), _compare_rep,
            NULL, NULL, con->eq_reps, NULL);
        
    return con;
}

/**
 * Completes the representation of a part by filling in the contents of the
 * representation.
 * 
 * @param diagr     the diagram containing the part.
 * @param p_idx     the index of a polygon in the part.
 * @param rep       the cycrep that will represent the part, already treated by
 *                  init_part.
 * @param poly_cons poly_cons[p] holds the connections to polygon p for all 
 *                  polygons in the part.
 * @param master    master[p] is true iff polygon p is in a representation that
 *                  is a master of the current representation.
 */
void fill_part(const diagram* diagr, size_t p_idx, cycrep* part, int* master){
    
    /* The part starts out empty, with only the NULL connection present */
    part->n_flavidx = 0;
    
    /* This method traverses the part twice. The first time sets up some 
     * relevant sizes, and loads poly_cons into rep->fsp_cons to remove the 
     * tie to polygon indices. poly_idcs bridges the transition.
     * The second traversal fills in the actual representation, and relies on
     * the first for setup. */
    
    size_t g_idx = 0;
    size_t g_mark, p_mark;
    polygon* poly = diagr->polys[p_idx];
    gon_t gon;    
    
    /* The representation contains one entry per line on the boundary of the
     * part, without counting flavour-split lines. Propagators merely add
     * to an existing entry. Therefore, it is important that the second 
     * traversal starts immediately after a singlet or external leg. One such
     * position will be recorded in start_g and start_p. */
            
    /* Traverses the part - flavour splits and singlet propagators count as
     * external. */
    int found_start = FALSE;
    for(;;){
        switch(poly->edges[g_idx].type){
                
            case EXT_LEG:
            case SINGLET:
                g_idx = (g_idx + 1) % poly->ngons;
                
                found_start = TRUE;
                g_mark = g_idx;
                p_mark = p_idx;
                
                break;
                
            case PROPGTR:
                /* Propagator: move to next polygon and add its connections to
                 * the list */
                gon = poly->gons[g_idx];
                p_idx = poly->edges[g_idx].idx;
                poly = diagr->polys[p_idx];
               
                for(g_idx = 0; poly->gons[g_idx] != gon; g_idx++);

                break;
                
            case FLSPLIT:
                /* Flavour split: ignore */
                g_idx = (g_idx + 1) % poly->ngons;
                break;
        }
        
        if(found_start)
            break;
    }   
    
    size_t r_idx = 0;    
    linerep lrep;
        
    /* Second traversal, covers the same as the first but starts at the marked
     * position */
    for(;;){
        /* initialises new entries */
        if(!part->array[r_idx].nlines){
            part->array[r_idx].nlines = 1;
            /* Max lines: (ngons-1) internal ones, plus 1 for external */
            part->array[r_idx].lines = salloc(diagr->ngons * sizeof(linerep));
            part->array[r_idx].g_idx_in_diagr = g_idx;
        }
        
        switch(poly->edges[g_idx].type){
            case EXT_LEG:
                /* External leg: fill in line 0 (info necessary for single-
                 * polygon parts) and move on to next entry. */
                part->array[r_idx].lines[0] = (linerep) {
                    1, poly->order, part->fsp_cons[p_idx], p_idx};
                               
                part->n_flavidx++;
                        
                r_idx++;
                g_idx = (g_idx + 1) % poly->ngons;
                
                break;
                
            case PROPGTR: 
                /* Propagator: add new line to entry. */
                lrep = (linerep) {
                    _actual_dist(diagr, p_idx, g_idx),
                    poly->order,
                    part->fsp_cons[p_idx],
                    p_idx        
                };
                
                part->array[r_idx].lines[part->array[r_idx].nlines] = lrep;    
                part->array[r_idx].nlines++;
                
                gon = poly->gons[g_idx];
                p_idx = poly->edges[g_idx].idx;
                poly = diagr->polys[p_idx];
                
                for(g_idx = 0; poly->gons[g_idx] != gon; g_idx++);
                
                break;
                
            case FLSPLIT:
                /* Flavour split: ignore */                
                g_idx = (g_idx + 1) % poly->ngons;
                
                break;
                
            case SINGLET:
                /* Singlet: like external leg, but also add special "line" 
                 * pointing to the singlet connection. */
                
                part->array[r_idx].lines[0] = (linerep) {
                    1, poly->order, part->fsp_cons[p_idx], p_idx};
                        
                /* Singlet "line" has length 0 to mark it as special,
                 * and holds the unique connection to the singlet. */
                lrep = (linerep) {
                    0, 0, 
                    represent_singlet(diagr, p_idx,
                            poly->edges[g_idx].idx, master),
                    p_idx
                };
                part->array[r_idx].lines[part->array[r_idx].nlines] = lrep;
                part->array[r_idx].nlines++;
                           
                r_idx++;
                g_idx = (g_idx + 1) % poly->ngons;
                
                break;
        }
        
        if(p_idx == p_mark && g_idx == g_mark)
            break;
        
    }
}

/**
 * Represents the special connection across a singlet propagator.
 * This works much like a restricted case of represent_fsp_con.
 * 
 * @param diagr     the diagram containing the polygon.
 * @param p_idx     the index of the polygon on this side of the singlet.
 * @param s_idx     the index of the polygon on the other side of the singlet.
 * @param master    master[p] is true iff polygon p is contained in a part that
 *                  is a master of the part containing polygon p_idx.
 * @return  a newly allocated comprep representing the connection across the 
 *          singlet.
 */
comprep* represent_singlet(const diagram* diagr, size_t p_idx, size_t s_idx, 
        int* master){
    
    comprep* singlet = salloc(sizeof(comprep));
    singlet->nreps = 1;
    singlet->eq_reps = scalloc(1, sizeof(size_t));
    singlet->reps = salloc(1 * sizeof(cycrep*));
    
    if(master[s_idx])
        singlet->reps[0] = NULL;
    else{
        int* m = salloc(diagr->npolys * sizeof(int));
        memcpy(m, master, diagr->npolys * sizeof(int));
        m[p_idx] = TRUE;
        singlet->reps[0] = represent_part(diagr, NULL, s_idx, m);
        free(m);
    }
    
    return singlet;
}

/**
 * Computes the distance along the perimeter of a part, not counting flavour 
 * split edges, between two adjacent points on a polygon.
 * 
 * @param diagr the diagram containing the polygon.
 * @param p_idx the index of the polygon.
 * @param g_idx the distance will be taken between the gon of this index and
 *              its successor.
 * @return the positive distance.
 */
size_t _actual_dist(const diagram* diagr, size_t p_idx, size_t g_idx){
    size_t dist = 0;
    
    polygon* poly = diagr->polys[p_idx];
    gon_t gon, target = poly->gons[(g_idx + 1) % poly->ngons];
    
    for(;;){
        switch(poly->edges[g_idx].type){
            case SINGLET:
            case EXT_LEG:
                dist++;
                /* Intentional fall-through */
            case FLSPLIT:
                g_idx = (g_idx + 1) % poly->ngons;
                
                break;
                
            case PROPGTR: 
                                
                gon = poly->gons[g_idx];
                p_idx = poly->edges[g_idx].idx;
                poly = diagr->polys[p_idx];
                
                for(g_idx = 0; poly->gons[g_idx] != gon; g_idx++);
                
                break;
             
        }
        
        if(poly->gons[g_idx] == target)
            return dist;
    }
}
