#ifndef LOWIO_CPP_
#define LOWIO_CPP_
#include<iostream>
#include<fstream>
#include<string>
#include<string.h>
#include"struct.cpp"
#include"config.h"
// #include"utile.cpp"
using namespace std;
//底层的磁盘的读写,读取写入spb,inode,添加一个目录项

//将对于磁盘的io放在此处



//===========================函数的申明==========================
void opendisk();
Inode readInode(int index);
User  readUser(int pos);
int writeInode(Inode an_inode);
int addInode(Inode inode);
Superblock getSuperBlock();
int writeSuperBlock(Superblock spb);
int addChild2Dir(Inode parent_node,string childname,int inode_id);
int getaFreeBlockAddress();
void writeDir(int pos,Directory dir);
//===============================================================



//===========================函数实现============================
void opendisk(){
    if(!fs.is_open()){
    fs.open(diskname.c_str(),ios_base::in|ios_base::out|ios_base::binary);
    }else{
        cout<<"磁盘已经打开"<<endl;
    }
}
void closedisk()
{
    if(fs.is_open())
    {
        fs.close();
    }else{
        cout<<"请先打开磁盘"<<endl;
    }
}


//inode的编号=====>对应的inode
//编号从0开始对应数组的下标,id就是下标
Inode readInode(int index){
    Inode temp;
    opendisk();
    fs.seekg(2*sizeof(Block)+index*sizeof(Inode),ios_base::beg);
    fs.read((char *)&temp,sizeof(temp));
    closedisk();
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
    opendisk();
    fs.seekg(sizeof(Block),ios_base::beg);
    fs.read((char *)&spb,sizeof(Superblock));
    closedisk();
    return spb;
}

int writeSuperBlock(Superblock spb){
    opendisk();
    fs.seekp(sizeof(Block),ios_base::beg);
    fs.write((char*)&spb,sizeof(spb));
    closedisk();
    return 0;
}

