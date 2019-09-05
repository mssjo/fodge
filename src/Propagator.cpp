/* 
 * File:   Propagator.cpp
 * Author: Mattias
 * 
 * Created on 19 June 2019, 16:09
 */

#include "Propagator.hpp"

/**
 * @brief Constructs an ordinary propagator.
 * 
 * @param momenta the momenta it carries.
 * @param n_mom the total number of momenta in the diagram.
 * @param src_order the source vertex order.
 * @param dst_order the destination vertex order.
 * 
 * The resulting propagator will be normalised. 
 */
Propagator::Propagator(mmask momenta, int n_mom, 
        int src_order, 
        int dst_order) 
: Propagator(momenta, n_mom, src_order, 0, dst_order, 0)
{}

/**
 * @brief Constructs a singlet propagator.
 * 
 * @param momenta the momenta it carries.
 * @param n_mom the total number of momenta in the diagram.
 * @param src_order the source vertex order.
 * @param src_prev  the momenta of the previous vertex leg at source.
 * @param dst_order the destination vertex order.
 * @param dst_prev  the momenta of the previous vertex leg at dest.
 * 
 * The resulting propagator will be normalised. 
 */
Propagator::Propagator(mmask momenta, int n_mom,
            int src_order, mmask src_prev, 
            int dst_order, mmask dst_prev)
: momenta(momenta), 
        src_order(src_order), src_prev(src_prev), 
        dst_order(dst_order), dst_prev(dst_prev), 
        n_mom(n_mom)
{
    normalise();
}

/**
 * @brief Constructs a propagator resulting from applying a permutation
 * to the momentum indices of another propagator.
 * 
 * @param orig the original propagator.
 * @param cycl the permutaton, must have size equal to @c n_mom.
 * 
 * The resulting propagator will be normalised.
 */
Propagator::Propagator(const Propagator& orig, 
        const permute::Permutation& cycl) 
: momenta(cycl.permute_bits(orig.momenta)), 
        src_order(orig.src_order), src_prev(cycl.permute_bits(orig.src_prev)),
        dst_order(orig.dst_order), dst_prev(cycl.permute_bits(orig.dst_prev)), 
        n_mom(orig.n_mom)
{
    assert(n_mom == cycl.size());
    normalise();
}

/**
 * @brief Normalises a propagator so that all its momenta are in a 
 * canonical form under conservation of momentum.
 * 
 * By COM, a propagator carrying a set of momenta is equivalent to
 * a propagator carrying the complementary set of momenta going in
 * the opposite direction. There therefore exists an ambiguity between
 * the set and its complement.
 * 
 * The canonical form is that in which the set contains the fewest
 * momenta, and if it contains exactly half the momenta, the one
 * not containing the momentum with the highest index is chosen.
 */
void Propagator::normalise(){
    mmask last_mask = 1 << (n_mom-1);
    mmask all_mask = (1 << n_mom) - 1;
    
    src_prev = normalise_mmask(src_prev, last_mask, all_mask);
    dst_prev = normalise_mmask(dst_prev, last_mask, all_mask);
    mmask norm = normalise_mmask(momenta, last_mask, all_mask);
    
    if(norm != momenta){
        std::swap(src_order, dst_order);
        std::swap(src_prev, dst_prev);
        momenta = norm;
    }
}

/**
 * @brief Auxiliary method to @link Propagator::normalise @endlink.
 * 
 * @param m a bitmask representing a set of momenta.
 * @param last_mask a bitmask representing the highest-indexed momentum.
 * @param all_mask a bitmask representing the set of all momenta.
 * @return @p m or its complement, depending on which is the canonical form.
 */
mmask Propagator::normalise_mmask(mmask m, mmask last_mask, mmask all_mask)
const {
    int count = bitwise::bitcount(m);
    if(count > n_mom/2 || (count == n_mom/2 && (m & last_mask)))
        m ^= all_mask;
    return m;
}

/**
 * @brief Compares two propagators for sorting purposes.
 * 
 * @param p1 the first propagator.
 * @param p2 the second propagator.
 * @return @c true if @p p1 should precede @p p2, @c false otherwise.
 * 
 * The comparison is rather arbitrary, as long as it is consistent.
 * It is done order before previous-leg momenta before propagator momenta,
 * source before destination.
 */
bool operator<(const Propagator& p1, const Propagator& p2){

    if(p1.src_order != p2.src_order)
        return p1.src_order < p2.src_order;
    if(p1.dst_order != p2.dst_order)
        return p1.dst_order < p2.dst_order;
    if(p1.src_prev != p2.src_prev)
        return p1.src_prev < p2.src_prev;
    if(p1.dst_prev != p2.dst_prev)
        return p1.dst_prev < p2.dst_prev;
    
    return p1.momenta < p2.momenta;
}


/**
 * @brief Compares two propagators for equality.
 * 
 * @param p1 the first propagator.
 * @param p2 the other propagator.
 * @return @c true if and only if they are completely identical.
 */
bool operator==(const Propagator& p1, const Propagator& p2){
    return (p1.momenta == p2.momenta) 
            && (p1.src_order == p2.src_order) 
            && (p1.dst_order == p2.dst_order)
            && (p1.src_prev == p2.src_prev)
            && (p1.dst_prev == p2.dst_prev);
}

/**
 * @brief Prints a compact representation of a propagator.
 * 
 * @param out the stream to which the propagator should be printed.
 * @param p the propagator.
 * @return the stream.
 * 
 * The propagator is printed with X's and .'s representing the 1's and
 * 0's of the momentum mask. The source and destination orders are printed
 * as @code (src -> dest) @endcode, with previous-leg momenta written
 * in brackets if needed.
 */
std::ostream& operator<<(std::ostream& out, const Propagator& p){
    bool print_prev = p.src_prev || p.dst_prev;
    
#define HI_CHAR 'X'
#define LO_CHAR '.'
    
    bitwise::print_bits(p.momenta, p.n_mom, out, HI_CHAR, LO_CHAR);
    
    out << " (" << p.src_order;
    if(print_prev){
        out << "[";
        bitwise::print_bits(p.src_prev, p.n_mom, out, HI_CHAR, LO_CHAR);
        out << "]";
    }    
    out << " -> " << p.dst_order;
    if(print_prev){
        out << "[";
        bitwise::print_bits(p.dst_prev, p.n_mom, out, HI_CHAR, LO_CHAR);
        out << "]";
    }
    out << ")";
    
    return out;
}

/**
 * @brief Prints a header matching the printouts as an aid when reading them.
 * 
 * @param out the stream to which the header should be printed.
 */
void Propagator::print_header(std::ostream& out) const {
    bool print_prev = src_prev || dst_prev;
    int w = 0;
    for(int ord = std::max(src_order, dst_order); ord > 0; ord /= 10, w++);
    
    for(int i = 0; i < n_mom; i++)
        out << (i % 10);
    
    out << "  " << std::string(w, ' ');
    if(print_prev){
        out << " ";
        for(int i = 0; i < n_mom; i++)
            out << (i % 10);
        out << " ";
    }
    
    out << "    " << std::string(w, ' ');
    if(print_prev){
        out << " ";
        for(int i = 0; i < n_mom; i++)
            out << (i % 10);
        out << " ";
    }
    out << " ";   
}
