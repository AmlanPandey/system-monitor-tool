#ifndef SYSINFO_H
#define SYSINFO_H

#include <string>
#include <vector>

struct ProcessorStats {
    unsigned long long user, nice, system, idle;
    unsigned long long total, active;
};

struct MemStats {
    long totalMemKB;
    long freeMemKB;
    long availableMemKB;
};


struct ProcDetails {
    int pid;
    std::string user;
    std::string command;
    float cpuPercent;
    float memPercent;
    unsigned long memUsageKB; 
    char state;
};


ProcessorStats fetchCPUStats();
MemStats fetchMemoryInfo();
double getUptime();
std::vector<int> findAllPIDs();
ProcDetails fetchProcessDetails(int pid);
std::string findProcessUser(int pid);
void sortProcList(std::vector<ProcDetails>&, int);

#endif
