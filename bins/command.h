#ifndef _COMMAND_H_
#define _COMMAND_H_

#include<iostream>
#include<map>
#include<vector>
#include<string>
#include <termios.h>
#include <unistd.h>

#include"../struct.cpp"
#include"../utile.cpp"
using namespace std;

typedef int (*FnPtr)(vector<string>);
std::map<std::string,FnPtr> commandMap;
User currentUser;
Inode currentInode;
string PWD="/";

inline void eatline(){while (std::cin.get()!='\n') continue;}

string getpasswdfromcin(){
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    string s;
    getline(cin, s);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cout<<endl;
    return s;
}

int ls(vector<string> args);
int cd(vector<string> args);
int pwd(vector<string> args);
int common_mkdir(vector<string> args);
int whoami(vector<string> args);
int login(vector<string> args);
int help(vector<string> args);
int touch(vector<string> args);
int cat(vector<string> args);
int clear(vector<string> args);
int chgrp(vector<string> args);
int chmod(vector<string> args);
int chown(vector<string> args);
int passwd(vector<string> args);
int edit(vector<string> args);
int rm(vector<string> args);
int rmdir(vector<string> args);
int cp(vector<string> args);
int ln(vector<string> args);
int mv(vector<string> args);
//初始化commandMap()
void initCommands(){
    commandMap.clear();
    commandMap.insert(pair<string,FnPtr>("ls",ls));
    commandMap.insert(pair<string,FnPtr>("cd",cd));
    commandMap.insert(pair<string,FnPtr>("pwd",pwd));
    commandMap.insert(pair<string,FnPtr>("mkdir",common_mkdir));
    commandMap.insert(pair<string,FnPtr>("whoami",whoami));
    commandMap.insert(pair<string,FnPtr>("login",login));
    commandMap.insert(pair<string,FnPtr>("su",login));
    commandMap.insert(pair<string,FnPtr>("help",help));
    commandMap.insert(pair<string,FnPtr>("touch",touch));
    commandMap.insert(pair<string,FnPtr>("cat",cat));
    commandMap.insert(pair<string,FnPtr>("clear",clear));
    commandMap.insert(pair<string,FnPtr>("chmod",chmod));
    commandMap.insert(pair<string,FnPtr>("chown",chown));
    commandMap.insert(pair<string,FnPtr>("chgrp",chgrp));
    commandMap.insert(pair<string,FnPtr>("passwd",passwd));
    commandMap.insert(pair<string,FnPtr>("edit",edit));
    commandMap.insert(pair<string,FnPtr>("rm",rm));
    commandMap.insert(pair<string,FnPtr>("cp",cp));
    commandMap.insert(pair<string,FnPtr>("mv",mv));
    commandMap.insert(pair<string,FnPtr>("ln",ln));
    commandMap.insert(pair<string,FnPtr>("rmdir",rmdir));
}


//cp source target
int cp(vector<string> args){
    return 0;
}


