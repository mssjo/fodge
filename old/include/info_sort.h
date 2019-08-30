/* 
 * File:   info_sort.h
 * Author: Mattias Sjoe
 *
 * Created on 22 February 2019, 11:09
 */

#ifndef INFO_SORT_H
#define	INFO_SORT_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "fodge_util.hpp"

#ifdef	__cplusplus
extern "C" {
#endif

    /**
     * A more informative counterpart to the standard qsort. In tandem with the
     * sort, it provides information about the sorted array and how it relates
     * to the unsorted input. This is useful if the comparison operation is very
     * expensive, since it removes the need to do any comparisons beyond those
     * necessary for sorting. 
     * 
     * The extra information arrays (whence, whither, rank, rep) must be large 
     * enough to contain nitems elements of size size, and will be filled in 
     * during sorting. If some are left NULL, they (but not the rest) are 
     * ignored.
     * 
     * The sort uses mergesort, since this allows rank to be conveniently 
     * filled. It therefore runs in O(N log N) time with O(N) auxiliary storage
     * (N == nitems) and is stable (which ironic, since whence/whither removes
     * the need for stability).
     * 
     * @param list      a pointer to the list to be sorted.
     * @param nitems    the number of items in the list.
     * @param size      the size (in bytes) of each item. 
     * @param compare   the comparison function to be used for the sorting.
     * @param whence    if provided, whence[i] will be the index that the ith
     *                  item in the SORTED list had in the UNSORTED list.      
     * @param whither   if provided, whither[i] will be the index that the ith
     *                  item in the UNSORTED list gets in the SORTED list.
     *                  The sorted and unsorted arrays are connected through
     *                  whence and whither, and they are each other's inverses:
     *                  whence[whither[i]] == i and whither[whence[i]] == i.
     * @param rank      if provided, it will be filled with numbers starting
     *                  from zero. If two elements in the sorted list are equal,
     *                  the corresponding elements of rank will also be equal;
     *                  otherwise, they increase in steps of 1. Thus, stretches
     *                  of equal elements can be read form this array without
     *                  the need for further comparisons.
     * @param unique    if provided, unique[i] will be the lowest number such 
     *                  that rank[unique[i]] equals rank[i] under the comparison 
     *                  function; thus, unique points to a unique representative 
     *                  of each stretch of equal elements.
     */
    void info_sort(void* list, size_t nitems, size_t size, 
            int (*compare)(const void*, const void*), 
            size_t* whence, size_t* whither, size_t* rank, size_t* unique);
    
    /**
     * Permutes an array according to a permutation array. If the permutation
     * array was generated as whither in a call to info_sort, the effect will be
     * to rearrange the array in the same way as the array that was sorted.
     * 
     * @param list      the array.
     * @param nitems    the number of items in list and perm.
     * @param size      the size (in bytes) of each item in list.
     * @param perm      the permutation array: list[i] will be mapped to
     *                  list[perm[i]], with list viewed as having elements of
     *                  size size.
     */
    void permute(void* list, size_t nitems, size_t size, const size_t* perm);


#ifdef	__cplusplus
}
#endif

#endif	/* INFO_SORT_H */

