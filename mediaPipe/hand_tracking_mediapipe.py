import cv2 as cv
from cvzone.HandTrackingModule import HandDetector
import socket
import pickle
import struct

# using UDP protocol to send data -> doesn't require initial connection
# broadcasting the data!
width, height = 1280, 720

# Camera
capture = cv.VideoCapture(0) # 0 = device number
capture.set(3, width) # setting width
capture.set(4, height) # setting height

# Detector
detector = HandDetector(maxHands = 2, detectionCon = 0.8) # number of hands detected

# Communication
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) #STREAM for TCP, DGRAM for UDP
cameraAddressPort = ('127.0.0.1', 7007)

# Packet size in bytes
packet_size = int(65507 / 8)

while True:

    # get frame
    success_flag, image = capture.read()
    # get hands
    hands, image = detector.findHands(image)

    data = [] # a list of data points we are sending to unity

    # we are sending a landmark list -> 21 in total, every has 3 coordinates: x,y,z
    if hands:
        for hand in hands:
            # Get the list of landmarks (points)
            lmList = hand['lmList']
            # print(lmList)
    
        # print(hand) # this is a dictionary; we are interested in lmList and type
        # but it also prints out bbox and center of it

        # get the detected hand -> left or right ofc
        # handtype = hand['type']
        # print(handtype)

        # since we are sending coordinates to unity, we need to
        # split them so that they are contained in only one list
        # without being split in sublists  
            for lm in lmList:
                data.extend([lm[0], height - lm[1], lm[2]]) # upper left corner = (0,0) in opencv; lower right corner = (0,0) in unity
            # processing data and sending clean data to unity project
            # print(data)

    image = cv.resize(image, (0, 0), None, 0.5, 0.5)
    cv.imshow('Stream', image)

    cv.waitKey(1)