#pragma once
#include <fstream>

using namespace std;

class ATLASLog {
private:
        ofstream debugLog;
        string fileName;

public:
        ATLASLog(const string &debugLogFile);
        virtual ~ATLASLog();

        template <typename T>
                friend ATLASLog& operator <<(ATLASLog& thisLog, T const& value) {
                std::cout << value;
                thisLog.debugLog << value;
                std::cout << endl;
                thisLog.debugLog << endl;
                return thisLog;
        }
};

extern ATLASLog debug;
