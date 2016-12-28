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
        map<string,FnPtr>::iterator it;
        it=commandMap.find(str);
        commandMap.erase(it);
        //有意思,一点点小魔法
        //古老的ascii协议
        //这东西叫ascii转义序列
        cout<<"\033[1;31m没有这条命令\e[0m"<<str<<endl;
    }

    return 0;
}


void shell(){
    vector<string> job;
    login(job);
    initCommands();//将所有的命令进行装载

    job=getJob();

    while(job[0]!="exit")
    {
        work(job);
        job=getJob();
    }
}

#endif
