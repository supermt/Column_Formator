//
// Created by superMT on 2017/11/26.
//

#include "untils.h"


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

void intToHeadCounter(unsigned int num, BYTE *result) {

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

long
readAsText(const char *file_path, std::vector<Column_Description> column_desc, std::vector<Column_Unit> *column_array) {
    std::ifstream in(file_path);
    std::string filename;
    std::string line;
    int wordCount = 0;
    unsigned long row_counter = 0;
    if (in) {
        while (getline(in, line)) {
            // read a single line, split it into a vector of string by '\t'
            std::vector<std::string> columns;
            SplitString(line, columns, "\t");
            int column_pointer = 0;
            row_counter++;
            // process every single row
            for (column_pointer = 0; column_pointer < columns.size(); column_pointer++) {
                std::string val = columns[column_pointer];
                Column_Unit unit_value;
                unit_value.row = row_counter;

                // judge the column type
                switch (column_desc[column_pointer].type) {
                    case TYPE_LONG: {
                        if (!val.size()) {
                            // condition for an empty long value
                            unit_value.value.empty = true;
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
                        if (!val.size()) {
                            unit_value.value.empty = true;
                        } else {
                            unit_value.value.string_value = val;
                        }
                        break;
                    }
                }
                column_array[column_pointer].push_back(unit_value);
                wordCount += val.length();
            }
        }
    } else {
        std::cout << "no such file" << std::endl;
    }

    return
            wordCount;
}

long addLongAsBytes(unsigned char *array, long offset, long target_value) {
    for (int ix = 0; ix < 8; ++ix) {
        int ofs = 64 - (ix + 1) * 8;
        array[ix + offset] = ((target_value >> ofs) & 0xff);
    }
    return offset + 8;
}


std::vector<Column_Description> phrase_table(std::string table_description) {
    std::vector<std::string> columns;
    SplitString(table_description, columns, ",");
    std::vector<Column_Description> result;
    for (std::string column:columns) {
        Column_Description temp;
        int pos = column.find(':');

        temp.column_name = column.substr(0, pos);

        std::string column_type = column.substr(pos + 1);
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

long addOneByte(BYTE array[], long offset, unsigned char target_value) {
    array[offset] = target_value;
    return offset + 1;
}

long mergeByteArrays(BYTE data[], long data_length, BYTE dest[], long start_position) {
    for (int i = 0; i < data_length; i++) {
        dest[start_position + i] = data[i];
    }

    return data_length + start_position;
}
