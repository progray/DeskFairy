#include "FileLoader.h"
#include <fstream>
#include "Logger.h"

using namespace std;

Bytes* FileLoader::Load(const std::string filePath, unsigned int* outSize)
{
    const char* path = filePath.c_str();

    int size = 0;
    struct stat statBuf;
    if (stat(path, &statBuf) == 0)
    {
        size = statBuf.st_size;
    }

    std::fstream file;
    char* buf = new char[size];

    file.open(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        Logger::Print("Unable to open file: %s", filePath.c_str());
        return NULL;
    }
    file.read(buf, size);
    file.close();

    *outSize = size;
    return reinterpret_cast<Bytes*>(buf);
}

void FileLoader::Release(Bytes* byteData)
{
    delete[] byteData;
}