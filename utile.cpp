#ifndef UTILE_CPP_
#define UTILE_CPP_
//辅助函数,一些常用的工具类函数

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
void rmChildFromDir(Inode node,string file);
Inode getInode(string path);
Inode getInode(Inode parent,string path);
string getPath(Inode node);
void init_dir();//创建初始文件夹
void write2File(Inode node ,string);
int touch(string path,string permission,int userid,int groupid);
int saveTopasswd(User user);
int saveToGroup(Group group);
int getgroupid(string grpname);
int getUserId(string usrname);
int leagleUser(User user);
bool leaglePath(Inode node ,string path);
bool canI(Inode node,User user,int action);//用于权限的鉴定
//dir是一个文件夹,需要其他的参数写在参数里面
typedef int (*dirFun)(Directory dir,vector<string> args);//处理文件夹的函数
//================函数实现=======================================

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

    mkdir("/home/richard","drwxr-xr-x",1,0);
    mkdir("/home/richard/playground","drwxr-xr-x",1,0);
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

    //保存组名对象group_id
    touch("/etc/groups","frwx------",0,0);//只有root管理员用户组才可以查看,不保存字符串,直接用二进制保存用户信息,方便使用

    Group admin;
    strcpy(admin.name,"admin");
    admin.id=0;
    Group guest;
    strcpy(guest.name,"guest");
    guest.id=1;
    saveToGroup(admin);
    saveToGroup(guest);
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
    }else if(path.find_first_of("/")==0){
        return getInode(readInode(0),path.substr(1,path.length()));
    }else if(path.find_first_of("/")==string::npos){
        //最后一项了,可能是文件或文件夹
        int nodeId=getInodeidFromDir(parent,path);
        if(nodeId==-1){
            cout<<path<<"不存在"<<endl;
            return parent;
        }
        Inode node=readInode(nodeId);
        return node;
    }else{
        //还有至少各个分隔符所以是文件夹
        string childname=path.substr(0,path.find_first_of("/"));
        int nodeId=getInodeidFromDir(parent,childname);
        if(nodeId==-1){
            cout<<"没有"<<childname<<"这个文件夹"<<endl;
            return parent;
        }
        parent=readInode(nodeId);
        if(parent.permissions[0]!='d'){
            cout<<parent.filename<<"不是一个文件夹"<<endl;
        }
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
//根据文件夹输出一定的内容
//如果可以显示返回1表示显示成功
//否则返回0
int showDir(Directory dir,vector<string> args){
    Inode node=readInode(dir.inode_id);
    //如果拥有权限则输出
    int user_id=std::stoi(args[0]);
    int group_id=std::stoi(args[1]);
    User temp;
    temp.user_id=user_id;
    temp.group_id=group_id;
    if(canI(node,temp,1)){
        cout<<dir.name<<endl;
        return 1;
    }else{
        return 0;
    }
}


//显示详细的信息
int showDirDetial(Directory dir ,vector<string> args){
    Inode node=readInode(dir.inode_id);
    //如果拥有权限则输出
    int user_id=std::stoi(args[0]);
    int group_id=std::stoi(args[1]);
    User temp;
    temp.user_id=user_id;
    temp.group_id=group_id;
    if(canI(node,temp,1)){
        cout<<node.permissions<<" "<<node.inode_id<<" "<<node.user_id<<" "<<node.group_id
            <<" "<<node.filesize<<"B "<<node.mtime<<" "<<dir.name<<endl;
        return 1;
    }
    return 0;
}

void traverse_ls(Inode node,dirFun func ,User user){
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
        }
    }
}
string getPath(Inode node){
    Superblock spb=getSuperBlock();
    if(node.inode_id==spb.root_inode)
        return "/";
    string path="";
    Inode temp=node;
    int parent_id;
    parent_id=getInodeidFromDir(temp,"..");
    while(temp.inode_id!=parent_id){//不是根节点
        path=temp.filename+path;
        path="/"+path;
        temp=readInode(parent_id);
        parent_id=getInodeidFromDir(temp,"..");
    }
    return path;
}


//node当前的节点号,user需要鉴定的用户,action所对应的请求
//action 1 2 3 对应的是读写和执行
bool canI(Inode node,User user,int action){//用于权限的鉴定
    string permission=node.permissions;
    string rwx="rwx";
    if(node.user_id==user.user_id){
        if(permission[action]==rwx[action-1])
            return true;
    }else if(node.group_id==user.group_id){
        if(permission[action+3]==rwx[action-1])
            return true;
    }else{
        if(permission[action+6]==rwx[action-1])
            return true;
    }
    return false;
}

