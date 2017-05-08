#ifndef TARGET_H
#define	TARGET_H
#include <vector>
#include <string>

using namespace std;

class target {
public:
    target(string nm,vector<string> *depends);
    target();
    virtual ~target();
    vector<string> *depends;
    vector<string> *commands;
    string *name;
    bool make();
    bool checkCicleDep(std::string to_check);
private:
};

#endif	/* TARGET_H */

