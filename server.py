# get your local ip address: ifconfig | grep "inet " | grep -Fv 127.0.0.1 | awk '{print $2}'
import socket
from speech_synthesis_micAzure import generate_xml, generateSpeech
 
localIP = input("Enter your IP-address:")

localPort   = 20001

bufferSize  = 1024

 



# Create a datagram socket

UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

 

# Bind to address and ip

UDPServerSocket.bind((localIP, localPort))

 

print("UDP server up and listening")

 

# Listen for incoming datagrams

while(True):

    bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)

    message = bytesAddressPair[0]

    address = bytesAddressPair[1]

    clientMsg = "Message from Client:{}".format(message)
    clientIP  = "Client IP Address:{}".format(address)
    
    print(clientMsg)
    print(clientIP)

   
    message = input("Enter message to send:")
    bytesToSend         = str.encode(message)
    # Sending a reply to client

    UDPServerSocket.sendto(bytesToSend, address)

    # Synthesize speech by TTS
    _ = generateSpeech(text=generate_xml(message))