//ln source target     建立一个硬链接
//ln -s source tartget//建立一个软连接
int ln(vector<string> args){
    if(args.size()<3){
        cout<<"命令格式错误"<<endl;
    }else{
        if(args[1]=="-s"){

            //建立软链接,在一个文件中写入source文件的链接地址
            if(args.size()!=4)
                return 0;
            string source=args[2];
            string target=args[3];
            if(target.find_first_of("/")==string::npos){
                //直接在此文件夹中创建链接
                int id=getInodeidFromDir(currentInode,target);
                if(id!=-1){
                    cout<<target<<"已经存在"<<endl;
                }else{
                    if(canI(currentInode,currentUser,2)){
                        int childid=touch(getPath(currentInode)+"/"+target,
                                "lrwxrwxrwx",currentUser.user_id,currentUser.group_id);
                        currentInode=readInode(currentInode.inode_id);
                        Inode temp=readInode(childid);
                        write2File(temp,source);
                    }else{
                        cout<<"你没有此项权限"<<endl;
                    }
                }
            }else{
                //可能是相对路径或者绝对路径
                string parent_path=get_parentPath(target);
                if(leaglePath(currentInode,parent_path)){
                    Inode node=getInode(currentInode,parent_path);//得到文件夹的Inode
                    if(node.permissions[0]!='d'){
                        cout<<node.filename<<"不是文件夹"<<endl;
                        return 0;
                    }
                    //是否已经存在同名文件
                    string childname=getChildName(target);
                    int id=getInodeidFromDir(node,childname);
                    if(id!=-1){
                        cout<<childname<<"已经存在"<<endl;
                        return 0;
                    }else{
                        if(canI(node,currentUser,2)){
                            string path=getPath(node);
                            if(path=="/")
                                path=path+childname;
                            else
                                path=path+"/"+childname;
                            int child_node_id=touch(path,"lrwxrwxrwx",currentUser.user_id,currentUser.group_id);
                            Inode temp=readInode(child_node_id);
                            write2File(temp,source);
                            currentInode=readInode(currentInode.inode_id);
                        }else{
                            cout<<"permission denied"<<endl;
                        }
                    }
                }else{
                    cout<<parent_path<<"不存在"<<endl;
                }
            }
        }else{
            //建立硬链接
            if(args.size()!=3){
                cout<<"命令格式错误"<<endl;
            }else{
                string source=args[1];
                string target=args[2];
                if(leaglePath(currentInode,source)){
                    if(leaglePath(currentInode,target)){
                        cout<<"已经存在同名文件"<<endl;
                    }else{
                        Inode source_node=getInode(currentInode,source);

                        if(target.find_first_of("/")==string::npos){
                            //在当前Inode下新建文件项
                            if(canI(currentInode,currentUser,2)){
                                Directory temp;
                                temp.inode_id=source_node.inode_id;
                                strcpy(temp.name,target.c_str());
                                addChild2Dir(currentInode,temp);
                                source_node.links++;
                                writeInode(source_node);
                                currentInode=readInode(currentInode.inode_id);
                            }else{
                                cout<<"没有权限"<<endl;
                            }
                        }else{
                            //相对或者绝对路径
                            string parent_path=get_parentPath(target);
                            if(leaglePath(currentInode ,parent_path)){
                                Inode node=getInode(currentInode,parent_path);
                                if(node.permissions[0]!='d'){
                                    cout<<"Not a Directory"<<endl;
                                }else{
                                    if(canI(node,currentUser,2)){
                                        Directory temp;
                                        temp.inode_id=source_node.inode_id;
                                        string childname=getChildName(target);
                                        strcpy(temp.name,childname.c_str());
                                        addChild2Dir(node,temp);
                                        source_node.links++;
                                        writeInode(source_node);
                                        currentInode=readInode(currentInode.inode_id);
                                    }else{
                                        cout<<"permission denied"<<endl;
                                    }
                                }

                            }else{
                                cout<<parent_path<<"不存在"<<endl;
                            }
                        }
                    }

                }else{
                    cout<<"源文件不存在"<<endl;
                }
            }
        }
    }
    return 0;
}


