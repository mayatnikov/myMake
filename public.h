/* 
 */

#ifndef PUBLIC_H
#define	PUBLIC_H
#include <regex>
#include <vector>
#include <string>
#include <map>
#include "target.h"

extern int dbg;  // if(trace) {
extern std::vector<std::pair <std::string, std::string> > g_variables;
extern std::vector<target> g_targets;
extern std::string g_make_file;
extern bool g_check_only;

typedef std::map< std::string, std::string > subst_map;


#endif	/* PUBLIC_H */

