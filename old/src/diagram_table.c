
#include "fodge.hpp"

diagram* _fill_table(diagram_table* tab, size_t ngons, size_t order){ 
    idprintf("Generating O(p^%d) %d-point diagrams...\n", OP(order), ngons);
    MORE_INDENT;
    
    if(!tab->table[order]){
        tab->table[order] 
                = scalloc(1 + TABIDX(tab->max_ngons), sizeof(diagram*));
    }
    if(tab->table[order][TABIDX(ngons)]){
        LESS_INDENT;
        return tab->table[order][TABIDX(ngons)];
    }

    diagram* diagr = make_contact_diagram(ngons, order);
    for(size_t o = order; o >= order/2 && o+1 > 0; o--){
        for(size_t n = ngons - 2; n >= ngons/2 && n >= 4; n -= 2){
            if(!tab->table[o] || !tab->table[o][TABIDX(n)]){
                _fill_table(tab, n, o);
            }
            
            diagr = merge_diagrams(diagr, 
                    grow_diagrams(tab->table[o][TABIDX(n)], ngons-n, order-o));
        }
    }
    
    LESS_INDENT;
    idprintf("[done]\n");
    
    return (tab->table[order][TABIDX(ngons)] = diagr);
}

diagram_table* make_table(
        size_t max_ngons, size_t max_order, 
        int split, int singlet, 
        enum fill_level fill
){
    init_diagram_id();
    init_indent();
    
    if(max_ngons < 4 || max_ngons % 2){
        printf("Invalid diagram size!\n");
        return NULL;
    }
    
    diagram_table* table = salloc(sizeof(diagram_table));
    
    table->max_ngons = max_ngons;
    table->max_order = max_order;
    table->table = scalloc(1 + max_order, sizeof(diagram**));
    
    if(fill == MIN_FILL)
        _fill_table(table, max_ngons, max_order);
    else if(fill == MAX_FILL){
        for(size_t order = max_order; order+1 > 0; order--)
            _fill_table(table, max_ngons, order);
    }
    else if(fill == NO_FILL){
        _fill_table(table, max_ngons, max_order);
        for(size_t o = 0; o <= table->max_order; o++){
            if(!table->table[o])
                continue;

            for(size_t n = 0; n < TABIDX(table->max_ngons); n++){
                if(!table->table[o][n])
                    continue;

                delete_diagram(table->table[o][n], TRUE);
            }
        }
    }
    
    if(split){
        for(size_t o = 1; o <= max_order; o++){
            for(size_t n = 0; n <= TABIDX(max_ngons); n++){
                if(table->table[o] && table->table[o][n]){
                    table->table[o][n] = split_diagrams(table->table[o][n]);
                    table->table[o][n] = remove_zero_fsp(table->table[o][n]);
                }
            }
        }
    }
    
    if(singlet){
        for(size_t o = 2; o <= max_order; o++){
            for(size_t n = 0; n <= TABIDX(max_ngons); n++){
                if(table->table[o] && table->table[o][n]){
                    table->table[o][n] = singlet_diagrams(table->table[o][n]);
                    table->table[o][n] = remove_zero_fsp(table->table[o][n]);
                }
            }
        }
    }
    
    printf("\n\n");
    return table;
}

diagram* get_diagram(diagram_table* tab, size_t ngons, size_t order, size_t index){
    if(ngons < 4 || ngons > tab->max_ngons || order > tab->max_ngons)
        return NULL;
    
    diagram* list = _fill_table(tab, ngons, order);
    for(; index > 0; index--, list = list->next){
        if(!list)
            return NULL;
    }
    
    return list;
}

void print_table(diagram_table* tab){
    if(!tab){
        printf("[no table]\n");
        return;
    }
    
    for(size_t o = 0; o <= tab->max_order; o++){
        if(!tab->table[o])
            continue;
        
        for(size_t n = 0; n <= TABIDX(tab->max_ngons); n ++){
            if(!tab->table[o][n])
                continue;
            
            print_diagram(tab->table[o][n], TRUE);
        }
    }
}

void count_table(diagram_table* tab, int count_detail){
    if(!tab){
        printf("[no table]\n");
        return;
    }
    
    for(size_t o = 0; o <= tab->max_order; o++){
        if(!tab->table[o])
            continue;
        
        for(size_t n = 0; n <= TABIDX(tab->max_ngons); n ++){
            if(!tab->table[o][n])
                continue;
            
            count_diagrams(tab->table[o][n], count_detail);
        }
    }
}

void delete_table(diagram_table* tab){
    if(!tab)
        return;
    
    for(size_t o = 0; o <= tab->max_order; o++){
        if(!tab->table[o])
            continue;
        
        for(size_t n = 0; n <= TABIDX(tab->max_ngons); n++){
            if(!tab->table[o][n])
                continue;
            
            delete_diagram(tab->table[o][n], TRUE);
        }
        
        free(tab->table[o]);
    }
    free(tab->table);
    
    free(tab);
}