//mv source target
//将一个dir写到另一个dir文件夹下
//名字也要改
int mv(vector<string> args){

    if(args.size()!=3){
        cout<<"格式错误"<<endl;
    }else{
        string source=args[1];
        string target=args[2];
        if(leaglePath(currentInode,source)){
            Inode source_parent_node,target_parent_node;
            string source_name,target_name;
            if(source.find_first_of("/")==string::npos){
                source_parent_node=currentInode;
                source_name=source;
            }else{
                string parent_path=get_parentPath(source);
                source_parent_node=getInode(currentInode,parent_path);
                source_name=getChildName(source);
            }

            if(target.find_first_of("/")==string::npos){
                target_parent_node=currentInode;
                target_name=target;
            }else{
                string parent_path=get_parentPath(target);
                if(leaglePath(currentInode,parent_path)){
                    target_parent_node=getInode(currentInode,parent_path);
                }else{
                    cout<<parent_path<<"不存在"<<endl;
                    return 0;
                }
                target_name=getChildName(target);
            }

            if(canI(source_parent_node,currentUser,2)){
                //源文件的读写权
                int id=getInodeidFromDir(target_parent_node,target_name);
                if(id==-1){
                    //如果target不存在就在文件夹中新建一个
                    if(canI(target_parent_node,currentUser,2)){
                        if(target_parent_node.permissions[0]!='d'){
                            cout<<target_parent_node.filename<<"不是文件夹"<<endl;
                        }else{
                            int id=getInodeidFromDir(source_parent_node,source_name);
                            addChild2Dir(target_parent_node,target_name,id);
                            source_parent_node=readInode(source_parent_node.inode_id);
                            rmChildFromDir(source_parent_node,source_name);
                            currentInode=readInode(currentInode.inode_id);
                        }
                    }else{
                        cout<<"permissiom denied"<<endl;
                    }
                }else{
                    //如果target是一个文件夹就将source移到文件夹中
                    Inode node=readInode(id);
                    if(node.permissions[0]=='d'){
                        //在node文件夹下面添加
                        if(canI(node ,currentUser,2)){
                            int id=getInodeidFromDir(node,source_name);
                            if(id==-1){
                                int id=getInodeidFromDir(source_parent_node,source_name);
                                addChild2Dir(node,source_name,id);
                                source_parent_node=readInode(source_parent_node.inode_id);
                                rmChildFromDir(source_parent_node,source_name);
                                currentInode=readInode(currentInode.inode_id);
                            }else{
                                cout<<source_name<<"已近存在"<<endl;
                            }
                        }else{
                            cout<<node.filename<<" permissiom denied"<<endl;
                        }
                    }else{
                        cout<<target_name<<"不是一个文件夹"<<endl;
                    }
                    //如果target是个已近存在的文件那么就不可以
                }

            }else{
                cout<<"permission denied"<<endl;
            }


        }else{
            cout<<"源文件不存在"<<endl;
        }



    }
    return 0;
}


//删除文件夹,只有当文件夹是空的时候才可以删除
//格式,rmdir dirname
int rmdir(vector<string> args){
    if(args.size()!=2){
        return 0;
    }else{
        int id=getInodeidFromDir(currentInode,args[1]);
        if(id==-1){
            cout<<args[1]<<"不存在"<<endl;
        }else{
            Inode node=readInode(id);
            if(node.permissions[0]!='d'){
                cout<<args[1]<<"不是一个文件夹"<<endl;
            }else{
                if(canI(node,currentUser,2)){
                    if(node.filesize>2*sizeof(Directory)){
                        cout<<args[1]<<"非空不可以被删除"<<endl;
                    }else{
                        if(node.links!=1){
                            cout<<"请先删除所有硬链接文件"<<endl;
                        }else{
                            reduceFilesize(node,node.filesize);
                            rmChildFromDir(currentInode,args[1]);
                            rmInode(node.inode_id);
                            currentInode=readInode(currentInode.inode_id);
                        }
                    }

                }else{
                    cout<<"对不起你没有此权限"<<endl;
                }
            }
        }
    }
    return 0;
}



