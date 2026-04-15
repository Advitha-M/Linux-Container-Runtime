#ifndef MONITOR_IOCTL_H
#define MONITOR_IOCTL_H
#include <linux/ioctl.h>

struct container_config {
    pid_t host_pid;
    unsigned long soft_limit_mib;
    unsigned long hard_limit_mib;
};

#define IOC_MAGIC 'c'
#define REGISTER_CONTAINER _IOW(IOC_MAGIC, 1, struct container_config)
#define UNREGISTER_CONTAINER _IOW(IOC_MAGIC, 2, pid_t)
#endif
