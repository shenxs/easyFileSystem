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


void showspb(){
    Superblock spb=getSuperBlock();

    cout<<"============================================\n";
    cout<<"文件系统名"<<spb.fsname<<endl
        <<"文件系统magicNumber"<<spb.magicnumber<<endl
        <<"根目录Inode编号"<<spb.root_inode<<endl
        <<"已经使用的Inode的数量"<<spb.inode_usered<<endl
        <<"Inode 总数"<<spb.inode_number<<endl;

    cout<<"==============\n";

    cout<<"空闲栈\n";
    cout<<"当前空闲数量"<<spb.blocks.free<<endl;
    // cout<<"下一组地址"<<spb.blocks.next_adress<<endl;
    int j=0;
    for(int i=0;i<spb.blocks.free;i++)
    {
        cout<<spb.blocks.blocks[i]<<" ";
        if (j==10){
            cout<<endl;
            j=0;
        }else{
            j++;
        }
    }

    cout<<"\n============================================\n";
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


    int p=0;
    for(int i=0;p<node.filesize;i++)
    {
        int address=getFileAddress(node,p);
        int x=address/sizeof(Block);
        cout<<"blockaddress["<<i<<"]="<<x<<endl;
        p+=sizeof(Block);

    }
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