//删除当前文件夹下的某个文件,不可以是文件夹
//格式 rm filename
int rm(vector<string> args){
    //需要删除文件的内容以及文件的Inode,
    //只有当链接数目是1的时候,需要删除,才会清空源文件
    if(args.size()<=1){
        cout<<"格式错误"<<endl;
    }else if(args.size()==2){
        int id=getInodeidFromDir(currentInode,args[1]);
        if(id==-1){
            cout<<args[1]<<"不存在"<<endl;
        }else{
            Inode node=readInode(id);
            if(node.permissions[0]=='d'){
                cout<<"删除文件夹请使用rmdir命令\n";
            }else{
                if(canI(node,currentUser,2)){
                    rmChildFromDir(currentInode,args[1]);
                    if(node.links==1){
                        reduceFilesize(node,node.filesize);
                        rmInode(node.inode_id);
                    }else{
                        node.links--;
                        writeInode(node);
                    }
                    currentInode=readInode(currentInode.inode_id);
                    cout<<"成功删除文件"<<endl;
                }else{
                    cout<<"你没有此权限"<<endl;
                }
            }
        }
    }
    return 0;
}
//向一个文件内写入一些内容,会将其原来的内容覆盖
//格式 edit filename something
int edit(vector<string> args){
    if(args.size()!=3){
        cout<<"格式错误:\n"
            <<"edit <filename> <something>\n";
    }else{
        int id=getInodeidFromDir(currentInode,args[1]);
        if(id==-1){
            cout<<args[1]<<"不存在"<<endl;
        }else{
            Inode node=readInode(id);
            if(node.permissions[0]=='d'){
                cout<<args[1]<<"是一个文件夹"<<endl;
            }else{
                if(canI(node,currentUser,2)){
                    string something=args[2];
                    write2File(node,something);
                }else{
                    cout<<"对不起你没有写入权限"<<endl;
                }
            }

        }
    }
    return 0;
}
//格式是 chgrp grpname filename
//filename是当前目录下的文件,是路径的话可能存在不存在的情况
int chgrp(vector<string> args){
    if(args.size()!=3){
        cout<<"格式错误"<<endl;
        return 0;
    }else{
        string grpname=args[1];
        string file=args[2];
        int id=getInodeidFromDir(currentInode,file);
        if(id==-1){
            cout<<"该文件或者目录不存在"<<endl;
        }else{
            Inode node=readInode(id);
            if(canI(node,currentUser,2)){
                int group_id=getgroupid(grpname);
                if(group_id==-1){
                    cout<<"组名错误"<<endl;
                }else{
                    node.group_id=group_id;
                    writeInode(node);
                    cout<<"修改成功"<<endl;
                }
            }else{
                cout<<"你没有权限写入"<<endl;
            }
        }
    }
    return 0;
}

//格式类似于chgrp
//只能引用于当前文件夹
//TODO让其支持绝对路径和相对路径

int chown(vector<string> args){
    // chown root filename
    if(args.size()!=3){
        cout<<"格式错误"<<endl;
    }else{
        string usrname=args[1];
        string file=args[2];
        int id=getInodeidFromDir(currentInode,file);
        if(id==-1){
            cout<<"该文件不存在"<<endl;
        }else{
            Inode node=readInode(id);
            if(canI(node,currentUser,2)){
                int id=getUserId(usrname);
                if(id==-1){
                    cout<<"此用户名"<<endl;
                }else{
                    node.user_id=id;
                    writeInode(node);
                    cout<<"修改成功"<<endl;
                }
            }else{
                cout<<"你没有修改的权限"<<endl;
            }
        }
    }
    return 0;
}

