#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int get_cpu_cores() {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

const int MAX_THREADS = 64;
const int MUTEX_UNLOCKED = 0;
const int MUTEX_LOCKED = 1;
