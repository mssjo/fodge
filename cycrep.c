/* This file contains the main implementation of cycrep.h 
 * More can be found in represent.c
 *
 * Author: Mattias Sjoe
 */


#include "cycrep.h"
#include "fodge.h"

#define BOOTH_NIL -1
/**
 * Finds the lexicographically least form of a cycrep by a modified version of
 * Booth's algorithm. With no information present, the ordinary algorithm is
 * used. When a higher level gives a unique LLF, no more normalisation is 
 * needed. When a higher level is periodic, the lower level can decrease the
 * resulting degeneracy of the LLF, but the algorithm only needs to look at each
 * period, not at each character.
 * 
 * @param rep   the cycrep
 * @param level the level to look at - all previous levels must already have
 *              been visited!
 */
size_t _booth_normalise(const cycrep* rep, rep_level level){
    if(rep->period == rep->length)
        return rep->offset;
    
    // The step determines the resolution of the algorithm
    size_t step = rep->period ? rep->period : 1;
    
    // Failure function - see Booth's description
    int* ffunc = salloc(2*rep->length/step * sizeof(int));
    ffunc[0] = BOOTH_NIL;
    
    // noffs is the current best estimate of the final offset
    size_t noffs = 0;
    // fval - ffunc value, comp - comparison result (comparisons are expensive!)
    int fval;
    int comp;
    
    // Runs the algorithm. The cycrep is traversed in chunks given by step and
    // offset by the offset given by the previous layer.
    // The failure function has one element for each such chunk, hence frequent
    // rescaling by step one way or the other. 
    // It is simpler this way, I promise!
    for(size_t idx = step; idx < 2*rep->length; idx += step){
        fval = ffunc[(idx - noffs)/step - 1];
        comp = compare_self(rep, idx, (1 + fval)*step + noffs, step, level, 
                FALSE);
        
        while(fval != BOOTH_NIL && comp != 0){
            if(comp < 0)
                noffs = idx - (1 + fval)*step;
            fval = ffunc[fval];
            
            comp = compare_self(rep, idx, (1 + fval)*step + noffs, step, level, 
                    FALSE);
        }
        if(fval == BOOTH_NIL && comp != 0){
            if(comp < 0)
                noffs = idx;
            
            ffunc[(idx - noffs)/step] = BOOTH_NIL;
        }
        else
            ffunc[(idx - noffs)/step] = 1 + fval;
    }
    
    return rep->offset + noffs;
}

/**
 * Determines the period of the cycrep. If a previous period exists, the new one
 * can only be a multiple of that.
 * @param rep   the cycrep
 * @param level the level to look at - all previous levels must already have
 *              been visited!
 * @return  the period at this level
 */
size_t _find_period(const cycrep* rep, rep_level level){
    size_t limit = rep->period ? rep->period : rep->length;
    size_t step  = rep->period ? rep->period : 1;
    
    int comp;
    for(
            size_t period = step; 
            period <= rep->length/2; 
            period += step)
    {
        if(rep->length % period)
            continue;
        
        /* anti_doublecount activated to prevent symmetry excess */
        comp = compare_self(rep, 0, period, limit, level, TRUE);
        if(comp == 0)
            return period;
    }
    
    return rep->length;
}

/**
 * Normalises a cycrep by determining its period and offset.
 * 
 * @param rep   the cycrep - its contents will be updated by this method
 * @param level the level to look at - all previous levels must already have
 *              been visited!
 */
void normalise_cycrep(cycrep* rep, rep_level level){
    if(!rep)
        return;
    
    rep->offset = _booth_normalise(rep, level);
    rep->period = _find_period    (rep, level);
}

#define ARROFFS(r,i)    r->array[(i + r->offset) % r->length]

/**
 * Compares two cycreps lexicographically. This is a more expensive operation 
 * the more similar they are. 
 * 
 * The comparison is done entirely for each level before moving on to
 * the next in case of equality. If the cycreps are of inequal length, the
 * shorter is considered less regardless of other content.
 * 
 * @param rep_1 one cycrep
 * @param rep_2 the other cycrep
 * @param level the level (or levels; use bitwise or) to look at
 * @return  a negative value if rep_1 is less than rep_2, a positive value if
 *          rep_1 is greater than rep_2, or 0 if they are equal.
 */
