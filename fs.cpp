#include<iostream>
#include<string>
#include<fstream>
#include"utile.cpp"
#include"config.h"
#include"lowio.cpp"
using namespace std;


//用于初始化文件系统
void init(){
    cout<<"准备初始化文件系统"<<endl;

    //打开虚拟磁盘
    opendisk();

    if(!fs.is_open()){
        cout<<"虚拟磁盘打开失败"<<endl;

    }else{
        cout<<"成功打开虚拟磁盘"<<endl;

        closedisk();
        init_fs();

        if(gooddisk()){
            cout<<"发现文件系统,准备装载"<<endl;
        }else{
            cout<<"未发现文件系统,准备重新初始化"<<endl;
            init_fs();
        }
    }

}
