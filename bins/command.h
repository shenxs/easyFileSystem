#ifndef _COMMAND_H_
#define _COMMAND_H_

#include<iostream>
#include<map>
#include<vector>
#include<string>
#include"../struct.cpp"
#include"../utile.cpp"
using namespace std;

typedef int (*FnPtr)(vector<string>);
std::map<std::string,FnPtr> commandMap;
User currentUser;
string PWD="/";

int ls(vector<string> args);
int cd(vector<string> args);
int pwd(vector<string> args);

//初始化commandMap()
void initCommands(){
    commandMap.clear();
    commandMap.insert(pair<string,FnPtr>("ls",ls));
    commandMap.insert(pair<string,FnPtr>("cd",cd));
    commandMap.insert(pair<string,FnPtr>("pwd",pwd));
}

int pwd(vector<string> args){
    cout<<PWD<<endl;
    return 0;
}
int ls(vector<string> args){
    //显示当前目录下的内容
    //遍历目录项,应用输出文件名函数
    if(args.size()==1){
        //没有任何参数,显示当前文件夹下的内容
        Inode node=getInode(PWD);
        traverse_ls(node,showDir,currentUser);
    }
    return 0;

}
int cd(vector<string> args){
    //遍历文件项,返回指定的inode的编号
    //修改pwd
    string path=args[1];
    Inode node=getInode(PWD);

}
#endif
