#include <stdlib.h>
#include <regex>
#include "target.h"
#include "public.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "stringProcessor.h"

using namespace std;

target::target(string nm, vector<string> *dep) {
    name = new string(nm);
    commands = new vector<string>;
    depends = dep;
}

target::target() {
    depends = new vector<string>;
    commands = new vector<string>;
}

target::~target() {
}

/*
 * TODO - здесь проверка на зацикливание
 */

bool target::checkCicleDep(string to_check) {
    bool rc = true;
    //	if(name.compare(to_check)==0) rc=false; // имя совпало !
    //	if(parent!=nullptr) rc= parent->checkCicleDep(to_check);
    return rc;
}

/*
 * рекурсивно выполнить все цели, начиная с заданной в запросе 
 */

bool target::make() {
    bool resp = false;
    struct stat fileInfo;
    time_t file_time;
    log(4,"make: Start for target:[%s]\n", orig->c_str());

    // запомнить время создания цели (0 - если это не файл)
    if (stat(name->c_str(), &fileInfo) == 0) {
        file_time = mktime(localtime(&fileInfo.st_mtime));
    } else file_time = 0L;

    // проверка всех зависимостей и их выполнение (рекурсия)
    for (unsigned int i = 0; i < depends->size(); i++) {
        
        target * tg2 = get_target(depends->at(i)); // найти цель в полном списке
        if (tg2 == nullptr) {
         //   exit(-1); // не понятно что делать с этой целью
        } else  {
            log(5,"старт вложенного target name = %s\n", tg2->name->c_str());
            resp = tg2->make();
        }
    }

    // выполнить все команды данной цели
    log(4,"make: target name:[%s] exec-list sz:[%lu]\n", name->c_str(), commands->size());
    for (unsigned int i = 0; i < commands->size(); i++) {
        log(4,"make: target:[%s] exec:[%s]\n", name->c_str(), commands->at(i).c_str());
    /* ---------------------------------------------------------------------- */    
        
        printf(">\tsystem(\"%s\");\n",commands->at(i).c_str() );
        // system("....);  // TODO   ВЫПОЛНИТЬ ЦЕЛЕВУЮ КОМАНДУ
    /* ---------------------------------------------------------------------- */    
        resp = true; // этот make что то сделал
    }
    return resp; // true если что то делал
}


/*
 * проверка на цель типа %.o:%.cpp
 * добавляет цель созданную по шаблону в глобальный список
 * возврат вновь сконструированной цели 
 */
target * get_target_with_template(string name, target templ) {

    target * resp = nullptr;

    log(5,"get_target_with_template: Cтарт проверки по шаблону---------------%s\n","");
    regex file_ext_exp("(\\w+)\\.(.+)");
    smatch mfile;
    regex_search(name, mfile, file_ext_exp);

    regex target_exp("(\\%+)\\.(.+)");
    smatch mtarget;
    string s1(templ.name->c_str());
    regex_search(s1, mtarget, target_exp);

    if(templ.depends->size()==0) {
        log(5,"get_target_with_template: зависимостей нет - пропускаем%s\n","");
        return resp; // пропустить цель без зависимостей
    } 
    
    smatch dtarget;
    string s2(templ.depends->at(0).c_str());
    regex_search(s2, dtarget, target_exp);

    if (mfile.size() == 3 &&
            mtarget.size() == 3 &&
            dtarget.size() == 3 &&
            mtarget[1].str().compare(0, 1, "%") == 0 &&
            mtarget[2].str().compare(mfile[2].str()) == 0) { // найден подходящий шаблон
        log(5,"get_target_with_template: Шаблон:[%s] и файл: [%s] - совпадают\n", 
                templ.orig->c_str(), name.c_str());
        string outFile = name; // @$
        string srcFile = mfile[1].str() + "." + dtarget[2].str(); // ^$
        log(5,"get_target_with_template: Шаблон: подстановки var(@$)=%s var(^$)=%s\n", srcFile.c_str(), outFile.c_str());
        // создаем новый target из шаблона
        resp = new target();
        resp->name = new string(outFile);
        resp->depends->push_back(srcFile);
        resp->orig = new string((outFile+":"+srcFile+" # template gen" ).c_str());  
        for (unsigned int i = 0; i < templ.commands->size(); i++) {
            string exec = templ.commands->at(i);
            log(5,"get_target_with_template: шаблон: in exec %s\n", exec.c_str());
            findAndReplace(exec, string("$^"), srcFile);
            findAndReplace(exec, string("$@"), outFile);
            log(5,"get_target_with_template: шаблон: out exec %s\n", exec.c_str());
            resp->commands->push_back(exec);
            log(4,"get_target_with_template: Создана цель по шаблону %s\n", 
                    (outFile+":"+srcFile+" > "+ exec).c_str());
        }
 //       g_targets.push_back(*resp);
    } else {
        log(5,"get_target_with_template: Шаблон: [%s] и файл: [%s] - НЕ совпадают\n", 
                templ.orig->c_str(), name.c_str());
    }
    return resp;
}

/*
 * получить из глобального списка ссылку на цель по ее имени
 * nullptr - если не найдено
 * 
 */
target * get_target(string name) {
    target * response;
    response = nullptr;
    log(4,"get_target: поиск цели с именем: [%s] в списке sz=%lu\n",name.c_str(),g_targets.size());
    for (unsigned int i=0;i<g_targets.size();i++) {
        log(5,"get_target: проверка элемента: [%s]\n",g_targets.at(i).name->c_str());
        if(g_targets.at(i).name->compare(name) == 0 ) {
            response = & g_targets.at(i);
            log(4,"get_target: цель с именем: [%s] - НАЙДЕНА\n",name.c_str());
            break;  // выход при первом совпадении
        }
        else if((response = get_target_with_template(name,g_targets.at(i) ))!=nullptr){
            log(5,"get_target: цель с именем: [%s] - создана по шаблону\n",response->name->c_str());
            break;  // выход при первом совпадении
        }
        else { 
            log(5,"get_target: элемент:[%s] - не подходит, пропустить\n",g_targets.at(i).name->c_str()); 
        }
    }
    return response;
}

/*
 * определить время создания файла ( 0L - файл не найден )
 */
time_t getFileTime(string fileName) {
    time_t response;
    struct stat fileInfo;
    if (stat(fileName.c_str(), &fileInfo) != 0) { // Use stat( ) to get the info
        response = 0L;
    } else {
        response = mktime(localtime(&fileInfo.st_mtime));
    }
    return response;
}
