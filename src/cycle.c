
#include "fodge.h"

cycle* make_cycle(CYCLE_T* array, uint length){
    if(!array || !length)
        return NULL;
    
    cycle* cycl = malloc(sizeof(cycle));
    cycl->length = length;
    cycl->array = array;
        
    cycl->modified = TRUE;
    return cycl;
}

cycle* copy_cycle(const cycle* orig){
    if(!orig)
        return NULL;
    
    CYCLE_T* array = malloc(orig->length * sizeof(CYCLE_T));
    for(idx_t i = 0; i < orig->length; i++){
        array[i] = COPY_TYPE(CYCLE_T) (orig->array[i]);
    }
    
    cycle* copy = make_cycle(array, orig->length);
        
    copy->modified = TRUE;    
    return copy;
}

uint get_length(cycle* cycl){
    if(!cycl)
        return 0;
    
    return cycl->length;
}

uint _booth_normalise(cycle* c){
    int* f = malloc(2*c->length * sizeof(int));
    f[0] = -1;
    uint k = 0;
    int i, comp;
    CYCLE_T cj;
    
    for(int j = 1; j < 2*c->length; j++){
        i = f[j-k-1];
        cj = c->array[j % c->length];
        comp = COMPARE_TYPE(CYCLE_T)(cj, c->array[(k+i+1) % c->length]);
        while(i != -1 && comp != 0){
            if(comp < 0)
                k = j-i-1;
            i = f[i];
            
            comp = COMPARE_TYPE(CYCLE_T)(cj, c->array[(k+i+1) % c->length]);
        }
        if(i == -1 && comp != 0){
            if(comp < 0)
                k = j;
            f[j-k] = -1;
        }
        else
            f[j-k] = i+1;
    }
    
    return k;    
}

uint _find_period(cycle* cycl){
    for(uint period = 1; period <= cycl->length/2; period++){
        if(cycl->length % period)
            continue;
        
        if(compare_offset(cycl, period) == 0)
            return period;
    }
    
    return cycl->length;
}

uint get_offset(cycle* cycl){
    if(!cycl)
        return 0;
    
    normalise(cycl);
    return cycl->offset;
}

uint get_period(cycle* cycl){
    if(!cycl)
        return 0;
    
    normalise(cycl);
    return cycl->period;
}

CYCLE_T get_element(cycle* cycl, idx_t index, int LLR){
    if(!cycl)
        return (CYCLE_T) 0;
    return cycl->array[index + (LLR ? get_offset(cycl) : 0)];
}

CYCLE_T set_element(cycle* cycl, CYCLE_T elem, idx_t index, int LLR){
    if(!cycl)
        return (CYCLE_T) 0;
    
    if(LLR)
        index = (index + get_offset(cycl)) % cycl->length;
    
    CYCLE_T old = cycl->array[index];
    cycl->array[index] = elem;
    
    return old;
}

CYCLE_T set_periodic(cycle* cycl, CYCLE_T elem, idx_t index, int LLR){
    if(!cycl)
        return (CYCLE_T) 0;
    
    if(LLR)
        index = (index + get_offset(cycl)) % cycl->length;
    
    CYCLE_T old = cycl->array[index];
    cycl->array[index] = elem;
    
    for(idx_t i = get_period(cycl); i < cycl->length; i += cycl->period){
        cycl->array[index + i] = COPY_TYPE(CYCLE_T)(elem);
    }
    
    return old;
}

void normalise(cycle* cycl){
    if(!cycl || !cycl->modified)
        return;
    
    cycl->modified = FALSE;
    
    cycl->offset = _booth_normalise(cycl);
    cycl->period = _find_period(cycl);
}

int compare_cycle(cycle* c_1, cycle* c_2){
    if(!c_1 && !c_2)
        return 0;
    if(!c_1)
        return -1;
    if(!c_2)
        return +1;
    
    normalise(c_1);
    normalise(c_2);
    
    int comp;
    uint minlen = c_1->length < c_2->length ? c_1->length : c_2->length;
    for(int i = 0; i < minlen; i++){
        comp = COMPARE_TYPE(CYCLE_T) (
                c_1->array[(i+c_1->offset) % c_1->length],
                c_2->array[(i+c_2->offset) % c_2->length]);
        if(comp)
            return comp;
    }
    
    return c_1->length - c_2->length;
}

int compare_offset(cycle* cycl, uint offset){
    if(!cycl)
        return 0;
    normalise(cycl);
    
    int comp;
    for(int i = 0; i < cycl->length; i++){
        comp = COMPARE_TYPE(CYCLE_T) (
                cycl->array[i],
                cycl->array[(i+offset) % cycl->length]);
        if(comp)
            return comp;
    }
    
    return 0;
}

void print_cycle(cycle* cycl, int LLR, const char* format){
    if(!cycl)
        printf("[empty cycle]");
    
    uint offset = LLR ? get_offset(cycl) : 0;
    
    for(int i = 0; i < cycl->length; i++){
        printf(format, cycl->array[(i+offset) % cycl->length]);
    }
}

void delete_cycle(cycle* cycl, int deep){
    if(!cycl)
        return;
    
    if(deep){
        for(idx_t i = 0; i < cycl->length; i++){
            DELETE_TYPE(CYCLE_T) (cycl->array[i]);
        }
    }
    
    free(cycl->array);
    free(cycl);
}
