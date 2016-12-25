#ifndef SHELL_CPP_
#define SHELL_CPP_

#include<iostream>
#include<string>
#include<string.h>
#include"struct.cpp"
#include"utile.cpp"
using namespace std;

void login();
string getJob();
int work(string job);


void shell(){
    login();
    while(1){
        string job=getJob();
        work(job);
    }
}


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

string getJob(){
    cout<<"#";

    string job;
    cin>>job;
    return job;
}

int work(string job){
    return 0;
}

#endif
