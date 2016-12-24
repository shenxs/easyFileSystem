#ifndef STRING_FUNC_CPP
#define STRING_FUNC_CPP

#include<string>
using namespace std;

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

#endif