//传入的inode包括除了id之外的所有信息
int addInode(Inode inode){
    //向inode数组添加一个新的inode,如果成功则返回新的Inode的id,第一个inode编号为0
    //如果添加失败则返回-1
    //inode的id通过遍历inode节点来判断因为inode可能会被删除,造成间断
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
//返回新的文件项在磁盘上的位置(第多少个字节)
//返回-1说明添加失败
int addChild2Dir(Inode parent_node,string childname,int inode_id){
    // showInode(parent_node);
    Directory child;
    child.inode_id=inode_id;
    strcpy(child.name,childname.c_str());
    //找到parent对应文件的末尾
    short int block_index=0;//文件末尾所在的块号
    int pianyi=0;//文件末尾对应的块号的偏移量
    if(parent_node.blocknum<=3){
        //直接地址,如果已经满了,下一块还是直接地址
        //如果当前块放不下一个目录项了
        pianyi=parent_node.filesize%sizeof(Block);
        if(pianyi==0){
            //重新申请一块空闲的地址
            int new_block_address=getaFreeBlockAddress();
            parent_node.blockaddress[parent_node.blocknum]=new_block_address;
            parent_node.blocknum++;
            block_index=new_block_address;
            pianyi=0;
        }else{
            //直接使用现有的地址
            block_index=parent_node.blockaddress[parent_node.blocknum-1];
            pianyi=parent_node.filesize%sizeof(Block);
        }
    }else if(parent_node.blocknum==4){
        block_index=parent_node.blocknum-1;
        pianyi=parent_node.filesize%sizeof(Block);
        if((sizeof(Block)-pianyi)<sizeof(Directory)or pianyi==0){
            //重新申请一块空闲的地址
            short int new_address=getaFreeBlockAddress();
            parent_node.blocknum++;
            parent_node.blockaddress[4]=new_address;
            short int store_address=getaFreeBlockAddress();

            fs.open(diskname.c_str(),ios_base::in|ios_base::out|ios_base::binary);
            fs.seekp(new_address,ios_base::beg);
            fs.write((char*)&store_address,sizeof(store_address));
            block_index=store_address;
            pianyi=0;
        }else{
            //直接使用现有的地址
            block_index=parent_node.blockaddress[parent_node.blocknum-1];
            pianyi=parent_node.filesize%sizeof(Block);
        }
    }else if(parent_node.blocknum>=5&&parent_node.blocknum<(4+addressNumber)){
        //一级间接地址
        if(parent_node.filesize%sizeof(Block)==0){
            //已经满了,需要一个新的块
            short int store_address=getaFreeBlockAddress();
            parent_node.blocknum++;
            fs.open(diskname.c_str(),ios_base::in|ios_base::out|ios_base::binary);
            fs.seekp(parent_node.blockaddress[4]+sizeof(short int)*(parent_node.blocknum-4),ios_base::beg);
            fs.write((char *)&store_address,sizeof(store_address));
            block_index=store_address;
            pianyi=0;
        }else{
            fs.open(diskname.c_str(),ios_base::in|ios_base::out|ios_base::binary);
            fs.seekg(parent_node.blockaddress[4]*sizeof(Block)+(parent_node.blocknum-5)*sizeof(short int),ios_base::beg);
            fs.read((char*)&block_index,sizeof(block_index));
            pianyi=parent_node.filesize%sizeof(Block);
        }


    }else if(parent_node.blocknum==(4+(sizeof(Block)/sizeof(short int)))){//第一间接地址即将用完的情况
        if(parent_node.filesize%(sizeof(Block))==0){
            //已经用完一级间接地址,启用第二间接地址

            short int first_address=getaFreeBlockAddress();
            parent_node.blockaddress[5]=first_address;
            fs.open(diskname.c_str(),ios_base::in|ios_base::out|ios_base::binary);
            fs.seekp(first_address*sizeof(Block),ios_base::beg);
            short int second_address=getaFreeBlockAddress();
            fs.write((char *)&second_address,sizeof(second_address));
            short int third_address=getaFreeBlockAddress();
            fs.seekp(second_address*sizeof(Block),ios_base::beg);
            fs.write((char *)&third_address,sizeof(third_address));

            block_index=third_address;
            pianyi=0;
        }else{//是一级间接地址的最后一块了
            fs.open(diskname.c_str(),ios_base::in|ios_base::out|ios_base::binary);
            fs.seekg(parent_node.blockaddress[4]*sizeof(Block)+(parent_node.blocknum-5)*sizeof(short int),ios_base::beg);
            fs.read((char*)&block_index,sizeof(block_index));
            pianyi=parent_node.filesize%sizeof(Block);
        }
    }else if(parent_node.blocknum>(4+addressNumber)
            and parent_node.blocknum<(4+addressNumber+addressNumber*addressNumber)){//已经二级索引了
        if(parent_node.filesize%sizeof(Block)==0){//如果是二级索引,需要一个新的block
            fs.open(diskname.c_str(),ios_base::in|ios_base::out|ios_base::binary);
            short int first_address=parent_node.blockaddress[5];

            int p1=(parent_node.blocknum-4-addressNumber)/ addressNumber;
            int p2=(parent_node.blocknum-4-addressNumber)% addressNumber;
            if(p2==0)//需要一个新的二级索引
            {
                short int new_first=getaFreeBlockAddress();
                fs.seekp(first_address*sizeof(Block)+p1*sizeof(short int),ios_base::beg);
                fs.write((char *)&new_first,sizeof(new_first));
                fs.seekp(new_first*sizeof(Block),ios_base::beg);
                short int real_address=getaFreeBlockAddress();
                fs.write((char*)&real_address,sizeof(real_address));
                block_index=real_address;
                pianyi=0;
                parent_node.blocknum++;
            }else{
                //在对应的二级索引里面添加一个新的项目就可以了
                short int real_address=getaFreeBlockAddress();
                fs.seekg(first_address*sizeof(Block)+(p1-1)*sizeof(short int),ios_base::beg);
                short second_address;
                fs.read((char*)&second_address,sizeof(second_address));
                fs.seekp(second_address*sizeof(Block)+p2,ios_base::beg);
                fs.write((char *)&real_address,sizeof(real_address));

                block_index=real_address;
                pianyi=0;
            }

        }else{
            int addressNumber=(sizeof(Block))/sizeof(short int);//一个块可以存放多少的地址项
            fs.open(diskname.c_str(),ios_base::in|ios_base::out|ios_base::binary);

            short int first_address=parent_node.blockaddress[5];
            int p1=(parent_node.blocknum-4-addressNumber)/ addressNumber;
            fs.seekg(first_address*sizeof(Block)+p1*sizeof(short int),ios_base::beg);
            short int second_address;
            fs.read((char* )&second_address,sizeof(second_address));

            int p2=(parent_node.blocknum-4-addressNumber)% addressNumber;
            fs.seekg(second_address*sizeof(Block)+p2*sizeof(short int),ios_base::beg);
            short int real_address;
            fs.read((char*)&real_address,sizeof(real_address));

            block_index=real_address;
            pianyi=parent_node.filesize%sizeof(Block);
        }

    }else{
        cout<<"未知错误"<<endl;
        return -1;
    }
    fs.close();
    parent_node.filesize+=sizeof(child);
    writeInode(parent_node);
    writeDir(block_index*sizeof(Block)+pianyi,child);
    return block_index*sizeof(Block)+pianyi;
}

//返回一个可用的空闲块,如果没有空闲块则返回-1
int getaFreeBlockAddress(){
    Superblock spb=getSuperBlock();
    if(spb.blocks.free>1)
    {
        spb.blocks.free--;
        writeSuperBlock(spb);
        return spb.blocks.blocks[spb.blocks.free];
    }else if(spb.blocks.free==1){
        if(spb.blocks.next_adress==0){//已经是最后一组了
            cout<<"磁盘耗尽,没有空余块了"<<endl;
            return -1;
        }else{
            //当前的组中所有的块都已经使用了
            //将现在的lNode写回磁盘
            Lnode a_lnode;
            a_lnode.free=spb.blocks.free;
            a_lnode.next_adress=spb.blocks.next_adress;
            for(int i=0;i<100;i++){
                a_lnode.blocks[i]=spb.blocks.blocks[i];
            }
            opendisk();
            fs.seekp(sizeof(Block)*(spb.blocks.next_adress-100),ios_base::beg);
            fs.write((char *)&a_lnode,sizeof(a_lnode));
            //将下一组的自由块的Lnode装入spb
            fs.seekp(sizeof(Block)*spb.blocks.next_adress,ios_base::beg);
            fs.read((char *) &a_lnode,sizeof(a_lnode));

            closedisk();
            spb.blocks.free=a_lnode.free;
            spb.blocks.next_adress=a_lnode.next_adress;
            for(int i=0;i<100;i++){
                spb.blocks.blocks[i]=a_lnode.blocks[i];
            }
            writeSuperBlock(spb);
            int freeblock=getaFreeBlockAddress();
            //将spb写回
            return freeblock;
        }
    }else{
        cout<<"出现未知错误"<<endl;
        return -1;
    }
}

//将某个目录项写到pos所指的地方
void writeDir(int pos, Directory dir){
    opendisk();
    fs.seekp(pos,ios_base::beg);
    fs.write((char *)&dir,sizeof(dir));
    closedisk();
}

//pos是指User所在的地址
User readUser(int pos){
    opendisk();
    fs.seekg(pos,ios_base::beg);
    User temp;
    fs.read((char *)&temp,sizeof(temp));
    closedisk();
    return temp;
}
#endif
