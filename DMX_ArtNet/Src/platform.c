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

    return (void *)1;
}

void memcpy_pbuf(struct pbuf *p, const void *source, unsigned int num) {
    unsigned int offset = 0;
    struct pbuf *q;

    for (q = p; q != NULL; q = q->next) {
        memcpy(q->payload, source + offset, q->len);
        offset += q->len;

        if (q->len == q->tot_len) {
            break;
        }
    }
}

char pbufcpy_mem(void *target, const struct pbuf *p, unsigned int maxlen) {
    unsigned int offset = 0;
    const struct pbuf *q;

    for (q = p; q != NULL; q = q->next) {
        if (maxlen < q->len) {
            return 0;
        }

        memcpy(target + offset, q->payload, q->len);
        offset += q->len;
        maxlen -= q->len;

        if (q->len == q->tot_len) {
            break;
        }
    }

    return -1;
}

void memclr(void *target, unsigned int len) {
    if ((uintptr_t)target % sizeof(long) == 0 && len % sizeof(long) == 0) {
        long *ldst = (long *)target;

        for (int i = 0; i < len / sizeof(long); i++) {
            ldst[i] = 0;
        }
    } else {
        char *cdst = (char *)target;

        for (int i = 0; i < len; i++) {
            cdst[i] = 0;
        }
    }
}

int memcmp(const void *a, const void *b, unsigned int num) {
    if ((uintptr_t)a % sizeof(long) == 0 && (uintptr_t)b % sizeof(long) == 0 && num % sizeof(long) == 0) {
        long *la = (long *)a;
        long *lb = (long *)b;

        for (int i = 0; i < (num / sizeof(long)); i++) {
            if (la[i] != lb[i]) {
                return 0;
            }
        }
    } else {
        char *ca = (char *)a;
        char *cb = (char *)b;

        for (int i = 0; i < num; i++) {
            if (ca[i] != cb[i]) {
                return 0;
            }
        }
    }

    return -1;
}
