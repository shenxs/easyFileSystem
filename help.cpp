#ifndef HELP_CPP_
#define HELP_CPP_

//方便调试的时候输出信息
#include<iostream>
#include"struct.cpp"
#include"lowio.cpp"
using namespace std;


void showDirContent(Inode inode){
    char filetype=inode.permissions[0];
    if(filetype=='d'){
        Directory temp;
        opendisk();
        fs.seekg(inode.blockaddress[0]*sizeof(Block),ios_base::beg);
        for(unsigned int i=0;i<inode.filesize/sizeof(Directory);i++){
            fs.read((char*)&temp,sizeof(temp));
            cout<<temp.name<<"\t"<<temp.inode_id<<endl;
        }
        closedisk();
    }
}

void showInode(Inode node){
    cout<<"\n\n==========================================\n";
    cout<<node.filename<<"\tfilename\n"
        <<node.permissions<<"\t permission\n"
        <<node.links<<"\tlinks\n"
        <<node.inode_id<<"\tid\n"
        <<node.blocknum<<"\tblocknum\n"
        <<node.mtime<<"\tmtime\n"
        <<node.filesize<<"\tfileszie\n"
        <<node.group_id<<"\tgroup_id\n"
        <<node.user_id<<"\tuser_id\n";

    for(int i=0;i<node.blocknum;i++)
        cout<<"blockaddress["<<i<<"]="<<node.blockaddress[i]<<endl;

    showDirContent(node);
    cout<<"==========================================\n";
}


//将现有的Inode的节点全部打印
void showInodes(){
    int number=getSuperBlock().inode_usered;
    for(int i=0;i<number;i++){
        showInode(readInode(i));
    }
}

#endif
