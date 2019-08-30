
#include "info_sort.hpp"

/** Compares a[i] and b[j] using *compare, with a and b being treated as having
 *  elements of size size. */
#define CMPARR(a,i,b,j) (*compare)((a + (i)*size), (b + (j)*size))
/** Sets d[i] to the value of s[j], with a and b being treated as having 
 *  elements of size size. */
#define SETARR(d,i,s,j) memcpy((d + (i)*size), (s + (j)*size), size);

/**
 * Merges two sorted array-halves.
 * 
 * @param la    contains the sorted halves.
 * @param lb    the merger will be written here.
 * @param begin the first index (inclusive) of the fist half.
 * @param mid   the last index (exclusive) of the first half, and the first
 *              index (inclusive) of the second half.
 * @param end   the last index (exclusive) of the second half.
 * @param size  the size of the elements.
 * @param compare   the comparison function.
 * @param wa    if non-NULL, these elements will be copied to wb just like those
 *              of la are copied to lb.
 * @param wb    (see wa)
 * @param rank  see the rank argument of info_sort. Only used if non-NULL.
 */
void _merge(
        char* la, char* lb, size_t begin, size_t mid, size_t end, 
        size_t size, int (*compare)(const void*, const void*),
        size_t* wa, size_t* wb, size_t* rank
){
    size_t i = begin, j = mid;
    size_t r = 0;
    if(rank)
        rank[0] = r;
    
    int comp;
    
    for(size_t k = begin; k < end; k++){
        if(i < mid && (j >= end || (comp = CMPARR(la, i, la, j)) <= 0)){
            if(wa)
                wb[k] = wa[i];
            if(rank)
                rank[k] = (k > 0 && CMPARR(lb, k-1, la, i)) ? ++r : r;
            
            SETARR(lb, k, la, i);
            i++;
            
            //Comparisons are expensive. If we had equality, we can save one
            //comparison by inserting both elements after each other instead
            //of just iterating on.
            if(i < mid && j < end && comp == 0){
                k++;
                
                if(wa)
                    wb[k] = wa[j];
                if(rank)
                    rank[k] = r;
                
                SETARR(lb, k, la, j);
                j++;
            }
        } 
        else {
            if(wa)
                wb[k] = wa[j];
            if(rank)
                rank[k] = (k > 0 && CMPARR(lb, k-1, la, j)) ? ++r : r;
            
            SETARR(lb, k, la, j);
            j++;
        }
    }
}

/**
 * Splits an array into halves, sorts them recursively, and merges them into
 * another array. No extra memory is allocated; la and lb alternatingly serve
 * each other's roles in the recursion.
 * 
 * @param la    the array to be sorted. 
 * @param lb    the sorted elements are written here.
 * @param begin the first index (inclusive) of the array.
 * @param end   the last index (exclusive) of the array.
 * @param size  the size of the elements.
 * @param compare   the comparison function.
 * @param wa    if non-NULL, these elements will be copied to wb just like those
 *              of la are copied to lb.
 * @param wb    (see wa)
 * @param rank  see the rank argument of info_sort. Only used if non-NULL.
 */
void _split_merge(
        char* la, char* lb, size_t begin, size_t end, size_t size, 
        int (*compare)(const void*, const void*),
        size_t* wa, size_t* wb, size_t* rank
){
    if(end - begin < 2){
        if(rank && (end - begin > 0))
            rank[begin] = 0;
        
        return;
    }
    
    size_t mid = (end + begin)/2;
        
    //Splits the list into two halves and sorts them recursively.
    //Even if non-NULL, rank is only useful at the topmost level of recursion,
    //so it is not passed on to lower levels.
    _split_merge(lb, la, begin, mid, size, compare, wb, wa, NULL);
    _split_merge(lb, la, mid,   end, size, compare, wb, wa, NULL);
    
    //Merges the lists - note the reversal of la and lb.
    _merge(la, lb, begin, mid, end, size, compare, wa, wb, rank);
    
}

void info_sort(
        void* list, size_t nitems, size_t size, 
        int (*compare)(const void*, const void*),
        size_t* whence, size_t* whither, size_t* rank, size_t* unique){
            
    char* la = (char*) list;
    char* lb = salloc(nitems * size);
    memcpy(lb, la, nitems * size);
    
    size_t* wa = whence;
    size_t* wb = NULL;
    if(whither && !whence)
        wa = salloc(nitems * sizeof(size_t));
    if(whence || whither){
        wb = salloc(nitems * sizeof(size_t));
        for(size_t i = 0; i < nitems; i++){
            wa[i] = i;
            wb[i] = i;
        }
    }
    
    size_t* r = (unique && !rank) ? salloc(nitems * size) : rank;
    
    _split_merge(lb, la, 0, nitems, size, compare, wb, wa, r);
    
    if(unique){
        for(size_t i = 0, rep_i = 0; i < nitems; i++){
            if(r[i] != r[rep_i])
                rep_i = i;
            
            unique[i] = rep_i;
        }
        
        if(!rank)
            free(r);
    }
    
    if(whither){
        for(size_t i = 0; i < nitems; i++)
            whither[wa[i]] = i;
        if(!whence)
            free(wa);
    }
        
    free(wb);
    free(lb);
}

void permute(void* list, size_t nitems, size_t size, const size_t* perm){
    void* temp = malloc(size);
    
    /* We use an elegant algorithm mentioned by RinRisson at Stack Overflow.
     * It runs in (probably) N log N, with constant auxiliary space. */
    for(size_t i = 0, j = 0; i < nitems-1; i++){
        j = perm[i];
        while(j < i)
            j = perm[j];
        
        SETARR(temp, 0, list, i);
        SETARR(list, i, list, j);
        SETARR(list, j, temp, 0);
    }
    
    free(temp);
}

/* //Functionality test of infosort
int _compar(const void* a, const void* b){
    return COMPARE(*((char*) a), *((char*) b));
}
int main(int argc, char** argv){
    char* str = "All makt aat Tengil, vaar befriare!";
    size_t len = strlen(str);
    
    char* arr = safe_malloc(len);
    memcpy(arr, str, len);
    size_t* whence = safe_malloc(len * sizeof(size_t));
    size_t* whither = safe_malloc(len * sizeof(size_t));
    size_t* rank = safe_malloc(len * sizeof(size_t));
    
    info_sort(arr, len, 1, _compar, whence, whither, rank);
    
    for(int i = 0; i < len; i++)
        printf("%3c", arr[i]);
    printf("\n");
    for(int i = 0; i < len; i++)
        printf("%3c", arr[whither[i]]);
    printf("\n");
    for(int i = 0; i < len; i++)
        printf("%3c", str[whence[i]]);
    printf("\n");
    for(int i = 0; i < len; i++)
        printf("%3zd", whither[i]);
    printf("\n");
    for(int i = 0; i < len; i++)
        printf("%3zd", whence[i]);
    printf("\n");
    for(int i = 0; i < len; i++)
        printf(" %2zd", rank[i]);
    printf("\n");
}
*/
