def parse_gdb_output(file_path):
    threads = {}
    current_thread = None

    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith('Thread'):
                current_thread = line.split()[1]  # Extract thread identifier
                threads[current_thread] = []
            elif line.strip().startswith('#'):
                if current_thread:
                    threads[current_thread].append(line.strip())

    return threads

def print_threads_without_wait(threads):
    for thread, frames in threads.items():
        if frames and 'wait' not in frames[0].lower():
            print(f"Thread {thread}:")
            for frame in frames:
                print(frame)
            print()

# Usage
gdb_file_path = 'gdb.txt'  # Path to your GDB output file
threads = parse_gdb_output(gdb_file_path)
print_threads_without_wait(threads)
