#ifndef STRUCT_CPP_
#define STRUCT_CPP_
#include <time.h>

// link node
//成组链接法的链接节点,记录free数目,下一个组的起始位置,free block的下标
struct Lnode {
  int free;        //空闲块的数量
  int next_adress; //下一个组的开头的块号
  int blocks[100];
};

struct Superblock {
  char fsname[16];
  int inode_number; //一共有多少inode
  int inode_usered; //已经使用了的inode
  int magicnumber;  //一个数字
  int root_inode;
  Lnode blocks; //存放当前可用的
};

struct Block {
  char content[512];
};

//目录让一个块可以放下整数个文件项
struct Directory {
  char name[14];      //文件名
  short int inode_id; // inode号
};

// Inode的结构
struct Inode {
  char filename[14];    //文件名
  int inode_id;         // i节点号
  int user_id;          //用户组ID
  int group_id;         //所属组ID
  char permissions[10]; //权限drwxrwxrwx
  time_t mtime;         //上一次文件内容变动的时间
  int filesize;         //文件大小
  int blocknum;         //文件所使用的磁盘块的实际数目
  int blockaddress
      [6];     //文件数据block的位置,前4个为直接，第5个为一次间接,第6个为2次间接
  short links; //链接数，即有多少文件名指向这个inode
};

struct User {
  char name[16];
  char password[16];
  int user_id;
  int group_id;
};

struct Group {
  char name[16];
  int id;
};

#endif