int compare_cycrep(const cycrep* rep_1, const cycrep* rep_2, 
        rep_level level){
        
    if(!rep_1){
        return rep_2 ? +1 : 0;
    }
    if(!rep_2)
        return -1;
        
            
    int comp;
    
    /* Descending order wrt n_flavidx to avoid disturbances from
     *  zero-index parts; 
     * ascending order otherwise for efficiency in comparisons. */
    if(!rep_1->n_flavidx)
        return rep_2->n_flavidx ? +1 : 0;
    if(!rep_2->n_flavidx)
        return -1;
    
    if((comp = COMPARE(rep_1->n_flavidx, rep_2->n_flavidx)))
        return comp;
    if((comp = COMPARE(rep_1->length, rep_2->length)))
        return comp;
        
    for(size_t i = 0; i < rep_1->length; i++){
        comp = COMPARE(ARROFFS(rep_1, i).nlines, ARROFFS(rep_2, i).nlines);
        if(comp)
            return comp;
        
        for(size_t j = 0; j < ARROFFS(rep_1, i).nlines; j++){
            if(level & TOP_LEVEL){
                comp = COMPARE(
                        ARROFFS(rep_1, i).lines[j].len, 
                        ARROFFS(rep_2, i).lines[j].len);
                if(comp)
                    return comp;
            }
            if(level & ORD_LEVEL){
                comp = COMPARE(
                        ARROFFS(rep_1, i).lines[j].ord, 
                        ARROFFS(rep_2, i).lines[j].ord);
                if(comp)
                    return comp;
            }
            if(level & FSP_LEVEL){
                comp = compare_comprep(
                        ARROFFS(rep_1, i).lines[j].con, 
                        ARROFFS(rep_2, i).lines[j].con);
                if(comp)
                    return comp;
            }
        }
    }    
    
    return 0;
}

/**
 * Is similar to compare_cycrep, but instead compares two (possibly overlapping)
 * segments of the same cycrep. Is used to find periods and normalisation.
 * 
 * @param rep   the cycrep
 * @param idx_1 the start of the first segment (counting in the LLF)
 * @param idx_2 the start of the second segment (counting in the LLF)
 * @param seg_length  the length of the segments - elements outside this are not
 *              compared
 * @param level the level to look at - all previous levels must already have
 *              been visited!
 * @param anti_doublecount  Somtimes, symmetries may be double-counted if they
 *                          exist thanks to identical connections: both symmetry
 *                          of a part and symmetry under exchange of the
 *                          connected parts is counted. Setting anti_doublecount
 *                          to TRUE prevents this by treating equal but distinct
 *                          connections as different at FSP_LEVEL.
 * @return  a negative value if the first segment is less than the second, a 
 *          positive value if the first is greater than the second, or 0 if 
 *          they are equal.
 */
int compare_self(const cycrep* rep, size_t idx_1, size_t idx_2, 
        size_t seg_length, rep_level level, int anti_doublecount){
    
    int comp;
    for(size_t i = 0; i < seg_length; i++){
        comp = COMPARE(
                ARROFFS(rep, i+idx_1).nlines, 
                ARROFFS(rep, i+idx_2).nlines);
        if(comp)
            return comp;
        
        for(size_t j = 0; j < ARROFFS(rep, i+idx_1).nlines; j++){
            if(level & TOP_LEVEL){
                comp = COMPARE(
                        ARROFFS(rep, i+idx_1).lines[j].len, 
                        ARROFFS(rep, i+idx_2).lines[j].len);
                if(comp)
                    return comp;
            }
            if(level & ORD_LEVEL){
                comp = COMPARE(
                        ARROFFS(rep, i+idx_1).lines[j].ord, 
                        ARROFFS(rep, i+idx_2).lines[j].ord);
                if(comp)
                    return comp;
            }
            if(level & FSP_LEVEL){
                if(anti_doublecount){
                    comp = COMPARE(
                            (size_t) ARROFFS(rep, i+idx_1).lines[j].con, 
                            (size_t) ARROFFS(rep, i+idx_2).lines[j].con);
                }
                else{
                    comp = compare_comprep(
                            ARROFFS(rep, i+idx_1).lines[j].con, 
                            ARROFFS(rep, i+idx_2).lines[j].con);
                }
                if(comp)
                    return comp;
            }
        }
    }
    
    return 0;
}

/**
 * Prints a cycrep. 
 * 
 * @param rep   the cycrep
 * @param width the number of characters to use for each element in the rows
 * @param level the level or levels (use bitwise or) to be printed
 */
void print_cycrep(const cycrep* rep, const diagram* diagr){
    if(!rep){
        idprintf("[null]\n");
        return;
    }
    
    for(size_t i = 0; i < rep->length; i++){
        idprintf("gon %*zd:\n", decimal_width(rep->length-1), i);
        MORE_INDENT;
        
        for(size_t j = 0; j < ARROFFS(rep, i).nlines; j++){
            idprintf("line %*zd: ", decimal_width(ARROFFS(rep, i).nlines-1), j);
            
            if(ARROFFS(rep, i).lines[j].len > 0){
                printf("%*zd gons down, order %*zd, ", 
                        decimal_width(diagr->ngons-1), 
                        ARROFFS(rep, i).lines[j].len,
                        decimal_width(diagr->order),
                        ARROFFS(rep, i).lines[j].ord);
                
                
                if(ARROFFS(rep, i).lines[j].con){
                    printf("connected to:\n");
                    MORE_INDENT;
                    print_comprep(
                            ARROFFS(rep, i).lines[j].con, 
                            diagr);
                    LESS_INDENT;
                }
                else{
                    printf("no connection.\n");
                }
            }
            else{
                printf("%*s", (int)strlen(" gons down, order , ") 
                        + decimal_width(diagr->ngons-1)
                        + decimal_width(diagr->order), " ");
                printf("singlet-connected to:\n");
                MORE_INDENT;
                print_comprep(
                        ARROFFS(rep, i).lines[j].con, 
                        diagr);
                LESS_INDENT;
            }
            
        }
        LESS_INDENT;
    }
}

