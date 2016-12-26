#include<iostream>
#include<string>
#include"fs.cpp"
#include"struct.cpp"
#include"help.cpp"
#include"shell.cpp"
using namespace std;

void test();

int main(){
    init();//初始化文件系统
    shell();//启动shell
    // test();
    return 0;
}

void test(){
    showInodes();
    // cout<<get_parentPath("/home/richard/playground");
    // showInode(readInode(getInodeidFromDir(getInode("/etc"),"passwd")));
    // showInode(getInode(readInode(0),"home/richard/playground"));
    string str="sdsdd";
    // showInodes();
}
