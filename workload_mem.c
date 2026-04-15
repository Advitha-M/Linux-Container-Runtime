#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    size_t size = 1024 * 1024 * 100; // 100MB
    printf("🧠 Memory Workload: Allocating 100MB...\n");
    char *ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
        printf("✅ Allocation successful. Sleeping...\n");
        sleep(10);
    } else {
        perror("Allocation failed");
    }
    return 0;
}
