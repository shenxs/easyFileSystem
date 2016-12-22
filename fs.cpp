#include<iostream>
#include<string>
#include<fstream>
#include"utile.cpp"
using namespace std;

string diskname="virtualdisk";


//用于初始化文件系统
void init(){
    cout<<"准备初始化文件系统"<<endl;
    fstream fs;

    //打开虚拟磁盘
    fs.open(diskname.c_str(),ios_base::in|ios_base::out|ios_base::binary);

    if(!fs.is_open()){
        cout<<"虚拟磁盘打开失败"<<endl;

    }else{
        cout<<"成功打开虚拟磁盘"<<endl;

        init_fs(diskname);

        if(gooddisk(diskname)){
            cout<<"发现文件系统,准备装载"<<endl;
            // loadfs(diskname);
        }else{
            cout<<"未发现文件系统,准备重新初始化"<<endl;
            init_fs(diskname);
            loadfs(diskname);
        }
    }

}
