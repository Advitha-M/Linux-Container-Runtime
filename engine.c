#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <unistd.h>
#include <string.h>

#define STACK_SIZE (1024 * 1024)
static char container_stack[STACK_SIZE];

// THE PATH IS NOW HARD-CODED TO BE 100% SURE
const char* FIXED_ROOTFS = "/home/advitha/Desktop/container_project/rootfs-beta";

int container_main(void* arg) {
    printf("📦 Container: Using path: %s\n", FIXED_ROOTFS);

    sethostname("container-alpha", 15);

    // 1. Force the chroot to the specific folder
    if (chroot(FIXED_ROOTFS) != 0) {
        perror("❌ chroot failed");
        return 1;
    }
    chdir("/");

    // 2. Mount /proc
    mount("proc", "/proc", "proc", 0, NULL);

    // 3. Try to run the shell
    char *child_argv[] = {"/bin/sh", NULL};
    printf("🚀 Starting /bin/sh...\n");
    
    if (execv("/bin/sh", child_argv) == -1) {
        perror("❌ execv failed");
        umount2("/proc", MNT_DETACH);
        return 1;
    }
    return 0;
}

int main() {
    printf("🚀 Supervisor: Starting with Hard-Coded Path...\n");

    long pid = clone(container_main, container_stack + STACK_SIZE, 
                    CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS | SIGCHLD, NULL);

    if (pid == -1) {
        perror("clone failed");
        return 1;
    }

    waitpid(pid, NULL, 0);
    printf("🏁 Supervisor: Finished.\n");
    return 0;
}
