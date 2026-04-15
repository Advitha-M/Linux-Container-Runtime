#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>
#include <linux/timer.h>
#include "monitor_ioctl.h"

MODULE_LICENSE("GPL");

static int major;
static struct class* cls;
static struct timer_list monitor_timer;

struct monitored_container {
    pid_t pid;
    unsigned long hard_limit;
    struct list_head list;
};

static LIST_HEAD(container_list);

// This function checks the memory of all containers in our list
void check_memory(struct timer_list *t) {
    struct monitored_container *entry, *tmp;
    struct task_struct *task;
    unsigned long rss;

    list_for_each_entry_safe(entry, tmp, &container_list, list) {
        task = pid_task(find_get_pid(entry->pid), PIDTYPE_PID);
        if (task) {
            // Get Resident Set Size (RSS) in MiB
            rss = get_mm_rss(task->mm) << (PAGE_SHIFT - 10) / 1024;
            
            if (rss > entry->hard_limit) {
                printk(KERN_INFO "Monitor: KILLING PID %d. Usage: %lu MiB exceeds limit %lu MiB\n", 
                       entry->pid, rss, entry->hard_limit);
                send_sig(SIGKILL, task, 0);
            }
        }
    }
    // Reset timer to run again in 1 second
    mod_timer(&monitor_timer, jiffies + msecs_to_jiffies(1000));
}

static long ioctl_handler(struct file *f, unsigned int cmd, unsigned long arg) {
    struct container_config config;
    struct monitored_container *new_c;

    if (cmd == REGISTER_CONTAINER) {
        copy_from_user(&config, (struct container_config __user *)arg, sizeof(config));
        new_c = kmalloc(sizeof(*new_c), GFP_KERNEL);
        new_c->pid = config.host_pid;
        new_c->hard_limit = config.hard_limit_mib;
        list_add(&new_c->list, &container_list);
        printk(KERN_INFO "Monitor: Now tracking PID %d\n", config.host_pid);
    }
    return 0;
}

static struct file_operations fops = { .unlocked_ioctl = ioctl_handler };

static int __init m_init(void) {
    major = register_chrdev(0, "container_monitor", &fops);
    cls = class_create("container_class");
    device_create(cls, NULL, MKDEV(major, 0), NULL, "container_monitor");
    
    // Start the periodic timer
    timer_setup(&monitor_timer, check_memory, 0);
    mod_timer(&monitor_timer, jiffies + msecs_to_jiffies(1000));
    
    return 0;
}

static void __exit m_exit(void) {
    del_timer(&monitor_timer);
    // (Cleanup list logic here)
    device_destroy(cls, MKDEV(major, 0));
    class_unregister(cls);
    class_destroy(cls);
    unregister_chrdev(major, "container_monitor");
}

module_init(m_init);
module_exit(m_exit);
