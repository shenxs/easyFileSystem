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
int login(vector<string> args);
int help(vector<string> args);
int touch(vector<string> args);
//初始化commandMap()
void initCommands(){
    commandMap.clear();
    commandMap.insert(pair<string,FnPtr>("ls",ls));
    commandMap.insert(pair<string,FnPtr>("cd",cd));
    commandMap.insert(pair<string,FnPtr>("pwd",pwd));
    commandMap.insert(pair<string,FnPtr>("mkdir",common_mkdir));
    commandMap.insert(pair<string,FnPtr>("whoami",whoami));
    commandMap.insert(pair<string,FnPtr>("login",login));
    commandMap.insert(pair<string,FnPtr>("su",login));
    commandMap.insert(pair<string,FnPtr>("help",help));
    commandMap.insert(pair<string,FnPtr>("touch",touch));
}

//touch如果没有此文件则创建一个空文件,如果有此文件则更新其时间戳
int touch(vector<string> args){
    cout<<"施工中"<<endl;
    return 0;
}
int help(vector<string> args){
    map<string,FnPtr>::iterator it=commandMap.begin();
    cout<<"当前可用命令列表"<<endl;
    for(;it!=commandMap.end();it++){
        cout<<it->first<<endl;
    }
    cout<<"exit"<<endl;
    return 0;
}
int login(vector<string> args){
    string username,password;
    int flag=0;
    User a_user;
    while(!flag){
        cout<<"Login as:";cin>>username;
        cout<<username<<"'s password:";cin>>password;
        strcpy(a_user.name,username.c_str());
        strcpy(a_user.password,password.c_str());
        flag=leagleUser(a_user);
    }
    cout<<"Welcome!"<<endl;

    // 登录完成后需要设定初始的环境变量
    currentUser=readUser(flag);
    if(strcmp(a_user.name,"root")==0)
        PWD="/root";
    else{
        string home="/home/";
        PWD=home+a_user.name;
    }
    currentInode=getInode(PWD);
    cin.ignore();
    return 0;

}
int pwd(vector<string> args){
    cout<<getPath(currentInode)<<endl;
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
        if(path[0]=='/'){//如果是绝对路径
            Inode node=getInode(readInode(0),path.substr(1,path.length()));
            traverse_ls(node,showDir,currentUser);
        }else if(path=="-l"){//如果是有-l参数
            Inode node=getInode(currentInode,path);
            traverse_ls(node,showDirDetial,currentUser);

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
    //新建一个文件夹,如果用户对于当前文件夹有可写权限的话
    //检查是否有重名
    //TODO检查文件名的合法性
    string permissions=currentInode.permissions;
    if(args.size()==1){
        cout<<"请输入新建文件夹的文件名"<<endl;
        return -1;
    }else if(canI(currentInode,currentUser,2)){
        string new_dir_name=args[1];
        string path=getPath(currentInode)+new_dir_name;
        //需要检查重名
        if(getInodeidFromDir(currentInode,new_dir_name)==-1){
            mkdir(path,"drwxrwxr-x",currentUser.user_id,currentUser.group_id);
        }else{
            cout<<"已存在此文件夹"<<endl;
        }
    }else{
        cout<<"对不起,你没有当前目录的写权限"<<endl;
    }

    //写入新的文件夹后需要重读currentInode因为文件的大小变了
    currentInode=readInode(currentInode.inode_id);
    return 0;
}
#endif
