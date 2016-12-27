#ifndef UTILE_CPP_
#define UTILE_CPP_

#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<string.h>
#include"struct.cpp"
#include"config.h"
#include"string_func.cpp"
#include"lowio.cpp"
#include"help.cpp"
using namespace std;

//================函数申明=======================================
int mkdir(string path,string permission,int userid ,int groupid);
Inode getInode(string path);
Inode getInode(Inode parent,string path);
int getFileAddress(Inode node,int i);
void init_dir();//创建初始文件夹
void init_fs();
int touch(string path,string permission,int userid,int groupid);
int saveTopasswd(User user);
int leagleUser(User user);
//dir是一个文件夹,需要其他的参数写在参数里面
typedef int (*dirFun)(Directory dir,vector<string> args);//处理文件夹的函数
//================函数实现=======================================
//返回一个初始化的超级块
Superblock init_superBolck(){
    Superblock spb;
    spb.magicnumber=maggci_number;
    strcpy(spb.fsname,"easyfs");
    spb.inode_number=(data_start-2)*block_size/sizeof(Inode);
    spb.root_inode=0;//根文件夹的inode的下标
    spb.inode_usered=1;//已经使用了的inode的数量,根节点已经使用了一个
    spb.blocks.free=stack_size;
    spb.blocks.next_adress=stack_size+data_start;

    for(int i=0;i<spb.blocks.free;i++){
        spb.blocks.blocks[i]=data_start+i;
    }
    return spb;
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
    fs.seekp(2*sizeof(Block),ios_base::beg);

    int inode_number=(data_start-2)*block_size/sizeof(Inode);

    for(int i=0;i<inode_number;i++)
    {
        an_inode.inode_id=i;
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
    int i;
    for(i=0;i<((end-start)/block_size/stack_size);i++){
        a_lnode.free=100;
        a_lnode.next_adress=data_start+(i+1)*100;
        for(int j=0;j<stack_size;j++){
            a_lnode.blocks[j]=data_start+i*100+j;
        }
        fs.write((char*)&a_lnode,sizeof(a_lnode));
        fs.seekp((48+100*(i+1))*512,ios_base::beg);
    }
    //除了最后一组,其他的都比较相似
    streampos near_end=fs.tellp();
    if(near_end==end){
        //说明已经结束,需要将最后一组的第一个块中的next修改为0
        fs.seekp(-(stack_size*block_size),ios_base::cur);
        a_lnode.next_adress=0;
        fs.write((char*)&a_lnode,sizeof(a_lnode));
    }else{
        //说明最后还有一些不到一百个的块
        a_lnode.free=(end-near_end)/sizeof(Block);
        a_lnode.next_adress=0;
        for(int j=0;j<a_lnode.free;j++){
            a_lnode.blocks[j]=data_start+i*100+j;
        }
    }
    closedisk();
    //创建必要的文件
    init_dir();
}

//判断一个磁盘上是否已有一个文件系统
bool gooddisk(){
    //通过超级块上的标记判断是否有一个文件系统
    //超级块是第1块,第0块为导引块,MBR保留
    // fs.seekg(sizeof(Block),ios_base::beg);//指向第1块
    // Superblock *spb=new Superblock;
    // fs.read((char *)spb,sizeof(Superblock));
    // fs.close();
    Superblock spb=getSuperBlock();
    bool result;
    if(spb.magicnumber==maggci_number){
        result=true;
    }else{
        result=false;
    }
    return result;
}

//返回一个可用的空闲块,如果没有空闲块则返回0
void init_dir(){
    //创建根文件夹,/root,/home,/etc/passwd

    //根目录的inode,只有根目录特殊需要手动创建
    Inode an_inode;
    strcpy(an_inode.filename,"/");
    an_inode.inode_id=0;
    an_inode.user_id=0;//root用户id为0;
    an_inode.group_id=0;//
    strcpy(an_inode.permissions,"drwxr-xr-x");
    an_inode.mtime=time(0);
    an_inode.filesize=2*sizeof(Directory);//初始只有. 和..
    an_inode.blocknum=1;
    an_inode.blockaddress[0]=getaFreeBlockAddress();
    an_inode.links=1;

    Directory d1,d2;
    d1.inode_id=d2.inode_id=0;//根目录的上级和当前都是自身
    strcpy(d1.name,".");
    strcpy(d2.name,"..");

    writeInode(an_inode);

    writeDir(an_inode.blockaddress[0]*sizeof(Block),d1);
    writeDir(an_inode.blockaddress[0]*sizeof(Block)+sizeof(d1),d2);


    //前缀,目录名,permission,userid ,groupid;
    mkdir("/root","drwxr-xr-w",0,0);
    mkdir("/home","drwxr-xr-w",0,0);
    mkdir("/etc","drwxr-xr-w",0,0);

    mkdir("/home/richard","drwxr-xr-x",1,1);
    mkdir("/home/richard/playground","drwxr-xr-x",1,1);
    // //创建passwd文件保存用户的账号信息

    touch("/etc/passwd","frwx------",0,0);//只有root管理员用户组才可以查看,不保存字符串,直接用二进制保存用户信息,方便使用

    User root;
    strcpy(root.name,"root");
    strcpy(root.password,"12345");
    root.user_id=0;
    root.group_id=0;
    User richard;
    strcpy(richard.name,"richard");
    strcpy(richard.password,"12345");
    richard.user_id=1;
    richard.group_id=0;
    saveTopasswd(root);//作为一个特殊的函数
    saveTopasswd(richard);
}

//新建一个文件夹
int mkdir(string path,string permission,int userid ,int groupid)
{
    Inode parent_node,new_node;

    //1得到上一级目录的inode节点
    string parent_path=get_parentPath(path);
    parent_node=getInode(readInode(0),parent_path.substr(1,parent_path.length()));
    // cout<<parent_node.permissions<<endl;
    //2新添加一个inode的节点

    strcpy(new_node.filename,getChildName(path).c_str());
    // cout<<getChildName(path)<<endl;
    new_node.blockaddress[0]=getaFreeBlockAddress();
    strcpy(new_node.permissions,permission.c_str());
    new_node.user_id=userid;
    new_node.group_id=groupid;
    new_node.filesize=2*sizeof(Directory);
    new_node.mtime=time(0);
    new_node.blocknum=1;
    new_node.links=1;
    int new_node_id=addInode(new_node);

    //3在parent_node文件内容中新添加一个目录项
    addChild2Dir(parent_node,getChildName(path),new_node_id);

    //4将初始的文件内容写入inode对应的Block
    Directory dir1,dir2;
    strcpy(dir1.name,".");
    strcpy(dir2.name,"..");
    dir1.inode_id=new_node_id;
    dir2.inode_id=parent_node.inode_id;

    //写入初始文件夹
    writeDir(new_node.blockaddress[0]*sizeof(Block),dir1);
    writeDir(new_node.blockaddress[0]*sizeof(Block)+sizeof(dir1),dir2);

    return 0;
}
//inode代表一个目录的inode,从中找到相应的文件(目录或者文件)所对应的inode的id
//如果没有找到则返回-1
int getInodeidFromDir(Inode inode,string filename){
    unsigned int dirs=inode.filesize/sizeof(Directory);//最多有多少的目录项
    for(unsigned int i=0;i<dirs;i++){
        int address=getFileAddress(inode,i*sizeof(Directory));
        opendisk();
        fs.seekg(address,ios_base::beg);
        Directory temp;
        fs.read((char*)&temp,sizeof(temp));
        closedisk();
        string tempname=temp.name;
        if(filename==tempname)
            return temp.inode_id;
    }
    return -1;
}

//递归式的有路径找到对应的inode

// Inode[0]   etc/home/playground
Inode getInode(Inode parent,string path){
    // cout<<path<<endl;
    if(path==""){
        //已近找完了
        return parent;
    }else if(path.find_first_of("/")==string::npos){
        //最后一项了
        int nodeId=getInodeidFromDir(parent,path);
        if(nodeId==-1){
            return parent;
        }
        Inode node=readInode(nodeId);
        return node;
    }else{
        string childname=path.substr(0,path.find_first_of("/"));
        int nodeId=getInodeidFromDir(parent,childname);
        if(nodeId==-1){
            return parent;
        }
        parent=readInode(nodeId);
        string new_path=path.substr(path.find_first_of("/")+1,path.length());
        return getInode(parent,new_path);
    }
}

//路径对应inode的节点,绝对路径
Inode getInode(string path){
    return getInode(readInode(0),path.substr(1,path.length()));
}

//创建一个路径为path的文件,文件内容为空
//但是也分配一个块
int touch(string path,string permission,int userid,int groupid){
    string parent_path=get_parentPath(path);
    string childname=getChildName(path);
    Inode parent_node,child_node;
    parent_node=getInode(parent_path);

    strcpy(child_node.filename,childname.c_str());
    child_node.blocknum=1;
    child_node.filesize=0;
    child_node.mtime=time(0);
    strcpy(child_node.permissions,permission.c_str());
    child_node.blockaddress[0]=getaFreeBlockAddress();
    // cout<<child_node.blockaddress[0]<<endl;
    child_node.links=1;
    child_node.user_id=userid;
    child_node.group_id=groupid;

    int child_node_id=addInode(child_node);
    Directory file;
    strcpy(file.name,childname.c_str());
    file.inode_id=child_node_id;
    addChild2Dir(parent_node,childname,child_node_id);

    return child_node_id;
}

int saveTopasswd(User user){
    string path="/etc/passwd";
    Inode node=getInode(path);
    if(node.filesize==0){
        int pos=node.blockaddress[0];
        opendisk();
        fs.seekp(pos*sizeof(Block),ios_base::beg);
        fs.write((char *)&user,sizeof(user));
        fs.close();
        node.filesize=node.filesize+sizeof(User);
        writeInode(node);
        return 0;
    }else{//遍历,如果已有则更新,没有则在末尾添加
        unsigned int i=0;
        int pos=node.blockaddress[0];
        opendisk();
        fs.seekp(pos*sizeof(Block),ios_base::beg);
        for(;i<node.filesize/sizeof(User);i++){
            User temp;
            fs.read((char*)&temp,sizeof(temp));
            if(temp.name==user.name){
                fs.seekp(-sizeof(User),ios_base::cur);
                fs.write((char *)&user,sizeof(user));
                break;
            }
        }
        if(i==node.filesize/sizeof(User)){
            fs.write((char *)&user,sizeof(User));
            node.filesize+=sizeof(User);
        }
        closedisk();
        writeInode(node);
        return i;
    }
}

//如果用户合法则返回其在硬盘上的位置
//不合法则返回0
int leagleUser(User user){
    Inode node=getInode("/etc/passwd");
    opendisk();
    fs.seekg(node.blockaddress[0]*sizeof(Block),ios_base::beg);
    int userNumber=node.filesize/sizeof(User);
    // cout<<userNumber<<endl;
    for(int i=0;i<userNumber;i++){
        User rightUser;
        fs.read((char *)&rightUser,sizeof(rightUser));
        // cout<<rightUser.name<<" "<<rightUser.password<<endl;
        string name=rightUser.name;
        string password=rightUser.password;
        if(user.name==name && user.password==password){
            int pos=(int)fs.tellg()-sizeof(User);
            closedisk();
            return pos;
        }
    }
    closedisk();
    return 0;
}

//一个文件夹就像是放着好多目录项的一个数组,此函数将原本不连续的
//文件地址变成简单连续的地址,i即一个文件夹中的第几个文件项,下表从0开始
//如果没有则返回的地址为-1
//返回的是目录项在磁盘上的位置
//链表需要存储前后的地址,又浪费空间,也不能利用间隔
//文件内部是连续的,文件夹删除的时候要将空闲区用最后个文件项填补上
//node 代表一个文件,i是抽象的,将文件看做连续分配后相对起始位置的偏移量
//抽象之后可以连续的使用文件,不用关心底层的存储
int getFileAddress(Inode node ,int i){
    //这是一种函数映射关系,将抽象的地址转换为实际的地址
    //从0个字节开始
    if(i>=node.filesize||i<0)
        return -1;
    else{
        short block_index,pianyi;//磁盘块号和盘内偏移量
        //直接地址
        pianyi=i%sizeof(Block);
        if(i<4*sizeof(Block)){
            int index=i/sizeof(Block);
            block_index=node.blockaddress[index];
        }else if(i<4*sizeof(Block)+addressNumber*sizeof(Block)){
            //一级间接
            int index=4;
            index=node.blockaddress[index];
            int p1=(i/sizeof(Block))-4;
            opendisk();
            fs.seekg(index*sizeof(Block)+p1*sizeof(short),ios_base::beg);
            fs.read((char *)&block_index,sizeof(block_index));
            closedisk();
        }else if(i<4*sizeof(Block)+
                addressNumber*sizeof(Block)
                +addressNumber*addressNumber*sizeof(Block)){
            //二级间接
            short index=5;
            index=node.blockaddress[index];
            int x=((i/(sizeof(Block)))-4-addressNumber)/addressNumber;
            int y=((i/sizeof(Block))-4-addressNumber)%addressNumber;
            opendisk();
            fs.seekg(index*sizeof(Block)+x*sizeof(short),ios_base::beg);
            fs.read((char*)&index,sizeof(index));
            fs.seekg(index*sizeof(Block)+y*sizeof(short),ios_base::beg);
            fs.read((char *)sizeof(block_index),sizeof(block_index));
            closedisk();

        }
        int result=block_index*sizeof(Block)+pianyi;
        return result;
    }
}

//根据文件夹输出一定的内容
int showDir(Directory dir,vector<string> args){
    Inode node=readInode(dir.inode_id);
    //如果拥有权限则输出
    int user_id=std::stoi(args[0]);
    int group_id=std::stoi(args[1]);
    if(node.user_id==user_id){
        if(node.permissions[1]=='r'){
            cout<<dir.name;
        }

    }else if(node.group_id==group_id){
        if(node.permissions[4]=='r'){
            cout<<dir.name;
        }
    }else{
        if(node.permissions[7]=='r'){
            cout<<dir.name;
        }
    }
    return 0;
}

void traverse_ls(Inode node,dirFun func ,User user){
    //遍历一个文件夹下的内容,对其每一个文件项应用dirFun
    //需要考虑一级,二级索引

    //是否是一个文件夹?
    if(node.permissions[0]!='d'){
        cout<<"这不是一个文件夹"<<endl;
    }else{
        int dir_number=node.filesize/sizeof(Directory);
        for(int i=0;i<dir_number;i++){
            int address=getFileAddress(node,i*sizeof(Directory));
            opendisk();
            fs.seekg(address,ios_base::beg);
            Directory temp;
            fs.read((char*)&temp,sizeof(temp));
            closedisk();
            vector<string> args;
            args.push_back(std::to_string(user.user_id));
            args.push_back(std::to_string(user.group_id));
            //此处应该有文件夹的输出
            func(temp,args);
            cout<<'\t';
        }
        cout<<endl;

    }
}

#endif
