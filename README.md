# Distributed Graph Coloring (Fall 2024)

This repository is organized into two main directories:

- **`redis/`**: Contains the source code for the Redis database, including a custom command `add_graph` implemented specifically for this project.  
- **`color/`**: Contains the implementation of the distributed graph coloring algorithm.

---

## **Getting Started**

### **Building Redis**
1. Navigate to the `redis/` directory:
   ```bash
   cd redis
2. Build Redis along with the custom command add_graph:
    ```bash
    make all

This will compile Redis and include the custom functionality required for this project.

### **Running the Redis Server**
1. Start the Redis server with the provided configuration file:
```bash
./src/redis-server redis.conf

./src/redis-cli ping
```
1.Use the custom add_graph command from the Redis CLI
### **Adding a Graph Dataset**

```bash
./src/redis-cli add_graph <path_to_graph>
```


### **Distributed Graph Coloring**
### **Preparing for Distributed Execution**

1. Copy the color/1server directory to all remote nodes in your cluster:

```bash
    scp -r color/1server <user>@<remote_server>:<destination_path>
```

2. Update the master.sh script
    
    Modify the remote server names and file paths as required for your specific cluster setup.

3. Update the main.sh script:
    Configure the script to partition the graph appropriately across clients based on your requirements.


### **Executing the Distributed Graph Coloring Algorithm**

1. Run the master script:

```bash
./master.sh
```

### Note

Used Dataset 

1. DBLP-coauthorship dataset
2. CL-1000-1D7-TRIAL1 




