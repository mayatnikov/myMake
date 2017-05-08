#include <stdlib.h>
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

bool target::checkCicleDep(string to_check) {
    bool rc = true;
    //	if(name.compare(to_check)==0) rc=false; // имя совпало !
    //	if(parent!=nullptr) rc= parent->checkCicleDep(to_check);
    return rc;
}

bool target::make() {
    bool resp = false;
    struct stat fileInfo;
    time_t file_time;
    if (dbg >= 4) printf("make: Start for target:[%s]\n", name->c_str());
    // запомнить время создания цели (0 - если это не файл)
    if (stat(name->c_str(), &fileInfo) == 0) {
        file_time = mktime(localtime(&fileInfo.st_mtime));
    } else file_time = 0L;
    for (unsigned int i = 0; i < depends->size(); i++) {
        target * tg2 = get_target(depends->at(i)); // найти цель в полном списке
        if (tg2 == nullptr) {
         //   exit(-1); // не понятно что делать с этой целью
        } else resp = tg2->make();
    }
    if (dbg >= 4) printf("make: target name:[%s] exec-list sz:[%lu]\n", name->c_str(), commands->size());
    for (unsigned int i = 0; i < commands->size(); i++) {
        if (dbg >= 4) printf("make: target:[%s] exec:[%s]\n", name->c_str(), commands->at(i).c_str());
        resp = true; // этот make что то сделал
    }
    return resp;
}