//
// Created by 郑向阳 on 2018/4/25.
//

#ifndef FJT_NOTFOUNDURLS_H
#define FJT_NOTFOUNDURLS_H
#include "httpd.h"
#include "uthash.h"

#define MAX_CACHE_SIZE 60000


struct NotFoundUrl {
    char *url;
    UT_hash_handle hh;
};



int init_not_found_urls_cache(apr_pool_t *pool);

int isUrlNotFound(char *url);

void addUrlNotFound(char *url);

#endif //FJT_NOTFOUNDURLS_H
