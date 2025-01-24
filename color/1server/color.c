#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include <unistd.h>
typedef struct {
    char lock_key_1[128];  // Change from char** to char[] (array of chars)
    char lock_key_2[128];
    char turn_key[128];
    char node_name[128];
    
} ClientGraphPetersonLock;


  /*
        Example:
            nodeName = "15"
            neighborName = "34"
            nodeFlagVar = "flag15_34_15"
            neighborFlagVar = "flag15_34_34"
            turnVar = "turn15_34"

        flag15_34_15            flag15_34_34
        +-----+    turn15_34   +-----+
        | 15  |----------------| 34  |
        +-----+                +-----+
     */

int getMinimalValueNotInArray(int *arr, int arrLength, int lowerBound, int upperBound) {
    int range = upperBound - lowerBound + 1;
    
    // Dynamically allocate memory for the used values array
    int *usedValues = (int *)calloc(range, sizeof(int));
    if (usedValues == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE); // Exit with an error
    }

    // Mark used values within the range [lowerBound, upperBound]
    for (int i = 0; i < arrLength; i++) {
        if (arr[i] >= lowerBound && arr[i] <= upperBound) {
            usedValues[arr[i] - lowerBound] = 1;
        }
    }

    // Find the first available unused value
    for (int i = lowerBound; i <= upperBound; i++) {
        if (usedValues[i - lowerBound] == 0) {
            free(usedValues); // Free the allocated memory
            return i;
        }
    }

    // Debug message: range too small
    printf("getMinimalValueNotInArray: error: the range is too small\n");

    // Free the allocated memory and recursively call with an extended range
    free(usedValues);
    return getMinimalValueNotInArray(arr, arrLength, lowerBound, upperBound + arrLength + 1);
}

void print_locks_array(ClientGraphPetersonLock *locks_array, int num_neighbors) {
    for (int i = 0; i < num_neighbors; i++) {
        printf("Lock %d:\n", i + 1);
        printf("  lock_key_1: %s\n", locks_array[i].lock_key_1);
        printf("  lock_key_2: %s\n", locks_array[i].lock_key_2);
        printf("  turn_key: %s\n", locks_array[i].turn_key);
    }
}

void acquire_lock(redisContext *context, ClientGraphPetersonLock *lock,const char *log_file_path) {
    redisReply *my_flag, *turn_var, *other_flag, *turn_var_new;

        FILE *log_file = fopen(log_file_path, "a");
        if (log_file == NULL) {
            fprintf(stderr, "Error: Failed to open log file for writing: %s\n", log_file_path);
            return;
        }
        
        my_flag= redisCommand(context, "SET %s %s", lock->lock_key_1 , "1");
        fprintf(log_file, "setting own flag %s as %s was %s ",lock->lock_key_1, "1", my_flag->str);

        
        
        turn_var= redisCommand(context, "SET %s %s ",lock->turn_key, lock->node_name); 
        fprintf(log_file, "setting turn_var %s as %s ",lock->turn_key, lock->node_name);


        other_flag= redisCommand(context, "GET %s", lock->lock_key_2);
        turn_var_new = redisCommand(context, "GET %s", lock->turn_key);

        printf("Values of old turn value is %s and new turn val is %s",lock->node_name, turn_var_new->str );
        fprintf(log_file, "acquiring_locks %s",lock->node_name);
        
        if(other_flag->str == NULL){

            printf("%s doesn't exist",lock->lock_key_2);
        }
        else
        {
            printf("%s is set as %s", lock->lock_key_2, other_flag->str);
        }
        while (other_flag->str != NULL &&
                   strcmp(other_flag->str, "1") == 0 &&
                   strcmp(turn_var_new->str, lock->node_name) == 0){

                turn_var_new = redisCommand(context, "GET %s", lock->turn_key);
                other_flag= redisCommand(context, "GET %s", lock->lock_key_2);

               
                fprintf(log_file, "Waiting for Lock turn_var_new =%s and other_var_new %s",lock->turn_key , lock->lock_key_2);
                   }

        
        freeReplyObject(other_flag);
        freeReplyObject(turn_var_new);
        freeReplyObject(my_flag);

}
void release_lock(redisContext *context, ClientGraphPetersonLock *lock) {
    redisReply *reply;

    // Delete the first lock key
    reply = redisCommand(context, "SET %s %s", lock->lock_key_1,"0");
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Error: Failed to delete lock_key_1: %s\n", lock->lock_key_1);
    }
    if (reply) {
        freeReplyObject(reply);
    }

    
}

