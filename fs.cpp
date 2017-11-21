#ifndef FS_CPP_
#define FS_CPP_


#include<iostream>
#include<string>
#include<fstream>
#include"utile.cpp"
#include"config.h"
#include"lowio.cpp"
using namespace std;


//======================函数申明==========================
void init();
void init_fs();
bool gooddisk();
Superblock init_superBolck();
//======================函数实现==========================

//用于初始化文件系统
void init(){
    //打开虚拟磁盘
    opendisk();
    if(!fs.is_open()){
        cout<<"虚拟磁盘打开失败"<<endl;
    }else{
        closedisk();
        // init_fs();
        if(gooddisk()){

        }else{
            cout<<"未发现文件系统,准备重新初始化"<<endl;
            init_fs();
        }
    }

}
//在虚拟磁盘上初始化一个可用的文件系统
void init_fs(){
    Block blocks[block_number];
    opendisk();
    fs.write((char *)&blocks,sizeof(blocks));

    //跳过MBR
    fs.seekp(sizeof(Block));
    Superblock spb;
    spb=init_superBolck();
    fs.write((char*) &spb,sizeof(spb));

    //存放inode数组
    Inode an_inode;
    an_inode.inode_id=-1;//一开始初始化为-1 说明此块没有被使用
    fs.seekp(2*sizeof(Block),ios_base::beg);
    int inode_number=(data_start-2)*block_size/sizeof(Inode);
    for(int i=0;i<inode_number;i++)
    {
        fs.write((char*)&an_inode,sizeof(an_inode));
    }

    //前48个block用于mbr,superblock和inode数组
    //inodes从第三个block开始存放,45个block可以存放45*512/sizeof(inode)

    fs.seekp(data_start*sizeof(Block),ios_base::beg);

    streampos start,end;//数据的起始位置和结束位置
    start=fs.tellp();
    fs.seekp(0,ios_base::end);
    end=fs.tellp();
    Lnode a_lnode;

    fs.seekp(data_start*sizeof(Block),ios_base::beg);

    int lastSub1=(((end-start)/sizeof(Block))/100 );

    for(int i=1;i<=lastSub1;i++){
        if(i==lastSub1){
            a_lnode.free=99;
            a_lnode.blocks[0]=0;
            for(int j=0;j<stack_size;j++){
                a_lnode.blocks[j]=data_start+i*100+j;
            }
        }else{
            a_lnode.free=100;
            for(int j=0;j<stack_size;j++){
                a_lnode.blocks[j]=data_start+i*100+j;
            }
            fs.write((char*)&a_lnode,sizeof(a_lnode));
            fs.seekp((48+100*i)*512,ios_base::beg);
        }
    }
    closedisk();
    //创建必要的文件
    init_dir();
}

//返回一个初始化的超级块
Superblock init_superBolck(){
    Superblock spb;
    spb.magicnumber=maggci_number;
    strcpy(spb.fsname,"easyfs");
    spb.inode_number=(data_start-2)*block_size/sizeof(Inode);
    spb.root_inode=0;//根文件夹的inode的下标
    spb.inode_usered=1;//已经使用了的inode的数量,根节点已经使用了一个
    spb.blocks.free=stack_size;
    // spb.blocks.next_adress=stack_size+data_start;

    for(int i=0;i<spb.blocks.free;i++){
        spb.blocks.blocks[i]=data_start+i;
    }
    return spb;
}

//判断一个磁盘上是否已有一个文件系统
bool gooddisk(){
    //通过超级块上的标记判断是否有一个文件系统
    //超级块是第1块,第0块为导引块,MBR保留
    Superblock spb=getSuperBlock();
    bool result;
    if(spb.magicnumber==maggci_number){
        result=true;
    }else{
        result=false;
    }
    return result;
}

#endif
