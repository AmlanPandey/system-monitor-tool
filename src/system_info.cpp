#include "system_info.h"

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
    ProcessorStats stats{};
    stats.user = stats.nice = stats.system = stats.idle = 0;
    stats.total = stats.active = 0;

    ifstream f("/proc/stat");
    if (!f.is_open()) return stats;

    string line;
    if (getline(f, line)) {
        istringstream ss(line);
        string label;
        ss >> label; 
        if (label == "cpu") {
            ss >> stats.user >> stats.nice >> stats.system >> stats.idle;
            stats.active = stats.user + stats.nice + stats.system;
            stats.total = stats.active + stats.idle;
        }
    }
    return stats;
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
    DIR* d = opendir("/proc");
    if (!d) return pids;

    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        if (entry->d_type == DT_DIR) continue; 
       
    }
    closedir(d);

   
    d = opendir("/proc");
    if (!d) return pids;
    while ((entry = readdir(d)) != nullptr) {
        string name = entry->d_name;
        bool allDigits = !name.empty() && (find_if(name.begin(), name.end(),
                      [](char c){ return !isdigit(c); }) == name.end());
        if (allDigits) {
            int pid = atoi(name.c_str());
            if (pid > 0) pids.push_back(pid);
        }
    }
    closedir(d);
  
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
    string statusPath = "/proc/" + to_string(pid) + "/status";
    ifstream f(statusPath);
    if (!f.is_open()) return "unknown";

    string line;
    while (getline(f, line)) {
        istringstream ss(line);
        string key;
        ss >> key;
        if (key == "Uid:") {
            int uid = -1;
            ss >> uid;
            if (uid >= 0) {
                struct passwd* pw = getpwuid(uid);
                if (pw) return string(pw->pw_name);
            }
            break;
        }
    }
    return "unknown";
}

void sortProcList(vector<ProcDetails>& list, int mode) {
    if (mode == 0) {
        sort(list.begin(), list.end(), [](const ProcDetails& a, const ProcDetails& b){
            return a.pid < b.pid;
        });
    } else if (mode == 1) {
        sort(list.begin(), list.end(), [](const ProcDetails& a, const ProcDetails& b){
            return a.cpuPercent > b.cpuPercent;
        });
    } else { 
        sort(list.begin(), list.end(), [](const ProcDetails& a, const ProcDetails& b){
            return a.memUsageKB > b.memUsageKB;
        });
    }
}

