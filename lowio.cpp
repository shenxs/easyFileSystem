#ifndef LOWIO_CPP_
#define LOWIO_CPP_
#include "config.h"
#include "struct.cpp"
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
// #include"utile.cpp"
using namespace std;
//底层的磁盘的读写,读取写入spb,inode,添加一个目录项

//将对于磁盘的io放在此处

//===========================函数的申明==========================
void opendisk();
Inode readInode(int index);
User readUser(int pos);
int writeInode(Inode an_inode);
int addInode(Inode inode);
int rmInode(int id);
Superblock getSuperBlock();
int writeSuperBlock(Superblock spb);
int addChild2Dir(Inode parent_node, Directory dir);
int addChild2Dir(Inode parent_node, string childname, int inode_id);
int getaFreeBlockAddress();
void freeBlock(int n);
void writeDir(int pos, Directory dir);
void writeLnode(int pos, Lnode lnode);
int getFileAddress(Inode node, int i);
void reduceFilesize(Inode node, int i);
//===============================================================

//===========================函数实现============================
void opendisk() {
    if (!fs.is_open()) {
        fs.open(diskname.c_str(), ios_base::in | ios_base::out | ios_base::binary);
    } else {
        cout << "磁盘已经打开" << endl;
    }
}
void closedisk() {
    if (fs.is_open()) {
        fs.close();
    } else {
        cout << "请先打开磁盘" << endl;
    }
}

// inode的编号=====>对应的inode
//编号从0开始对应数组的下标,id就是下标
Inode readInode(int index) {
    Inode temp;
    opendisk();
    fs.seekg(2 * sizeof(Block) + index * sizeof(Inode), ios_base::beg);
    fs.read((char *)&temp, sizeof(temp));
    closedisk();
    return temp;
}

//将一个Inode写回磁盘,如果成功则返回0
int writeInode(Inode an_inode) {
    opendisk();
    fs.seekp(2 * sizeof(Block) + an_inode.inode_id * sizeof(an_inode),
            ios_base::beg);
    fs.write((char *)&an_inode, sizeof(an_inode));
    closedisk();
    return 0;
}

//将某个Inode删除,并不删除其文件内容只是将inode的id设置为-1
//顺便spb中已经使用的Inode的大小也减1
int rmInode(int id) {
    Inode temp;
    temp.inode_id = -1;
    opendisk();
    fs.seekp(2 * sizeof(Block) + id * sizeof(Inode), ios_base::beg);
    fs.write((char *)&temp, sizeof(temp));
    closedisk();
    Superblock spb = getSuperBlock();
    spb.inode_usered--;
    writeSuperBlock(spb);
    return 0;
}
Superblock getSuperBlock() {
    Superblock spb;
    opendisk();
    fs.seekg(sizeof(Block), ios_base::beg);
    fs.read((char *)&spb, sizeof(Superblock));
    closedisk();
    return spb;
}

int writeSuperBlock(Superblock spb) {
    opendisk();
    fs.seekp(sizeof(Block), ios_base::beg);
    fs.write((char *)&spb, sizeof(spb));
    closedisk();
    return 0;
}

//传入的inode包括除了id之外的所有信息
int addInode(Inode inode) {
    //向inode数组添加一个新的inode,如果成功则返回新的Inode的id,第一个inode编号为0
    //如果添加失败则返回-1
    // inode的id通过遍历inode节点来判断因为inode可能会被删除,造成间断
    Superblock spb = getSuperBlock();
    int id = -1;
    for (int i = 0; i < spb.inode_number; i++) {
        opendisk();
        Inode temp;
        fs.seekg(2 * sizeof(Block) + i * sizeof(Inode), ios_base::beg);
        fs.read((char *)&temp, sizeof(temp));
        closedisk();
        if (temp.inode_id == -1) {
            id = i;
            break;
        }
    }
    if (id == -1) {
        return -1;
        //说明已经不能添加新的inode了
    }
    inode.inode_id = id;
    spb.inode_usered++;
    writeSuperBlock(spb);
    writeInode(inode);
    return inode.inode_id;
}

int addChild2Dir(Inode parent_node, Directory dir) {
    return addChild2Dir(parent_node, dir.name, dir.inode_id);
}

