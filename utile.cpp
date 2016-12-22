#include<iostream>
#include<fstream>
#include<string>
#include<string.h>
#include"struct.cpp"
#include"config.h"
using namespace std;


int getaFreeBlockAddress(Superblock spb);

//创建初始文件夹
void init_dir(string diskname);
//读取并返回当前的superblock;
Superblock getSuperBlock(string diskname);

//返回一个初始化的超级块
Superblock init_superBolck(){
    Superblock spb;
    spb.magicnumber=maggci_number;
    strcpy(spb.fsname,"easyfs");
    spb.inode_number=(data_start-2)*block_size/sizeof(Inode);
    spb.root_inode=0;//根文件夹的inode的下标
    spb.blocks.free=100;
    spb.blocks.next_adress=148;

    for(int i=0;i<100;i++){
        spb.blocks.blocks[i]=48+i;
    }
    return spb;
}

//装载文件系统
void loadfs(string diskname){

}

//在虚拟磁盘上初始化一个可用的文件系统
void init_fs(string diskname){
    Block blocks[block_number];
    fstream fs;
    fs.open(diskname.c_str(),ios_base::in|ios_base::out|ios_base::binary);
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
    fs.close();
    //创建必要的文件
    init_dir(diskname);

}

//判断一个磁盘上是否已有一个文件系统
bool gooddisk(string diskname){
    //通过超级块上的标记判断是否有一个文件系统
    fstream fs;
    fs.open(diskname.c_str(),ios_base::in|ios_base::binary);
    if(!fs.is_open()){
        cout<<"文件打开失败"<<endl;
        return false;
    }else{
        //超级块是第1块,第0块为导引块,MBR保留
        fs.seekg(sizeof(Block),ios_base::beg);//指向第1块
        Superblock *spb=new Superblock;
        fs.read((char *)spb,sizeof(Superblock));
        fs.close();
        bool result;
        if(spb->magicnumber==maggci_number){
            result=true;
        }else{
            result=false;
        }
        return result;
    }
}

Superblock getSuperBlock(string diskname){
    Superblock spb;
    fstream fs;
    fs.open(diskname.c_str(),ios_base::in|ios_base::binary);
    fs.seekg(sizeof(Block),ios_base::beg);
    fs.read((char *)&spb,sizeof(Superblock));

    fs.close();
    return spb;
}


//返回一个可用的空闲块,如果没有空闲块则返回0
int getaFreeBlockAddress(Superblock spb){
    if(spb.blocks.free>1)
    {
        spb.blocks.free--;
        return spb.blocks.blocks[spb.blocks.free];
    }else if(spb.blocks.free==1){
        if(spb.blocks.next_adress==0){//已经是最后一组了
            return 0;
        }else{//当前的组中所有的块都已经使用了
            //将现在的lNode写回磁盘
            Lnode a_lnode;
            a_lnode.free=spb.blocks.free;
            a_lnode.next_adress=spb.blocks.next_adress;
            for(int i=0;i<100;i++){
                a_lnode.blocks[i]=spb.blocks.blocks[i];
            }
            //将下一组的自由块的Lnode装入spb

            fstream fs;
            fs.open("virtualdisk",ios_base::in|ios_base::binary);
            fs.seekp(sizeof(Block)*(spb.blocks.next_adress-100),ios_base::beg);
            fs.write((char *)&a_lnode,sizeof(a_lnode));
            //将下一组的自由块的Lnode装入spb
            fs.seekp(sizeof(Block)*spb.blocks.next_adress,ios_base::beg);
            fs.read((char *) &a_lnode,sizeof(a_lnode));

            spb.blocks.free=a_lnode.free;
            spb.blocks.next_adress=a_lnode.next_adress;
            for(int i=0;i<100;i++){
                spb.blocks.blocks[i]=a_lnode.blocks[i];
            }

            int freeblock=getaFreeBlockAddress(spb);

            //将spb写回
            fs.seekp(sizeof(Block),ios_base::beg);
            fs.write((char* )&spb,sizeof(spb));
            return freeblock;


        }
    }else{
        cout<<"出现未知错误"<<endl;
        return 0;
    }
}

void init_dir(string diskname){
    //创建根文件夹,/root,/home,/etc/users
    Superblock spb=getSuperBlock(diskname);

    //根目录的inode
    Inode an_inode;
    an_inode.inode_id=0;
    an_inode.user_id=0;//root用户id为0;
    an_inode.group_id=0;//
    strcpy(an_inode.permissions,"drwxr-xr-x");
    cout<<an_inode.permissions<<endl;
    an_inode.mtime=time(0);
    an_inode.filesize=2*sizeof(Directory);//初始只有. 和..
    an_inode.blocknum=1;
    an_inode.blockaddress[0]=getaFreeBlockAddress(spb);
    an_inode.links=0;

    Directory d1,d2;
    d1.inode_id=d2.inode_id=0;//根目录的上级和当前都是自身
    strcpy(d1.name,".");
    strcpy(d2.name,"..");

    fstream fs;
    fs.open(diskname.c_str(),ios_base::in|ios_base::binary);

    //写回inode
    fs.seekp(2*sizeof(Block),ios_base::beg);
    fs.write((char *)&an_inode,sizeof(an_inode));

    //写入文件夹
    fs.seekp(an_inode.blockaddress[0]*sizeof(Block),ios_base::beg);
    fs.write((char*) &d1,sizeof(d1));
    fs.write((char*) &d2,sizeof(d2));

    //前缀,目录名,permission,userid ,groupid;
    // mkdir("/",""
}
