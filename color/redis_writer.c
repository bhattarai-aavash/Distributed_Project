#include <stdio.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <redis_host> <key> <value>\n", argv[0]);
        return 1;
    }

    const char *redis_host = argv[1];  // Redis server address
    const char *key = argv[2];         // Key to store
    const char *value = argv[3];       // Value to store
    int redis_port = 6379;             // Default Redis port

    // Connect to Redis
    redisContext *context = redisConnect(redis_host, redis_port);
    if (context == NULL || context->err) {
        if (context) {
            printf("Connection error: %s\n", context->errstr);
            redisFree(context);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        return 1;
    }

    // Set key-value pair
    redisReply *reply = redisCommand(context, "SET %s %s", key, value);
    if (reply == NULL) {
        printf("SET command failed\n");
        redisFree(context);
        return 1;
    }
    printf("SET %s %s: %s\n", key, value, reply->str);
    freeReplyObject(reply);

    // Get value for the key
    reply = redisCommand(context, "GET %s", key);
    if (reply->type == REDIS_REPLY_STRING) {
        printf("GET %s: %s\n", key, reply->str);
    } else {
        printf("GET %s failed\n", key);
    }
    freeReplyObject(reply);

    // Cleanup
    redisFree(context);
    return 0;
}
