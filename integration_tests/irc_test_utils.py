import socket
import subprocess
import time
import threading
import os

class ExecutableIrcServer:
    def __init__(self, port=6667, executable_path=None, password="pass"):
        self.port = port
        if executable_path is None:
            raise ValueError("executable_path must be provided to ExecutableIrcServer")
        self.executable_path = executable_path
        self.password = password
        self.process = None
        self.output_thread = None
        self.server_output = []

    def _read_output(self, stream):
        for line in iter(stream.readline, ''):
            self.server_output.append(line.strip())
        stream.close()

    def start(self):
        if not os.path.exists(self.executable_path):
            raise FileNotFoundError(f"Server executable not found at: {self.executable_path}")

        print(f"Starting IRC server from executable on port {self.port}...")
        self.process = subprocess.Popen(
            [self.executable_path, str(self.port), self.password],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            bufsize=1,
            universal_newlines=True
        )
        self.server_output = []
        self.output_thread = threading.Thread(target=self._read_output, args=(self.process.stdout,))
        self.output_thread.daemon = True
        self.output_thread.start()
        time.sleep(2)

    def stop(self):
        if self.process:
            print("Stopping executable IRC server...")
            self.process.terminate()
            self.process.wait(timeout=5)
            if self.process.poll() is None:
                self.process.kill()
            print("Executable IRC server stopped.")
        if self.output_thread:
            self.output_thread.join(timeout=1)

    def get_server_output(self):
        return self.server_output

class DockerIrcServer:
    def __init__(self, compose_path, host='localhost', port=6667, password="pass"):
        self.compose_path = compose_path
        self.host = host
        self.port = port
        self.password = password

    def start(self):
        if not os.path.exists(self.compose_path):
            raise FileNotFoundError(f"Docker compose file not found at: {self.compose_path}")
        
        print(f"Starting IRC server from Docker Compose file: {self.compose_path}...")
        subprocess.run(
            ["docker-compose", "-f", self.compose_path, "up", "-d"],
            check=True,
            capture_output=True
        )
        time.sleep(5) # Give Docker container more time to start

    def stop(self):
        print("Stopping Docker IRC server...")
        subprocess.run(
            ["docker-compose", "-f", self.compose_path, "down"],
            check=True,
            capture_output=True
        )
        print("Docker IRC server stopped.")

    def get_server_output(self):
        # For Docker, we might not have direct access to stdout in the same way.
        # This could be implemented by capturing logs from the container if needed.
        return ["Docker server output not implemented."]

class IrcClient:
    def __init__(self, host='localhost', port=6667):
        self.host = host
        self.port = port
        self.socket = None
        self.buffer = ""

    def connect(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((self.host, self.port))
        self.socket.settimeout(2) # Set a timeout for receiving data

    def disconnect(self):
        if self.socket:
            self.socket.close()
            self.socket = None

    def send_command(self, command):
        if self.socket:
            message = f"{command}\r\n"
            self.socket.sendall(message.encode())
            print(f"Sent: {command}")

    def receive_response(self, timeout=2):
        start_time = time.time()
        while True:
            try:
                data = self.socket.recv(4096).decode()
                if data:
                    self.buffer += data
                    # Check for complete messages (ending with \r\n)
                    if "\r\n" in self.buffer:
                        messages = self.buffer.split("\r\n")
                        self.buffer = messages.pop() # Keep incomplete message in buffer
                        return [msg for msg in messages if msg] # Return non-empty messages
                elif time.time() - start_time > timeout:
                    break # Timeout if no data received
            except socket.timeout:
                break # Timeout
            except Exception as e:
                print(f"Error receiving data: {e}")
                break
        
        # If we have anything left in the buffer, return it as a single message
        if self.buffer:
            msg = self.buffer
            self.buffer = ""
            return [msg]
        return []

    def get_full_response(self, timeout=2, delay=0.1):
        """Receives all available responses within a timeout."""
        responses = []
        start_time = time.time()
        while time.time() - start_time < timeout:
            new_responses = self.receive_response(timeout=delay)
            if new_responses:
                responses.extend(new_responses)
                start_time = time.time() # Reset timeout if new data arrives
            else:
                time.sleep(delay) # Wait a bit before trying again
        return responses