int saveToGroup(Group group){
    string path="/etc/groups";
    Inode node=getInode(path);
    if(node.filesize==0){
        int pos=node.blockaddress[0];
        opendisk();
        fs.seekp(pos*sizeof(Block),ios_base::beg);
        fs.write((char *)&group,sizeof(group));
        fs.close();
        node.filesize=node.filesize+sizeof(Group);
        writeInode(node);
        return 0;
    }else{//遍历,如果已有则更新,没有则在末尾添加
        unsigned int i=0;
        int pos=node.blockaddress[0];
        opendisk();
        fs.seekp(pos*sizeof(Block),ios_base::beg);
        for(;i<node.filesize/sizeof(Group);i++){
            Group temp;
            fs.read((char*)&temp,sizeof(temp));
            if(temp.name==group.name){
                fs.seekp(-sizeof(Group),ios_base::cur);
                fs.write((char *)&group,sizeof(group));
                break;
            }
        }
        if(i==node.filesize/sizeof(Group)){
            fs.write((char *)&group,sizeof(Group));
            node.filesize+=sizeof(Group);
        }
        closedisk();
        writeInode(node);
        return i;
    }
}
int getgroupid(string grpname){
    Inode node=getInode("/etc/groups");
    int group_amount=node.filesize/sizeof(Group);
    for(int i=0;i<group_amount;i++){
        int address=getFileAddress(node,i*sizeof(Group));
        opendisk();
        fs.seekg(address,ios_base::beg);
        Group temp;
        fs.read((char *)&temp,sizeof(temp));
        closedisk();
        string name=temp.name;
        if(name==grpname)
            return temp.id;
    }
    return -1;
}

//得到用户名对应的id
//如果没有找到的话就返回-1
int getUserId(string usrname){
    Inode node=getInode("/etc/passwd");
    int user_amount=node.filesize/sizeof(User);
    for(int i=0;i<user_amount;i++){
        int address=getFileAddress(node,i*sizeof(User));
        opendisk();
        fs.seekg(address,ios_base::beg);
        User temp;
        fs.read((char *)&temp,sizeof(temp));
        closedisk();
        string name=temp.name;
        if(name==usrname){
            return temp.user_id;
        }
    }
    return -1;
}

//将一个子项目文件或者文件夹或者链接从父目录中删除
//注意并不删除文件内容只是将文件inde_id和文件项目从父目录中删除
//不可以删除 . ..
void rmChildFromDir(Inode node,string file){
    if(node.permissions[0]!='d'){
        cout<<"这不是一个文件夹"<<endl;
    }else{
        if(file=="."||file==".."){
            cout<<"不可以删除"<<file<<",请直接删除文件夹"<<endl;
            return;
        }else{
            int dirs=node.filesize/sizeof(Directory);
            for(int i=0;i<dirs;i++){
                Directory temp;
                int address=getFileAddress(node,i*sizeof(Directory));
                opendisk();
                fs.seekg(address,ios_base::beg);
                fs.read((char*)&temp,sizeof(temp));
                closedisk();
                string name=temp.name;
                if(name==file){
                    //找到了
                    if(i!=dirs-1){
                        //不是最后一个文件夹那么直接将最后一个文件夹填到此处
                        int last=getFileAddress(node,(dirs-1)*sizeof(Directory));
                        opendisk();
                        fs.seekg(last,ios_base::beg);
                        fs.read((char*)&temp,sizeof(temp));
                        fs.seekp(address,ios_base::beg);
                        fs.write((char*)&temp,sizeof(temp));
                        closedisk();
                    }
                    reduceFilesize(node,sizeof(Directory));
                    return;
                }
            }
            cout<<"没找到此文件"<<endl;
        }
    }
}
Inode getInodeorNull(Inode parent,string path){
    // cout<<path<<endl;
    if(path==""){
        //已近找完了
        return parent;
    }else if(path.find_first_of("/")==0){
        return getInode(readInode(0),path.substr(1,path.length()));
    }else if(path.find_first_of("/")==string::npos){
        //最后一项了,可能是文件或文件夹
        int nodeId=getInodeidFromDir(parent,path);
        if(nodeId==-1){
            parent.inode_id=-1;
            return parent;
        }
        Inode node=readInode(nodeId);
        return node;
    }else{
        //还有至少各个分隔符所以是文件夹
        string childname=path.substr(0,path.find_first_of("/"));
        int nodeId=getInodeidFromDir(parent,childname);
        if(nodeId==-1){
            parent.inode_id=-1;
            return parent;
        }
        parent=readInode(nodeId);
        if(parent.permissions[0]!='d'){
            parent.inode_id=-1;
            return parent;
        }
        string new_path=path.substr(path.find_first_of("/")+1,path.length());
        return getInode(parent,new_path);
    }
}


bool leaglePath(Inode node ,string path){
    Inode result=getInodeorNull(node,path);
    if(result.inode_id==-1)
        return false;
    else
        return true;
}

void write2File(Inode node ,string something){

    reduceFilesize(node,node.filesize);//将其清空
    node=readInode(node.inode_id);
    //将新的内容写入其中
    for(int i=0;i<something.length();i++){
        char c=something[i];
        int address=getFileAddress(node,i);
        opendisk();
        fs.seekp(address,ios_base::beg);
        fs.write((char*)&c,sizeof(c));
        closedisk();
        node=readInode(node.inode_id);
        node.filesize++;
        writeInode(node);
    }
}
#endif
