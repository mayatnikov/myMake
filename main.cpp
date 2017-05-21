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
int dbg=0;  // уровень отладки по умолчанию=0 нет отладки
vector<pair <string, string> > g_variables;  // список голобальных переменных
vector<target> g_targets;    // список целей
target *current_target=nullptr;  // текущая цель на обработке
string g_make_file("Makefile");  // имя Make-file по умолчанию 
bool g_check_only = false;     // признак не исполнять а только проверить порядок действий
string start_target_name("all"); // стартовая цель (меняется через cli полследний параметр 
// ---------------------

/*
 * ОБРАБОТКА строки с ПЕРЕМЕННОЙ
 * запомнить в глобальный vector<pair<string,string>>
 */
void parse_var(string in) {
    log(2,"VARIABLE:%s\n", in.c_str());
//    regex delete_space("\\s*(.*)\\s*");

    string var_name;
    string var_value;
    size_t found;
        found = in.find_first_of("=");
        if(dbg>=3)  { printf("\tdelemiter at: %lu ",found); }
        var_name = trim(in.substr(0,found));
        
        var_value = trim(in.substr(found+1,in.size()));
    log(1,"VAR:\tNAME=[%s]\tVAL=[%s]\n",var_name.c_str(),var_value.c_str()); 
    g_variables.push_back(make_pair( var_name,var_value));
}

/*
 * ОБРАОТКА ЦЕЛИ
 * конструирует новый target 
 * и запоминает его в глобальном vector
 */
void parse_target(string in) {
    string target_name;
    string str_depends;
    vector<string> *depends;
    
    log(2,"parse_target: TARGET:%s\n", in.c_str());
    size_t found;
    found = in.find_first_of(":");
    log(3,"parse_target:\t delemiter at: %lu\n", found);
    target_name = trim(in.substr(0, found));
    str_depends = trim(in.substr(found + 1, in.size()));
    depends = split2words(str_depends);
    target *t; // = new target();
    t= new target();
    t->orig=new string(in);    
    t->name=new string(target_name);
    t->depends=depends;
    current_target=t;
    log(1,"parse_target:\t target=[%s]\t depends=[%s]\n",
            current_target->name->c_str(),
            str_depends.c_str());
    g_targets.push_back(*t);
}

/*
 * ОБРАБОТКА СТРОКИ С SHELL-КОМАНДОЙ
 * все строки начинающиеся с табуляции запоминаются в 
 * текущем target в vector команд - target->commands
 */
void parse_exec(string in) {
    
    string exec(trim(in));
    log(4,"parse_exec: in=[%s]\n", in.c_str());
    log(4,"parse_exec: out=[%s]\n", exec.c_str());
    
    if(current_target!=nullptr) {
        current_target->commands->push_back(exec.c_str());
        log(1,"parse_exec:\t command=[%s]\t target=[%s]\n", 
                exec.c_str(),current_target->name->c_str());
    }
}

/*
 * ОБРАБОТКА СТРОКИ С INCLUDE [какой то файл]
 * строки файла обрабатываются как вставка строк в текущий процесс
 * include могут содержать include ( рекурсивная функция parse_file - parse_include
 * защиты от зацикливания include - НЕТ ! 
 */
int parse_include(string in) {
    
    string file_name(trim(trim(in).substr(7)));
    
    log(4,"parse_include: include=[%s]\n", file_name.c_str());
    return parse_file(file_name);

}

/*
 ЧТЕНИЕ MAKE-ФАЙЛА
 * построчное чтение и вызов методов в зависимости от типа строки 
 */
int parse_file(string file_name) {
    int rc = 0;
    ifstream file(file_name.c_str());
//    vector<string> v; //Вектор строк
    string S; //  read line

    while (std::getline(file, S)) {
        log(4,"parse_file: --------- start new line ----------%s\n",""); 
        if (S.back() == '\r') S.pop_back();
        if (S.back() == '\\') { S.pop_back(); more_lines(S,file); }
// сделать замену переменных
        subst_map substitutions ( &g_variables[0], &g_variables[0] + g_variables.size() );
    
        log(5,"parse_file: subst in:[%s]\n",S.c_str());
        S = do_substitutions( S, substitutions );
        log(5,"parse_file: subst out:[%s]\n",S.c_str());
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
            case 'I': // command (exec)
                parse_include(S);
                break;
        }
    }
    file.close();
    return rc;
}

/*
 * РАЗБОР ОПЦИЙ КОМАНДНОЙ СТРОКИ
 * допустимые опции:
 * -f имя_make_файла
 * -n только анализ ни каких действий не выполняется
 * -d уровень_отладочных_сообщений, больше число больше сообщений (максимум=5)
 * последний_параметр - имя выполняемой цели
 * 
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
        log(2,"target:%s \n", argv[argc - 1]); 
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

    log(3,"---> старт обработки make-файла, формирование модели обработки %s ...\n","");
    parse_file(g_make_file);
        
    log(3,"---> старт обработки правил ...%s \n","");
    target * start_target;
    if((start_target= get_target(start_target_name))!= nullptr) {
        start_target->parent = nullptr;  // это начало цепочки
        start_target->make(); 
    }
    return rc;
};
