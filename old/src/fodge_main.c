/* 
 * File:   fodge_main.c
 * Author: Mattias
 *
 * Created on 11 February 2019, 14:23
 */

#include <stdio.h>
#include <stdlib.h>

#include "fodge.hpp"
#include "fodge_TikZ.hpp"
#include "fodge_FORM.hpp"

/*
 * 
 */
int main(int argc, char** argv) {
        
    int include_singlets = TRUE;
    int include_flsplits = TRUE;
    
    int count_diagrams = FALSE;
    int count_detail = 1;
    
    int make_TikZ = FALSE;
    int make_FORM = FALSE;
    int print_debug = FALSE;
    
    enum fill_level fill_mode = MAX_FILL;
    
    size_t max_ngons = 4;
    size_t max_order = 0;
    int argn = 0;
    
    char* filename = NULL;
    
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            for(int j = 1; argv[i][j]; j++){
                switch(argv[i][j]){
                    case 'u':
                        include_singlets = FALSE;       break;
                    case 's':
                        include_flsplits = FALSE;       break;
                        
                    case 't':
                        make_TikZ = TRUE;               break;
                    case 'f':
                        make_FORM = TRUE;               break;
                    case 'd':
                        print_debug = TRUE;             break;
                        
                    case 'c': 
                        count_diagrams = TRUE;
                        if(argv[i][j+1] == '='){
                            j += 2;
                            if(argv[i][j] >= '0' && argv[i][j] <= '2')
                                count_detail = argv[i][j] - '0';
                            else{
                                printf("Unrecognised option: '%c'\n", 
                                        argv[i][j]);
                                exit(EXIT_FAILURE);
                            }
                        }
                        break;
                        
                    case 'm':
                        fill_mode = MIN_FILL;           break;
                    case 'M':
                        fill_mode = MAX_FILL;           break;
                    case 'o':
                        fill_mode = NO_FILL;            break;
                        
                    case '-':
                        break;
                        
                    default:
                        printf("Unrecognised flag: '%c'\n", argv[i][j]);
                        exit(EXIT_FAILURE);                               
                }
            }
        }
        else if(argv[i][0] >= '0' && argv[i][0] <= '9'){
            argn++;
            if(argn == 1){
                max_order = (size_t) atoi(argv[i]);
                if(max_order % 2 || max_order < 2){
                    printf("Invalid momentum order - "
                            "only positive even orders allowed!");
                    exit(EXIT_FAILURE);
                }
                max_order = INV_OP(max_order);
            }
            else if(argn == 2)
                max_ngons = (size_t) atoi(argv[i]);
            else{
                printf("Too many numerical arguments!\n");
                exit(EXIT_FAILURE);
            }
        }
        else if(!filename)
            filename = argv[i];
        else{
            printf("Unrecognised argument: \"%s\"", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
    
    if(!filename)
        filename = "fodge";
    
    diagram_table* table = make_table(
            max_ngons, max_order, 
            include_flsplits, include_singlets,
            fill_mode);
    
    printf("\n\n");
    
    if(print_debug){
        print_table(table);
        printf("\n\n");
    }
    if(count_diagrams){
        count_table(table, count_detail);
        printf("\n\n");
    }
    if(make_TikZ){
        char* texname = salloc(strlen(filename) + 5);
        sprintf(texname, "%s.tex", filename);
        FILE* tex = fopen(texname, "w");
                
        printf("TikZing diagrams > %s ...", texname);
        TikZ_table(tex, table, DRAW_POLYGON , .4, .05);
        fclose(tex);
        free(texname);
    }
    
    if(make_FORM)
        fodge_FORM(filename, table->table[max_order][TABIDX(max_ngons)]);
        
    delete_table(table);
    
    return (EXIT_SUCCESS);
}
