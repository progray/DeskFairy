#include "Logger.h"
#include <cstdio>
#include <stdarg.h>
#include <iostream>
#include <string>
#include <fstream>
#include <QTime>
#include <QDate>
#include <QDebug>
#include <QDir>
#include "Defines.h"

namespace
{
    std::ofstream out;
    std::string logFilePath;
}

std::string Logger::GetTimeString()
{
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    std::string s;
    s = std::to_string(date.year()) + "_" +
        std::to_string(date.month()) + "_" +
        std::to_string(date.day()) + "_" +
        std::to_string(time.hour()) + "_" +
        std::to_string(time.minute()) + "_" +
        std::to_string(time.msec());
    return s;
}

void Logger::Print(const std::string format, ...)
{
#ifdef _DEBUG
    va_list args;
    char buf[256];

    va_start(args, format);
    vsnprintf_s(buf, sizeof(buf), format.c_str(), args);

    if (Def::debugLogMode == DEBUG_TO_STANDARD)
    {
        std::cerr << buf << std::endl;
    }
    else if (Def::debugLogMode == DEBUG_TO_FILE)
    {
        MakeDir(Def::debugLogFilePath);
        if (logFilePath.empty())
        {
            logFilePath = std::string(Def::debugLogFilePath) + "/log_" + GetTimeString() + ".txt";
        }
        out.open(logFilePath, std::ios::app);
        if (out)
        {
            out << buf << std::endl;
            out.close();
        }
    }
    else if(Def::debugLogMode == DEBUG_TO_QT)
    {
        qDebug() << buf << endl;
    }

    va_end(args);
#endif // _DEBUG
}

void Logger::MakeDir(std::string path)
{
    QDir dir;
    if (!dir.exists(path.c_str()))
        dir.mkdir(path.c_str());
}
