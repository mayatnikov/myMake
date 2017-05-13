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
    string *orig;
    vector<string> *depends;
    vector<string> *commands;
    target *parent;
    string *name;
    bool make();
    bool checkCicleDep();
private:
};

#endif	/* TARGET_H */

