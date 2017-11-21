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
    return 0;
}

//需要测试的时候可以在此测试
void test(){
}
