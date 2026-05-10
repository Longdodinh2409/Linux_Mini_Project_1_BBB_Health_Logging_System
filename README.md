# [Linux] Embedded System Health Monitor for BeagleBone Black (AM335x)
A robust, high-performance data logging system designed for BeagleBone Black (AM335x) to monitor real-time system metrics. This project demonstrates mastery of Inter-Process Communication (IPC), Multithreading, and System Programming on Embedded Linux.

### 🚀 Project Overview
The system follows a hybrid Producer-Consumer architecture to ensure low-latency data handling and reliable persistence:
+ **Producer Process:** A multi-threaded engine that polls system data from `/proc` and `/sys` file systems.
+ **Consumer Process:** A dedicated logger that processes, timestamps, and persists data to local storage.

### 🛠 Tech Stack & Architecture
+ **Language:** C 
+ **IPC**: POSIX Shared Memory (`shm_open`, `mmap`) for high-speed data exchange.
+ **Synchronization**: * Named Semaphores to coordinate data flow between independent processes.
  + **Process-Shared Mutex** (`PTHREAD_PROCESS_SHARED`) for thread-safe access to the circular buffer.
+ **Concurrency**: pthread library for independent sensor polling (RAM availability & Network Link State).
+ **Platform**: BeagleBone Black (Debian/Linux).

### 🧠 Key Challenges & Solutions
This project showcases deep-level troubleshooting in system programming:
+ **Deadlock Prevention:** Implemented a strategic "Wake-up Call" in Signal Handlers using multiple sem_post to ensure all blocked threads terminate safely during shutdown.
+ **IPC Resource Management:** Developed a rigorous Graceful Stop mechanism to ensure shm_unlink and sem_unlink are called, preventing OS resource leaks.
+ **Real-time Parsing:** Efficiently extracting data from Linux virtual file systems (/proc/meminfo and sysfs).

### 📁 File Structure
+ `shared_def.h`: The "Contract" – defining the shared memory layout and synchronization primitives.
+ `producer.c`: Multi-threaded data collection engine.
+ `consumer.c`: Logging and data processing engine.
+ `Makefile`: Automated build system with -lrt -lpthread.
