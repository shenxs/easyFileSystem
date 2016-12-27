#ifndef SHELL_CPP_
#define SHELL_CPP_
#include<iostream>
#include<string>
#include<map>
#include<sstream>
#include<vector>
#include<string.h>
#include"struct.cpp"
#include"utile.cpp"
#include"lowio.cpp"
#include"./bins/command.h"
#include"config.h"
using namespace std;

void login();
vector<string> getJob();
int work(vector<string> job);
void shell();


void login(){
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
}

vector<string> getJob(){
    cout<<currentUser.name<<'@'<<hostname;
    // cout<<':'<<PWD;
    if(currentUser.user_id==0)
        cout<<"#";
    else
        cout<<'$';

    vector<string> job;
    string line;

    //这里很重要,需要先清空,不然会崩溃
    //c++太脏
    cin.clear();
    std::getline(cin,line);
    istringstream iss(line);
    for(string arg;iss>>arg;job.push_back(arg)){}
    return job;
}

int work(vector<string> job){

    string str=job[0];
    if(commandMap[str]!=NULL){
        commandMap[str](job);
    }else{
        //有意思,一点点小魔法
        //古老的ascii协议
        //这东西叫ascii转义序列
        cout<<"\033[1;31m没有这条命令\e[0m"<<str<<endl;
    }

    return 0;
}


void shell(){
    login();
    initCommands();//将所有的命令进行装载
    vector<string> job;

    job=getJob();

    while(job[0]!="exit")
    {
        work(job);
        job=getJob();
    }
}

#endif
