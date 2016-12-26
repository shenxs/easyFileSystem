#include<iostream>
#include<string>
#include<fstream>
#include"utile.cpp"
#include"config.h"
#include"lowio.cpp"
using namespace std;


//用于初始化文件系统
void init(){

    //打开虚拟磁盘
    opendisk();

    if(!fs.is_open()){
        cout<<"虚拟磁盘打开失败"<<endl;
    }else{
        closedisk();
        init_fs();
        if(gooddisk()){

        }else{
            cout<<"未发现文件系统,准备重新初始化"<<endl;
            init_fs();
        }
    }

}
