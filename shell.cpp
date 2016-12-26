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
#include"./bins/command.h"
using namespace std;


void login();
vector<string> getJob();
int work(vector<string> job);
void shell();

void login(){
    string username,password;
    bool flag=false;
    while(!flag){
        cout<<"Login as:";cin>>username;
        cout<<username<<"'s password:";cin>>password;
        User a_user;
        strcpy(a_user.name,username.c_str());
        strcpy(a_user.password,password.c_str());
        flag=leagleUser(a_user);
    }
    cout<<"Welcome!"<<endl;
}

vector<string> getJob(){
    cout<<"#";
    vector<string> job;
    string line;
    cin.clear();
    getline(cin,line);
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
        cout<<"\033[1;31m没有这条命令\e[0m"<<endl;
    }

    return 0;
}


void shell(){
    // login();
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
