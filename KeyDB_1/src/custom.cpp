#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef OBJ_ENCODING_SET
#define OBJ_ENCODING_SET 2  // Example value; confirm with your codebase
#endif


void helloWorldCommand(client *c) {
    addReplyBulkCBuffer(c, "hello", 5);
}



void addGraphCommand(client *c) {
    if (c->argc < 2) {
        addReplyError(c, "Not enough arguments for add_graph command");
        return;
    }

    // Assume c->argv[1] is the filename containing the graph data
    char *filename = (char*) c->argv[1]->m_ptr; 
    printf("\n\n HERE\n\n");
    printf("\n \n%s \n\n ",filename); // Correctly access the filename from argv[1]
    FILE *file = fopen(filename, "r");
    if (!file) {
        addReplyError(c, "Could not open file ");
        return;
    }
   
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        int node1, node2;
        if (sscanf(line, "%d,%d", &node1, &node2) == 2) {
            // Print the edge being processed to the server terminal
            printf("Processing edge: %d <-> %d\n", node1, node2);

            // Create keys for each node's neighbors
            char nodeKey1[32], nodeKey2[32];
            snprintf(nodeKey1, sizeof(nodeKey1), "node_%d_neighbours", node1);
            snprintf(nodeKey2, sizeof(nodeKey2), "node_%d_neighbours", node2);

            robj *set1 = lookupKeyWrite(c->db, createStringObject(nodeKey1, strlen(nodeKey1)));
            robj *set2 = lookupKeyWrite(c->db, createStringObject(nodeKey2, strlen(nodeKey2)));

            // Ensure set1 and set2 are valid sets or create them
            if (set1 == NULL) {
                set1 = setTypeCreate(sdsfromlonglong(node2));
                dbAdd(c->db, createStringObject(nodeKey1, strlen(nodeKey1)), set1);
            } else if (set1->type != OBJ_SET) {
                addReplyError(c, "WRONGTYPE Operation against a key holding the wrong kind of value");
                continue;
            }

            if (set2 == NULL) {
                set2 = setTypeCreate(sdsfromlonglong(node1));
                dbAdd(c->db, createStringObject(nodeKey2, strlen(nodeKey2)), set2);
            } else if (set2->type != OBJ_SET) {
                addReplyError(c, "WRONGTYPE Operation against a key holding the wrong kind of value");
                continue;
            }

            // Add each node as a neighbor of the other
            if (setTypeAdd(set1, sdsfromlonglong(node2))) {
                signalModifiedKey(c, c->db, createStringObject(nodeKey1, strlen(nodeKey1)));
                notifyKeyspaceEvent(NOTIFY_SET, "sadd", createStringObject(nodeKey1, strlen(nodeKey1)), c->db->id);
                g_pserver->dirty++;
            }
            if (setTypeAdd(set2, sdsfromlonglong(node1))) {
                signalModifiedKey(c, c->db, createStringObject(nodeKey2, strlen(nodeKey2)));
                notifyKeyspaceEvent(NOTIFY_SET, "sadd", createStringObject(nodeKey2, strlen(nodeKey2)), c->db->id);
                g_pserver->dirty++;
            }

            // Assign a color to each node
            char colorKey1[32], colorKey2[32];
            snprintf(colorKey1, sizeof(colorKey1), "node_%d_color", node1);
            snprintf(colorKey2, sizeof(colorKey2), "node_%d_color", node2);

            // Set the color for each node with "0" as default value
            setKey(c, c->db, createStringObject(colorKey1, strlen(colorKey1)), createStringObject("0", 1));
            setKey(c, c->db, createStringObject(colorKey2, strlen(colorKey2)), createStringObject("0", 1));
        }
    }

    fclose(file);
    addReply(c, shared.ok); // Use shared.ok for a success response
}
