/* 
 * File:   fodge.h
 * Author: Mattias
 *
 * Created on 11 February 2019, 14:30
 */

#ifndef FODGE_H
#define	FODGE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "fodge_util.hpp"
#include "cycrep.hpp"
    
    enum poly_ptr_type {
        EXT_LEG = 0, 
        PROPGTR = 1 << 0, 
        SINGLET = 1 << 1, 
        FLSPLIT = 1 << 2
    };
    
#define CARRIES_FLAVIDX(e) (e.type == EXT_LEG || e.type == PROPGTR)
    
    typedef struct{
        size_t idx;
        enum poly_ptr_type type;
    } poly_ptr;
    
    struct polygon_struct {
        size_t ngons;
        size_t order;
        size_t split_budget;
        
        gon_t* gons;
        poly_ptr* edges;
    };
    
    polygon* make_polygon(const gon_t* gons, const poly_ptr* edges, 
            size_t ngons, size_t order);
    polygon* copy_polygon(const polygon* orig);
    polygon** copy_polygons(polygon* const* orig, polygon* add, 
            size_t npolys);
        
    void print_polygon(const diagram* diagr, size_t p_idx);
    
    void delete_polygons(polygon** polys, size_t npolys);
    
    struct diagram_struct {
        size_t ngons;
        size_t order;
        size_t sym;
        
        gon_t* gons;
        size_t* gon_idx;
        size_t* edges;
        
        size_t npolys;
        polygon** polys;
        
        comprep* rep;
        
        diagram* next; 
        
        size_t diagram_id;
    };
    
    diagram* make_contact_diagram(size_t ngons, size_t order);
    diagram* copy_diagram(const diagram* orig, int recursive);
    
    diagram* grow_diagrams(const diagram* base, size_t ngons, size_t order);
    diagram* split_diagrams(diagram* base);
    diagram* singlet_diagrams(diagram* base);
    diagram* remove_zero_fsp(diagram* list);
    
    int compare_diagrams(diagram* diagr_1, diagram* diagr_2);
    diagram* insert_diagram(diagram* list, diagram* diagr);
    diagram* merge_diagrams(diagram* list_1, diagram* list_2);
    
    void count_diagrams(const diagram* list, int count_detail);
    size_t get_n_flavidx(const diagram* diagr, size_t idx);
    void print_diagram(const diagram* diagr, int recursive);
        
    diagram* delete_diagram(diagram* diagr, int recursive);
    
    typedef struct {
        size_t max_ngons;
        size_t max_order;
        
        diagram*** table;
    } diagram_table; 
    
    enum fill_level{
        NO_FILL, MIN_FILL, MAX_FILL
    };
    diagram_table* make_table(size_t max_ngons, size_t max_order, int split, int singlet, 
            enum fill_level fill);
    diagram* get_diagram(diagram_table* tab, size_t ngons, size_t order, 
            size_t index);
    
    void print_table(diagram_table* tab);
    void count_table(diagram_table* tab, int count_detail);
    
    void delete_table(diagram_table* tab);

#ifdef	__cplusplus
}
#endif

#endif	/* FODGE_H */

