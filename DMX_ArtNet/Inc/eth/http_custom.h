#ifndef __HTTP_CUSTOM_H_
#define __HTTP_CUSTOM_H_

#include "lwip/apps/fs.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "systimer.h"

typedef struct {
    void *connection;
    const char *redirectTo;
    void (*postHander)(struct pbuf *);
} HttpPostData;

void httpc_init(struct netif *netif);
void httpc_timeout();

int fs_open_custom(struct fs_file *file, const char *name);
void fs_close_custom(struct fs_file *file);
int fs_read_custom(struct fs_file *file, char *buffer, int count);

err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd);
err_t httpd_post_receive_data(void *connection, struct pbuf *p);
void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len);

#endif
