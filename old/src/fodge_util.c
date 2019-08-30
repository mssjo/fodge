
#include "fodge_util.hpp"

size_t integer_pow(size_t base, size_t exp){
    size_t res = 1;
    while(exp){
        if(exp & 1){
            if((res * base) / base != res){
                printf("\nInteger overflow: %zd^%zd", base, exp);
                exit(EXIT_FAILURE);
            }
            
            res *= base;
        }
        
        exp >>= 1;
        
        if((base * base) / base != base){
            printf("\nInteger overflow: %zd^%zd", base, exp);
            exit(EXIT_FAILURE);
        }
        base *= base;
    }
    
    return res;
}

int integer_comp(const void* a, const void* b){
    return COMPARE(*((const size_t*) a), *((const size_t*) b));
}

/**
 * Calculates the number of characters needed to print an integer in a certain
 * base.
 * @param i     the integer
 * @param radix the base
 * @return  the number of characters, including the minus sign if negative.
 */
int integer_width(int i, int radix){
    if(radix < 2)
        return 0;
    
    if(i == 0)
        return 1;
    if(i < 0)
        return 1 + integer_width(-i, radix);
    
    int width = 0;
    while(i){
        i /= radix;
        width++;
    }
    
    return width;
}

/**
 * Calculates the number of characters needed to print an integer in base 10.
 * @param d the integer
 * @return  the number of characters, including the minus sign if negative.
 */
int decimal_width(int d){
    return integer_width(d, 10);
}

void init_diagram_id(){
    _diagram_id = 0;
}


void init_indent(){
    _indent_level = 0;
}
/**
 * Prints the whitespace needed for indentation of a line.
 * @param indent    the level of indentation
 */
void indent_line(){
    for(uint i = 0; i < _indent_level; i++)
        printf(INDENT);
}

void idprintf(const char* fmt, ...){
    indent_line();
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

/**
 * Is to scalloc what realloc is to salloc. Extends a block of memory, and sets
 * the contents of the added part to zero.
 * @param ptr   a pointer to the block of memory
 * @param old_n the current number of elements in the block
 * @param new_n the number of elements to be extended to
 * @param size  the size, in bytes, of the elements in the block    
 * @return  a pointer to the extended block (this may or may not be the same as
 *          the pointer that was supplied)
 */
void* srecalloc(void* ptr, size_t old_n, size_t new_n, size_t size){
    if(!ptr)
        return NULL;
    
    ptr = srealloc(ptr, new_n * size);
    memset(ptr + old_n*size, 0, (new_n - old_n) * size);
    return ptr;
}

/**
 * A safer version of malloc. Crashes with an error message rather than 
 * returning NULL on failure.
 * @param n the number of bytes to be allocated.
 * @return  a pointer to the allocated memory, guaranteed non-NULL.
 */
void* salloc(size_t n){
    void* ptr = malloc(n);
    if(!ptr){
        printf("ERROR: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}
/**
 * A safer version of calloc. Crashes with an error message rather than 
 * returning NULL on failure.
 * @param nitems    the number of items to be allocated.
 * @param size      the size in bytes of the items.
 * @return  a pointer to the allocated and zeroed memory, guaranteed non-NULL.
 */
void* scalloc(size_t nitems, size_t size){
    void* ptr = calloc(nitems, size);
    if(!ptr){
        printf("ERROR: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}
/**
 * A safer version of realloc. Crashes with an error message rather than 
 * returning NULL on failure.
 * @param ptr   a pointer to the block to be reallocated.
 * @param n     the number of bytes to which the block should be extended.
 * @return  a pointer to the extended block (may or may not be the same as ptr),
 *          guaranteed non-NULL unless ptr was NULL.
 */
void* srealloc(void* ptr, size_t n){
    if(!ptr)
        return NULL;
    
    ptr = realloc(ptr, n);
    if(!ptr){
        printf("ERROR: Failed to reallocate memory!\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}
