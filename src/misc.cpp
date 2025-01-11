#include <misc.h>

char* read_file(const char* path)
{
    FILE* file = fopen(path, "rb");

    if (!file)
        return nullptr;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* data = (char*)malloc(size + 1);
    fread(data, 1, size, file);
    data[size] = 0;

    fclose(file);

    return data;
}