//可以改变用户的密码
//直接使用passwd可更改当前用户的密码
//passwd + username可以更改 username的名字
int passwd(vector<string> args){
    if(args.size()==1){
        cout<<"更改当前用户"<<currentUser.name<<"的密码"<<endl;
        cout<<"请输入当前的密码 :";
        string oldpass=getpasswdfromcin();
        if(oldpass==currentUser.password){
            cout<<"请输入新密码 :";
            string pass1=getpasswdfromcin();
            cout<<"请再次输入密码 :";
            string pass2=getpasswdfromcin();
            if(pass1==pass2){
                strcpy(currentUser.password,pass1.c_str());
                saveTopasswd(currentUser);
                cout<<"密码修改成功"<<endl;
            }else{
                cout<<"密码不配"<<endl;
            }
        }else{
            cout<<"密码错误"<<endl;
        }
    }else if(args.size()==2){
        cout<<"施工中,尝试切换到某个账号在改变其密码"<<endl;
    }else{
        cout<<"命令格式错误"<<endl;
    }
    return 0;
}
int chmod(vector<string> args){
    //参数 chmod 777/444/ file
    if(args.size()!=3){
        cout<<"格式错误"<<endl;
    }else{
        string newmod=args[1];
        int moduser=(newmod[0]-('7'-7));
        int modegrp=(newmod[1]-('7'-7));
        int modeotr=(newmod[2]-('7'-7));
        cout<<"newmod="<<moduser<<modegrp<<modeotr<<endl;
        string file=args[2];
        string mode="";
        int id=getInodeidFromDir(currentInode,file);
        if(id==-1){
            cout<<file<<"不存在"<<endl;
        }else{
            Inode node=readInode(id);
            if(canI(node,currentUser,2)){
                int b=1;
                for(int i=0;i<3;i++){
                    if(modeotr&(1<<i)){
                        if(b==1){
                            mode="x"+mode;
                        }else if(b==2){
                            mode="w"+mode;
                        }else{
                            mode="r"+mode;
                        }
                    }else{
                        mode="-"+mode;
                    }
                    b++;
                }
                b=1;
                for(int i=0;i<3;i++){
                    if(modegrp&(1<<i)){
                        if(b==1){
                            mode="x"+mode;
                        }else if(b==2){
                            mode="w"+mode;
                        }else{
                            mode="r"+mode;
                        }
                    }else{
                        mode="-"+mode;
                    }
                    b++;
                }
                b=1;
                for(int i=0;i<3;i++){
                    if(moduser&(1<<i)){
                        if(b==1){
                            mode="x"+mode;
                        }else if(b==2){
                            mode="w"+mode;
                        }else{
                            mode="r"+mode;
                        }
                    }else{
                        mode="-"+mode;
                    }
                    b++;
                }
                mode=node.permissions[0]+mode;
                strcpy(node.permissions,mode.c_str());
                writeInode(node);
                cout<<"修改成功"<<endl;
            }else{
                cout<<"对不起你没有此权限"<<endl;
            }
        }

    }
    return 0;
}

//清屏
int clear(vector<string> args){
    cout<<" \033c";
    return 0;
}
//输出一个中的内容,只能是f
int cat(vector<string> args){
    if(args.size()==1){
        return -1;
    }else{
        string file=args[1];
        int InodeId=getInodeidFromDir(currentInode,file);
        if(InodeId==-1){
            cout<<file<<"不存在"<<endl;
        }else{
            Inode node=readInode(InodeId);
            if(node.permissions[0]=='d'){
                cout<<node.filename<<"是一个文件夹"<<endl;
            }else{
                if(canI(node,currentUser,1)){
                    for(int i=0;i<node.filesize;i++){
                        int address=getFileAddress(node,i);
                        opendisk();
                        fs.seekg(address,ios_base::beg);
                        char c;
                        fs.read((char *)&c,sizeof(c));
                        closedisk();
                        cout<<c;
                    }
                    cout<<endl;
                }else{
                    cout<<"对不起你没有访问权限"<<endl;
                }
            }
        }

    }
    return 0;
}

//touch如果没有此文件则创建一个空文件,如果有此文件则更新其时间戳
//touch name
//new file or updatefile mtime
//TODO 检查文件名是否合法
int touch(vector<string> args){
    if(args.size()<=1){
        return -1;
    }else{
        string name=args[1];
        int id=getInodeidFromDir(currentInode,name);
        if(canI(currentInode,currentUser,2)){
            if(id==-1){
                //如果不存在此文件则新建
                string path=getPath(currentInode);
                path=path+"/"+name;
                touch(path,"frw-rw-r--",currentUser.user_id,currentUser.group_id);
                currentInode=readInode(currentInode.inode_id);
                return 0;
            }else{
                //已经存在这个文件了,更新他的时间戳
                Inode temp=readInode(id);
                temp.mtime=time(0);
                writeInode(temp);
                return 0;
            }
        }else{
            cout<<"对不起没有权限"<<endl;
        }
    }
    return 0;
}

