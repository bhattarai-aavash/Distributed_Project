import redis
import argparse

def is_coloring_valid(host):
    # Connect to the Redis server
    redis_client = redis.StrictRedis(host=host, port=6379, decode_responses=True)
    
    # Get all nodes and their neighbors
    keys = redis_client.keys("node_*_neighbours")
    error_count = 0  # Initialize error counter
    node_count = 0  # Initialize node counter
    
    # Use pipeline to reduce network round trips
    pipeline = redis_client.pipeline()
    
    # Pre-fetch all node colors in a single pipeline execution
    for key in keys:
        node = key.split("_")[1]
        pipeline.get(f"node_{node}_color")
        pipeline.smembers(key)
    results = pipeline.execute()
    
    # Process nodes and neighbors
    for i in range(0, len(keys)):
        node = keys[i].split("_")[1]
        node_color = results[i * 2]  # Get node color
        neighbors = results[i * 2 + 1]  # Get neighbors
        node_count += 1  # Increment node counter
        
        for neighbor in neighbors:
            neighbor_color = redis_client.get(f"node_{neighbor}_color")
            
            # Check if the current node and neighbor have the same color
            if node_color == neighbor_color:
                print(f"Coloring error: Node {node} and Node {neighbor} both have color {node_color}")
                error_count += 1
    
    print(f"Total nodes processed: {node_count}")
    
    if error_count == 0:
        print("Graph coloring is valid.")
    else:
        print(f"Graph coloring has {error_count} error(s).")
    
    return error_count

if __name__ == "__main__":
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Check graph coloring validity.")
    parser.add_argument("--host", type=str, required=True, help="Redis server hostname or IP address.")
    args = parser.parse_args()
    
    # Call the function with the provided hostname
    is_coloring_valid(args.host)
