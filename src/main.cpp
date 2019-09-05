/* 
 * File:   main.cpp
 * Author: Mattias
 *
 * Created on 12 June 2019, 15:34
 */

#include "fodge.hpp"
#include "Diagram.hpp"
#include "permute.hpp"

#include <getopt.h>
#include <fstream>
#include <sstream>
#include <cstdlib>


using namespace std;
using namespace permute;


bool parse_flav_split(char* str, vector< vector<int> >& flav_splits){
    
    vector<int> flav_split = vector<int>();
    int r = 0;
    
    while(*str){
        if(*str == ','){
            if(r == 0){
                cerr << "ERROR: Missing number in flavour split before ','\n"; 
                return true;
            }
            
            flav_split.push_back(r);
            r = 0;
            
            str++;
        }
        else if(isspace(*str)){
            if(flav_split.empty()){
                cerr << "ERROR: Missing flavour split before ' '\n";
                return true;
            }
            if(r == 0){
                cerr << "ERROR: Missing number in flavour split before ' '\n";
                return true;
            }
            
            flav_split.push_back(r);
            r = 0;
            
            sort(flav_split.begin(), flav_split.end());
            flav_splits.push_back(flav_split);
            
            flav_split = vector<int>();
            
            while(isspace(*str))
                str++;
        }
        else if(*str >= '0' && *str <= '9'){
            r = strtol(str, &str, 10);
        }
        else{
            cerr << "ERROR: Unknown character in flavour split: '" << *str << "'\n";
            return true;
        }
    }
    
    flav_split.push_back(r);
    flav_splits.push_back(flav_split);
    
    return false;
}

void print_help(){
   
                                                                            //64-col here         
    cout << " " FODGE_VERSION " -- by Mattias Sjo, 2019                     \n"
            "\n"
            " FODGE generates flavour-ordered diagrams. For a description of\n"
            " the uses of flavour-ordered diagrams, see the artivle by      \n"
            " Bijnens, Kampf & Sjo, 2019 (arXiv:1909:XXXXX).                \n"
            "\n"
            " To generate all O(p^m) n-point flavour-ordered diagrams, run  \n"
            " $ fodge <m> <n>. For additional options, see below.           \n"
            "\n"
            " OPTION                DESCRIPTION                             \n"
            "\n"
            " -h [--help]           Prints this help message.               \n"
            " -v [--verbose]        Prints extra debug information about the\n"
            "                       generation process.                     \n"
            " -O [--order]          Sets the order of the diagrams. The     \n"
            "                       first unnamed argument to fodge is      \n"
            "                       interpreted as an argument to -O.       \n"
            " -N [--number-of-legs] Sets the number of legs on the diagrams.\n"
            "                       The second unnamed argument to fodge is \n"
            "                       interpreted as an argument to -N.       \n"
            " -s [--singlets]       Enables U(1) singlet propagators. This  \n"
            "                       is the default mode.                    \n"
            " -S [--no-singlets]    Disables U(1) singlet propagators.      \n"
            " -i [--include-flav-split]     Removes all diagrams that do not\n"
            "                       have the specified flavour splits.      \n"
            "                       Flavour splits are entered as integers  \n"
            "                       separated by commas. To enter more than \n"
            "                       one, use multiple -i's or write them    \n"
            "                       space-separated inside quotes like      \n"
            "                       \"2,2,4 3,5\".                          \n"
            " -x [--exclude-flav-split]     Works like -i, but removes all  \n"
            "                       diagrams that DO have the specified     \n"
            "                       flavour splits.                         \n"
            " -l [--list-diagrams]  Gives a short summary table of the gene-\n"
            "                       rated diagrams.                         \n"
            " -d [--detailed-list]  Prints details about all generated dia- \n"
            "                       grams. All labellings are listed, each  \n"
            "                       described by a permutation followed by  \n"
            "                       specifications of the propagator        \n"
            "                       momenta. An 'X' in a position indicates \n"
            "                       that the corresponding momentum is in-  \n"
            "                       cluded in the propagator. (a -> b) in-  \n"
            "                       dicates that the propagator flows from  \n"
            "                       an O(p^a) vertex to an O(p^b) one. For  \n"
            "                       singlet propagators, the momentum of the\n"
            "                       adjacent vertex leg on each side is also\n"
            "                       marked with X's.                        \n"
            " -f [--generate-form]  Generates three .hf files to the output \n"
            "                       directory. These can be used for ampli- \n"
            "                       tude calculations using FORM. Further   \n"
            "                       instructions are printed at the top of  \n"
            "                       the files.                              \n"
            " -t [--generate-tikz]  Generates a .tex file to the output     \n"
            "                       directory, which can be used for drawing\n"
            "                       the diagrams using TikZ. Further in-    \n"
            "                       structions are printed at the top of the\n"
            "                       files.                                  \n"
            " -T [--tikz-split]     Splits the -t output into multiple      \n"
            "                       files, each containing the specified    \n"
            "                       number of diagrams. This can prevent TeX\n"
            "                       memory overflow.                        \n"
            " -r [--tikz-radius]    Sets the radius of the TikZ diagrams to \n"
            "                       the given value, in cm. The default     \n"
            "                       radius is adjusted to the number of     \n"
            "                       legs.                                   \n"
            " -o [--output-dir]     Changes the output directory. Defaults  \n"
            "                       to \"output/\".                         \n"
            " -n [--output-name]    Adds the given string to the beginnning \n"
            "                       of all output filenames. M<n>p<m> is al-\n"
            "                       ways included in the names to specify   \n"
            "                       the order and number of legs.           \n"
            "\n"
            " If you have questions, please email mattias.sjo@thep.lu.se. \n\n";
}

