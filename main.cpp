#include "untils.h"

int main() {

    using namespace std;

    vector<Column_Description> column_description = phrase_table(TableDescription);

    int column_count = column_description.size();

    vector<int> long_column;

    vector<int> string_column;

    int index = 0;
    for (Column_Description column:column_description) {
        cout << index << "\tcolumn name: " << column.column_name << "\tcolumn type: " << column.type << endl;
        if (column.type == TYPE_STRING) {
            string_column.push_back(index);
        } else {
            long_column.push_back(index);
        }
        index++;
    }

    vector<Column_Unit> column_array[column_count];

    // read the file as text, fill the column array by the column units
    long readlength = readAsText(SourceFilePath, column_description, column_array);

    std::cout << "have read " << readlength << " characters from file" << std::endl;

    int row_total = column_array[0].size();

    std::cout << "contains " << row_total << " rows total" << std::endl;

    // now we get a vector, for each of the vector, contains a array of data

    // using alloc will cost too much memory resource, please remember use free each time finished using the block

    // the max block size is 4MB (according to recent testing data and discussion)

    // process the long type first
    for (int long_index:long_column) {
        vector<Long_Column> tempColumn;
        long group_pointer = 0;
        cout << column_array[long_index].size() << endl;
        for (int row = 0; row < row_total; row++) {
            if (row >= column_array[long_index].size()) break; // if this column is totally empty, break out
            // for each cell
            // if the empty signal has been set to empty
            if (!column_array[long_index][row].value.empty) {
                Long_Column cell;
                cell.value = column_array[long_index][row].value.numeric_value;
                cell.row = column_array[long_index][row].row;
                tempColumn.push_back(cell);
            }
        }

        char file_name[20];
        sprintf(file_name, "column_%d", long_index);
        string target_file = TargetDirPrefix;
        target_file.append(file_name);
        FILE *target_fd = fopen(target_file.c_str(), "wb");
        cout << target_file << endl;
        // load each unit in tempColumn
        // for each 256 row, combine them into a group
        // record the base offset(group set) as the index of this group and how many rows the group contains

        for (long j = 0; j < tempColumn.size(); j += MAX_OFFSET) {
            int i = 0;
            long group_set = 0 + j;
            BYTE byte_row[9 * MAX_OFFSET];
            memset(byte_row, 0, 9 * MAX_OFFSET);
            int offset = 0;
            vector<long> value_area;
            value_area.push_back(99999);
            while (true) {
                Long_Column cell = tempColumn[i + group_set];
                if (value_area[0] != cell.value) value_area.push_back(cell.value);
                offset = cell.row - group_set;
                // for each row in the group
                // + ------- + --------------------- +
                // | 1 byte  | 8 byte  ..............|
                // + ------- + --------------------- +
                addOneByte(byte_row, 0 + i * 9, (offset - 1));
                addLongAsBytes(byte_row, 1 + i * 9, cell.value);
                i++;
                if (offset >= MAX_OFFSET) break;
                if (group_set + i >= tempColumn.size()) break;
            }
            // offset is bigger than MAX_OFFSET , skip to the next group


            BYTE group[10000];

            // for the group head
            // + ------------------- + ----------------------------------- +
            // | total rows (1byte)  | start index (8 byte)  ..............|
            // + ------------------- + ----------------------------------- +
            int byte_pointer = 0;
            // total rows usually equals to i
            byte_pointer = addOneByte(group, byte_pointer,
                                      (unsigned char) i); // when i equals 256 , it will be transformed to 0
            byte_pointer = addLongAsBytes(group, byte_pointer, group_set);
            byte_pointer = mergeByteArrays(byte_row, i * 9, group, byte_pointer);
            fwrite(group, sizeof(BYTE), byte_pointer, target_fd);
            group_pointer += byte_pointer;
        }
        fclose(target_fd);
    }

    // process the string columns
    for (int string_index:string_column) {
        vector<String_Column> tempColumn;
        long group_pointer = 0;
        for (int row = 0; row < row_total; row++) {
            if (row >= column_array[string_index].size()) break; // if this column is totally empty, break out
            // for each cell
            // if the empty signal has been set to empty
            if (!column_array[string_index][row].value.empty) {
                String_Column cell;
                cell.value = column_array[string_index][row].value.string_value;
                cell.row = column_array[string_index][row].row;
                tempColumn.push_back(cell);
            }
        }

        char file_name[20];
        sprintf(file_name, "column_%d", string_index);
        string target_file = TargetDirPrefix;
        target_file.append(file_name);
        FILE *target_fd = fopen(target_file.c_str(), "wb");
        cout << target_file << endl;
        // load each unit in tempColumn
        // for each 256 row, combine them into a group
        // record the base offset(group set) as the index of this group and how many rows the group contains

        for (long j = 0; j < tempColumn.size(); j += MAX_OFFSET) {
            int i = 0;
            long group_set = 0 + j;
            vector<BYTE> byte_row;

            while (true) {
                String_Column cell = tempColumn[i + group_set];
                int offset = cell.row - group_set;
                // for each string in the group
                // + ------- + ---------------------- + ------------------------------------- +
                // | 1 byte  | 2 byte (string length) | n byte (record in string length)  ....|
                // + ------- + ---------------------- + ------------------------------------- +

                byte_row.push_back((unsigned char) (offset - 1));
                BYTE length[2];
                intToHeadCounter(cell.value.length(), length);
                byte_row.push_back(length[0]);
                byte_row.push_back(length[1]);

                for (unsigned char ch : cell.value) {
                    byte_row.push_back(ch);
                }

                i++;
                if (offset >= MAX_OFFSET) break;
                if (group_set + i >= tempColumn.size()) break;
            }
            // offset is bigger than MAX_OFFSET , skip to the next group

            int byte_count = byte_row.size() + 1 + 8;

            BYTE *group;
            group = static_cast<BYTE *>(calloc(sizeof(BYTE), byte_count));

            // for the group head
            // + ------------------- + ----------------------------------- +
            // | total rows (1byte)  | start index (8 byte)  ..............|
            // + ------------------- + ----------------------------------- +
            int byte_pointer = 0;
            // total rows usually equals to i
            byte_pointer = addOneByte(group, byte_pointer, (unsigned char) i);
            byte_pointer = addLongAsBytes(group, byte_pointer, group_set);

            int data_length = byte_row.size();

            for (int i = 0; i < data_length; i++) {
                group[byte_pointer + i] = byte_row[i];
            }

            byte_pointer += byte_row.size();

            fwrite(group, sizeof(BYTE), byte_pointer, target_fd);
            free(group);
            group_pointer += byte_pointer;
        }
        fclose(target_fd);
    }

    cout << "data transformed, view" << TargetDirPrefix << endl;

    return 0;
}