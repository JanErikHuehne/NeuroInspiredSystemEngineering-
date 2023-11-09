import socket 

word_to_send = "6"
bytesToSend= str.encode(word_to_send)
udp_client_socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
udp_client_socket.sendto(bytesToSend, ("192.168.0.3", 20001))