// Function to get all neighbors of a node using SMEMBERS
char** get_all_neighbours(redisContext *context, const char *node_name, int *neighbour_count) {
    // Generate the Redis key for the neighbors
    char key[256];
    snprintf(key, sizeof(key), "%s_neighbours", node_name);

    // Send the Redis SMEMBERS command
    redisReply *reply = redisCommand(context, "SMEMBERS %s", key);

    // Check if the key exists and if the reply is valid
    if (reply == NULL || reply->type == REDIS_REPLY_NIL) {
        fprintf(stderr, "Error: Node %s does not exist or has no neighbors.\n", node_name);
        freeReplyObject(reply);
        return NULL;
    }

    // Check if the reply contains an array
    if (reply->type != REDIS_REPLY_ARRAY) {
        fprintf(stderr, "Error: Unexpected reply type for node %s neighbors.\n", node_name);
        freeReplyObject(reply);
        return NULL;
    }

    // Allocate memory for the neighbor list
    int count = reply->elements;
    char **neighbor_list = malloc(sizeof(char*) * count);
    if (neighbor_list == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        freeReplyObject(reply);
        return NULL;
    }

    // Copy neighbors from the reply into the list
    for (int i = 0; i < count; i++) {
        neighbor_list[i] = strdup(reply->element[i]->str);
        // printf("\n %s neighboours of %s \n", neighbor_list[i], node_name);
    }

    *neighbour_count = count; // Return the number of neighbors
    freeReplyObject(reply);
    return neighbor_list;
}

// Comparator function for numerical sorting
int compare_neighbours(const void *a, const void *b) {
    // Convert the strings to integers and compare them
    int num_a = atoi(*(const char **)a);
    int num_b = atoi(*(const char **)b);
    return num_a - num_b;
}

// Function to sort the neighbor list numerically
void sort_neighbours(char **neighbour_list, int neighbour_count) {
    qsort(neighbour_list, neighbour_count, sizeof(char *), compare_neighbours);
}


int get_node_color(redisContext *context, const char *node_name) {
    char key[256];
    snprintf(key, sizeof(key), "node_%s_color", node_name);
    redisReply *reply = redisCommand(context, "GET %s", key);
    if (reply == NULL || reply->type == REDIS_REPLY_NIL) {
        fprintf(stderr, "Error: Node %s color not found.\n", node_name);
        return -1; // Return -1 if not found
    }

    
    return atoi(reply->str);
}


int check_boundary_node(redisContext *context, const char *node_name, int last_node_id_of_last_task, int first_node_id_of_first_task)

{
    char key[256];
    snprintf(key, sizeof(key), "%s_neighbours", node_name);

    // Send the Redis SMEMBERS command
    redisReply *reply = redisCommand(context, "SMEMBERS %s", key);

    int count = reply->elements;
    char **neighbor_list = malloc(sizeof(char*) * count);
    
    

    // Copy neighbors from the reply into the list
    for (int i = 0; i < count; i++) {
        neighbor_list[i] = strdup(reply->element[i]->str);
        
        int nbr_id_int = atoi(neighbor_list[i]);
    
        if(nbr_id_int < first_node_id_of_first_task || nbr_id_int > last_node_id_of_last_task){
            // So the node has a neigbhour in another partition
            // printf(" \n %s  is a boundary node ", node_name);

            return 1;
            }

        // printf("\n %s neighboours of %s \n", neighbor_list[i], node_name);
    }
    // printf("\n %s  is not a boundary node ",node_name);
     // Return the number of neighbors
    freeReplyObject(reply);
    return 0;
}


