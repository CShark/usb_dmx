#include "platform.h"

void *memcpy(void *destination, const void *source, unsigned int num) {
    if ((uintptr_t)destination % sizeof(long) == 0 &&
        (uintptr_t)source % sizeof(long) == 0 &&
        num % sizeof(long) == 0) {

        long *lsrc = (long *)source;
        long *ldst = (long *)destination;

        for (int i = 0; i < num / sizeof(long); i++) {
            ldst[i] = lsrc[i];
        }
    } else {
        char *csrc = (char *)source;
        char *cdst = (char *)destination;

        for (int i = 0; i < num; i++) {
            cdst[i] = csrc[i];
        }
    }
}

void memcpy_pbuf(struct pbuf *p, const void *source, unsigned int num) {
    unsigned int offset = 0;
    struct pbuf *q;

    for (q = p; q != NULL; q = q->next) {
        memcpy(q->payload, source + offset, q->len);
        offset += q->len;

        if(q->len == q->tot_len) {
            break;
        }
    }
}