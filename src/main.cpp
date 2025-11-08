#include "system_info.h"
#include <ncurses.h>
#include <unistd.h>
#include <csignal>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <cerrno>
#include <cstring>

using namespace std;

void setupTerminalUI() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(1000);
}


void renderSystemStats() {
    MemStats mem = fetchMemoryInfo();
    double uptime = getUptime();

    mvprintw(0, 0, "Simple System Monitor (student-friendly)");
    mvprintw(1, 0, "Uptime: %.2f hrs", uptime / 3600.0);
    mvprintw(2, 0, "Memory: %ld MB total  |  %ld MB available  |  %ld MB used",
             mem.totalMemKB / 1024,
             mem.availableMemKB / 1024,
             (mem.totalMemKB - mem.availableMemKB) / 1024);
}


void renderProcessList(const vector<ProcDetails>& procs, int startRow, int highlightedRow) {
    mvprintw(startRow, 0, "%-7s %-10s %-6s %-10s %-7s %s", "PID", "USER", "STATE", "MEM(MB)", "MEM%", "COMMAND");
    int row = startRow + 1;
    int maxRows = LINES - 4; 

    for (size_t i = 0; i < procs.size(); ++i) {
        if (row >= maxRows) break;
        if ((int)i == highlightedRow) attron(A_REVERSE);

        double memMB = procs[i].memUsageKB / 1024.0;
        mvprintw(row, 0, "%-7d %-10s %-6c %-10.2f %-7.2f %s",
                 procs[i].pid,
                 procs[i].user.c_str(),
                 procs[i].state,
                 memMB,
                 procs[i].memPercent,
                 procs[i].command.c_str());
        if ((int)i == highlightedRow) attroff(A_REVERSE);
        ++row;
    }
}

bool terminateProcess(int pid) {
  
    if (kill(pid, SIGTERM) == 0) {
        mvprintw(LINES - 1, 0, "Sent SIGTERM to PID %d", pid);
        refresh();
        return true;
    }

   
    if (errno == ESRCH) {
        mvprintw(LINES - 1, 0, "PID %d does not exist (already exited).", pid);
        refresh();
        return false;
    }


    if (kill(pid, SIGKILL) == 0) {
        mvprintw(LINES - 1, 0, "SIGTERM failed; sent SIGKILL to PID %d", pid);
        refresh();
        return true;
    }

    mvprintw(LINES - 1, 0, "Failed to terminate PID %d: %s", pid, strerror(errno));
    refresh();
    return false;
}

int main() {
    setupTerminalUI();

    int sortMode = 2; 
    int highlighted = 0;
    bool running = true;

    while (running) {
        clear();
        renderSystemStats();

        
        vector<int> pids = findAllPIDs();
        vector<ProcDetails> procs;
        MemStats mem = fetchMemoryInfo();

        for (int pid : pids) {
            ProcDetails p = fetchProcessDetails(pid);
            p.user = findProcessUser(pid);
           
            p.memPercent = (mem.totalMemKB > 0) ? (100.0f * p.memUsageKB / (float)mem.totalMemKB) : 0.0f;
            procs.push_back(p);
        }

        sortProcList(procs, sortMode);

     
        if (highlighted >= (int)procs.size()) highlighted = max(0, (int)procs.size() - 1);

        renderProcessList(procs, 4, highlighted);

      
      mvprintw(LINES - 3, 0, "Sort: [P]ID | [C]PU | [M]em   Navigate: UP/DOWN   [K]ill   [Q]uit");
        //mvprintw(LINES - 2, 0, "Tip: CPU sort is available but shows 0.0 unless extended to capture jiffies over time.");

        refresh();

        int ch = getch(); 
        if (ch == ERR) {
      
            continue;
        }

        switch (ch) {
            case 'p': case 'P':
                sortMode = 0; break;
            case 'c': case 'C':
                sortMode = 1; break;
            case 'm': case 'M':
                sortMode = 2; break;
            case KEY_UP:
                if (highlighted > 0) --highlighted;
                break;
            case KEY_DOWN:
                if (highlighted < (int)procs.size() - 1) ++highlighted;
                break;
            case 'k': case 'K':
                if (!procs.empty() && highlighted >= 0 && highlighted < (int)procs.size()) {
                    int pidToKill = procs[highlighted].pid;
                    terminateProcess(pidToKill);
                   
                    this_thread::sleep_for(chrono::milliseconds(700));
                }
                break;
            case 'q': case 'Q':
                running = false; break;
            default:
               
                break;
        }
    }

    endwin();
    return 0;
}