//将一个新的子文件添加到parent下
//返回新的文件项在磁盘上的位置(第多少个字节)
//返回-1说明添加失败
int addChild2Dir(Inode parent_node, string childname, int inode_id) {
    // showInode(parent_node);
    Directory child;
    child.inode_id = inode_id;
    strcpy(child.name, childname.c_str());
    //找到parent对应文件的末尾
    int address = getFileAddress(parent_node, parent_node.filesize);
    if (address == -1) {
        return -1;
    } else {
        writeDir(address, child);
        parent_node.filesize += sizeof(child);
        writeInode(parent_node);
        return address;
    }
}

//返回一个可用的空闲块,如果没有空闲块则返回-1
int getaFreeBlockAddress() {
    Superblock spb = getSuperBlock();
    if(spb.blocks.free>1&&spb.blocks.free<=100){
        int result=0;
        int point=spb.blocks.free;
        if(point>100)
            cout<<"point大于100";
        point--;
        result=spb.blocks.blocks[point];
        spb.blocks.free=point;
        writeSuperBlock(spb);
        if(result<48||result>2048)
            cout<<"空闲块地址出错"<<endl;
        return result;
    }else if(spb.blocks.free==1){
        //最后一块,存着下一个组的情况
        if(spb.blocks.blocks[0]==0){
            cout<<"磁盘已经用完了"<<endl;
            return -1;
        }else{
            int address=spb.blocks.blocks[0];
            // cout<<"用完了一组"<<endl<<address<<endl;
            Lnode lnode;
            opendisk();
            fs.seekg(address*sizeof(Block),ios_base::beg);
            fs.read((char *)&lnode,sizeof(lnode));
            closedisk();
            if(lnode.free>100){
                cout<<address<<endl;
                cout<<"出现错误,读取盘块栈出错"<<endl;
                return -1;
            }
            spb.blocks=lnode;
            writeSuperBlock(spb);
            return address;
        }
    }else{
        cout<<"获取未使用的盘块时出,现未知错误"<<endl;
        return -1;
    }
}

//释放一个块,将其存放到可用的
void freeBlock(int n) {
    Superblock spb = getSuperBlock();
    Lnode lnode = spb.blocks;
    if (lnode.free < 100) {
        lnode.blocks[lnode.free] = n;
        lnode.free++;
        spb.blocks = lnode;
        writeSuperBlock(spb);
    } else {
        //当前已经有100个可以使用的block了,将其写回
        // spb中可以新建一个block,初始为0
        writeLnode(sizeof(Block) * n, lnode);
        Lnode new_lnode;
        new_lnode.free = 1;
        new_lnode.blocks[0]=n;
        spb.blocks = new_lnode;
        writeSuperBlock(spb);
    }
}

//将某个目录项写到pos所指的地方
void writeDir(int pos, Directory dir) {
    opendisk();
    fs.seekp(pos, ios_base::beg);
    fs.write((char *)&dir, sizeof(dir));
    closedisk();
}

// pos是指User所在的地址
User readUser(int pos) {
    opendisk();
    fs.seekg(pos, ios_base::beg);
    User temp;
    fs.read((char *)&temp, sizeof(temp));
    closedisk();
    return temp;
}