void set_node_color(redisContext *context, const char *node_name, int color, const char *log_file_path) {
    char key[256];
    snprintf(key, sizeof(key), "%s_color", node_name);
    
    // Convert the integer color to string
    char color_str[10];
    snprintf(color_str, sizeof(color_str), "%d", color);
    
    // Open the log file in append mode
    FILE *log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        fprintf(stderr, "Error: Failed to open log file for writing: %s\n", log_file_path);
        return;
    }

    // Log node name and color to the log file
    fprintf(log_file, "Setting color for node: %s, Color: %d\n", node_name, color);

    // Set the color in Redis
    redisReply *reply = redisCommand(context, "SET %s %s", key, color_str);
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Error: Failed to set color for node %s.\n", node_name);
        // Log the error to the log file
        fprintf(log_file, "Error: Failed to set color for node: %s\n", node_name);
        if (reply != NULL) {
            freeReplyObject(reply);
        }
        fclose(log_file); // Close the log file
        return; // Exit the function
    }

    // Log success to the log file
    fprintf(log_file, "Successfully set color for node: %s, Color: %d\n", node_name, color);

    freeReplyObject(reply);
    fclose(log_file); // Close the log file
}


int get_node_id(const char *node_name) {
    const char *prefix = "node_";
    size_t prefix_len = strlen(prefix);

    // Check if the input starts with the prefix "node_"
    if (strncmp(node_name, prefix, prefix_len) != 0) {
        fprintf(stderr, "Error: Invalid node name format: %s\n", node_name);
        return -1; // Return an error value
    }

    // Extract the numeric part of the node name and convert to integer
    int node_id = atoi(node_name + prefix_len);
    return node_id;
}
char* extract_number_from_string(const char *str) {
    // Find the position of the underscore
    const char *underscore_pos = strchr(str, '_');
    if (underscore_pos == NULL) {
        return NULL;  // Return NULL if there's no underscore in the string
    }
    
    // Allocate memory to hold the number string (after the underscore)
    char *result = (char*)malloc(strlen(underscore_pos) * sizeof(char));
    if (result == NULL) {
        return NULL;  // Return NULL if memory allocation fails
    }
    
    // Copy the part of the string after the underscore
    strcpy(result, underscore_pos + 1);  // Skip the underscore itself

    return result;
}

int get_status(redisContext *context, const char *node_name) {
    char key[256];
    
    snprintf(key, sizeof(key), "node_%s_status", node_name);
    redisReply *reply = redisCommand(context, "GET %s", key);
    if (reply == NULL) {
        printf("error");
        fprintf(stderr, "Error: Failed to execute GET command for node %s.\n", node_name);
        // Handle the error as appropriate, possibly by reconnecting
        return 0;
    }
    if (reply->type == REDIS_REPLY_NIL) {
        // Key does not exist
       
        freeReplyObject(reply);
        return 0;
    }
    if (reply->type != REDIS_REPLY_STRING) {
        fprintf(stderr, "Error: Unexpected reply type %d for node %s.\n", reply->type, node_name);
        freeReplyObject(reply);
        return 0;
    }
    
    int status = atoi(reply->str);
    
    return status;
}

