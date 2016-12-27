#ifndef    CONFIG_H_
#define    CONFIG_H_

#include<string>
#include<fstream>
//记录文件系统的常量
//一些经常用到的全局参数
int block_size=512;
int maggci_number=9527;
int block_number=1024*2;
int data_start=48;
int stack_size=100;//成组链接法的一组的大小
int addressNumber=block_size/sizeof(short );//一个块可以存放多少的地址项
std::string diskname="virtualdisk";
std::string hostname="dog";
std::fstream fs; //文件流
#endif
