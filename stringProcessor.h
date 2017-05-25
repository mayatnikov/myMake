/* 
 * File:   stringProcessor.h
 * Author: vitaly
 *
 * Created on May 6, 2017, 5:27 PM
 */

#ifndef STRINGPROCESSOR_H
#define	STRINGPROCESSOR_H
#include <string>
#include <fstream>
#include <regex>
#include <map>
#include <sstream>
#include "target.h"

using namespace std;

char check_line_type(string);
string trim(string );
void more_lines(string & , ifstream & );
// std::string do_substitutions(std::string const &, std::subst_map const &);
vector<string> * split2words(string in);
target * get_target(string);
void findAndReplace(string &str, string const &,string const &);
bool findVar(string name);
#endif	/* STRINGPROCESSOR_H */

