
#include "fodge.h"

polygon* make_polygon(const gon_t* gons, const poly_ptr* edges, 
        size_t ngons, size_t order){
    polygon* poly = salloc(sizeof(polygon));
    poly->ngons = ngons;
    poly->order = order;
    poly->split_budget = order;
    
    if(gons){
        poly->gons = salloc(ngons * sizeof(gon_t));
        memcpy(poly->gons, gons, ngons * sizeof(gon_t));
    }
    else 
        poly->gons = scalloc(ngons, sizeof(gon_t));
    
    if(edges){
        poly->edges = salloc(ngons * sizeof(poly_ptr));
        memcpy(poly->edges, edges, ngons * sizeof(poly_ptr));
    }
    else
        poly->edges = scalloc(ngons, sizeof(poly_ptr));
    
    
    return poly;
}

polygon** copy_polygons(polygon* const* orig, polygon* add, size_t npolys){
    size_t np = add ? npolys + 1 : npolys;
    
    polygon** copy = salloc(np * sizeof(polygon*));
    for(size_t i = 0; i < npolys; i++)
        copy[i] = copy_polygon(orig[i]);
        
    if(add)
        copy[np - 1] = add;
    
    return copy;
}

polygon* copy_polygon(const polygon* orig){
    if(!orig)
        return NULL;
        
    polygon* copy = malloc(sizeof(polygon));
    
    copy->ngons = orig->ngons;
    copy->order = orig->order;
    copy->split_budget = orig->split_budget;
    
    copy->gons = salloc(copy->ngons * sizeof(gon_t));
    memcpy(copy->gons, orig->gons, copy->ngons * sizeof(gon_t));
    
    copy->edges = salloc(copy->ngons * sizeof(poly_ptr));
    memcpy(copy->edges, orig->edges, copy->ngons * sizeof(poly_ptr));
    
    return copy;
}

void print_polygon(const diagram* diagr, size_t p_idx){
    
    polygon* poly = diagr->polys[p_idx];
    
    uint w = decimal_width(diagr->npolys - 1);
    
    printf("%zd[%zd:%zd]:(", p_idx, poly->order, poly->split_budget);
    for(size_t i = 0; i < poly->ngons; i++){
        printf("%zd", diagr->gon_idx[poly->gons[i]]);
        switch(poly->edges[i].type){
            case EXT_LEG:
                printf(".%*c - ", w, '.');
                break;
            case PROPGTR:
                printf("p%*zd - ", w, poly->edges[i].idx);
                break;
            case FLSPLIT:
                printf("f%*zd - ", w, poly->edges[i].idx);
                break;
            case SINGLET:
                printf("s%*zd - ", w, poly->edges[i].idx);
        }
    }
    printf("%zd)", diagr->gon_idx[poly->gons[0]]);
}

void delete_polygons(polygon** polys, size_t npolys){   
    for(size_t i = 0; i < npolys; i++){
        free(polys[i]->gons);
        free(polys[i]->edges);
        free(polys[i]);
    }
        
    free(polys);
}