/**
 * Deletes a cycrep and its array, freeing up their memory.
 * @param rep   the cycrep
 */
void delete_cycrep(cycrep* rep){
    if(!rep)
        return;
    
    for(size_t i = 0; i < rep->n_cons; i++){
        if(rep->fsp_cons[i])
            delete_comprep(rep->fsp_cons[i]);
    }
    for(size_t i = 0; i < rep->length; i++){
        for(size_t j = 0; j < rep->array[i].nlines; j++){
            if(rep->array[i].lines[j].len == 0)
                delete_comprep(rep->array[i].lines[j].con);
        }
        free(rep->array[i].lines);
    }
        
    free(rep->fsp_cons);
    free(rep->array);
    free(rep);
}

/**
 * Computes the full symmetry factor of a diagram from its cycrep. The symmetry
 * factor is the ratio between the total number of forms of the representation
 * and the number of inequivalent forms.
 * @param crep      a comprep cycrep
 * @param length    the ngons of the corresponding diagram
 * @return  the symmetry factor.
 */
size_t get_symmetry(const comprep* crep){
    int sym = 1;
        
    size_t eq_fact = 1;
    for(size_t i = 0, eq_idx = 0; i < crep->nreps; i++){
        /* If this equality does not hold, there are singlets that break the
         * cyclic symmetry. */
        if(crep->reps[i]->length == crep->reps[i]->n_flavidx)        
            sym *= crep->reps[i]->length / crep->reps[i]->period;
        
        if(crep->eq_reps[eq_idx] == crep->eq_reps[i])
            eq_fact *= 1 + i - eq_idx;
        else{
            sym *= eq_fact;
            eq_idx = i;
            eq_fact = 1;
        }
    }
        
    return sym * eq_fact;
}

/**
 * This is the master comparison function for cyclic representations. It 
 * compares two comprep cycreps first by number of cycreps, and then by each
 * of its component cycreps level by level.
 * @param crep_1    a compund cycrep
 * @param crep_2    another comprep cycrep
 * @return  a positive value if crep_1 is lexicographically greater than crep_2,
 *          a negative value if it is less than crep_2, or zero if they are
 *          identical, i.e. representing equivalent diagrams.
 */
int compare_comprep(const comprep* crep_1, const comprep* crep_2){
    int comp;
    
    if(!crep_1)
        return crep_2 ? +1 : 0;
    if(!crep_2)
        return -1;
        
    comp = COMPARE(crep_1->nreps, crep_1->nreps);
    if(comp)
        return comp;
    
    for(size_t i = 0; i < crep_1->nreps; i++){
        comp = COMPARE(crep_1->eq_reps[i], crep_2->eq_reps[i]);
        if(comp)
            return comp;
    }
    
    
    //TODO? use rank/unique to avoid comparing multiple equal diagrams
    for(size_t i = 0; i < crep_1->nreps; i++){
        comp = compare_cycrep(crep_1->reps[i], crep_2->reps[i], TOP_LEVEL);
        if(comp)
            return comp;
    }
    for(size_t i = 0; i < crep_1->nreps; i++){
        comp = compare_cycrep(crep_1->reps[i], crep_2->reps[i], ORD_LEVEL);
        if(comp)
            return comp;
    }
    for(size_t i = 0; i < crep_1->nreps; i++){
        comp = compare_cycrep(crep_1->reps[i], crep_2->reps[i], FSP_LEVEL);
        if(comp)
            return comp;
    }
    
    return 0;
}


/**
 * Prints a comprep to the standard output. Each level is represented
 * by a line, and separate components are separated by vertical bars. All
 * components are printed sorted and normalised.
 * @param crep  the comprep cycrep to be printed
 * @param width the number of characters to allocate to each element
 */
void print_comprep(const comprep* crep, const diagram* diagr){
    if(!crep){
        idprintf("[null]\n");
        return;
    }
    
    for(size_t i = 0; i < crep->nreps; i++){
        idprintf("Part %*zd:\n", decimal_width(crep->nreps-1), i);
        MORE_INDENT;
        print_cycrep(crep->reps[i], diagr);
        LESS_INDENT;
    }
}

void delete_comprep(comprep* crep){
    if(!crep)
        return;
    
    for(size_t i = 0; i < crep->nreps; i++)
        delete_cycrep(crep->reps[i]);

    free(crep->reps); 
    free(crep->eq_reps);
    free(crep);
}