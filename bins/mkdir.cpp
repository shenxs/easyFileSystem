#include<fstream>
#include<iostream>
#include"../config.h"
using namespace std;
string diskname="virtualdisk";

int mkdir(string parent,string name,string permission,int userid,int groupid){

    fstream fs;
    fs.open(diskname.c_str(),ios_base::in|ios_base::binary);


    return 0;

}
