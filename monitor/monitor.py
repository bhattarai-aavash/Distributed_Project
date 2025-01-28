import redis
import time
import logging
import argparse
import signal
import sys
from concurrent.futures import ThreadPoolExecutor

# Global variable to keep track of the monitoring status
monitoring = True

# Helper function to configure logging for each server
def configure_logging(server_name):
    logging.basicConfig(
        filename=f'{server_name}_key_monitor.log',
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s'
    )

# Helper function to categorize commands as read or write
def is_read_command(command_name):
    read_commands = {'get', 'mget', 'keys', 'scan', 'exists'}
    return command_name.lower() in read_commands

def is_write_command(command_name):
    write_commands = {'set', 'mset', 'incr', 'decr', 'del', 'expire'}
    return command_name.lower() in write_commands

# Monitoring function for a single Redis server
def monitor_redis_server(server_name, host, port, password, interval):
    configure_logging(server_name)
    try:
        # Connect to Redis
        redis_client = redis.StrictRedis(
            host=host,
            port=port,
            password=password,
            decode_responses=True
        )

        # Verify connection
        if not redis_client.ping():
            logging.error(f"[{server_name}] Unable to connect to Redis server.")
            return

        logging.info(f"[{server_name}] Connected to Redis server.")

        # Initialize counters
        total_reads = 0
        total_writes = 0

        while monitoring:  # Check if monitoring flag is True
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
            logging.info(f"[{server_name}] New Reads: {new_reads}, New Writes: {new_writes}")
            print(f"[{server_name}] New Reads: {new_reads}, New Writes: {new_writes}")

            # Wait for the next interval
            time.sleep(interval)

    except redis.exceptions.ConnectionError as e:
        logging.error(f"[{server_name}] Redis connection error: {e}")
    except Exception as e:
        logging.error(f"[{server_name}] An error occurred: {e}")

# Graceful shutdown handler
def shutdown_signal_handler(signal, frame):
    global monitoring
    print("\nShutting down gracefully...")
    monitoring = False  # Set monitoring flag to False to stop the monitoring loop
    logging.info("Monitoring process has been terminated.")
    sys.exit(0)  # Exit the script gracefully

# Main function
if __name__ == "__main__":
    # Set up signal handler for graceful shutdown
    signal.signal(signal.SIGINT, shutdown_signal_handler)  # Handle Ctrl+C

    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Monitor multiple Redis servers.")
    parser.add_argument(
        '--servers',
        nargs='+',
        required=True,
        help="List of servers in the format: server_name:host:port[:password]"
    )
    parser.add_argument(
        '--interval',
        type=int,
        default=1,
        help="Interval (in seconds) for monitoring updates."
    )

    args = parser.parse_args()

    # Extract server details
    server_configs = []
    for server in args.servers:
        parts = server.split(':')
        if len(parts) < 3:
            print(f"Invalid server format: {server}. Expected format: server_name:host:port[:password]")
            continue
        server_name = parts[0]
        host = parts[1]
        port = int(parts[2])
        password = parts[3] if len(parts) > 3 else None
        server_configs.append((server_name, host, port, password))

    # Monitor each server concurrently
    with ThreadPoolExecutor() as executor:
        for server_name, host, port, password in server_configs:
            executor.submit(monitor_redis_server, server_name, host, port, password, args.interval)
