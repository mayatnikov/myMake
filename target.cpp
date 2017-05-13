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

/*
 * проверка наличия циклической зависимости = true, если есть зависимость от самого себя
 */
bool target::checkCicleDep() {
    bool rc = false;
    for(target *t = this;t->parent!=nullptr;t=t->parent) {
        if(name->compare(*(t->parent->name)) == 0) { // в цепочке нашлась цель с таким же именем - БЕДА!
          fprintf(stderr,"ERROR: Циклическая зависимость, эта target name=[%s] совпадает с предыдущей в цепочке=[%s]\n",
                  name->c_str(),t->orig->c_str());
          rc=true;
          break;
        }   
    }
    return rc;
}

/*
 * рекурсивно выполнить все цели, начиная с заданной в запросе 
 */

bool target::make() {
    bool resp = false;
    struct stat fileInfo;
    time_t file_time;
    log(4, "make: Start for target:[%s]\n", orig->c_str());

    // запомнить время создания цели (0 - если это не файл)
    if (stat(name->c_str(), &fileInfo) == 0) {
        file_time = mktime(localtime(&fileInfo.st_mtime));
    } else file_time = 0L;

    // проверка всех зависимостей и их выполнение (рекурсия)
    for (unsigned int i = 0; i < depends->size(); i++) {

        target * tg2 = get_target(depends->at(i)); // найти цель в полном списке
        if (tg2 == nullptr) {
            //   exit(-1); // не понятно что делать с этой целью
        } else {
            tg2->parent = this;
            if(tg2->checkCicleDep()==true) { exit(-1); }
            log(5, "старт вложенного target name = %s\n", tg2->name->c_str());
            resp = tg2->make();
        }
    }
    
    // после выполнения всех вложенных целей выполнить все команды данной цели
    // выполнить все команды данной цели
    log(4, "make: target name:[%s] exec-list sz:[%lu]\n", name->c_str(), commands->size());
    for (unsigned int i = 0; i < commands->size(); i++) {
        log(2, "make: target:[%s] exec:[%s]\n", name->c_str(), commands->at(i).c_str());
        /* ---------------------------------------------------------------------- */

        log(1,">\tsystem(\"%s\");\n", commands->at(i).c_str());
        if(!g_check_only) {
        /* system("....);  //    ВЫПОЛНИТЬ ЦЕЛЕВУЮ КОМАНДУ
        ---------------------------------------------------------------------- */
            log(0,"myMake: %s\n",commands->at(i).c_str());
            int rc = system(commands->at(i).c_str());
            if(rc!=0) fprintf(stderr,">make: ERROR при выполнении:[%s]\n", commands->at(i).c_str());
        }
        resp = true; // этот make что то сделал
    }
    return resp; // true если что то делал
}

/*
 * проверка на цель типа %.o:%.cpp
 * 
 * возврат вновь сконструированной цели 
 */
target * get_target_with_template(string name, target templ) {

    target * resp = nullptr;

    log(5, "get_target_with_template: Cтарт проверки по шаблону ---%s\n", "");
    // имя файла отделяем имя от расширения --> mfile
    regex file_ext_exp("([\\w\\+]+)\\.(.+)");
    smatch mfile;
    regex_search(name, mfile, file_ext_exp);

    // target: отделяем имя от расширения --> mtarget
    regex target_exp("(\\%+)\\.(.+)");
    smatch mtarget;
    string s1(templ.name->c_str());
    regex_search(s1, mtarget, target_exp);

    if (templ.depends->size() == 0) {
        log(5, "get_target_with_template: зависимостей нет - пропускаем%s\n", "");
        return resp; // пропустить цель без зависимостей
    }
// depends: считаем что списка тут не бывает! берем первый элемент и отделяем имя от расширения --> dtarget 
    smatch mdepends;
    string s2(templ.depends->at(0).c_str());
    regex_search(s2, mdepends, target_exp);
    log(5, "get_target_with_template: Target:[%s . %s] Depends:[%s . %s] File:[%s . %s]\t",
                              mtarget[1].str().c_str(),mtarget[2].str().c_str(),
                              mdepends[1].str().c_str(),mdepends[2].str().c_str(),
                              mfile[1].str().c_str(),  mfile[2].str().c_str());

    if (mfile.size() == 3 &&
            mtarget.size() == 3 &&
            mdepends.size() == 3 &&
            mtarget[1].str().compare(0, 1, "%") == 0 &&
            mtarget[2].str().compare(mfile[2].str()) == 0) { // найден подходящий шаблон
        log(5," Шаблон подходит - пробуем%s\n","");        
        string outFile = name; // @$
        // исходное имя файла как результат но с расширением из шаблона, то есть ../core/ff.o пораждает ../core/ff.cpp 
        string srcFile = name;
        findAndReplace(srcFile,mfile[1].str()+"."+mfile[2].str(),mfile[1].str()+"."+mdepends[2].str());
        //   путь/имя_результата , ищем имя.расширение  , меняем на имя.расширение_из_dependency
        
        log(5, "get_target_with_template: переменные для подстановки подстановки var(@$)=%s var(^$)=%s\n", 
                srcFile.c_str(), outFile.c_str());
        if (getFileTime(srcFile) == 0L) { // файла с таким именем нет - значит шаблон не подходит 
            log(2, "get_target_with_template: WARINING: source file=%s not found, skip this target\n", 
                    srcFile.c_str());
        }
        else {  // ура! все подошло - шаблон найден,  создаем новый target из шаблона
            resp = new target();
            resp->name = new string(outFile);
            resp->depends->push_back(srcFile);
            resp->orig = new string((outFile + ":" + srcFile + " # template gen").c_str());
            for (unsigned int i = 0; i < templ.commands->size(); i++) {
                string exec = templ.commands->at(i);
                log(5, "get_target_with_template: шаблон: in exec %s\n", exec.c_str());
                findAndReplace(exec, string("$^"), srcFile);
                findAndReplace(exec, string("$@"), outFile);
                log(5, "get_target_with_template: шаблон: out exec %s\n", exec.c_str());
                if(getFileTime(outFile) > getFileTime(srcFile) ) {
                    log(4, "File %s up to date - пропустить исполнение\n", outFile.c_str());
                }
                else {
                    resp->commands->push_back(exec);
                    log(4, "get_target_with_template: Создана цель по шаблону %s\n",
                            (outFile + ":" + srcFile + " > " + exec).c_str());
                }
            }
        }
        //       g_targets.push_back(*resp);
    } else {
        log(5, " Шаблон: [%s] и файл: [%s] - НЕ совпадают, пропускаем\n",
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
    log(4, "get_target: поиск цели с именем: [%s] в списке sz=%lu\n", name.c_str(), g_targets.size());
    for (unsigned int i = 0; i < g_targets.size(); i++) {
        log(5, "get_target: проверка элемента: [%s]\n", g_targets.at(i).name->c_str());
        if (g_targets.at(i).name->compare(name) == 0) {
            response = &g_targets.at(i);
            log(4, "get_target: цель с именем: [%s] - НАЙДЕНА\n", name.c_str());
            break; // выход при первом совпадении
        } else if ((response = get_target_with_template(name, g_targets.at(i))) != nullptr) {
            log(5, "get_target: цель с именем: [%s] - создана по шаблону\n", response->name->c_str());
            break; // выход при первом совпадении
        } else {
            log(5, "get_target: элемент:[%s] - не подходит, пропустить\n", g_targets.at(i).name->c_str());
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
