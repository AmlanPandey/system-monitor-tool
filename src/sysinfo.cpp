#include "sysinfo.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <errno.h>

using namespace std;

ProcessorStats fetchCPUStats() {
    ProcessorStats cpu{};
    ifstream file("/proc/stat");
    if (!file.is_open()) return cpu;

    string label;
    file >> label; 
    if (label != "cpu") return cpu;

    file >> cpu.user >> cpu.nice >> cpu.system >> cpu.idle;

   
    cpu.active = cpu.user + cpu.nice + cpu.system;
    cpu.total = cpu.active + cpu.idle;

    return cpu;
}


MemStats fetchMemoryInfo() {
    MemStats m{};
    m.totalMemKB = m.freeMemKB = m.availableMemKB = 0;

    ifstream f("/proc/meminfo");
    if (!f.is_open()) return m;

    string line;
    while (getline(f, line)) {
        istringstream ss(line);
        string key;
        long value = 0;
        ss >> key >> value;
        if (key == "MemTotal:") m.totalMemKB = value;
        else if (key == "MemFree:") m.freeMemKB = value;
        else if (key == "MemAvailable:") {
            m.availableMemKB = value;
      
            break;
        }
    }
    return m;
}

double getUptime() {
    double up = 0.0;
    ifstream f("/proc/uptime");
    if (f.is_open()) f >> up;
    return up;
}

vector<int> findAllPIDs() {
    vector<int> pids;
    DIR* dir = opendir("/proc");
    if (!dir) return pids;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;

        
        if (!name.empty() && all_of(name.begin(), name.end(), ::isdigit)) {
            pids.push_back(stoi(name));
        }
    }

    closedir(dir);
    sort(pids.begin(), pids.end());
    return pids;
}


ProcDetails fetchProcessDetails(int pid) {
    ProcDetails proc;
    proc.pid = pid;
    proc.user = "unknown";
    proc.command = "";
    proc.cpuPercent = 0.0f;
    proc.memPercent = 0.0f;
    proc.memUsageKB = 0;
    proc.state = '?';

    string statPath = "/proc/" + to_string(pid) + "/stat";
    ifstream statFile(statPath);
    if (statFile.is_open()) {
        string statLine;
        if (getline(statFile, statLine)) {
            size_t lp = statLine.find('(');
            size_t rp = statLine.rfind(')');
            if (lp != string::npos && rp != string::npos && rp > lp) {
                proc.command = statLine.substr(lp + 1, rp - lp - 1); 
                string after = statLine.substr(rp + 2); 
                istringstream ss(after);
                vector<string> tok;
                string t;
                while (ss >> t) tok.push_back(t);
                if (!tok.empty()) proc.state = tok[0][0];
                if (tok.size() > 21) {
                    long rssPages = 0;
                    try {
                        rssPages = stol(tok[21]);
                    } catch (...) { rssPages = 0; }
                    long pageSizeBytes = sysconf(_SC_PAGESIZE);
                    proc.memUsageKB = (rssPages * pageSizeBytes) / 1024;
                }
            }
        }
    }
    string cmdPath = "/proc/" + to_string(pid) + "/cmdline";
    ifstream cmdFile(cmdPath);
    if (cmdFile.is_open()) {
        string cmdLine;
        getline(cmdFile, cmdLine, '\0'); 
        if (!cmdLine.empty()) {
          
            cmdFile.clear();
            cmdFile.seekg(0, ios::beg);
            string all;
            string piece;
            while (getline(cmdFile, piece, '\0')) {
                if (!all.empty()) all += ' ';
                all += piece;
            }
            if (!all.empty()) proc.command = all;
        }
    }

    return proc;
}


string findProcessUser(int pid) {
    char statusPath[256];
    snprintf(statusPath, sizeof(statusPath), "/proc/%d/status", pid);

    FILE* file = fopen(statusPath, "r");
    if (file) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            int uid;
            if (sscanf(line, "Uid:\t%d", &uid) == 1) {
                fclose(file);
                struct passwd* pw = getpwuid(uid);
                return pw ? pw->pw_name : "unknown";
            }
        }
        fclose(file);
    }
    return "unknown";

}

void sortProcList(std::vector<ProcDetails>& list, int mode) {
    if (mode == 1) { 
        std::sort(list.begin(), list.end(), [](const ProcDetails& a, const ProcDetails& b){
            return a.cpuPercent > b.cpuPercent;
        });
    } else { 
        std::sort(list.begin(), list.end(), [](const ProcDetails& a, const ProcDetails& b){
            return a.memUsageKB > b.memUsageKB;
        });
    }
}
