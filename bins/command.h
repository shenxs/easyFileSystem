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
Inode currentInode;
string PWD="/";

int ls(vector<string> args);
int cd(vector<string> args);
int pwd(vector<string> args);
int common_mkdir(vector<string> args);
int whoami(vector<string> args);
//初始化commandMap()
void initCommands(){
    commandMap.clear();
    commandMap.insert(pair<string,FnPtr>("ls",ls));
    commandMap.insert(pair<string,FnPtr>("cd",cd));
    commandMap.insert(pair<string,FnPtr>("pwd",pwd));
    commandMap.insert(pair<string,FnPtr>("mkdir",common_mkdir));
    commandMap.insert(pair<string,FnPtr>("whoami",whoami));
}

int pwd(vector<string> args){
    string path="/";
    Inode temp=currentInode;
    int parent_id;
    parent_id=getInodeidFromDir(temp,"..");
    while(temp.inode_id!=parent_id){//不是根节点
        path=temp.filename+path;
        path="/"+path;
        temp=readInode(parent_id);
        parent_id=getInodeidFromDir(temp,"..");
    }
    cout<<path<<endl;
    return 0;
}
int whoami(vector<string> args){
    cout<<"name:\t"<<currentUser.name<<endl;
    cout<<"id:\t"<<currentUser.user_id<<endl;
    cout<<"group_id:\t"<<currentUser.group_id<<endl;
    return 0;
}
int ls(vector<string> args){
    //显示当前目录下的内容
    //遍历目录项,应用输出文件名函数
    if(args.size()==1){
        //没有任何参数,显示当前文件夹下的内容
        // Inode node=getInode(PWD);
        traverse_ls(currentInode,showDir,currentUser);
    }else{//包含一个路径的参数
        string path=args[1];
        if(path[0]=='/'){
            Inode node=getInode(readInode(0),path.substr(1,path.length()));
            traverse_ls(node,showDir,currentUser);
        }else{
            Inode node=getInode(currentInode,path);
            traverse_ls(node,showDir,currentUser);
        }
    }
    return 0;
}
int cd(vector<string> args){
    //遍历文件项,返回指定的inode的编号
    //修改pwd
    string path=args[1];
    // Inode node=getInode(PWD);
    if(path[0]=='/')//绝对路径
    {
        currentInode=getInode(readInode(0),path.substr(1,path.length()));
    }else{//相对路径
        currentInode=getInode(currentInode,path);
    }
    return 0;
}
int common_mkdir(vector<string> args){

}
#endif
