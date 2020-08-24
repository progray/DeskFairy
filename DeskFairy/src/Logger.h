#pragma once

#include <string>
#include <fstream>

class Logger
{
public:
    static std::string GetTimeString();

    static void Print(const std::string format, ...);

    static void MakeDir(std::string path);
};

