/* 
 * File:   Permutation.cpp
 * Author: Mattias
 * 
 * Created on 02 July 2019, 10:34
 */

#include <numeric>

#include "Permutation.hpp"

namespace permute{

/**
 * @brief Generates the identity permutation.
 * @param size  the size of the permutation. Must be greater than zero.
 */
Permutation::Permutation(size_t size) : map(size) {
    assert(size > 0);
    std::iota(map.begin(), map.end(), 0);
}

/**
 * @brief Generates a permutation from an initialiser list.
 * @param il the initialiser list.
 * 
 * The initialiser list must contain a valid permutation as per
 * @link Permutation::is_permutation @endlink. No checks are made
 * here to verify that.
 */
Permutation::Permutation(std::initializer_list<size_t> il) : map(il) 
{}

/**
 * @brief Private conversion constructor.
 * @param perm  the internal representation of the permutation.
 */
Permutation::Permutation(std::vector<size_t>& perm) : map(perm) {};

/**
 * @brief Creates an identity permutation. 
 * 
 * This functionality is also provided by a
 * constructor, but is given as a named function for clarity.
 * 
 * @param size  the size of the permutation. Must be greater than 0.
 * @return  the identity permutation.
 */
Permutation Permutation::identity(size_t size){
    return Permutation(size);
}
/**
 * @brief Creates a cyclic permutation.
 * 
 * @param size  the size of the permutation. Must be greater than 0.
 * @param coffs the cyclic offset: 1 for a one-step left-rotation, etc.
 * @return  the cyclic permutation.
 */
Permutation Permutation::cyclic(size_t size, size_t coffs){
    assert(size > 0);
    
    auto perm = std::vector<size_t>(size);
    for(size_t i = 0; i < size; i++)
        perm[i] = (i + coffs) % size;
    return Permutation(perm);
}

/**
 * @brief Constructs the reverse of a permutation.
 * @return a permutation that reverses the elements in addition to whatever
 *          effect the original permutation had.
 */
Permutation Permutation::reverse() const {
    size_t s = size();
    auto rev = std::vector<size_t>(s);
    for(size_t i = 0; i < s; i++)
        rev[i] = map[s - i - 1];
    
    return Permutation(rev);
}

/**
 * @brief Constructs the inverse of a permutation.
 * @return the unique permutation such that @code *this * inverse() @endcode
 *          is the identity.
 */
Permutation Permutation::inverse() const {
    size_t s = size();
    auto inv = std::vector<size_t>(s, 0);
    for(size_t i = 0; i < s; i++)
        inv[map[i]] = i;
    
    return Permutation(inv);
}

/**
 * @brief Computes the order of a permutation.
 * 
 * The order is the smallest integer @c m such that @code (*this) ^ m @endcode
 * is the identity. It is equal to the least common multiple of the lengths of 
 * all cycles in the cycle decomposition of the permutation.
 * 
 * @return the order, also called the <i>period</i>, not the same thing as what
 * is called <i>size</i> here.
 */
size_t Permutation::order() const {
    auto decomp = cycle_type();
    
    //Efficient calculation using an gcd-based sum over
    //the cycle type.
    return std::accumulate(decomp.begin(), decomp.end(), 1, 
            [](size_t a, size_t b) {
                auto gcd = [] (size_t a, size_t b) {
                  for(;;){
                      if(a == 0) return b;
                      b %= a;
                      if(b == 0) return a;
                      a %= b;
                  }  
                };
                
                return a / gcd(a, b) * b;               
            });
}

/**
 * @brief Checks whether a permutation is the identity.
 * 
 * @return @c true if and only if the permutation is the identity. 
 */
bool Permutation::is_identity() const {
    for(size_t i = 0; i < map.size(); i++)
        if(map[i] != i)
            return false;
    
    return true;
}

/**
 * @brief Finds the parity of a permutation.
 * 
 * The parity is even if the permutation is a composition of an even number of
 * element swaps, and odd otherwise. Since an even-length cycle is an odd 
 * permutation and vice versa, the parity is equivalent to the parity of the
 * number of even-length cycles in the cycle decomposition of the permutation.
 * 
 * @return  an integer that is odd if the permutation is odd, and even if it is
 *          even.
 */
int Permutation::parity() const {
    size_t n_evens = 0;
    for(size_t c_len : cycle_type())
        if(c_len % 2 != 0)
            n_evens++;
    
    return (int) (n_evens % 2);
}

/**
 * @brief Applies a permutation to another permutation.
 * 
 * This has the same effect as 
 * @code permute(p.perm.begin(), offset, block_len) @endcode .
 * Without the optional argument, it behaves the same as 
 * @code p *= (*this) @endcode
 * when the permutations are of the same size. 
 * 
 * @param p         the permutation to be modified.
 * @param offset    If provided, the permutations will be shifted by this much
 *                  relative to each other.
 * @param block_len If proveded the application will be block-wise, with blocks
 *                  of this length.
 * @return  A reference to the modified @p p, which allows chaining.
 */
Permutation& Permutation::permute(
    Permutation& p, size_t offset, size_t block_len) const
{
    permute(p.map.begin(), offset, block_len);
    return p;
}

/**
 * @brief Swaps two indices in a permutation.
 * 
 * This method allows manual modification of a permutation without the risk of
 * breaking the permutation property.
 * 
 * @param i the location of the first index.
 * @param j the location of the second index.
 * @return a reference to @c *this ,  to allow chaining.
 */
Permutation& Permutation::swap(size_t i, size_t j){
    std::swap(map[i], map[j]);
    return *this;
}

/**
 * @brief Composes two permutations.
 * 
 * This constructs the permutation that is equivalent to applying @p p2 followed
 * by @p p1. This right-to-left ordering is conventional for compositions.
 * 
 * @param p1    the left-hand permutation.
 * @param p2    the right-hand permutation.
 * @return  their composition.
 */
Permutation operator* (const Permutation& p1, const Permutation& p2){
    assert(p1.size() == p2.size());
    
    Permutation comp(p2);
    p1.permute(comp.map.begin());
    
    return comp;
}

/**
 * @brief Replaces one permutation with its composition with another.
 * 
 * This operator gives the same result as @code p1 = (p2 * p1) @endcode .
 * Note the reversal of the arguments due to the right-to-left ordering of 
 * compositions.
 * 
 * @param p1    the permutation to modify.
 * @param p2    the permutation to compose it with.
 * @return  the new value of @p p1 after the composition.
 */
Permutation& operator*= (Permutation& p1, const Permutation& p2){
    assert(p1.size() == p2.size());
    
    p2.permute(p1.map.begin());
    
    return p1;
}

/**
 * @brief Takes a permutation to an integer power.
 * 
 * The complexity of this operator is linear in the size of @p p (as inherited
 * from the composition operator) and logarithmic in @p pow. Since @p pow can
 * always be reduced to be less than @link Propagator::order p.order() @endlink, 
 * whose upper bound is roughly
 * exponential in @c p.size() , the worst-case complexity of this operator is
 * quadratic in the size of @p p .
 * 
 * @param p     a permutation.
 * @param pow   an integer.
 * @return  @p p composed with itself @p pow times.
 */
Permutation operator^ (const Permutation& p, size_t pow){
    //For large powers, it is definitely worthwile to find the order of p
    //to reduce pow to the smallest equivalent value
    if(pow > p.size())
        pow %= p.order();
    
    Permutation pp(p);
    Permutation res(p.size());
    
    for(; pow > 0; pow >>= 1){       
        if(pow & 1)
            res *= pp;
        
        pp *= pp;
    }
    
    return res;
}
/**
 * @brief Assigns @code p ^ pow @endcode to @p p.
 */
Permutation& operator^= (Permutation& p, size_t pow){
    p = p ^ pow;
    return p;
}

/**
 * @brief Takes one permutation modulo another.
 *  
 * Each permutation divides all other permutations into equivalence classes 
 * under composition with powers of this permutation. The modulo operation
 * maps each permutation to a unique representative of the equivalence class it
 * is in. 
 * 
 * The choice of representative is made by lexicographically minimising the 
 * internal representation of the permutation. This gives a simple and 
 * consistent choice, and guarantees that the identity permutation is returned
 * whenever possible.
 * 
 * The implementation is rather brute-force and not very efficient.
 * 
 * @param p1    the permutation to map to a unique equivalence class 
 *              representative.
 * @param p2    the permutation with respect to which the equivalence classes
 *              are defined
 * @return  a permutation @c q such that @code q * p2^n == p1 @endcode 
 *          for some @c n . It is chosen such that 
 *          @code p1 % p2 == p3 % p2 @endcode whenever 
 *          @code p3 == p1 * p2^n @endcode for some @c m .
 */
Permutation operator% (const Permutation& p1, const Permutation& p2){
    Permutation least(p1);
    Permutation comp;
    for(Permutation p(p2); !p.is_identity(); p *= p2){
        comp = p1 * p;
        if(comp.map < least.map)
            least = comp;
    }
    
    return least;
}
/**
 * @brief Assigns @code p1 % p2 @endcode to @p p.
 */
Permutation& operator%= (Permutation& p1, const Permutation& p2){
    p1 = p1 % p2;
    return p1;
}

/**
 * @brief Compares two permutations for equality.
 */
bool operator== (const Permutation& p1, const Permutation& p2){
    return p1.map == p2.map;
}

/**
 * @brief Compares two permutations for inequality.
 */
bool operator!= (const Permutation& p1, const Permutation& p2){
    return p1.map != p2.map;
}

/**
 * @brief prints a string representaiton of a permutation.
 * 
 * Prints a permutation as a space-separated list of its indices, enclosed in
 * parentheses. This is reminiscent of Cauchy's two-line notation, with the 
 * elements sorted so that the first line is the identity permutation, and with
 * the now-trivial first line omitted.
 * 
 * @param out   the output stream.
 * @param p     the permutation.
 * @return  the output stream, to allow chaining.
 */
std::ostream& operator<< (std::ostream& out, const Permutation& p){
    out << "( ";
    for(size_t i : p)
        out << i << " ";
    out << ")";
    
    return out;
}

/**
 * @brief Determines the <it>cycle type </it> of a permutation.
 * 
 * The cycle type is a sorted list of the lengths of the cycles in the cycle
 * decomposition of the permutation. The sum of all elements in the cycle type
 * is the size of the permutation.
 * 
 * For a cyclic permutation, the cycle type has a singe element. For the 
 * identity permutation, all elements of the cycle type are 1.
 * 
 * @return the cycle type as a sorted vector.
 */
std::vector<size_t> Permutation::cycle_type() const {
    auto decomp = std::vector<size_t>();
    auto visited = std::vector<bool>(size(), false);
    
    for(size_t i = 0, c_len; i < size(); i++){
        if(visited[i])
            continue;
        
        c_len = 0;
        for(size_t j = i; !visited[j]; j = map[j]){
            visited[j] = true;
            c_len++;
        }
        
        decomp.push_back(c_len);
    }
    
    std::sort(decomp.begin(), decomp.end());
    return decomp;    
}

/**
 * @brief Finds all fixed points under the permutation, i.e. the 
 * indices that are mapped to themselved.
 * 
 * @return the indices of the fixed points as a sorted vector. 
 */
std::vector<size_t> Permutation::fixed_points() const {
    auto fixed = std::vector<size_t>();
    
    for(size_t i = 0; i < size(); i++){
        if(map[i] == i)
            fixed.push_back(i);
    }
    
    return fixed;
}

}
