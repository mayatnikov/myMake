
#include "stringProcessor.h"
#include "public.h"

using namespace std;

/*
 *  определить тип строки ( переменная цель  команда  коментарии )
 */
char check_line_type(string in) {
    char rc = '?';

    regex var_exp("^[\\W]+\\s*=\\s*.+"); // допустимы пробелы
    regex target_exp("^[\\W]+:.*");
    regex exec_exp("\t.*");
    regex comment_exp("^#");

    if (regex_match(in, comment_exp)) rc = 'C';
    else if (regex_match(in, var_exp)) rc = 'V';
    else if (regex_match(in, target_exp)) rc = 'T';
    else if (regex_match(in, exec_exp)) rc = 'X';
    return rc;
}

//  удаление лишних пробелов слева и справа

string trim(string str) {
    size_t first = str.find_first_not_of("\t ");
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

// проверка на необходимость слияния строк

void more_lines(string & S, ifstream & in) {

    if (dbg >= 4) {
        printf("more_lines in:[%s]\n", S.c_str());
    }
    char last_char;
    last_char = S.back();
    // S.pop_back();
    string buff;
    bool last_line = false;
    while (!last_line) {
        getline(in, buff);
        if (dbg >= 4) {
            printf("buff:[%s]\n", buff.c_str());
        }
        if (buff.back() == '\r') buff.pop_back();
        if (buff.back() == '\\') {
            buff.pop_back();
            S.append(" ");
            S.append(trim(buff));
            //            more_lines(S,in);
        } else {
            if (dbg >= 4) {
                printf("more_lines last append:[%s]\n", buff.c_str());
            }
            S.append(buff);
            last_line = true;
        }
    }
    if (dbg >= 4) printf("more_lines out:[%s]\n", S.c_str());
}

string do_substitutions(string const &in, subst_map const &subst) {
    const char *pl1 = "$(";
    const char *pl2 = ")";

    ostringstream out;
    size_t pos = 0;
    for (;;) {
        size_t subst_pos = in.find(pl1, pos);
        size_t end_pos = in.find(pl2, subst_pos);
        if (end_pos == string::npos) break;
        out.write(&* in.begin() + pos, subst_pos - pos);
        subst_pos += strlen(pl1);
        string place_holder(in.substr(subst_pos, end_pos - subst_pos));
        subst_map::const_iterator subst_it = subst.find(place_holder);
        if (subst_it == subst.end()) throw runtime_error(string("не могу сделать замену переменной:") + place_holder);
        out << subst_it->second;
        pos = end_pos + strlen(pl2);
    }
    out << in.substr(pos, string::npos);
    return out.str();
}

/*
 * режем строку на слова
 */
vector<string>  * split2words(string in) {
    vector<string> *res;
    res = new std::vector<string>;
    istringstream iss(in);
    do {
        string sub;
        iss >> sub;
	if(dbg>=5) printf("split2word() substring: [%s]\n",sub.c_str());
	if(sub.size()>0) res->push_back(sub);
    } while (iss);
    if(dbg>=4) for (unsigned int i=0;i<res->size();i++) 
        printf("split2words:\t%d)[%s]\n",i,res->at(i).c_str()); //Вывод слов на экран
    return res;
}

target * get_target(string name) {
    target * response;
    response = nullptr;
    if(dbg>=4) printf("поиск цели с именем: [%s] в списке sz=%lu\n",name.c_str(),g_targets.size());
    for (unsigned int i=0;i<g_targets.size();i++) {
        if(dbg>=5) printf("проверка элемента: [%s]\n",g_targets.at(i).name->c_str());
        if(g_targets.at(i).name->compare(name) == 0 ) {
            response = & g_targets.at(i);
            if(dbg>=4) printf("цель с именем: [%s] - НАЙДЕНА\n",name.c_str());
            break;
        }
        else { 
            if(dbg>=5) printf("элемент:[%s] - не подходит, пропустить\n",g_targets.at(i).name->c_str()); 
        }
    }
    return response;
}