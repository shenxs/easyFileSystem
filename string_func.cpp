#ifndef STRING_FUNC_CPP
#define STRING_FUNC_CPP

//存放用于处理路径字符串的函数

#include<string>
#include<iostream>
using namespace std;


//===========================函数申明=================
string get_parentPath(string path);

string getChildName(string path);

//====================================================




//得到一个目录的父文件夹的路径
//  /  返回 /
//  /etc ===> /
//  /etc/passwd ===>  /etc
string get_parentPath(string path){
    if(path=="/"){
        return "/";
    }else{
        char seperator='/';
        int l=path.length();
        int i=l;
        while(path[i]!=seperator){
            i--;
        }
        if (i==0){
            return path.substr(0,1);
        }else{
            return path.substr(0,i);
        }
    }
}


//ie.
// /    ====> /
// /etc ===> etc
// /x/y/z/b =====>b
string getChildName(string path){
    int l=path.length();
    if(l==1){//只有一个根目录
        return path;
    }else{
        int i=l;
        while(path[i]!='/'){
            if(i==0){
                cout<<"这不是一个有效的路径"<<endl;
                return "不是一个有效路径";
            }
            i--;
       }
        return path.substr(i+1,l);
    }
}
#endif
