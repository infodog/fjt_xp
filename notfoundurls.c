//
// Created by 郑向阳 on 2018/4/25.
//

#include "notfoundurls.h"

struct NotFoundUrl *urls_cache = NULL;
apr_thread_mutex_t *hash_mutex;

int init_not_found_urls_cache(apr_pool_t *pool){
    apr_status_t status = apr_thread_mutex_create(&hash_mutex,APR_THREAD_MUTEX_DEFAULT,pool);
    if(status != APR_SUCCESS){
        return status;
    }
    return APR_SUCCESS;
}

int isUrlNotFound(char *url)
{
    struct NotFoundUrl *entry;
    apr_thread_mutex_lock(hash_mutex);
    HASH_FIND_STR(urls_cache, url, entry);
    if (entry) {
        // remove it (so the subsequent add will throw it on the front of the list)
        HASH_DEL(urls_cache, entry);
        HASH_ADD_KEYPTR(hh, urls_cache, entry->url, strlen(entry->url), entry);
        apr_thread_mutex_unlock(hash_mutex);
        return 1;
    }
    apr_thread_mutex_unlock(hash_mutex);
    return 0;
}

void addUrlNotFound(char *url){
    struct NotFoundUrl *entry, *tmp_entry;
    apr_thread_mutex_lock(hash_mutex);
    HASH_FIND_STR(urls_cache, url, entry);
    if(entry){
        //do nothing
        apr_thread_mutex_unlock(hash_mutex);
        return;
    }
    entry = malloc(sizeof(struct NotFoundUrl));
    entry->url = strdup(url);
    HASH_ADD_KEYPTR(hh, urls_cache, entry->url, strlen(entry->url), entry);
    // prune the cache to MAX_CACHE_SIZE
    if (HASH_COUNT(urls_cache) >= MAX_CACHE_SIZE) {
        HASH_ITER(hh, urls_cache, entry, tmp_entry) {
            // prune the first entry (loop is based on insertion order so this deletes the oldest item)
            HASH_DELETE(hh, urls_cache, entry);
            free(entry->url);
            free(entry);
            break;
        }
    }
    apr_thread_mutex_unlock(hash_mutex);
}


