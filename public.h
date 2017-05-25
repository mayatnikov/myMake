/* 
 */

#ifndef PUBLIC_H
#define	PUBLIC_H
#include <regex>
#include <vector>
#include <string>
#include <map>
#include "target.h"

#define DEBUG

#ifdef DEBUG
  #define log(lvl,format, args...) if(dbg>=lvl)fprintf (stdout, format, args)
#else
  #define log(...) ((void)0) //strip out PRINT instructions from code
#endif 


extern int dbg;  // if(trace) {
extern std::vector<std::pair <std::string, std::string> > g_variables;
extern std::vector<target> g_targets;
extern std::string g_make_file;
extern bool g_check_only;
bool find_target(string name);

typedef std::map< std::string, std::string > subst_map;
int parse_file(string );
time_t getFileTime(string );


#endif	/* PUBLIC_H */

