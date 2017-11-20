#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>


#define TableDescription "timestamp:timestamp,server_ip:string,client_ip:string,empty:string"

//#define MAX_LINE_LENGTH 1024
#define SourceFilePath "/home/supermt/codes/source"


#define TYPE_LONG 0
#define TYPE_STRING 1
#define TYPE_TIMESTAMP 2
#define TYPE_EMPTY 3

#define MAX_STRING_SIZE 500


#define TIME_STAMP_FORMAT "%Y-%m-%d %H:%M:%S"
using namespace std;


typedef unsigned char BYTE;
typedef BYTE HEAD_COUNTER[2];

struct Column_Unit {
    unsigned long row;
    union Column_Value {
        long numeric_value;
        BYTE *string_value; // add 2 more bytes for the head counter
    } value;
};

struct Column_Description {
    int type;
    string column_name;
};


void SplitString(const std::string &s, std::vector<std::string> &v, const std::string &c) {
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2) {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));
}


void intToHeadCounter(unsigned int num, HEAD_COUNTER result) {

    // codes below is only suitable for BIG-ENDIAN
    BYTE byteNum[4];
    for (int i = 0; i < 4; i++) {
        int offset = 32 - (i + 1) * 8;
        byteNum[i] = (BYTE) ((num >> offset) & 0xff);
    }
    int ENDIAN = BYTE_ORDER == 1234 ? 0 : 1; // if the BYTE_ORDER is 1234, it means this machine is using lit_endian
    result[0] = byteNum[2 - 2 * ENDIAN]; // if the CPU use BIG_ENDIAN, there is no need for transforming
    result[1] = byteNum[3 - 2 * ENDIAN]; // if the CPU use LIT_ENDIAN, need to offset for two bytes
    return;

}


long readAsText(const char *file_path, vector<Column_Description> column_desc, vector<Column_Unit> column_array[]) {
    ifstream in(file_path);
    string filename;
    string line;
    int wordCount = 0;
    unsigned long row_counter = 0;
    if (in) {
        while (getline(in, line)) {
            // read a single line, split it into a vector of string by '\t'
            vector<string> columns;
            SplitString(line, columns, "\t");
            int column_pointer = 0;
            // process every single row
            for (column_pointer = 0; column_pointer < columns.size(); column_pointer++) {
                string val = columns[column_pointer];
                row_counter++;
                Column_Unit unit_value;
                unit_value.row = row_counter;

                // judge the column type
                switch (column_desc[column_pointer].type) {
                    case TYPE_LONG: {
                        if (!val.size()) {
                            // condition for an empty long value
                            unit_value.value.numeric_value = INT64_MIN;
                        } else {
                            // fill the column unit by a long type
                            unit_value.value.numeric_value = atol(val.c_str());
                        }
                        break;
                    }
                    case TYPE_TIMESTAMP: {
                        // transform the string into a long argument, then store it as a long
                        long time_result;
//                        cout << "transforming: " << val << endl;
                        tm time_struct;
                        strptime(val.c_str(), TIME_STAMP_FORMAT, &time_struct);
                        time_result = mktime(&time_struct);
//                        cout << "transform result: " << time_result << endl; // 1511175296 2017/11/20 18:54:56
                        unit_value.value.numeric_value = time_result;
                        break;
                    }
                    case TYPE_STRING: {
                        // add a two-byte counter in the front of each string as the offset of this string
                        HEAD_COUNTER length;
                        intToHeadCounter(val.length(), length);
                        /*
                         * char a[] = "ab";
                         * BYTE* pByte = static_cast<BYTE*> (a);
                         * 因为BYTE: An 8-bit integer that is not signed
                         * 它和unsigned char可以安全转换.
                         * */
                        // invoke a block of memory, which can contain the whole string and its head counter
                        // remember an additional char position for '\0'
                        unit_value.value.string_value = static_cast<BYTE *>(calloc(2 + val.length() + 1, sizeof(char)));
                        unit_value.value.string_value[0] = length[0];
                        unit_value.value.string_value[1] = length[1];
                        int pointer = 2;
                        for (char ch:val) {
                            unit_value.value.string_value[pointer] = ch;
                        }
                        break;
                    }
                }

                column_array[column_pointer].push_back(unit_value);
                wordCount += val.length();
            }

        }
    } else {
        cout << "no such file" << endl;
    }

    return wordCount;
}


vector<Column_Description> phrase_table(std::string table_description) {
    vector<string> columns;
    SplitString(table_description, columns, ",");
    vector<Column_Description> result;
    for (string column:columns) {
        Column_Description temp;
        int pos = column.find(':');

        temp.column_name = column.substr(0, pos);

        string column_type = column.substr(pos + 1);
        if (column_type == "long") {
            temp.type = TYPE_LONG;
        } else if (column_type == "timestamp") {
            temp.type = TYPE_TIMESTAMP;
        } else {
            temp.type = TYPE_STRING;
        }
        result.push_back(temp);
    }

    return result;
}

int main() {

    vector<Column_Description> column_description = phrase_table(TableDescription);

    int column_count = column_description.size();

    for (Column_Description column:column_description) {
        cout << "column name: " << column.column_name << "\tcolumn type: " << column.type << endl;
    }

    vector<Column_Unit> column_array[column_count];

    // read the file as text, fill the column array by the column units
    long readlength = readAsText(SourceFilePath, column_description, column_array);

    std::cout << "have read " << readlength << " characters from file" << std::endl;

    std::cout << "contains " << column_array[0].size() << " rows total" << std::endl;

    // now we get a vector, for each of the vector, contains a array of data

    // using alloc will cost too much memory resource, please remember use free each time finished using the block


    return 0;
}