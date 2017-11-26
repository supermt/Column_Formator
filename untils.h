//
// Created by superMT on 2017/11/26.
//

#ifndef COLUMN_FORMATOR_UNTILS_H
#define COLUMN_FORMATOR_UNTILS_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>

#define TableDescription "logtime:timestamp,svr_ip:string,user_ip:string,port:long,host:string,url:string,req_args:string,strrange:string,http_code:long,send_bytes:long,handle_time:long,refer:string,user_agent:string,stdform:string,uin:long,isnormalclosed:long,url302:string,cdn:long,sample:long,filesize:long,inner_errcode:long,inner_filename:string,bizid:long,flow:long,clientappid:string,reverse_proxy:string,oc_id:string,str_reserve:string,vkey:string,int_reserve:long,province:long,isp:long,log_type:long,get_store_time:long,deliver_time:long,store_type:long,bit_rate:long,media_time:long,media_type:string,req_type:long,inner_errmsg:string,content_type:string,store_ip:string,resolution:long,reserve1:long,reserve2:long,reserve3:long,reserve4:string,reserve5:string"
//#define TableDescription "temp:long"

//#define MAX_LINE_LENGTH 1024
#define SourceFilePath "../sourcefile"
#define TargetDirPrefix "../target_column/"
#define TargetFilePrefix "/column"

#define TYPE_LONG 0
#define TYPE_STRING 1
#define TYPE_TIMESTAMP 2

#define MAX_OFFSET 256-1 // avoid the case that force transform make 256 become 0000 0000


#define MAX_STRING_SIZE 500


#define TIME_STAMP_FORMAT "%Y-%m-%d %H:%M:%S"

typedef unsigned char BYTE;
typedef BYTE HEAD_COUNTER[2];

struct Column_Unit {
    unsigned long row;
    struct Column_Value {
        bool empty = false;
        long numeric_value;
        std::string string_value; // add 2 more bytes for the head counter
    } value;
};

struct Long_Column {
    unsigned long row;
    long value;
};

struct String_Column {
    unsigned long row;
    std::string value;
};


struct Column_Description {
    int type;
    std::string column_name;
};


void SplitString(const std::string &s, std::vector<std::string> &v, const std::string &c);

void intToHeadCounter(unsigned int num, HEAD_COUNTER result);

std::vector<Column_Description> phrase_table(std::string table_description);

long
readAsText(const char *file_path, std::vector<Column_Description> column_desc, std::vector<Column_Unit> column_array[]);

long addLongAsBytes(unsigned char array[], long offset, long target_value);

long addOneByte(BYTE array[], long offset, unsigned char target_value);

long mergeByteArrays(BYTE data[], long data_length, BYTE dest[], long start_position);


#endif //COLUMN_FORMATOR_UNTILS_H

