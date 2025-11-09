# System Monitor Tool

##  Overview
The **System Monitor Tool** is a simple Linux-based project written in **C++** that displays real-time system information such as CPU usage, memory usage, and active processes.  


This project was created to understand **process handling**, **system calls**, and **terminal UI programming** in C++.

---

##  Build & Run

### 🛠️ Build the project
To build the project, simply open your terminal in the project folder and type:
```bash
make
```
### ▶️ Run the program
To run the System Monitor Tool, use the command:
```bash
./system_monitor_tool
```

### ⚙️ Features 

- **Displays system information:**  
  Shows real-time **uptime**, **total memory**, **available memory**, and **used memory**.

- **Process list:**  
  Displays all active processes along with:
  - **PID** — Process ID  
  - **USER** — Process owner  
  - **STATE** — Running, Sleeping, etc.  
  - **MEM(MB)** — Memory used by process  
  - **MEM%** — Memory percentage

- **Sorting options:**  
  - Press **`c`** → Sort processes by **CPU usage** (available mode)  
  - Press **`m`** → Sort processes by **Memory usage** (descending order)

- **Navigation:**  
  - Use **`ARROW_UP`** and **`ARROW_DOWN`** keys to move the highlight between processes

- **Terminate a process:**  
  - Press **`k`** → Kill the selected process (sends `SIGTERM`, and if it fails, `SIGKILL`)

- **Quit the program:**  
  - Press **`q`** → Exit the System Monitor Tool

- **Auto-refresh:**  
  The display automatically refreshes every second to show the latest process and system data.

### Screenshots

<img width="1715" height="835" alt="pic" src="https://github.com/user-attachments/assets/526ce9ca-bb2f-4eae-91b6-f6622efd85ea" />




