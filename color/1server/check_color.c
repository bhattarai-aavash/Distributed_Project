#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>

#define MAX_NODES 100000000
#define MAX_COLOR 1000

// Function to get the total number of nodes
int get_total_nodes(redisContext *context) {
    redisReply *reply;
    reply = redisCommand(context, "KEYS node_*_neighbours"); 

    int total_nodes = 0;
    if (reply->type == REDIS_REPLY_ARRAY) {
        total_nodes = reply->elements; 
    }
    freeReplyObject(reply);
    return total_nodes;
}

// Function to get adjacent nodes for a given node
int *get_adjacent_nodes(redisContext *context, int node, int *adj_size) {
    redisReply *reply;
    char command[256];
    snprintf(command, sizeof(command), "SMEMBERS node_%d_neighbours", node);
    reply = redisCommand(context, command);

    if (reply->type == REDIS_REPLY_ARRAY) {
        *adj_size = reply->elements;
        int *adj_nodes = malloc((*adj_size) * sizeof(int));
        for (size_t i = 0; i < reply->elements; i++) {
            adj_nodes[i] = atoi(reply->element[i]->str);
        }
        freeReplyObject(reply);
        return adj_nodes;
    } else {
        *adj_size = 0;
        freeReplyObject(reply);
        return NULL;
    }
}

// Function to get the color of a node
int get_node_color(redisContext *context, int node) {
    redisReply *reply;
    char command[256];
    snprintf(command, sizeof(command), "GET node_%d_color", node);
    reply = redisCommand(context, command);

    int color = -1;  // Default to -1 (no color)
    if (reply->type == REDIS_REPLY_STRING) {
        color = atoi(reply->str);  // Convert string to integer
    }
    freeReplyObject(reply);
    return color;
}

// Function to check if the coloring is valid (no adjacent nodes share the same color)
int check_coloring(redisContext *context, int total_nodes) {
    int mistakes = 0;

    for (int node = 0; node < total_nodes; node++) {
        int adj_size = 0;
        int *adj_nodes = get_adjacent_nodes(context, node, &adj_size);  

        int node_color = get_node_color(context, node);  // Get the color of the current node

        // Check all adjacent nodes
        for (int i = 0; i < adj_size; i++) {
            int adj_node = adj_nodes[i];
            int adj_color = get_node_color(context, adj_node);  // Get the color of the adjacent node

            // If the adjacent node has the same color as the current node, it's a mistake
            if (node_color == adj_color) {
                printf("Mistake: Node %d and Node %d have the same color %d\n", node, adj_node, node_color);
                mistakes++;
            }
        }

        free(adj_nodes);  
    }

    return mistakes;
}

// Main function
int main() {
    // Connect to Redis
    redisContext *context = redisConnect("localhost", 6379);
    if (context == NULL || context->err) {
        if (context) {
            printf("Error: %s\n", context->errstr);
            redisFree(context);
        } else {
            printf("Can't allocate redis context\n");
        }
        return 1;
    }

    // Get total number of nodes
    int total_nodes = get_total_nodes(context);
    printf("Total nodes in the graph: %d\n", total_nodes);

    // Check the coloring for mistakes
    int mistakes = check_coloring(context, total_nodes);
    if (mistakes == 0) {
        printf("Graph coloring is correct.\n");
    } else {
        printf("Total mistakes found: %d\n", mistakes);
    }

    // Free Redis context
    redisFree(context);

    return 0;
}
