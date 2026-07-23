import socket

ROBOT_IP = "192.168.1.124" # <-- Remplace par l'IP de ton robot
PORT = 7777
XOR_KEY = bytes([0xA5, 0x3C, 0x7F, 0x11, 0xDE])

class XorStream:
    def init(self):
        self.offset = 0
        
    def process(self, data: bytes) -> bytes:
        result = bytearray(len(data))
        for i in range(len(data)):
            result[i] = data[i] ^ XOR_KEY[self.offset % len(XOR_KEY)]
            self.offset += 1
        return bytes(result)

def main():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((ROBOT_IP, PORT))
        print(f"Connected to the robot at {ROBOT_IP}:{PORT}")
    except Exception as e:
        print(f"Connection error: {e}")
        return

    banner = b"CONSOLE_READY\n"
    while b"CONSOLE_READY\n" not in banner:
        banner += s.recv(1024)


    # 1 stream for sending, 1 stream for receiving
    tx_stream = XorStream()
    rx_stream = XorStream()

    try:
        while True:
            cmd = input("AIBO> ")
            if not cmd:
                continue
            
            # 1. Sending
            cmd_bytes = (cmd + "\n").encode('utf-8')
            encrypted_cmd = tx_stream.process(cmd_bytes)
            s.sendall(encrypted_cmd)

            if cmd.strip() == "QUIT":
                break

            # 2. Receiving
            response = s.recv(1024)
            if not response:
                print("The robot ended the connection")
                break
            
            decrypted_response = rx_stream.process(response)
            print("ROBOT:", decrypted_response.decode('utf-8', errors='replace').strip())

    except KeyboardInterrupt:
        print("\nEnding connection...")
    finally:
        s.close()

main()