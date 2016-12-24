#ifndef LOWIO_CPP_
#define LOWIO_CPP_
#include<iostream>
#include<fstream>
#include<string>
#include<string.h>
#include"struct.cpp"
#include"config.h"
using namespace std;
//底层的磁盘的读写,读取写入spb,inode,添加一个目录项

//将对于磁盘的io放在此处

//inode的编号=====>对应的inode
Inode readInode(int index){
    Inode temp;
    fs.open(diskname.c_str(),ios_base::in|ios_base::binary);
    fs.seekg(2*sizeof(Block)+index*sizeof(Inode),ios_base::beg);
    fs.read((char *)&temp,sizeof(temp));
    fs.close();
    return temp;
}

//将一个Inode写回磁盘,如果成功则返回0
int writeInode(Inode an_inode){
    fs.open(diskname.c_str(),ios_base::out|ios_base::binary);
    fs.seekp(2*sizeof(Block)+an_inode.inode_id*sizeof(an_inode),ios_base::beg);
    fs.write((char*)&an_inode,sizeof(an_inode));
    fs.close();
    return 0;
}

Superblock getSuperBlock(){
    Superblock spb;
    fstream fs;
    fs.open(diskname.c_str(),ios_base::in|ios_base::binary);
    fs.seekg(sizeof(Block),ios_base::beg);
    fs.read((char *)&spb,sizeof(Superblock));
    fs.close();
    return spb;
}

int writeSuperBlock(Superblock spb){
    fs.open(diskname.c_str(),ios_base::out|ios_base::binary);
    fs.seekp(sizeof(Block),ios_base::beg);
    fs.write((char*)&spb,sizeof(spb));
    fs.close();
    return 0;
}


#endif
