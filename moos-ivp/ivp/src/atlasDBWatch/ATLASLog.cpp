#include <fstream>
#include <iostream>
#include "ATLASLog.h"

using namespace std;

ATLASLog::ATLASLog(const std::string &debugLogFile) {
        this->fileName = debugLogFile;
        debugLog.open(fileName);
}

ATLASLog::~ATLASLog() {
        debugLog.close();
}

ATLASLog debug {"/tmp/atlasWatchDB.log"};
