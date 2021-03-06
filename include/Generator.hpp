/* 
 * File:   Generator.hpp
 * Author: Mattias
 * 
 * Implements group generators as iterators.
 * Definitions implemented in Generator.cpp
 *
 * Created on 27 June 2019, 16:11
 */

#ifndef GENERATOR_H
#define	GENERATOR_H

#include <vector>

#include "Permutation.hpp"

namespace permute{

/**
 * @brief A Generator is an input iterator that produces all elements in a group
 * of permutations. 
 * 
 * This is an abstract class; subclasses implment specific groups
 * and must obey the following properties:
 * <ul>
 *  <li> The Generator is initialised so that dereferencing it yields the
 *      identity permutation, and <tt>operator bool</tt> yields 
 *      @c false .
 *  <li> All permutations in the group are generated exactly once before any
 *      permutation is repeated.
 *  <li> When the entire group has been generated, and only then, should  
 *      <tt>operator bool</tt> yield @c true .
 *  <li> Other than the value of  <tt>operator bool</tt>, the
 *      Generator should otherwise be in the same state after traversing the
 *      entire group as it was immediately after initialisation. This allows
 *      for multi-pass use of a Generator.  <tt>operator bool</tt>
 *      shall be set to @c false upon incrementation.
 * </ul>
 */
class Generator 
: public std::iterator<std::input_iterator_tag, Permutation> 
{

public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Permutation;
    using reference = value_type const&;
    using pointer = value_type const*;
    using difference_type = ptrdiff_t;

    Generator(int n);

    /**
     * @brief Converts the Generator to a Boolean value indicating whether it
     * has completed a traversal of its group.
     * @return @c true if and only if the Generator has completed a traversal
     *          and is pointing to the identity permutation.
     */
    explicit operator bool() const  {   return !done;   }

    /**
     * @brief Dereferences the Generator.
     * @return a const reference to the current permutation.
     */
    reference operator*() const     {   return perm;    }
    /**
     * @brief Dereferences the Generator though a pointer.
     * @return a const pointer to the current permutation.
     */
    pointer operator->() const      {   return &perm;   }
    
    virtual Generator& operator++() = 0;

protected:
    /** The return value of <tt>operator bool</tt> */
    bool done;
    /** The current permutation */
    Permutation perm;
};

/**
 * @brief A Generator that generates the cyclic group @f$ Z_n @f$ 
 *      of @p n objects.
 */
class Zn_Generator : public Generator {

public:
    Zn_Generator(int n = 1);
    Zn_Generator(const Zn_Generator& other) = default;
    virtual ~Zn_Generator() = default;

    Zn_Generator& operator++();

private:
    /** The order of the group */
    int n;
    /** The number of steps taken so far; when <tt> count == n </tt>, 
     * the traversal is complete. */
    int count;    
};

/**
 * @brief A Generator that generates the full permutation group 
 * @f$ S_n @f$ of @p n objects. 
 * 
 * It employs the non-recursive Heap's algorithm, which
 * efficiently traverses the set of permutations in such a way that each
 * permutation differs from its predecessor by a single index exchange.
 */
class Sn_Generator : public Generator {

public:
    Sn_Generator(int n = 1);
    Sn_Generator(const Sn_Generator& other) = default;
    virtual ~Sn_Generator() = default;

    Sn_Generator& operator++();

private:
    int n;
    
    /** A stack of counters used in the non-recursive form of Heap's algorithm.*/
    std::vector<int> ctr_stack;
    /** The current location in the stack. */
    int stack_idx;

};

/**
 * @brief A Generator that generates the group @f$   Z_R @f$, 
 * where @f$ R @f$ is an ordered sequence of integers. 
 * 
 * This is the symmetry group of a product of traces,
 * where the ith trace contains a number of matrices equal to the ith element
 * in @f$ R @f$, under permutations of the set of matrices. The group 
 * combines cyclic permutations within each trace with block-wise exchanges of 
 * the contents of traces that contain equally many matrices.
 */
class ZR_Generator : public Generator {

public:
    ZR_Generator(const std::vector<int>& R);
    ZR_Generator(const std::initializer_list<int>& R);
    ZR_Generator(const ZR_Generator& other) = default;
    virtual ~ZR_Generator() = default;

    ZR_Generator& operator++();

private:     
    /** Lists the cyclic groups associated with all traces (first)
     * and the index at which that trace begins (second) */
    std::vector<std::pair<Zn_Generator, int>> cycl;
    /** Lists the permutation groups associated with all block-wise swaps 
     * (first), the index at which the first block starts (second.first),
     * and the size of the blocks (second.second) */
    std::vector<std::pair<Sn_Generator, std::pair<int, int>>> swap;
};

}

#endif	/* GENERATOR_H */

