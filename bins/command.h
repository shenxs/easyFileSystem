#ifndef _COMMAND_H_
#define _COMMAND_H_

#include<iostream>
#include<map>
#include<vector>
#include<string>
#include <termios.h>
#include <unistd.h>

#include"../struct.cpp"
#include"../utile.cpp"
using namespace std;

typedef int (*FnPtr)(vector<string>);
std::map<std::string,FnPtr> commandMap;
User currentUser;
Inode currentInode;
string PWD="/";

inline void eatline(){while (std::cin.get()!='\n') continue;}

string getpasswdfromcin(){
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    string s;
    getline(cin, s);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cout<<endl;
    return s;
}

int ls(vector<string> args);
int cd(vector<string> args);
int pwd(vector<string> args);
int common_mkdir(vector<string> args);
int whoami(vector<string> args);
int login(vector<string> args);
int help(vector<string> args);
int touch(vector<string> args);
int cat(vector<string> args);
int clear(vector<string> args);
int chgrp(vector<string> args);
int chmod(vector<string> args);
int chown(vector<string> args);
int passwd(vector<string> args);
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
    commandMap.insert(pair<string,FnPtr>("cat",cat));
    commandMap.insert(pair<string,FnPtr>("clear",clear));
    commandMap.insert(pair<string,FnPtr>("chmod",chmod));
    commandMap.insert(pair<string,FnPtr>("chown",chown));
    commandMap.insert(pair<string,FnPtr>("chgrp",chgrp));
    commandMap.insert(pair<string,FnPtr>("passwd",passwd));
}


int chgrp(vector<string> args){
    cout<<"施工中"<<endl;
    return 0;
}
int chown(vector<string> args){
    cout<<"施工中"<<endl;
    return 0;
}


//可以改变用户的密码
//直接使用passwd可更改当前用户的密码
//passwd + username可以更改 username的名字
int passwd(vector<string> args){
    if(args.size()==1){
        cout<<"更改当前用户"<<currentUser.name<<"的密码"<<endl;
        cout<<"请输入当前的密码 :";
        // cin.clear();
        string oldpass=getpasswdfromcin();
        if(oldpass==currentUser.password){
            cout<<"请输入新密码 :";
            string pass1=getpasswdfromcin();
            cout<<"请再次输入密码 :";
            string pass2=getpasswdfromcin();
            if(pass1==pass2){
                strcpy(currentUser.password,pass1.c_str());
                saveTopasswd(currentUser);
                cout<<"密码修改成功"<<endl;
            }else{
                cout<<"密码不配"<<endl;
            }
        }else{
            cout<<"密码错误"<<endl;
        }
    }else if(args.size()==2){
    }else{
        cout<<"命令格式错误"<<endl;
    }
    return 0;
}
int chmod(vector<string> args){
    cout<<"施工中"<<endl;
    return 0;
}

//清屏
int clear(vector<string> args){
    cout<<" \033c";
    return 0;
}
//输出一个中的内容,只能是f
int cat(vector<string> args){
    if(args.size()==1){
        return -1;
    }else{
        string file=args[1];
        int InodeId=getInodeidFromDir(currentInode,file);
        if(InodeId==-1){
            cout<<file<<"不存在"<<endl;
        }else{
            Inode node=readInode(InodeId);
            if(node.permissions[0]!='f'){
                cout<<node.filename<<"不是一个文件"<<endl;
            }else{
                if(canI(node,currentUser,1)){
                    for(int i=0;i<node.filesize;i++){
                        int address=getFileAddress(node,i);
                        opendisk();
                        fs.seekg(address,ios_base::beg);
                        char c;
                        fs.read((char *)&c,sizeof(c));
                        closedisk();
                        cout<<c;
                    }
                    cout<<endl;
                }else{
                    cout<<"对不起你没有访问权限"<<endl;
                }
            }
        }

    }
    return 0;
}


//touch如果没有此文件则创建一个空文件,如果有此文件则更新其时间戳
//touch name
//new file or updatefile mtime
//TODO 检查文件名是否合法
int touch(vector<string> args){
    if(args.size()<=1){
        return -1;
    }else{
        string name=args[1];
        int id=getInodeidFromDir(currentInode,name);
        if(canI(currentInode,currentUser,2)){
            if(id==-1){
                //如果不存在此文件则新建
                string path=getPath(currentInode);
                path=path+"/"+name;
                touch(path,"frw-rw-r--",currentUser.user_id,currentUser.group_id);
                currentInode=readInode(currentInode.inode_id);
                return 0;
            }else{
                //已经存在这个文件了,更新他的时间戳
                Inode temp=readInode(id);
                temp.mtime=time(0);
                writeInode(temp);
                return 0;
            }
        }else{
            cout<<"对不起没有权限"<<endl;
        }
    }
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
        eatline();
        cout<<username<<"'s password:";
        password=getpasswdfromcin();
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
    // cin.ignore();
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
            // Inode node=getInode(currentInode,path);
            traverse_ls(currentInode,showDirDetial,currentUser);
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
        Inode temp=getInode(readInode(0),path.substr(1,path.length()));
        if(temp.permissions[0]=='d')
            currentInode=temp;
        else{
            cout<<temp.filename<<"不是一个文件夹"<<endl;
        }
    }else{//相对路径

        Inode temp=getInode(currentInode,path);
        if(temp.permissions[0]=='d'){
            currentInode=temp;
        }else{
            cout<<temp.filename<<"不是一个文件夹"<<endl;
        }
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
        string path=getPath(currentInode)+"/"+new_dir_name;
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
