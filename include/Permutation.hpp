/* 
 * File:   Permutation.hpp
 * Author: Mattias
 *
 * Created on 02 July 2019, 10:34
 */

#ifndef PERMUTATION_H
#define	PERMUTATION_H

#include <cstddef>
#include <cassert>

#include <vector>
#include <numeric>
#include <algorithm>
#include <iostream>

namespace permute{

/**
 * @brief Class for representing permutations of objects. 
 * 
 * A permutation wraps an array of integer indices such that all indices between 
 * 0 and the maximum index are represented exactly once. The permutation only
 * supports modifications to itself that preserve this property.
 * 
 * The action of the permutation on a list of objects is to map the ith object 
 * to the jth one, where j is the ith index in the array.
 */
class Permutation {
public:    
    Permutation(size_t size = 1);
    Permutation(const Permutation& orig) = default;
    virtual ~Permutation() = default;
    
    Permutation(std::initializer_list<size_t> list);
    
    /**
     * @brief Constructs a permutation from a range of integers.
     * 
     * The range must represent a valid permutation as per 
     * @link is_permutation . The values in the range are left as-is and are
     * independent of the created permutation.
     * 
     * @tparam ForwardIt    a forward iterator whose @c value_type can be 
     *                      implicitly converted to @c size_t . 
     * @param begin an iterator to the beginning of the range.
     * @param end   an iterator past the end of the range.
     */
    template<class ForwardIt>
    Permutation(ForwardIt begin, ForwardIt end) : map() {
        assert(is_permutation(begin, end));
        map.reserve(std::distance(begin, end));
        for(auto it = begin; it != end; ++it)
            map.push_back(*it);
    }
    
    static Permutation identity(size_t size);
    static Permutation cyclic(size_t size, size_t coffs);
    
    /**
     * @brief Checks if a range of integers would constitute a valid 
     * permutation.
     * 
     * To be a valid permutation, the range must contain all values between 0
     * and its greatest value exactly once.
     * 
     * @tparam ForwardIt    a forward iterator whose @c value_type is an integer
     *                      type.
     * @param begin an iterator to the beginning of the range.
     * @param end   an iterator past the end of the range.
     */
    template<typename ForwardIt>
    static bool is_permutation(ForwardIt begin, ForwardIt end){
        size_t size = std::distance(begin, end);
        auto visited = std::vector<bool>(size, false);
        
        for(auto it = begin; it != end; it++){
            if(*it < 0 || *it >= size || visited[*it])
                return false;
            
            visited[*it] = true;
        }
        
        return true;
    }
    
    Permutation reverse() const;
    Permutation inverse() const;
    
    /**
     * Retrieves the size of the permutation, i.e. how many objects it permutes.
     * @return the size.
     */
    size_t size() const { return map.size();  }
    size_t order() const;
      
    bool is_identity() const;
    int parity() const;
    
    std::vector<size_t> cycle_type() const; 
    std::vector<size_t> fixed_points() const;
    
    typedef std::vector<size_t>::const_iterator            iterator;
    typedef std::vector<size_t>::const_reverse_iterator    reverse_iterator;
    
    /**
     * Iterator to the beginning of the permutation.
     * @return a const iterator.
     */
    iterator begin() const          {   return map.cbegin();   }
    /**
     * Iterator to the end of the permutation.
     * @return a const iterator.
     */
    iterator end() const            {   return map.cend();     }
    /**
     * Reverse iterator to the beginning of the permutation.
     * @return a const iterator.
     */
    reverse_iterator rbegin() const {   return map.crbegin();  }
    /**
     * Reverse iterator to the end of the permutation.
     * @return a const iterator.
     */
    reverse_iterator rend() const   {   return map.crend();    }
    
    /**
     * The first index in the permutation.
     * @return a copy of the index.
     */
    size_t front() const            {   return map.front();    }
    /**
     * The last index in the permutation.
     * @return a copy of the index.
     */
    size_t back() const             {   return map.back();     }
    /**
     * Retrieves an index in the permutation.
     * @param i a value in the range [0, size() ).
     * @return a copy of the ith index.
     */
    size_t operator[] (size_t i) const { return map.at(i);     }
    
    /**
     * @brief Applies a permutation to a collection of objects.
     * 
     * The permutation is performed in-place, using @c std::swap or 
     * @c std::swap_ranges to perform the permutation. The collection must have
     * at least @code offset + block_len * size() @endcode elements.
     * 
     * @tparam RandAccIt a random access iterator type.
     * @param iter      an iterator to the collection. 
     * @param offset    If provided, @p iter will be offset by this much before
     *                  the permutation is applied.
     * @param block_len If provided, the permutation will be applied to blocks
     *                  of this many objects rather than to individual objects.
     * @return  @p iter , which points to the same location in the modified
     *          collection.
     */
    template<class RandAccIt>
    RandAccIt permute (
            RandAccIt iter, 
            size_t offset = 0, size_t block_len = 1) const
    {
        auto it = iter + offset;
        for(size_t i = 0, j; i < map.size(); i++){
            j = map[i];
            while(j < i)
                j = map[j];
            
            if(block_len == 1)
                std::swap(it[i], it[j]);
            else
                std::swap_ranges(
                        it + (i * block_len), 
                        it + ((i+1) * block_len),
                        it + (j * block_len)
                );
        }
        
        return iter;
    } 
    /**
     * @brief Applies a permutation to the bits in a binary number.
     * 
     * This works like applying a permutation to a collection of objects,
     * with the bits in an integer representing the objects. The least
     * significant bit is treated as the first object in the collection.
     * If the type is smaller than required by the method 
     * (i.e. fewer than @code offset + block_len * size() @endcode bits),
     * the input will be treated as extended with zeroes and no error will 
     * occur.
     * 
     * @tparam B a binary integer type.
     * 
     * @param bits      the number whose bits are to be permuted. It is passed
     *                  by value, so the original is untouched.
     * @param offset    If provided, the permutation will be applied starting
     *                  at this many bits above the least significant one.
     *                  If @p block_len is also provided, the offset will be by
     *                  this many blocks.
     * @param block_len If provided, the permutation will be applied to blocks
     *                  of this many bits rather than to individual bits.
     * @return  the permuted number.
     */
    template<typename B>
    B permute_bits (B bits, size_t offset = 0, size_t block_len = 1) const {
        B one = (B) 1;
        B mask = one << block_len - one;
        
        B res = bits & ((one << offset) - one);
        bits >>= offset * block_len;
        for(size_t idx = 0; bits; bits >>= block_len, idx++){
            res |= (bits & mask) << (map[idx] + offset) * block_len;
        }
                
        return res;
    }
    
    Permutation& permute (Permutation& p, 
            size_t offset = 0, size_t block_len = 1) const;
    
    
    Permutation& swap(size_t i, size_t j);
        
    friend Permutation operator* (const Permutation& p1, const Permutation& p2);
    friend Permutation& operator*= (Permutation& p1, const Permutation& p2);
    
    friend Permutation operator^ (const Permutation& p, size_t pow);
    friend Permutation& operator^= (Permutation& p, size_t pow);
    
    friend Permutation operator% (const Permutation& p1, const Permutation& p2);
    friend Permutation& operator%= (Permutation& p1, const Permutation& p2);
    
    friend bool operator== (const Permutation& p1, const Permutation& p2);
    friend bool operator!= (const Permutation& p1, const Permutation& p2);
    
    friend std::ostream& operator<< (std::ostream& out, const Permutation& p);
    
private:
    std::vector<size_t> map;
    
    Permutation(std::vector<size_t>& perm);
};

}

#endif	/* PERMUTATION_H */