void read_color_of_neighbors(redisContext *context,
                              const char *node_id,
                              int first_node_id_of_first_task,
                              int last_node_id_of_last_task,
                              char **node_nbr_list_sorted,
                              int *nbr_color_list,
                              int num_neighbors,
                              ClientGraphPetersonLock *locks_array,const char *log_file_path) { // New parameter for locks array
    for (int nbr = 0; nbr < num_neighbors; nbr++) {
        const char *nbr_id = node_nbr_list_sorted[nbr];
        int nbr_id_int = atoi(nbr_id);
        int node_id_int = get_node_id(node_id);
        char *char_node = extract_number_from_string(node_id);
        FILE *log_file = fopen(log_file_path, "a");
        if (log_file == NULL) {
            fprintf(stderr, "Error: Failed to open log file for writing: %s\n", log_file_path);
            return;
        }


        if((nbr_id_int < first_node_id_of_first_task) || (nbr_id_int> last_node_id_of_last_task)){
            char bothId[64];
            int status = get_status(context,nbr_id);
            
            
            if(status== 0){
                fprintf(log_file,"\nSTATUS IS UNCOLORED\n");
                if (nbr_id_int < node_id_int) {
                    snprintf(bothId, sizeof(bothId), "%s_%s", nbr_id, char_node);
                } else {
                    snprintf(bothId, sizeof(bothId), "%s_%s", char_node, nbr_id);
                }

                // Create and store the lock in the locks array
                // printf("\n I am getting a lock\n");
                
                ClientGraphPetersonLock *lock = &locks_array[nbr]; // Reference the specific lock in the array
                snprintf(lock->lock_key_1, sizeof(lock->lock_key_1), "flag_%s_%s", bothId, char_node);
                snprintf(lock->lock_key_2, sizeof(lock->lock_key_2), "flag_%s_%s", bothId, nbr_id);
                snprintf(lock->turn_key, sizeof(lock->turn_key), "turn_%s", bothId);
                snprintf(lock->node_name, sizeof(lock->node_name), "%s", char_node);

                // fprintf(log_file,"Setting Up Locks");    
                // fprintf(log_file,"%s\n", lock->lock_key_1);
                // fprintf(log_file,"%s\n", lock->lock_key_2);
                // fprintf(log_file,"%s\n", lock->turn_key);
                // fprintf(log_file,"%s\n", lock->node_name);


                printf("\nSetting Up Locks for %s and %s\n", char_node,nbr_id);    
                printf("%s\n", lock->lock_key_1);
                printf("%s\n", lock->lock_key_2);
                printf("%s\n", lock->turn_key);
                printf("%s\n", lock->node_name);

                
                acquire_lock(context, lock,log_file_path);
                

        }
        }
        int nbr_color = get_node_color(context, nbr_id);
        nbr_color_list[nbr] = nbr_color;
        sleep(5);
    }
}

void set_status(redisContext *context, const char *node_name ){
    char key[256];
    snprintf(key, sizeof(key), "%s_status", node_name);
   
    redisReply *reply = redisCommand(context, "SET %s %s", key, "1");
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Error: Failed to set colored status for node %s.\n", node_name);
        if (reply != NULL) {
            freeReplyObject(reply);
        }
        return; // Exit the function
    }

    freeReplyObject(reply);
}




void process_node(redisContext *context, const char *node_name, 
                 int first_node_id_of_first_task, int last_node_id_of_last_task, const char *log_file_path) {
    int neighbour_count = 0;

    // Get all neighbors of the specified node
    char **neighbour_list = get_all_neighbours(context, node_name, &neighbour_count);
    int boundary_node =  check_boundary_node(context, node_name, last_node_id_of_last_task, first_node_id_of_first_task);
    FILE *log_file = fopen(log_file_path, "a");
        if (log_file == NULL) {
            fprintf(stderr, "Error: Failed to open log file for writing: %s\n", log_file_path);
            return;
        }
    // printf("\n Boundary node status of %s is %d \n", node_name,boundary_node);
    if (neighbour_list == NULL || neighbour_count == 0) {
        printf("No neighbors found for node %s\n", node_name);
        
    }

    // Sort the neighbors numerically
    sort_neighbours(neighbour_list, neighbour_count);

    
    fprintf(log_file,"Sorted neighbors of %s:\n", node_name);
    for (int i = 0; i < neighbour_count; i++) {
        // printf(" %s\n", neighbour_list[i]);
        fprintf(log_file, "%s\n", neighbour_list[i]);
        
    }

    // Allocate memory for locks and neighbor color list
    ClientGraphPetersonLock *locks_array = malloc(neighbour_count * sizeof(ClientGraphPetersonLock));
    int *nbr_color_list = (int *)malloc(neighbour_count * sizeof(int));
    int status;
    if (nbr_color_list == NULL) {
        printf("Memory allocation error for nbr_color_list\n");
        free(neighbour_list);
    }

    // Read the colors of the neighbors
    read_color_of_neighbors(context, node_name, first_node_id_of_first_task, last_node_id_of_last_task,
                            neighbour_list, nbr_color_list, neighbour_count, locks_array,log_file_path);

    // Print neighbor colors
    // for (int i = 0; i < neighbour_count; i++) {
    //     printf("%d\n", nbr_color_list[i]);
    // }

    // Determine the minimal unused color
    
    int new_color = getMinimalValueNotInArray(nbr_color_list, neighbour_count, 0, 1000);

    // Set the new color for the node
    
    set_node_color(context, node_name, new_color,log_file_path);
    set_status(context, node_name);
    
    
    // Release locks
    for (int i = 0; i < neighbour_count; i++) {
        release_lock(context, &locks_array[i]);
    }

    // Free allocated memory
    free(neighbour_list);
    free(nbr_color_list);
    free(locks_array);
}