int main(int argc, char** argv) {
    /*
    cout << argc;
    for(int i = 1; i < argc; i++)
        cout << "  " << argv[i];*/
    
    int n_legs, order;
    
    bool gen_form = false, gen_tikz = false;
    bool split_tikz = false, custom_radius = false;
    int tikz_split_size = 0;
    double radius = 0;
    
    bool list = false, detailed = false, verbose = false;
    
    string out_dir = "output/";
    string out_tag = ""; 
    
    bool singlets = true, incl_fsp = false;
    vector< vector<int> > flav_splits = vector< vector<int> >();
    
    opterr = 1;
    const char* short_opts = "hN:O:tT:r:fldvo:n:sSi:x:";
    struct option long_opts[] = {
        {"help",                no_argument,        0, 'h'},
        {"number-of-legs",      required_argument,  0, 'N'},
        {"order",               required_argument,  0, 'O'},
        {"generate-form",       no_argument,        0, 'f'},
        {"generate-tikz",       no_argument,        0, 't'},
        {"tikz-split",          required_argument,  0, 'T'},
        {"tikz-radius",         required_argument,  0, 'r'},
        {"list-diagrams",       no_argument,        0, 'l'},
        {"detailed-list",       no_argument,        0, 'd'},
        {"verbose",             no_argument,        0, 'v'},
        {"output-dir",          required_argument,  0, 'o'},
        {"output-name",         required_argument,  0, 'n'},
        {"singlets",            no_argument,        0, 's'},
        {"no-singlets",         no_argument,        0, 'S'},
        {"include-flav-split",  required_argument,  0, 'i'},
        {"exclude-flav-split",  required_argument,  0, 'x'},
        {0,0,0,0}
    };
    
    
    
    while(true){
        int opt_idx = -1;
        int c = getopt_long(argc, argv, short_opts, long_opts, &opt_idx);
        
        if(c == -1)
            break;
        
        if(opt_idx >= 0 && opt_idx < 16){
            cout << "Option --" << long_opts[opt_idx].name << " (-" << (char)c << ")";
            if(long_opts[opt_idx].has_arg)
                cout << " with argument \"" << optarg << "\"";
            cout << "\n";
        }
        else{
            cout << "Option -" << (char)c;
            if(strchr(short_opts, c)[1] == ':')
                cout << " with argument \"" << optarg << "\"";
            cout << "\n";
        }
        
        switch(c){
            case 'h':
                print_help();
                return 0;
                
            case 'N':
                n_legs = atoi(optarg);      break;
            case 'O':
                order = atoi(optarg);       break;
                
            case 'f':
                gen_form = true;            break;
            case 't':
                gen_tikz = true;            break;
            case 'T':
                split_tikz = true;
                tikz_split_size = atoi(optarg);  
                break;
            case 'r':
                custom_radius = true;
                radius = atof(optarg);     
                break;
                
            case 'l':
                list = true;                break;
            case 'd':
                detailed = true;            break;
            case 'v':
                verbose = true;             break;
                
            case 'o':
                out_dir = string(optarg);   break;
            case 'n':
                out_tag = string(optarg);   break;
                
            case 's':
                singlets = true;            break;
            case 'S':
                singlets = false;           break;
            case 'i':
                incl_fsp = true;
                //Intentional fall-through
            case 'x':
                if(!flav_splits.empty()){
                    cerr << "ERROR: multiple '--include-flav-split' or '--exclude-flav-split'\n";
                    return 1;
                }
                if(parse_flav_split(optarg, flav_splits))
                    return 1;
                break;
                
            default:
                cerr << "ERROR\n";
                return 1;
        }
    }       
    
    if(optind < argc)
        order = atoi(argv[optind++]);
    if(optind < argc)
        n_legs = atoi(argv[optind++]);
    if(optind < argc){
        cerr << "ERROR: too many unnamed arguments (max 2 allowed)\n";
        return 1;
    }
                /*
                
    
    
    
    po::options_description opts("Allowed options");
    opts.add_options()
        ("help,h", "print help message")
        ("number-of-legs,N", po::value<uint>()->default_value(4),
                            "the number of external legs on the diagrams, an even "
                            "integer greater or equal than 4. The first unnamed "
                            "argument is interpreted as the number of legs. If no "
                            "number is given, 4 is assumed.")
        ("order,O", po::value<uint>()->default_value(2),
                            "the order of the diagrams, a strictly positive even" 
                            "integer. The second unnamed argument is interpreted "
                            "as the order. If no order is given, 2 (NLO diagram) "
                            "is assumed.")
        ("generate-tikz,t", "generate TikZ code for drawing diagrams")
        ("tikz-split", po::value<uint>(),
                            "splits the generated TikZ file into several files, "
                            "each containing this many diagrams. Useful when a "
                            "single file overflows LaTeX's memory.")
        ("generate-form,f", "generate FORM code for calculating amplitudes.")
        ("list-diagrams,l", "prints a summary of the generated diagrams.")
        ("detailed-list,L", "prints a detailed description of the generated "
                            "diagrams and their distinct labellings.")
        ("output,o", po::value<string>(), 
                            "selects the directory in which generated files "
                            "are output. The default is \"output/\".")
        ("name,n", po::value<string>(),
                            "prepends a name to the generated files, e.g. "
                            "\"your_name_M8p6_diagr.hf\".")
        ("no-flavour-splits,F",
                            "excludes flavour-split diagrams.")
        ("no-singlets,S",   "excludes singlet diagrams.")
        ("include-flavour-split, I", po::value<string>()->multitoken(), 
                            "takes one or more flavour splits as options and "
                            "discards all diagrams that do not have these splits. "
                            "The splits should be specified as comma-separated lists "
                            "of numbers that add up to the number of legs, and several "
                            "splits can be given "
                            "(e.g. -N=6 --include-flavour-split=6 2,4 3,3)")
        ("exclude-flavour-split, X", po::value<string>()->multitoken(),
                            "works like include-flavour-split, but discards the "
                            "diagrams that have the specified splits rather than "
                            "those that do not.")
        ;
        */
    /*
    po::variables_map var_map;
    po::store(po::command_line_parser(argc, argv)
                    .options(opts)
                    .positional(pos_opts)
                    .run(), 
              var_map);
    po::notify(var_map);*/
    
        /*
    if(var_map.count("help")){
        cout << opts << std::endl;
        return 0;
    }
    */
        
    if(n_legs < 4 || n_legs % 2){
        cerr    << "ERROR: invalid number of legs: " << n_legs << "\n\t(must be even and >= 4)" 
                << endl;
        return 1;
    }
    
    if(order < 2 || order % 2){
        cerr    << "ERROR: invalid order: " << order << "\n\t(must be even and >= 2)" 
                << endl;
        return 1;
    }
    if(split_tikz && tikz_split_size < 1){
        cerr    << "ERROR: invalid tikz file split: " << tikz_split_size 
                << "\n\t(must be a strictly positive integer)"
                << endl;
        return 1;
    }
    if(custom_radius && radius <= 0){
        cerr    << "ERROR: invalid tikz radius: " << radius 
                << "\n\t(must be a strictly positive number)"
                << endl;
        return 1;
    }
    
    cout << "\n" 
         << " --*-*-- FODGE version 2.0 --*-*--\n"
         << " --*-*-- Mattias Sjo, 2019 --*-*--\n";
    cout << "\nGenerating diagrams...\n";
    
    auto diagrs = Diagram::generate(order, n_legs, singlets, true, verbose);
        
    cout << "\n";
    
    if(!flav_splits.empty()){
        cout << Diagram::filter_flav_split(diagrs, flav_splits, incl_fsp)
            << " diagrams removed by flavour split filter "
            << (incl_fsp ? "(inclusive)" : "(exclsive)") << "\n\n";
    }
    
    if(detailed && !diagrs.empty()){
        cout << "Generated diagrams:\n";
        int count = 0;
        for(Diagram& d : diagrs)
            cout << "[" << ++count << "] " << d << endl;
    }
        
    ostringstream filename;
    filename << out_dir << out_tag << (out_tag.empty() ? "M" : "_M") 
             << n_legs << "p" << order;
    
    if(gen_tikz){
        cout << "\n";
        
        if(Diagram::TikZ(filename.str(), diagrs, tikz_split_size, radius))
            return 1;
    }   
    
    if(gen_form){
        cout << "\n";
        
        if(Diagram::FORM(filename.str(), diagrs))
            return 1;
    }
    
    if(list && !diagrs.empty())
        Diagram::summarise(cout << "\n", diagrs);
    
    cout << "\nTotal diagrams: " << diagrs.size() << endl;
    
    return 0;
}

