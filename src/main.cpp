

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

void drawFrame(const string &title) {
    clear();
    box(stdscr, 0, 0);
    int mid = (COLS - title.size()) / 2;
    if (mid < 1) mid = 1;
    mvprintw(0, mid, "%s", title.c_str());
}


void drawMemoryBar(int row, int col, int width, long used_kb, long total_kb) {
    if (total_kb <= 0) return;
    double ratio = (double)used_kb / (double)total_kb;
    int fill = (int)(ratio * width);
    mvprintw(row, col, "[");
    for (int i = 0; i < width; ++i) {
        if (i < fill)
            mvprintw(row, col + 1 + i, "#");
        else
            mvprintw(row, col + 1 + i, "-");
    }
    mvprintw(row, col + 1 + width, "]");
    mvprintw(row, col + width + 4, "%.1f%%", ratio * 100);
}

void renderSystemStats() {
    MemStats mem = fetchMemoryInfo();
    double uptime = getUptime();
    long used = mem.totalMemKB - mem.availableMemKB;

    mvprintw(1, 2, "Uptime: %.2f hrs", uptime / 3600.0);
    mvprintw(2, 2, "Memory: %ld MB total | %ld MB used | %ld MB free",
             mem.totalMemKB / 1024,
             used / 1024,
             mem.availableMemKB / 1024);

    drawMemoryBar(3, 2, 40, used, mem.totalMemKB);
}


void renderProcessList(const vector<ProcDetails>& procs, int startRow, int highlight) {
    mvprintw(startRow, 2, "%-7s %-10s %-6s %-10s %-7s %s", 
             "PID", "USER", "STATE", "MEM(MB)", "MEM%", "COMMAND");

    mvhline(startRow + 1, 1, '-', COLS - 2);

    int row = startRow + 2;
    int maxRows = LINES - 6;

    for (size_t i = 0; i < procs.size() && row < maxRows; ++i, ++row) {
        if ((int)i == highlight) attron(A_REVERSE);
        double memMB = procs[i].memUsageKB / 1024.0;
        mvprintw(row, 2, "%-7d %-10s %-6c %-10.2f %-7.2f %s",
                 procs[i].pid,
                 procs[i].user.c_str(),
                 procs[i].state,
                 memMB,
                 procs[i].memPercent,
                 procs[i].command.c_str());
        if ((int)i == highlight) attroff(A_REVERSE);
    }
}


void renderFooter() {
    mvhline(LINES - 4, 1, '-', COLS - 2);
   mvprintw(LINES - 3, 0, "Sort:[C]PU | [M]em   Navigate: ARROW_UP / ARROW_DOWN   [K]ill   [Q]uit");

}


bool terminateProcess(int pid) {
    if (kill(pid, SIGTERM) == 0) {
        mvprintw(LINES - 2, 2, "Sent SIGTERM to PID %d", pid);
        refresh();
        return true;
    }
    if (errno == ESRCH) {
        mvprintw(LINES - 2, 2, "PID %d no longer exists.", pid);
        refresh();
        return false;
    }
    if (kill(pid, SIGKILL) == 0) {
        mvprintw(LINES - 2, 2, "SIGTERM failed; sent SIGKILL to PID %d", pid);
        refresh();
        return true;
    }
    mvprintw(LINES - 2, 2, "Failed to terminate PID %d: %s", pid, strerror(errno));
    refresh();
    return false;
}

int main() {
    setupTerminalUI();
    string title = "System Monitor Tool (C++)";
    int sortMode = 2;  
    int highlight = 0;
    bool running = true;

    while (running) {
        drawFrame(title);
        renderSystemStats();

      
        vector<int> pids = findAllPIDs();
        vector<ProcDetails> procs;
        MemStats mem = fetchMemoryInfo();
        for (int pid : pids) {
            ProcDetails p = fetchProcessDetails(pid);
            p.user = findProcessUser(pid);
            p.memPercent = (mem.totalMemKB > 0)
                ? (100.0f * p.memUsageKB / (float)mem.totalMemKB)
                : 0.0f;
            procs.push_back(p);
        }

        sortProcList(procs, sortMode);
        if (highlight >= (int)procs.size()) highlight = max(0, (int)procs.size() - 1);

        renderProcessList(procs, 5, highlight);
        renderFooter();

        refresh();
        int ch = getch();

        switch (ch) {
            case 'c': case 'C': sortMode = 1; break;
            case 'm': case 'M': sortMode = 2; break;
            case KEY_UP: if (highlight > 0) --highlight; break;
            case KEY_DOWN: if (highlight < (int)procs.size() - 1) ++highlight; break;
            case 'k': case 'K':
                if (!procs.empty() && highlight >= 0 && highlight < (int)procs.size()) {
                    terminateProcess(procs[highlight].pid);
                    this_thread::sleep_for(chrono::milliseconds(700));
                }
                break;
            case 'q': case 'Q':
                running = false;
                break;
        }
    }

    endwin();
    return 0;
}
