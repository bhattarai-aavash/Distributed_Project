import redis
import time
import logging

# Configure logging
logging.basicConfig(
    filename='redis_read_write_monitor.log',
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

# Redis server connection settings
REDIS_HOST = 'localhost'
REDIS_PORT = 6379
REDIS_PASSWORD = None  # Set if Redis requires authentication

# Helper function to categorize commands as read or write
def is_read_command(command_name):
    read_commands = {'get', 'mget', 'keys', 'scan', 'exists'}
    return command_name.lower() in read_commands

def is_write_command(command_name):
    write_commands = {'set', 'mset', 'incr', 'decr', 'del', 'expire'}
    return command_name.lower() in write_commands

# Monitoring function
def monitor_redis_read_write(interval=1):
    try:
        # Connect to Redis
        redis_client = redis.StrictRedis(
            host=REDIS_HOST,
            port=REDIS_PORT,
            password=REDIS_PASSWORD,
            decode_responses=True
        )

        # Verify connection
        if not redis_client.ping():
            logging.error("Unable to connect to Redis server.")
            return

        logging.info("Connected to Redis server.")

        # Initialize counters
        total_reads = 0
        total_writes = 0

        while True:
            stats = redis_client.info("commandstats")  # Get command stats
            read_count = 0
            write_count = 0

            # Iterate through command stats
            for command, data in stats.items():
                command_name = command.replace('cmdstat_', '')
                calls = data.get('calls', 0)

                if is_read_command(command_name):
                    read_count += calls
                elif is_write_command(command_name):
                    write_count += calls

            # Calculate new read/write since the last interval
            new_reads = read_count - total_reads
            new_writes = write_count - total_writes

            # Update totals
            total_reads = read_count
            total_writes = write_count

            # Log and print metrics
            logging.info(f"New Reads: {new_reads}, New Writes: {new_writes}")
            print(f"New Reads: {new_reads}, New Writes: {new_writes}")

            # Wait for the next interval
            time.sleep(interval)

    except redis.exceptions.ConnectionError as e:
        logging.error(f"Redis connection error: {e}")
    except Exception as e:
        logging.error(f"An error occurred: {e}")

if __name__ == "__main__":
    # Start monitoring with a 10-second interval
    monitor_redis_read_write(interval=1)
