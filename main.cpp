#include <map>
#include <vector>
#include <string>
#include <regex>
#include <fstream>
#include <unistd.h>
#include "target.h"
#include "stringProcessor.h"
#include "public.h"

using namespace std;
extern string do_substitutions( string const &in, subst_map const &subst );

/*
 * ГЛОБАЛЬНЫЕ ПАРАМЕТРЫ
 */
int dbg=4;  // уровень отладки по умолчанию
vector<pair <string, string> > g_variables;  // список голобальных переменных
vector<target> g_targets;    // список целей
target *current_target=nullptr;  // текущая цель на обработке
string g_make_file("Makefile");  // имя Make-file по умолчанию 
bool g_check_only = false;     // признак не исполнять а только проверить порядок действий
string start_target_name("all"); // стартовая цель (меняется через cli полследний параметр 
// ---------------------

/*
 * ОБРАБОТКА ПЕРЕМЕННОЙ
 */
void parse_var(string in) {
    if(dbg>=2) printf("VARIABLE:%s\n", in.c_str());
//    regex delete_space("\\s*(.*)\\s*");

    string var_name;
    string var_value;
    size_t found;
        found = in.find_first_of("=");
        if(dbg>=3)  { printf("\tdelemiter at: %lu ",found); }
        var_name = trim(in.substr(0,found));
        var_value = trim(in.substr(found+1,in.size()));
    if(dbg>=1)   printf("VAR:\tNAME=[%s]|VAL=[%s]\n",var_name.c_str(),var_value.c_str()); 
    g_variables.push_back(make_pair( var_name,var_value));
}

/*
 * ОБРАОТКА ЦЕЛИ
 */
void parse_target(string in) {
    string target_name;
    string str_depends;
    vector<string> *depends;
    
    if (dbg >= 2) printf("TARGET:%s\n", in.c_str());
    size_t found;
    found = in.find_first_of(":");
    if (dbg >= 3) {
        printf("\tdelemiter at: %lu\n", found);
    }
    target_name = trim(in.substr(0, found));
    str_depends = trim(in.substr(found + 1, in.size()));
    depends = split2words(str_depends);
    target *t; // = new target();
    t= new target();
    t->name=new string(target_name);
    t->depends=depends;
    current_target=t;
    if(dbg>=1) printf("TARGET: name=[%s] depends=[%s]\n",
            current_target->name->c_str(),
            str_depends.c_str());
    g_targets.push_back(*t);
}

/*
 * ОБРАБОТКА СТРОКИ С SHELL-КОМАНДОЙ
 */
void parse_exec(string in) {
    
    string exec(trim(in));
    if(dbg>=4) printf("EXEC in:[%s]\n", in.c_str());
    if(dbg>=4) printf("EXEC out:[%s]\n", exec.c_str());
    
    if(current_target!=nullptr) {
        current_target->commands->push_back(exec.c_str());
        if(dbg >= 1) printf("EXEC push command:[%s]-->%s\n", exec.c_str(),current_target->name->c_str());
    }
}

/*
 ЧТЕНИЕ MAKE-ФАЙЛА 
 */
int parse_file() {
    int rc = 0;
    ifstream file(g_make_file.c_str());
    vector<string> v; //Вектор строк
    string S; //  read line

    while (std::getline(file, S)) {
        if(dbg>=4) { printf("--------- start new line ----------\n"); }
        if (S.back() == '\r') S.pop_back();
        if (S.back() == '\\') { S.pop_back(); more_lines(S,file); }
// сделать замену переменных
        subst_map substitutions ( &g_variables[0], &g_variables[0] + g_variables.size() );
    
        if(dbg>=5) printf("subst in:[%s]\n",S.c_str());
        S = do_substitutions( S, substitutions );
        if(dbg>=5) printf("subst out:[%s]\n",S.c_str());
// ---        
        
        switch (check_line_type(S)) {
            case 'C': // коментарии
                // пропускаем молча
                break;
            case 'V': // переменная
                parse_var(S);
                break;
            case 'T': // цель (target)
                parse_target(S);
                break;
            case 'X': // command (exec)
                parse_exec(S);
                break;
        }
    }
    return rc;
}

/*
 РАЗБОР ОПЦИЙ КОМАНДНОЙ СТРОКИ
 */
void parse_options(int argc, char *argv[]) {
    int rez = 0;
    extern char *optarg;
    extern int optind, optopt;

    while ((rez = getopt(argc, argv, "d:f:n")) != -1) {
        switch (rez) {
            case 'f':
                g_make_file = optarg;
                break;
            case 'd':
                dbg = stoi(optarg);
                break;
            case 'n':;
                g_check_only = true;
                break;
            case ':': /* -f without operand */
                fprintf(stderr,"Option -%c requires an operand\n", optopt);
                break;
            case '?': printf("Error found !\n");
            /* TODO: сделать help по синтаксису */
                break;
        }
    }
    if (argc > optind) { 
        start_target_name = string(argv[argc - 1]);
        if(dbg>=2) printf("target:%s \n", argv[argc - 1]); 
    }
    else printf("Unknown target!\n");
    if(dbg>=2) {
        printf(">>>>>>>>>>>>>>>> Start mymake with options:\n");
        printf("make file(-f): %s \n", g_make_file.c_str());
        printf("debug level: %d \n", dbg);
        printf("check only(-n) YES-1/NO-0 %d \n",g_check_only);
        printf("---\n");
    }
}

int main(int argc, char *argv[]) {
    parse_options(argc, argv);
    int rc=0;
    
    
    if(dbg>=3) printf("---> старт обработки make-файла\n");
    parse_file();
        
    if(dbg>=3) printf("---> старт обработки правил\n");
    target * start_target;
    if((start_target= get_target(start_target_name))!= nullptr) {
       start_target->make(); 
    }
    return rc;
};
