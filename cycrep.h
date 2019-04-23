/* 
 * File:   cycrep.h
 * Author: Mattias
 *
 * Created on 13 February 2019, 17:53
 * 
 * A cycrep (cyclic representation) is a multi-layered string of characters
 * that represents a polygon diagram in a way that is compact, unique, and
 * carries the cyclic symmetry granted to it by flavour ordering. These are
 * used to compare diagrams when searching for symmetry factors and when
 * eliminating cyclically equivalent diagrams.
 * 
 * Each character in a cycrep corresponds to a gon of the diagram, in cyclic 
 * order around its perimeter. When flavour splittings are present, the diagram
 * is separated into several smaller ones, with the cut edges collapsing into
 * single gons.
 * 
 * A cycrep consists of three levels: the first level uniquely encodes the 
 * topology of the diagram (which legs and propagators go to which vertex); the 
 * second level encodes which order each vertex has; and the third and most
 * complex level encodes how the flavour splittings are laid out. When the third
 * level is used, the cycrep is split into several parts gathered into a 
 * compound cycrep, interlinked by the third level.
 * 
 * A cycrep always exists in its lexicographically least form (LLF) among all 
 * forms allowed by its symmetries. Lexicographic order is determined first by 
 * length (very simple to compare), then the first layer (simple), the second 
 * layer (equally simple) and finally the third layer (very complicated). Thus, 
 * a more complicated layer is only invoked when absolutely needed. Likewise, 
 * periodicity under cyclings is determined from the layers in order, with lower
 * layers having the ability to break symmetries of upper ones. Since it is rare
 * to have symmetry in both the first and second layer, the third layer is
 * seldom consulted. 
 */

#ifndef CYCREP_H
#define	CYCREP_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "fodge.h"
#include "info_sort.h"

    /** The levels of a cycrep. */
    typedef enum rep_level_enum {
        TOP_LEVEL = 1 << 0, 
        ORD_LEVEL = 1 << 1, 
        FSP_LEVEL = 1 << 2,
        NO_LEVEL  = 0,
        ALL_LEVELS = TOP_LEVEL | ORD_LEVEL | FSP_LEVEL
    } rep_level;
    
     typedef struct{
        size_t len;
        size_t ord;
        comprep* con;
        
        size_t p_idx_in_diagr;
    } linerep;
                
    /** Holds the multi-tiered information for each gon or collapsed flavour
     *  split. */
    typedef struct{
        /** The number of lines emerging from this gon. This is the length
         *  of line_len, line_ord and line_con. */
        size_t nlines;
        linerep* lines;
        
        size_t g_idx_in_diagr;
    } gonrep;   
    
    /** A cycrep representing a single cyclic diagram, or a flavour-split part 
     * thereof. */
    typedef struct {
        /** The number of gons. */
        size_t length;
        /** The number of gons that carry a flavour index, i.e. not flavour
         *  splits or singlets. */
        size_t n_flavidx;
        /** The representations of each gon. */
        gonrep* array;
        
        /** The amount of cyclic offset to apply when reading the array in order
         *  for it to assume its lexicographically least form. */
        size_t offset;
        /** The period of the cycrep, i.e. the smallest amount of cyclic offset
         *  that leaves the array equal at all orders. */
        size_t period;
        
        /** Contains the flavour-split connections to the represented part.
         *  This list just holds the objects so that they can be deleted later,
         *  since ownership is otherwise ill-defined. */
        comprep** fsp_cons;
        /** The length of fsp_cons. */
        size_t n_cons;
    } cycrep;
        
    cycrep* copy_cycrep(const cycrep* orig);
    
    void normalise_cycrep(cycrep* rep, rep_level level);
       
    int compare_cycrep(const cycrep* rep_1, const cycrep* rep_2, 
            rep_level level);
    int compare_self(const cycrep* rep, size_t idx_1, size_t idx_2, 
            size_t limit, rep_level level, int anti_doublecount);
    
    void print_cycrep(const cycrep* rep, const diagram* diagr);
    
    void delete_cycrep(cycrep* crep);
    
    /** 
     * A compound cycrep holds several interlinked cycreps, together 
     * representing the full complexity of a diagram.
     */
    struct compound_cycrep_struct {
        /** The number of cycreps. */
        size_t nreps;
        
        /** A lexicographically ordered list of cycreps, the components of the
         *  compound. */
        cycrep** reps;
        
        /** Two elements of this array are equal if and only if the 
         *  corresponding elements of reps are equal, thereby avoiding expensive
         *  cycrep comparisons beyond those necessary for sorting reps. */
        size_t* eq_reps;
        
        /** For each polygon index p in the represented diagram, poly_reps[i]
         *  represents the part that contains the polygon p. */
        cycrep** poly_reps;
    };
    
    comprep* copy_comprep(const comprep* orig);
    
    comprep* represent_diagram(const diagram* diagr);
    comprep* represent_fsp_con(const diagram* diagr, size_t p_idx, 
            int* master);
    comprep* represent_singlet(const diagram* diagr, size_t p_idx, size_t s_idx,
            int* master);
    cycrep* represent_part(const diagram* diagr, cycrep** poly_reps, 
            size_t p_idx, int* master);
    
    cycrep* init_part(const diagram* diagr, size_t p_idx, cycrep** poly_reps,
            int* master);
    void fill_part(const diagram* diagr, size_t p_idx, cycrep* part, 
            int* master);
    
    size_t _actual_dist(const diagram* diagr, size_t p_idx, size_t g_idx);
    
    size_t get_symmetry(const comprep* crep);
    
    int compare_comprep(const comprep* crep_1, 
            const comprep* crep_2);
    
    void print_comprep(const comprep* crep, const diagram* diagr);

    void delete_comprep(comprep* crep);

#ifdef	__cplusplus
}
#endif

#endif	/* CYCREP_H */