int help(vector<string> args){
    map<string,FnPtr>::iterator it=commandMap.begin();
    cout<<"当前可用命令列表"<<endl;
    for(;it!=commandMap.end();it++){
        cout<<it->first<<endl;
    }
    cout<<"exit"<<endl;
    return 0;
}
int login(vector<string> args){
    string username,password;
    int flag=0;
    User a_user;
    while(!flag){
        cout<<"Login as:";cin>>username;
        eatline();
        cout<<username<<"'s password:";
        password=getpasswdfromcin();
        strcpy(a_user.name,username.c_str());
        strcpy(a_user.password,password.c_str());
        flag=leagleUser(a_user);
    }
    cout<<"Welcome!"<<endl;
    // 登录完成后需要设定初始的环境变量
    currentUser=readUser(flag);
    if(strcmp(a_user.name,"root")==0)
        PWD="/root";
    else{
        string home="/home/";
        PWD=home+a_user.name;
    }
    currentInode=getInode(PWD);
    // cin.ignore();
    return 0;

}
int pwd(vector<string> args){
    cout<<getPath(currentInode)<<endl;
    return 0;
}
int whoami(vector<string> args){
    cout<<"name:\t"<<currentUser.name<<endl;
    cout<<"id:\t"<<currentUser.user_id<<endl;
    cout<<"group_id:\t"<<currentUser.group_id<<endl;
    return 0;
}
int ls(vector<string> args){
    //显示当前目录下的内容
    //遍历目录项,应用输出文件名函数
    if(args.size()==1){
        //没有任何参数,显示当前文件夹下的内容
        // Inode node=getInode(PWD);
        traverse_ls(currentInode,showDir,currentUser);
    }else{//包含一个路径的参数
        string path=args[1];
        if(path[0]=='/'){//如果是绝对路径
            Inode node=getInode(readInode(0),path.substr(1,path.length()));
            traverse_ls(node,showDir,currentUser);
        }else if(path=="-l"){//如果是有-l参数
            // Inode node=getInode(currentInode,path);
            traverse_ls(currentInode,showDirDetial,currentUser);
        }else{
            Inode node=getInode(currentInode,path);
            traverse_ls(node,showDir,currentUser);
        }
    }
    return 0;
}

int cd(vector<string> args){
    //遍历文件项,返回指定的inode的编号
    //修改pwd
    if(args.size()==1)
        return 0;
    string path=args[1];
    // Inode node=getInode(PWD);
    if(path[0]=='/')//绝对路径
    {
        Inode temp=getInode(readInode(0),path.substr(1,path.length()));
        if(temp.permissions[0]=='d')
            currentInode=temp;
        else{
            cout<<temp.filename<<"不是一个文件夹"<<endl;
        }
    }else{//相对路径

        Inode temp=getInode(currentInode,path);
        if(temp.permissions[0]=='d'){
            currentInode=temp;
        }else{
            cout<<temp.filename<<"不是一个文件夹"<<endl;
        }
    }
    return 0;
}
int common_mkdir(vector<string> args){
    //新建一个文件夹,如果用户对于当前文件夹有可写权限的话
    //检查是否有重名
    //TODO检查文件名的合法性
    string permissions=currentInode.permissions;
    if(args.size()==1){
        cout<<"请输入新建文件夹的文件名"<<endl;
        return -1;
    }else if(canI(currentInode,currentUser,2)){
        string new_dir_name=args[1];
        string path=getPath(currentInode)+"/"+new_dir_name;
        //需要检查重名
        if(getInodeidFromDir(currentInode,new_dir_name)==-1){
            mkdir(path,"drwxrwxr-x",currentUser.user_id,currentUser.group_id);
        }else{
            cout<<"已存在此文件夹"<<endl;
        }
    }else{
        cout<<"对不起,你没有当前目录的写权限"<<endl;
    }
    //写入新的文件夹后需要重读currentInode因为文件的大小变了
    currentInode=readInode(currentInode.inode_id);
    return 0;
}
#endif
