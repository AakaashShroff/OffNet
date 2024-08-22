Using the table below, create the connection between the ESP8266 and the LoRa SX1278 module:

Here is the wiring information in a table format:
 ______________________________________________________________________
|   LoRa Module Pin   |   ESP8266 Pin          |   NodeMCU Label      |
|---------------------|------------------------|----------------------|
| NSS (CS)            | GPIO15                 | D8                   |
| RESET               | GPIO16                 | D0                   |
| DIO0                | GPIO5                  | D1                   |
| SCK (SPI Clock)     | GPIO14                 | D5                   |
| MISO                | GPIO12                 | D6                   |
| MOSI                | GPIO13                 | D7                   |
| 3.3V                | 3.3V on ESP8266        | 3.3V                 |
| GND                 | GND on ESP8266         | GND                  |
-----------------------------------------------------------------------

- Once the wiring is done, upload the hub.ino code onto the esp8266. Note: For each hub you make, you need to change the deviceID as that is unique.
- Run the ESP8266 on a power cable or keep it connected to your computer to see the serial monitor while it runs.
- On the client device you will use for messaging, connect that device to the ESP8266 webserver that just came online as deviceID+"OffNet" with the default password OffNet@2024
- Once connected, in the node.py file make sure your user_id is unique on that ESP hub if not messages can be sent to duplicate users.
- In the node.py file, in the phonebook you can add usernames with their predifined user_ids and esp hub ids to communicate with just usernames. (This is something like a DNS)
- Run the node.py file, Enter the username you want to chat with and send the message.

- The data packets are of the format : SenderUserID:SenderESP_ID:ReceiverUserID:ReceiverESP_ID:Message

Output (This is a demo conversation using two hubs(ESP8266+LoRa) and two nodes(computers)): 

![output](https://github.com/user-attachments/assets/9040f8ea-9888-482a-96d2-92e5a5ecce98)
