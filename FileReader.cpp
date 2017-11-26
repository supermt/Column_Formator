//
// Created by superMT on 2017/11/26.
//

#include "untils.h"

#define LONG_EQUAL 0
#define LONG_NOT_EQUAL 1
#define STRING_EQUAL 2
#define STRING_NOT_EQUAL 4
#define BIGGER 8
#define SMALLER 16
#define BIGGER_EQUAL 32
#define SMALLER_EQUAL 64

// max block size is 4 KB
#define MAX_BLOCK_LENGTH 4*1024


#define HEAD_LENGTH 9
#define LONG_ROW_LENGTH 9
using namespace std;

long bytes2Long(BYTE *byteNum, int offset);

int main(int argc, char *argv[]) {

    // accept three arguments
    // column name , compare operator and target value

    if (argc < 4) {
        printf("error , program accept three arguments like \' File_Reader user_ip != 192.168.1.1 \'");
    }

    vector<Column_Description> column_description = phrase_table(TableDescription);

    int column_count = column_description.size();

    vector<int> long_column;

    vector<int> string_column;

    map<string, int> column_index_map;
    int index = 0;
    for (Column_Description column_desc : column_description) {
        column_index_map.insert(make_pair(column_desc.column_name, index));
        if (column_desc.type == TYPE_STRING) {
            string_column.push_back(index);
        } else {
            long_column.push_back(index);
        }
        index++;
    }


    printf("validating arguments\n");

    string column_name = "";
    column_name.append(argv[1]);
    cout << column_name << endl;
    string predicate = "";
    predicate.append(argv[2]);
    int compare_op = 0;
    string target_value = "";
    target_value.append(argv[3]);

    bool is_string = false;

    map<string, int>::iterator key = column_index_map.find(column_name);

    if (key != column_index_map.end()) {
        printf("Column \' %s \' located , at the index of %d ,\t", key->first.c_str(), key->second);
        if (TYPE_STRING == column_description[key->second].type) {
            is_string = true;
            printf("Column is in the type of STRING\n");
        } else {
            printf("Column is in the type of LONG\n");
        }
    }

    if (predicate == "==") {
        // if the column is string type
        compare_op = is_string ? STRING_EQUAL : LONG_EQUAL;
    } else if (predicate == "!=") {
        compare_op = is_string ? STRING_NOT_EQUAL : LONG_NOT_EQUAL;
        // if the column is string type
    } else if (predicate == ">") {
        compare_op = BIGGER;
    } else if (predicate == "<") {
        compare_op = SMALLER;
    } else if (predicate == "<=") {
        compare_op = SMALLER_EQUAL;
    } else if (predicate == ">=") {
        compare_op = BIGGER_EQUAL;
    }

    // add head information to the sender block
    BYTE block[MAX_BLOCK_LENGTH];
    long array_length = 0;
    // + ------------------------- + ----------------------- + ----------- + ------------- +
    // | 1 byte (compare operator) | 1 byte (lines in total) | 8 bytes.....| compare value |
    // + ------------------------- + ----------------------- + ----------- + ------------- +
    // adding the comparing operator
    block[0] = (unsigned char) compare_op;
    // if you choose to read all data from a file , this byte means nothing for you
    memset(&block[1], 0, 1); // set the second byte to '\0'
    memset(&block[2], 0, 8); // set the next 8 bytes to '\0'
    // adding the target value
    if (is_string) {
        // if it is the string , add a 64 bytes buffer ,initialize it
        memset(&block[10], 0, 64);
        mergeByteArrays(reinterpret_cast<BYTE *>(argv[3]), strlen(argv[3]), block, 10);
    } else {
        // initialize a 8 byte area for this head
        memset(&block[10], 0, 8);
        addLongAsBytes(block, 10, atol(argv[3]));
    }



    // opening the file storing the grouped data
    string filename = TargetDirPrefix;
    char file_subfix[FILENAME_MAX] = "";

    sprintf(file_subfix, "column_%d", key->second);

    filename.append(file_subfix);
    cout << filename << endl;

    FILE *source = fopen(filename.c_str(), "rb");


    fclose(source);
    return 0;
}

long bytes2Long(BYTE *byteNum, int offset) {
    long num = 0;
    for (int ix = 0; ix < 8; ++ix) {
        num <<= 8;
        num |= (byteNum[offset + ix] & 0xff);
    }
    return num;
}
