#ifndef _COMMAND_H_
#define _COMMAND_H_

#include"ls.cpp"
#include"cd.cpp"
#include<map>
#include<string>
#include"../struct.cpp"
using namespace std;

int ls(vector<string> args);

typedef int (*FnPtr)(vector<string>);

std::map<std::string,FnPtr> commandMap;
User currentUser;
string PWD="/";

//初始化commandMap()
void initCommands(){
    commandMap.clear();
    commandMap.insert(pair<string,FnPtr>("ls",ls));
    commandMap.insert(pair<string,FnPtr>("cd",cd));
}

#endif
