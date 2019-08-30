/* 
 * File:   bitwise.hpp
 * Author: Mattias
 * 
 * Utility templates for bitwise operations.
 *
 * Created on 09 July 2019, 10:57
 */

#ifndef BITWISE_H
#define	BITWISE_H

#include <climits>

namespace bitwise {

/**
 * @brief Undoes the shift operator @code 1 << s @endcode to retrieve @c s.
 * 
 * @tparam B    a binary integer type.
 * @param shifted   the shifted quantity. All sub-leading bits are ignored.
 * @return  the number @c s.
 */
template<typename B>
size_t unshift(B shifted){
    size_t unshifted = 0;
    for(B b = (shifted >> 1); b; b >>= 1, unshifted++);
    return unshifted;
}

/**
 * @brief Counts the number of 1-bits in an integer.
 * 
 * This uses Brian Kernighan's trick, whose cost goes as the number of 1-bits,
 * not as the total number of bits.
 * 
 * @tparam  B   a binary integer type.
 * @param bits  the integer.
 * @return  the number of 1-bits.
 */
template<typename B>
size_t bitcount(B bits){
    size_t count = 0;
    for(; bits; count++)
        bits &= (bits - 1);
    return count;
}

/**
 * @brief Reverses the binary representation of an integer.
 * 
 * The algorithm is due to Ken Raeburn and works in O(lg(n)) operations, where
 * n is the size of @p B in bits.
 * 
 * @tparam  B   a binary integer type.
 * @param bits  the integer.
 * @return  the reversed integer.
 */
template<typename B>
B reverse(B bits){
    size_t s = sizeof(B) * CHAR_BIT;
    B mask = ~((B) 0);         
    while((s >>= 1) > 0){
        mask ^= (mask << s);
        bits = ((bits >> s) & mask) | ((bits << s) & ~mask);
    }
}

/**
 * @brief prints the bits of a binary integer, least significant first.
 * 
 * @tparam B    a binary integer type
 * 
 * @param bits  the integer to be printed.
 * @param size  the number of bits to be printed; defaults to the number of bits
 *              needed to represent a variable of type @p B. If it is larger,
 *              errors may occur. Setting @p size to 0 is equivalent to setting
 *              it to @c unshift(bits), i.e. all bits until the most significant
 *              1-bit are printed.
 * @param out   the output stream to print to; defaults to @c std::cout.
 * @param high  the character used to represent a 1-bit; defaults to @c '1'.
 * @param low   the character used to represent a 0-bit; defaults to @c '0'.
 * @param reverse   if @c true, the bits are printed in the reverse order, so
 *                  that the output appears with the most significant bit first.
 * @return  an empty string, for convenient insertion into a chain of 
 *          left-shift operators.
 */
template<typename B>
void print_bits(B bits, size_t size = CHAR_BIT * sizeof(B),
        std::ostream& out = std::cout, char high = '1', char low = '0',
        bool reverse = false)
{
    B one = (B) 1;
    B mask;
    if(!reverse)
        mask = 1;
    else if(size > 0)
        mask = one << size;
    else
        mask = one << bitwise::unshift(bits);
    
    for(; (reverse && mask) 
            || (size && mask < (one << size)) 
            || (((mask-1) & bits) != bits);)
    {
        if(bits & mask)
            out << high;
        else
            out << low;
        
        if(reverse)
            mask >>= 1;
        else
            mask <<= 1;
    }
}

}

#endif	/* BITWISE_H */

