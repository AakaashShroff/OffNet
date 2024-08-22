import requests
from requests.exceptions import ConnectionError, ChunkedEncodingError, Timeout, RequestException
from http.server import BaseHTTPRequestHandler, HTTPServer
import threading
import time

phonebook = {
    "Alice": ["User1", "ESP1"],
    "Bob": ["User2", "ESP2"],
}

user_id = "User1"

SERVER_PORT = 8080

def send_message_to_esp8266(sender_esp_id, receiver_user_id, receiver_esp_id, message):
    full_message = f"{user_id}:{sender_esp_id}:{receiver_user_id}:{receiver_esp_id}:{message}"
    url = f"http://192.168.4.1/transmit?msg={full_message}"
    try:
        response = requests.get(url, timeout=10, stream=True) 
        response.raise_for_status()
        response_text = ''.join([chunk.decode('utf-8') for chunk in response.iter_content(chunk_size=128)])
        print("ESP8266 Response: " + response_text)
        if "Transmitted" in response_text:
            print("Message transmitted successfully via LoRa.")
        else:
            print("Failed to transmit message via LoRa.")
    except ConnectionError:
        print("Failed to connect to ESP8266")
    except ChunkedEncodingError:
        print("Chunked Encoding Error: Connection was closed prematurely")
    except Timeout:
        print("Request to ESP8266 timed out")
    except RequestException as e:
        print(f"An unexpected error occurred: {e}")

class RequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if "/transmit?msg=" in self.path:
            full_message = self.path.split("transmit?msg=")[-1]
            print(f"Received message: {full_message}")
            
            parts = full_message.split(":")
            if len(parts) == 5:
                sender_user, sender_esp, receiver_user, receiver_esp, message = parts

                if receiver_user == user_id:
                    print(f"Message for me: {message} from {sender_user} ({sender_esp})")

                    response_message = "Message received"
                    self.send_response(200)
                    self.send_header("Content-Type", "text/plain")
                    self.send_header("Content-Length", str(len(response_message)))
                    self.end_headers()
                    self.wfile.write(response_message.encode('utf-8'))
                else:
                    print("Message not intended for this user.")
                    self.send_error(404, "Message not for this user")
            else:
                print("Invalid message format received.")
                self.send_error(400, "Invalid message format")
        else:
            self.send_error(404, "Invalid request")

    def log_message(self, format, *args):
        return

def start_server():
    server = HTTPServer(('0.0.0.0', SERVER_PORT), RequestHandler)
    print(f"Server started on port {SERVER_PORT}")
    server.serve_forever()

if __name__ == "__main__":
    server_thread = threading.Thread(target=start_server)
    server_thread.daemon = True
    server_thread.start()
    time.sleep(1)

    try:
        while True:
            name = input("Enter the name of the person to communicate with: ")
            
            if name in phonebook:
                receiver_user_id, receiver_esp_id = phonebook[name]
                message = input("Enter the message to transmit: ")

                sender_esp_id = "ESP1"

                send_message_to_esp8266(sender_esp_id, receiver_user_id, receiver_esp_id, message)
            else:
                print("Name not found in the phonebook.")

    except KeyboardInterrupt:
        print("Server stopped.")
