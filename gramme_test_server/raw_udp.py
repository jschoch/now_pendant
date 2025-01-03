import socket

def start_udp_server(port=8080):
    # Create a socket object using IPv4 and UDP protocol
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Bind the socket to the specified port
    sock.bind(('192.168.11.3', port))
    
    print(f"UDP server is running on port {port}...")
    
    try:
        while True:
            # Receive data from the client
            data, addr = sock.recvfrom(1024)  # buffer size is 1024 bytes
            
            # Print the received data and the address of the sender
            print(f"Received packet from {addr}: {data.decode()}")
    except KeyboardInterrupt:
        print("Server shutting down.")
    finally:
        sock.close()

if __name__ == "__main__":
    start_udp_server()