//一个文件夹就像是放着好多目录项的一个数组,此函数将原本不连续的
//文件地址变成简单连续的地址,i即一个文件夹中的第几个文件项,下表从0开始
//如果没有则返回的地址为-1
//返回的是目录项在磁盘上的位置
//链表需要存储前后的地址,又浪费空间,也不能利用间隔
//文件内部是连续的,文件夹删除的时候要将空闲区用最后个文件项填补上
// node 代表一个文件,i是抽象的,将文件看做连续分配后相对起始位置的偏移量
//抽象之后可以连续的使用文件,不用关心底层的存储
int getFileAddress(Inode node, int i) {
    //这是一种函数映射关系,将抽象的地址转换为实际的地址
    //从0个字节开始
    //如果访问到filesize并且是一个块已经结束则分配一个新的块
    if (i > node.filesize || i < 0) {
        return -1;
    } else {
        short block_index, pianyi; //磁盘块号和盘内偏移量
        //直接地址
        pianyi = i % sizeof(Block);
        if (i < 4 * sizeof(Block)) {
            //直接地址
            int index = i / sizeof(Block);
            if (i == node.filesize && pianyi == 0) {
                //遇到末尾
                block_index = node.blockaddress[index];
                if (block_index < data_start || block_index > block_number) {
                    //所获得的地址不是一个有效的地址
                    node.blockaddress[index] = getaFreeBlockAddress();
                    node.blocknum++;
                    writeInode(node);
                    block_index = node.blockaddress[index];
                }
            } else {
                block_index = node.blockaddress[index];
            }
        } else if (i < 4 * sizeof(Block) + addressNumber * sizeof(Block)) {
            //一级间接
            int index = 4;
            if (i == node.filesize && pianyi == 0) {
                //遇到访问文件末尾的情况,且文件正好用完一个块
                int a1 = node.blockaddress[index];
                if (a1 < data_start || a1 > block_number) {
                    //如果blockaddress4中存放的是无效地址
                    short temp = getaFreeBlockAddress();
                    node.blockaddress[4] = temp;
                    temp = getaFreeBlockAddress();
                    opendisk();
                    fs.seekp(node.blockaddress[4] * sizeof(Block), ios_base::beg);
                    fs.write((char *)&temp, sizeof(temp));
                    closedisk();
                    node.blocknum++;
                    writeInode(node);
                    block_index = temp;
                } else {
                    // address[4]中有地址
                    short temp;
                    int p1 = (i / sizeof(Block)) - 4;
                    opendisk();
                    fs.seekg(a1 * sizeof(Block) + p1 * sizeof(short), ios_base::beg);
                    fs.read((char *)&temp, sizeof(temp));
                    closedisk();
                    if (temp < data_start || temp > block_number) {
                        temp = getaFreeBlockAddress();
                        opendisk();
                        fs.seekp(a1 * sizeof(Block) + p1 * sizeof(short), ios_base::beg);
                        fs.write((char *)&temp, sizeof(temp));
                        closedisk();
                        node.blocknum++;
                        writeInode(node);
                    }
                    block_index = temp;
                }
            } else {
                //普通的情况,访问的不是文件末尾或者是文件末尾但不是block末尾
                index = node.blockaddress[index];
                int p1 = (i / sizeof(Block)) - 4;
                opendisk();
                fs.seekg(index * sizeof(Block) + p1 * sizeof(short), ios_base::beg);
                fs.read((char *)&block_index, sizeof(block_index));
                closedisk();
            }
        } else if (i < 4 * sizeof(Block) + addressNumber * sizeof(Block) +
                addressNumber * addressNumber * sizeof(Block)) {
            // cout<<"二级间接"<<endl;
            short index = 5;
            short a1 = node.blockaddress[index];

            if (i == node.filesize && pianyi == 0) {
                // cout<<"需要加入新的块的情况应该有3种"<<endl;
                // if (a1 < data_start || a1 > block_number) {
                   if(i==(4+addressNumber)*sizeof(Block)){
                    // cout<<"从直接索引开始新建"<<endl;
                    short temp = getaFreeBlockAddress();
                    node.blockaddress[5] = temp;
                    temp = getaFreeBlockAddress();
                    opendisk();
                    fs.seekp(node.blockaddress[5] * sizeof(Block), ios_base::beg);
                    fs.write((char *)&temp, sizeof(temp));
                    closedisk();
                    short temp2 = getaFreeBlockAddress();
                    opendisk();
                    fs.seekp(temp * sizeof(Block), ios_base::beg);
                    fs.write((char *)&temp2, sizeof(temp2));
                    closedisk();
                    node.blocknum++;
                    writeInode(node);
                    block_index = temp2;
                } else {
                    //直接索引有效,检查是否需要一级索引
                    // cout<<"先读出一级索引"<<endl;
                    int x = ((i / (sizeof(Block))) - 4 - addressNumber) / addressNumber;
                    int y = ((i / sizeof(Block)) - 4 - addressNumber) % addressNumber;
                    short a2;
                    opendisk();
                    fs.seekg(sizeof(Block) * a1 + x * sizeof(short), ios_base::beg);
                    fs.read((char *)&a2, sizeof(a2));
                    closedisk();

                    // if (a2 < data_start || a2 > block_number) {
                    if(y==0){
                        // cout<<"如果a2无效的话"<<endl;
                        a2 = getaFreeBlockAddress();
                        opendisk();
                        fs.seekp(sizeof(Block) * a1 + x * sizeof(short), ios_base::beg);
                        fs.write((char *)&a2, sizeof(a2));
                        closedisk();
                        short a3 = getaFreeBlockAddress();
                        opendisk();
                        fs.seekp(a2 * sizeof(Block), ios_base::beg);
                        fs.write((char *)&a3, sizeof(a3));
                        closedisk();
                        node.blocknum++;
                        writeInode(node);
                        block_index = a3;
                    } else {
                        // cout<<"a2有效,读出a3"<<endl;
                        int y = ((i / sizeof(Block)) - 4 - addressNumber) % addressNumber;
                        short a3;
                        opendisk();
                        fs.seekg(a2 * sizeof(Block) + y * sizeof(short), ios_base::beg);
                        fs.read((char *)&a3, sizeof(a3));
                        closedisk();
                        // if (a3 < data_start || a3 > block_number) {
                        if(pianyi==0){
                            // cout<<"a3无效,需要新建一个块"<<endl;
                            a3 = getaFreeBlockAddress();
                            opendisk();
                            fs.seekp(a2 * sizeof(Block) + y * sizeof(short), ios_base::beg);
                            fs.write((char *)&a3, sizeof(a3));
                            closedisk();
                            node.blocknum++;
                            writeInode(node);
                            block_index = a3;
                        } else {
                            block_index = a3;
                        }
                        // cout<<"a2有效"<<endl;
                    }
                }
            } else {
                // cout<<"else"<<endl;
                int x = ((i / (sizeof(Block))) - 4 - addressNumber) / addressNumber;
                int y = ((i / sizeof(Block)) - 4 - addressNumber) % addressNumber;
                block_index=0;
                opendisk();
                fs.seekg(a1* sizeof(Block) + x * sizeof(short), ios_base::beg);
                fs.read((char *)&index, sizeof(index));
                fs.seekg(index * sizeof(Block) + y * sizeof(short), ios_base::beg);
                fs.read((char *)&block_index, sizeof(block_index));
                closedisk();
            }
            } else {
                cout << "超出文件系统所允许的最大的文件大小" << endl;
            }
            int result = block_index * sizeof(Block) + pianyi;
            return result;
        }
    }

    //将链接节点写回
    void writeLnode(int pos, Lnode lnode) {
        opendisk();
        fs.seekp(pos, ios_base::beg);
        fs.write((char *)&lnode, sizeof(lnode));
        closedisk();
    }

    //将文件的大小减小一个字节
    //需要将磁盘上的内容删除
    void reduceFilesizeBy1byte(Inode node) {
        if (node.filesize == 0){
            //size==0代表删除空文件
            // cout<<"删除空文件"<<endl;
            freeBlock(node.blockaddress[0]);
            return;
        }
        else {
            int pianyi = node.filesize % sizeof(Block);
            if (pianyi != 1) {
                //删除字节后并不需要修改索引
                int address = getFileAddress(node, node.filesize - 1);
                opendisk();
                fs.seekp(address, ios_base::beg);
                char c = 0;
                fs.write((char *)&c, sizeof(c));
                closedisk();
                node.filesize--;
                writeInode(node);
                return;
            } else {
                //释放节点
                //释放索引节
                //修改node的blocknumber
                if (node.filesize < 4 * sizeof(Block)) {
                    int tofree = node.filesize / sizeof(Block);
                    int a1 = node.blockaddress[tofree];
                    opendisk();
                    fs.seekg(a1 * sizeof(Block), ios_base::beg);
                    char c = 0;
                    fs.write((char *)&c, sizeof(c));
                    closedisk();
                    freeBlock(node.blockaddress[tofree]);
                    node.blocknum--;
                    node.blockaddress[tofree] = 0;
                    node.filesize--;
                    writeInode(node);
                    return;
                } else if (node.filesize <
                        4 * sizeof(Block) + addressNumber * sizeof(Block)) {
                    //一级索引
                    int x = (node.filesize / sizeof(Block)) - 4;
                    if (x == 0) {
                        //需要删除一级索引块
                        int a = node.blockaddress[4];
                        opendisk();
                        fs.seekp(a * sizeof(Block), ios_base::beg);
                        short a1;
                        fs.read((char *)&a1, sizeof(a1));
                        fs.seekp(a1 * sizeof(Block), ios_base::beg);
                        char c = 0;
                        fs.write((char *)&c, sizeof(c));
                        short d = 0;
                        fs.seekp(a * sizeof(Block), ios_base::beg);
                        fs.write((char *)&d, sizeof(d));
                        closedisk();
                        freeBlock(a1);
                        freeBlock(a);
                        node.blocknum--;
                        node.blockaddress[4] = 0;
                        node.filesize--;
                        writeInode(node);
                    } else {
                        //只是将索引块中的一项地址修改
                        int a = node.blockaddress[4];
                        opendisk();
                        fs.seekg(a * sizeof(Block) + x * sizeof(short), ios_base::beg);
                        short a1;
                        fs.read((char *)&a1, sizeof(a1));
                        short temp = 0;
                        fs.seekp(-(sizeof(short)), ios_base::cur);
                        fs.write((char *)&temp, sizeof(temp));
                        fs.seekp(a1 * sizeof(Block), ios_base::beg);
                        char c = 0;
                        fs.write((char *)&c, sizeof(c));
                        closedisk();
                        freeBlock(a1);
                        node.filesize--;
                        node.blocknum--;
                        writeInode(node);
                    }
                } else if (node.filesize <
                        4 * sizeof(Block) + addressNumber * sizeof(Block) +
                        addressNumber * addressNumber * sizeof(Block)) {
                    //二级索引
                    int x = ((node.filesize / sizeof(Block)) - 4 - addressNumber) / addressNumber;
                    int y = ((node.filesize / sizeof(Block)) - 4 - addressNumber) % addressNumber;
                    short a1 = node.blockaddress[5];
                    short a2, a3;
                    opendisk();
                    fs.seekg(sizeof(Block) * a1 + x * sizeof(short), ios_base::beg);
                    fs.read((char *)&a2, sizeof(a2));
                    fs.seekg(sizeof(Block) * a2 + y * sizeof(short), ios_base::beg);
                    fs.read((char *)&a3, sizeof(a3));
                    char c = 0;
                    fs.seekp(a3 * sizeof(Block), ios_base::beg);
                    fs.write((char *)&c, sizeof(c));
                    closedisk();

                    // cout<<"\t\ta3="<<a3<<endl;
                    freeBlock(a3);

                    short temp = 0;
                    //判断是否删除二级索引
                    if (y == 0) {
                        opendisk();
                        fs.seekp(a2 * sizeof(Block), ios_base::beg);
                        fs.write((char *)&temp, sizeof(temp));
                        closedisk();
                        freeBlock(a2);
                        // cout<<"\ta2="<<a2<<endl;
                    }



                    //是否删除一级索引
                    if (x == 0 && y==0) {
                        opendisk();
                        fs.seekp(a1 * sizeof(Block), ios_base::beg);
                        fs.write((char *)&temp, sizeof(temp));
                        closedisk();
                        freeBlock(a1);
                        // cout<<"a1="<<a1<<endl;
                        node.blockaddress[5] = 0;
                    }

                    node.filesize--;
                    node.blocknum--;
                    writeInode(node);
                }
            }
        }
    }

    //减小一个文件的大小,i代表从后往前数删除的字节数;
    void reduceFilesize(Inode node, int i) {
        //如果没有到一个块的边界那么只是将filesize减去相应的
        //如果到达边界那么还需要考虑归还一个块,并且将索引设置正确,
        //如果正好索引块也不再使用那么将索引块也需要归还
        if (node.filesize < i) {
            cout << "超出文件大小" << endl;
            return;
        } else {
            for (int j = 0; j < i; j++) {
                node = readInode(node.inode_id);
                reduceFilesizeBy1byte(node);
            }
        }
    }

#endif