int compare_keys(const void *a, const void *b) {
    const char *key_a = *(const char **)a;
    const char *key_b = *(const char **)b;

    // Extract numeric parts from the keys
    int num_a = atoi(strchr(key_a, '_') + 1); // Find '_' and convert the number after it
    int num_b = atoi(strchr(key_b, '_') + 1);

    return num_a - num_b; // Compare numerically
}

const char **fetch_keys(redisContext *context, const char *pattern, int *key_count) {
    redisReply *reply = redisCommand(context, "KEYS %s", pattern);
    if (reply == NULL || reply->type != REDIS_REPLY_ARRAY) {
        // printf("Failed to fetch keys with pattern %s\n", pattern);
        if (reply) freeReplyObject(reply);
        return NULL;
    }

    *key_count = reply->elements;

    const char **keys = malloc(*key_count * sizeof(char *));
    if (keys == NULL) {
        // printf("Memory allocation error for keys array\n");
        freeReplyObject(reply);
        return NULL;
    }

    for (size_t i = 0; i < reply->elements; i++) {
        char *key_with_suffix = strdup(reply->element[i]->str); // Duplicate the string
        if (!key_with_suffix) {
            printf("Memory allocation error for key duplication\n");
            for (size_t j = 0; j < i; j++) free((void *)keys[j]);
            free(keys);
            freeReplyObject(reply);
            return NULL;
        }

        // Remove `_neighbours` suffix, if present
        char *suffix = strstr(key_with_suffix, "_neighbours");
        if (suffix) {
            *suffix = '\0'; // Terminate the string before the suffix
        }

        keys[i] = key_with_suffix;
    }

    // Sort the keys numerically by the extracted numeric part
    qsort(keys, *key_count, sizeof(char *), compare_keys);

    freeReplyObject(reply);
    return keys;
}


int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s <first_node_id_of_first_task> <last_node_id_of_last_task>\n", argv[0]);
        return 1;
    }
    
    // Parse command-line arguments
    int first_node_id_of_first_task = atoi(argv[1]);
    int last_node_id_of_last_task = atoi(argv[2]);
    char *ip = argv[3];
    const char *log_file_path = argv[4];
    

    if (first_node_id_of_first_task > last_node_id_of_last_task) {
        printf("Error: first_node_id_of_first_task must be less than or equal to last_node_id_of_last_task.\n");
        return 1;
    }

    // Connect to Redis
    redisContext *context = redisConnect(ip, 6379);
    if (context == NULL || context->err) {
        if (context) {
            printf("Redis connection error: %s\n", context->errstr);
        } else {
            printf("Connection error: can't allocate Redis context\n");
        }
        return 1;
    }

    // Fetch keys from Redis
    int key_count = 0;
    const char **keys = fetch_keys(context, "node_*_neighbours", &key_count);
    if (keys == NULL || key_count == 0) {
        // printf("No keys found\n");
        redisFree(context);
        return 1;
    }
    // for (int i = 0; i < key_count; i++) {
    // printf("Key %d: %s\n", i + 1, keys[i]);
    // }

    
    // printf("Processing nodes:\n");

    int total_processed_nodes = 0;  // Counter for processed nodes
    
    // Process each node
    for (int i = first_node_id_of_first_task; i <=last_node_id_of_last_task ; i++) {
        // printf("Processing node: %s\n", keys[i]);
        process_node(context, keys[i], first_node_id_of_first_task, last_node_id_of_last_task,log_file_path);
        free((void *)keys[i]); // Free the key string

        total_processed_nodes++;  // Increment the counter
    }
    
    // Print the total number of processed nodes
    // printf("Total nodes processed: %d\n", total_processed_nodes);

    // Free the keys array and Redis context
    free(keys);
    redisFree(context);

    return 0;
}
