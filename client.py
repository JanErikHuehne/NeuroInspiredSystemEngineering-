import socket
serverAddress = input("Enter server address:")
serverAddressPort = (serverAddress, 200001)


msgFromClient       = "Hello UDP Server"
bytesToSend         = str.encode(msgFromClient)

bufferSize = 1024

UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
UDPClientSocket.sendto(bytesToSend, serverAddressPort)

msgFromServer = UDPClientSocket.recvfrom(bufferSize)
msg = "Message from Server {}".format(msgFromServer[0])
print(msg)