#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* key;
    int valid;
    int referenced;
} CacheItem;

typedef struct {
    int size;
    char policy;
    int capacityMisses;
    int compulsoryMisses;
    int currentIndex;
    CacheItem* cacheArray;
} Cache;

void initializeCache(Cache* cache, int size, char policy) {
    cache->size = size;
    cache->policy = policy;
    cache->capacityMisses = 0;
    cache->compulsoryMisses = 0;
    cache->currentIndex = 0;

    cache->cacheArray = (CacheItem*)malloc(size * sizeof(CacheItem));
    for (int i = 0; i < size; i++) {
        cache->cacheArray[i].key = NULL;
        cache->cacheArray[i].valid = 0;
        cache->cacheArray[i].referenced = 0;
    }
}

void evictItemFIFO(Cache* cache) {
    CacheItem* item = &cache->cacheArray[cache->currentIndex];
    if (item->valid) {
        item->valid = 0;
        cache->currentIndex = (cache->currentIndex + 1) % cache->size;
    }
}

void evictItemLRU(Cache* cache) {
    int lruIndex = 0;
    int minRef = cache->cacheArray[0].referenced;

    for (int i = 1; i < cache->size; i++) {
        if (cache->cacheArray[i].referenced < minRef) {
            lruIndex = i;
            minRef = cache->cacheArray[i].referenced;
        }
    }

    CacheItem* item = &cache->cacheArray[lruIndex];
    if (item->valid) {
        item->valid = 0;
        item->referenced = 0;
    }
}

void evictItemClock(Cache* cache) {
    while (1) {
        CacheItem* item = &cache->cacheArray[cache->currentIndex];
        if (item->valid) {
            if (item->referenced) {
                item->referenced = 0;
            } else {
                item->valid = 0;
                cache->currentIndex = (cache->currentIndex + 1) % cache->size;
                break;
            }
        }
        cache->currentIndex = (cache->currentIndex + 1) % cache->size;
    }
}

int checkCache(Cache* cache, char* item) {
    for (int i = 0; i < cache->size; i++) {
        if (cache->cacheArray[i].valid && strcmp(cache->cacheArray[i].key, item) == 0) {
            cache->cacheArray[i].referenced = 1;
            return 1; // HIT
        }
    }
    return 0; // MISS
}

void addToCache(Cache* cache, char* item) {
    int inserted = 0;
    for (int i = 0; i < cache->size; i++) {
        if (!cache->cacheArray[i].valid) {
            cache->cacheArray[i].key = item;
            cache->cacheArray[i].valid = 1;
            inserted = 1;
            break;
        }
    }

    if (!inserted) {
        cache->capacityMisses++;
        switch (cache->policy) {
            case 'F':
                evictItemFIFO(cache);
                break;
            case 'L':
                evictItemLRU(cache);
                break;
            case 'C':
                evictItemClock(cache);
                break;
        }
        for (int i = 0; i < cache->size; i++) {
            if (!cache->cacheArray[i].valid) {
                cache->cacheArray[i].key = item;
                cache->cacheArray[i].valid = 1;
                break;
            }
        }
    } else {
        cache->compulsoryMisses++;
    }
}

int main(int argc, char* argv[]) {
    int cacheSize = 0;
    char policy = 'F';

    // Parse command line arguments
    if (argc >= 3) {
        if (strcmp(argv[1], "-N") == 0) {
            cacheSize = atoi(argv[2]);
            policy = argv[3][1];
        } else {
            policy = argv[1][1];
        }
    }

    Cache cache;
    initializeCache(&cache, cacheSize, policy);

    char item[100];
    while (fgets(item, sizeof(item), stdin)) {
        item[strcspn(item, "\n")] = '\0'; // Remove trailing newline character

        if (checkCache(&cache, item)) {
            printf("HIT\n");
        } else {
            printf("MISS\n");
            addToCache(&cache, strdup(item));
        }
    }

    printf("%d %d\n", cache.compulsoryMisses, cache.capacityMisses);

    // Clean up
    for (int i = 0; i < cache.size; i++) {
        if (cache.cacheArray[i].key != NULL) {
            free(cache.cacheArray[i].key);
        }
    }
    free(cache.cacheArray);

    return 0;
}
