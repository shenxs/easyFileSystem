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



//===========================函数的申明
Inode readInode(int index);
int writeInode(Inode an_inode);
int addInode(Inode inode);
Superblock getSuperBlock();
int writeSuperBlock(Superblock spb);
int addChild2Dir(Inode parent_node,string childname,int inode_id);
//=========================================================




//===========================函数实现

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
    fs.open(diskname.c_str(),ios_base::out|ios_base::in|ios_base::binary);
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

//传入的inode包括除了id之外的所有信息
int addInode(Inode inode){
    //向inode数组添加一个新的inode,如果成功则返回新的Inode的id,第一个inode编号为0
    //如果添加失败则返回-1
    Superblock spb=getSuperBlock();
    int current=spb.inode_usered;
    if(current>=spb.inode_number){
        return -1;
        //说明已经不能添加新的inode了
    }
    inode.inode_id=current;

    spb.inode_usered++;
    writeSuperBlock(spb);
    writeInode(inode);
    return inode.inode_id;
}

//将一个新的子文件添加到parent下
int addChild2Dir(Inode parent_node,string childname,int inode_id){

    Directory child;
    child.inode_id=inode_id;
    strcpy(child.name,childname.c_str());
    //找到parent对应文件的末尾
    parent_node.filesize+=sizeof(child);//文件夹的大小增加

}
#endif
