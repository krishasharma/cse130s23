#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *key; // Key for cache item
    int valid; // Flag indicating if the item is valid
    int referenced; // Flag indicating if the item has been referenced
} CacheItem;

typedef struct {
    int size; // Size of the cache
    char policy; // Policy for cache eviction (FIFO, LRU, or Clock)
    int capacityMisses; // Number of capacity misses
    int compulsoryMisses; // Number of compulsory misses
    int currentIndex; // Current index for eviction in Clock policy
    CacheItem *cacheArray; // Array to store cache items
} Cache;

void initializeCache(Cache *cache, int size, char policy) {
    cache->size = size;
    cache->policy = policy;
    cache->capacityMisses = 0;
    cache->compulsoryMisses = 0;
    cache->currentIndex = 0;

    // Allocate memory for the cache array
    cache->cacheArray = (CacheItem *) malloc(size * sizeof(CacheItem));

    // Initialize each cache item
    for (int i = 0; i < size; i++) {
        cache->cacheArray[i].key = NULL;
        cache->cacheArray[i].valid = 0;
        cache->cacheArray[i].referenced = 0;
    }
}

void evictItemFIFO(Cache *cache) {
    CacheItem *item = &cache->cacheArray[cache->currentIndex];
    if (item->valid) {
        item->valid = 0;
        cache->currentIndex = (cache->currentIndex + 1) % cache->size;
    }
}

void evictItemLRU(Cache *cache) {
    int lruIndex = 0;
    int minRef = cache->cacheArray[0].referenced;

    // Find the least-recently used item
    for (int i = 1; i < cache->size; i++) {
        if (cache->cacheArray[i].referenced < minRef) {
            lruIndex = i;
            minRef = cache->cacheArray[i].referenced;
        }
    }

    CacheItem *item = &cache->cacheArray[lruIndex];
    if (item->valid) {
        item->valid = 0;
        item->referenced = 0;
    }
}

void evictItemClock(Cache *cache) {
    while (1) {
        CacheItem *item = &cache->cacheArray[cache->currentIndex];
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

int checkCache(Cache *cache, char *item) {
    // Check if the item is present in the cache
    for (int i = 0; i < cache->size; i++) {
        if (cache->cacheArray[i].valid && strcmp(cache->cacheArray[i].key, item) == 0) {
            cache->cacheArray[i].referenced = 1;
            return 1; // HIT
        }
    }
    return 0; // MISS
}

void addToCache(Cache *cache, char *item) {
    int inserted = 0;

    // Find an empty slot in the cache to insert the item
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

        // Evict an item based on the cache policy
        switch (cache->policy) {
        case 'F': evictItemFIFO(cache); break;
        case 'L': evictItemLRU(cache); break;
        case 'C': evictItemClock(cache); break;
        }

        // Insert the item into the cache after eviction
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

int main(int argc, char *argv[]) {
    int cacheSize = 0;
    char policy = 'F';

    if (argc < 2 || argc > 4) {
        fprintf(stderr, "Invalid command line arguments. Usage: . /cacher [-N <poilcy]\n");
        return 1;
    }

    if (argc == 1) {
        policy = argv[1][0];
    } else if (argc == 4) {
        if (strcmp(argv[1], "-N") == 0) {
            cacheSize = atoi(argv[2]);
            policy = argv[3][1];
        } else {
            fprintf(stderr, "Invalid command line argument. \n");
            return 1;
        }
    } else {
        fprintf(stderr, "Invalid command line argument. \n");
        return 1;
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
