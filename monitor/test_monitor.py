import redis
import time
import random
import string
import threading

# Connect to Redis server
redis_client = redis.StrictRedis(host='localhost', port=6379, db=0)

# Function to generate a random string (key or value)
def random_string(length=10):
    return ''.join(random.choices(string.ascii_letters + string.digits, k=length))

# Function to perform 10 SET operations per second
def perform_set_operations():
    while True:
        for _ in range(20):  # 10 SET operations
            key = random_string()
            value = random_string()
            redis_client.set(key, value)
        time.sleep(1)  # Wait for 1 second

# Function to perform 10 GET operations per second
def perform_get_operations():
    while True:
        for _ in range(20):  # 10 GET operations
            key = random_string()
            redis_client.get(key)  # This will return None if the key doesn't exist
        time.sleep(1)  # Wait for 1 second

# Main function to start threads for SET and GET operations
def main():
    # Start the SET and GET operations in parallel using threads
    set_thread = threading.Thread(target=perform_set_operations)
    get_thread = threading.Thread(target=perform_get_operations)

    set_thread.start()
    get_thread.start()

    set_thread.join()
    get_thread.join()

if __name__ == "__main__":
    main()
