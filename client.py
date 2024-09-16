import tkinter as tk  # PEP8: `import *` is not preferred
from PIL import Image, ImageTk
import cv2
from threading import Thread, Event
import tkinter.ttk as ttk
import time
import socket
from cvzone.HandTrackingModule import HandDetector
import numpy as np 
import math 
import copy 
from sklearn import tree
import pickle
from pathlib import Path
import time


class Client:

    def __init__(self,):
        # Variables for validation
        self.start_time = 0
        self.counterCollected = 0
        self.end_time = 0

        # Set up main window
        self.save = True 
        self.save_file = Path("./data3/RnR.txt").absolute()
        self.root = tk.Tk()
        self.root.geometry("920x650")
        self.root.resizable(False, False)
        self.root.configure(background='white')
        # Set up window widgets
        self.camera = tk.Label(self.root)
        self.camera.grid(row=0, column=2, padx=(50, 0), pady=(50,0), rowspan=100, columnspan=200)
        self.address_label = tk.Label(self.root, text="Server IP Adress ", bd=0, bg="white").grid(row=0, column=0, pady=(50,0))
        self.port_label = tk.Label(self.root, text="Server Port ", bd=0, bg="white").grid(row=1, column=0) 
        self.server_ip = tk.StringVar()
        self.server_port = tk.StringVar()
        self.current_word_text = ""
        # Ip and port entry fields 
        self.server_ip_entry = tk.Entry(self.root, textvariable=self.server_ip, )
        self.server_ip_entry.grid(row=0, column=1, pady=(50,0))
        self.server_port_entry = tk.Entry(self.root, textvariable=self.server_port)
        self.server_port_entry.grid(row=1, column=1)
        # Start button 
        self.start_button = tk.Button(self.root, text ="Start",borderwidth=0, command = self.start_button_pressed, height= 3, width=30)
        self.start_button.grid(row=3, column=0, columnspan = 2, pady=(230, 0))
        # Stop button
        self.stop_button = tk.Button(self.root, text ="End",borderwidth=0,  command = self.stop_button_pressed, height= 3, width=30)
        self.stop_button.grid(row=4, column=0, columnspan = 2,)
        self.stop_button_event = Event()
        self.stop_button_event.clear()
        self.stop_button['state'] = tk.DISABLED
        # Video camera feed
        self.cap = cv2.VideoCapture(0)
        self.width, self.height = 1280, 720

        # Camera
       
        self.cap.set(3, self.width) # setting width
        self.cap.set(4, self.height) # setting height
        # Char and Word text
        self.char = tk.Label(self.root, text="Current character", bd=0, bg="white", font=("'Segoe UI", 24) )
        self.char_moving = tk.Label(self.root, text="", bd=0, bg="white", font=("'Segoe UI", 24, "bold") )
        self.collect_next_char_event = Event()
        self.collect_next_char_event.clear()
        self.next_char_collect_event = Event()
        self.next_char_collect_event.clear()
        self.word_var = tk.StringVar()
        self.word_text = tk.Label(self.root, text="Word ", bd=0, bg="white", font=("'Segoe UI", 24))
        self.word_text_moving = tk.Label(self.root, textvariable=self.word_var, bd=0, bg="white", font=("'Segoe UI", 24, "bold") ) 
        self.counter = 0
        # next char bar
        self.var  = tk.IntVar()
        self.progessbar = ttk.Progressbar(self.root, variable=self.var, orient=tk.HORIZONTAL, length=250)  

        # self udp_client
        self.udp_client_socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

        #detector
        self.detector = HandDetector(maxHands = 2, detectionCon = 0.8) 

        self.clasifier = pickle.load(open("./models/dt_classifier2.pickle", "rb"))
        # start app 
        self.root.mainloop()

        
        

    def add_values(self):
        variable = self.var
        for x in range(40):
            if self.stop_button_event.is_set():
                return 
            time.sleep(0.0008)
            variable.set(x)
        self.collect_next_char_event.set()
        self.next_char_collect_event.wait()
        self.collect_next_char_event.clear()

    def add_bar(self,):
        while not self.stop_button_event.is_set():
            self.var.set(0)
            self.add_values()

    def start_button_pressed(self):
        print(self.server_ip.get())
        print(self.server_port.get())
        self.server_ip_entry['state'] = tk.DISABLED
        self.server_port_entry['state'] = tk.DISABLED
        self.stop_button_event.clear()
        self.word_var.set("")
        self.counter = 0 
        self.camera = tk.Label(self.root)
        self.camera.grid(row=0, column=2, padx=(50, 0), pady=(50,0), rowspan=100, columnspan=200)
        self.char.grid(row=101, column=2, padx=(50, 0), pady=(30,0))
        self.char_moving.grid(row=102, column=2, padx=(50, 0), pady=(20,0))
        self.word_text.grid(row=101, column=3, padx=(150, 0), pady=(30,0))
        self.word_text_moving.grid(row=102, column=3, padx=(150, 0), pady=(20,0))
        self.progessbar.grid(row=101, rowspan=4, column=0, columnspan=2, pady=(20, 0))
        self.start_button['state'] = tk.DISABLED
        self.stop_button['state'] = tk.NORMAL


        self.camera_thread = Thread(target=self.show_frame())
        self.camera_thread.start()

        self.pb_thread = Thread(target=self.add_bar)
        self.pb_thread.start()

        self.collect_char_thread = Thread(target=self.collect_char)
        self.collect_char_thread.start()
        self.start_time = time.time()


    def media_pipe_detection(self,):
        hands, _ = self.detector.findHands(copy.deepcopy(self.frame))
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
                    data.extend([lm[0], self.height - lm[1], lm[2]]) # upper left corner = (0,0) in opencv; lower right corner = (0,0) in unity
                # processing data and sending clean data to unity project
                # print(lmList)
                Handpoints = np.array(lmList)
                # print(Handpoints)
                EU = np.array(self.calculateDistances(Handpoints))
                
                EU /= EU.max()
                if self.save:
                    with open(self.save_file, "a") as f:
                        for e in EU:
                            f.write(str(e) + " ")
                        f.write("\n")
                    
                
                #print(EU[9])
                return int(self.clasifier.predict(EU[np.newaxis, :])[0]) # add dictionary/enom for nr to string detection 
                #return self.detectHandMovement(EU)
        return "Empty"
    def calculateDistances(self, handPoints):
        eu = [0.0] * 15
        # thumb to index
        eu[0] = math.sqrt((handPoints[4,0] - handPoints[8,0])**2 + (handPoints[4,1] - handPoints[8,1])**2 + (handPoints[4,2] - handPoints[8,2])**2)
        # thumb to middle
        eu[1] = math.sqrt((handPoints[4,0] - handPoints[12,0])**2 + (handPoints[4,1] - handPoints[12,1])**2 + (handPoints[4,2] - handPoints[12,2])**2)
        # thumb to ring
        eu[2] = math.sqrt((handPoints[4,0] - handPoints[16,0])**2 + (handPoints[4,1] - handPoints[16,1])**2 + (handPoints[4,2] - handPoints[16,2])**2)
        # thumb to pinky
        eu[3] = math.sqrt((handPoints[4,0] - handPoints[20,0])**2 + (handPoints[4,1] - handPoints[20,1])**2 + (handPoints[4,2] - handPoints[20,2])**2)
        # ring to pinky
        eu[4] = math.sqrt((handPoints[16,0] - handPoints[20,0])**2 + (handPoints[16,1] - handPoints[20,1])**2 + (handPoints[16,2] - handPoints[20,2])**2)
        # palm to thumb
        eu[5] = math.sqrt((handPoints[0,0] - handPoints[4,0])**2 + (handPoints[0,1] - handPoints[4,1])**2 + (handPoints[0,2] - handPoints[4,2])**2)
        #palm to index
        eu[6] = math.sqrt((handPoints[0,0] - handPoints[8,0])**2 + (handPoints[0,1] - handPoints[8,1])**2 + (handPoints[0,2] - handPoints[8,2])**2)
        #palm to middle
        eu[7] = math.sqrt((handPoints[0,0] - handPoints[12,0])**2 + (handPoints[0,1] - handPoints[12,1])**2 + (handPoints[0,2] - handPoints[12,2])**2)
        #palm to ring
        eu[8] = math.sqrt((handPoints[0,0] - handPoints[16,0])**2 + (handPoints[0,1] - handPoints[16,1])**2 + (handPoints[0,2] - handPoints[16,2])**2)
        #palm to pinky
        eu[9] = math.sqrt((handPoints[0,0] - handPoints[20,0])**2 + (handPoints[0,1] - handPoints[20,1])**2 + (handPoints[0,2] - handPoints[20,2])**2)
        # thumb to below pinky
        eu[10] = math.sqrt((handPoints[4,0] - handPoints[17,0])**2 + (handPoints[4,1] - handPoints[17,1])**2 + (handPoints[4,2] - handPoints[17,2])**2)
        #pinky to middle
        eu[11] = math.sqrt((handPoints[12,0] - handPoints[20,0])**2 + (handPoints[12,1] - handPoints[20,1])**2 + (handPoints[12,2] - handPoints[20,2])**2)#add interfinger distance
        #pinky to index
        eu[12] = math.sqrt((handPoints[8,0] - handPoints[20,0])**2 + (handPoints[8,1] - handPoints[20,1])**2 + (handPoints[8,2] - handPoints[20,2])**2)#add interfinger distance
        #index to ring
        eu[13] = math.sqrt((handPoints[8,0] - handPoints[16,0])**2 + (handPoints[8,1] - handPoints[16,1])**2 + (handPoints[8,2] - handPoints[16,2])**2)#add interfinger distance
        #index to middle
        eu[14] = math.sqrt((handPoints[8,0] - handPoints[12,0])**2 + (handPoints[8,1] - handPoints[12,1])**2 + (handPoints[8,2] - handPoints[12,2])**2)#add interfinger distance

        return eu
    

    def detectHandMovement(self, eu):
        # Calculate distances between hand points
        # Check the distances, compare with threshold, detect the hand movement
        # and play corresponding chords based on thresholds
        # ZERO
        #if (eu[0] < 40) and (eu[1] < 40) and (eu[2] < 40) and (eu[3] < 40):
        #    sign = '0'
        #    print(sign)
        # ONE
        #elif (eu[0] > 80) and (eu[1] < 50) and (eu[2] < 70) and (eu[3] < 80):
        if (eu[6] > 0.8) and (eu[7] < 0.5) and (eu[4] < 0.2):
            sign = '1'
        # TWO
        #elif (eu[0] > 70) and (eu[1] > 7) and (eu[2] < 40) and (eu[3] < 40) and (eu[4] < 30):
        elif (eu[5] < 0.6 and eu[6] > 0.8) and (eu[7] > 0.8) and (eu[4] < 0.2):
            sign = '2'
        # THREE 
        #elif (eu[5] > 60) and (eu[6] > 80) and (eu[7] > 80) and (eu[8] < 90) and (eu[9] < 90) and ( eu[4] < 40):
        elif (eu[5] > 0.6 and eu[6] > 0.8) and (eu[7] > 0.8) and (eu[4] < 0.2):
                sign = '3'
                
        else:
            sign = 'None'   
        """
        elif (eu[5] > 60) and (eu[6] > 50) and (eu[7] > 100) and (eu[8] > 100) and (eu[9] > 140) and (eu[10]< 30):
                sign = '4'
                print(sign)

        #FIVE (all fingertips must be away from origin)
        elif (eu[0] > 80) and (eu[1] > 80) and (eu[2] > 80) and (eu[3] > 80) and (eu[5:9]> [100,100,100,100,100]):
            sign = '5'
            print(sign)

        # SIX
        elif (eu[0] > 30) and (eu[1] > 30) and (eu[2] > 30) and (eu[3] < 30):
            sign = '6'
            print(sign)

        # SEVEN
        elif (eu[0] > 30) and (eu[1] > 30) and (eu[2] < 30) and (eu[3] > 30) and (eu[4] > 30):
            sign = '7'
            print(sign)

        # EIGHT
        elif (eu[0] > 30) and (eu[1] < 30) and (eu[2] > 30) and (eu[3] > 30):
            sign = '8'
            print(sign)

        # NINE
        elif (eu[0] < 30) and (eu[1] > 30) and (eu[2] > 30) and (eu[3] > 30):
            sign = '9'
            print(sign)
        """
        
        return sign

    def collect_char(self,):
        while not self.stop_button_event.is_set():
            self.next_char_collect_event.clear()
            self.collect_next_char_event.wait()
            # Do something with same frame, pass it to mediapipe in this thread
            print("collected")
            # self.counterCollected += 1
            # if self.counterCollected == 10:
            #     self.end_time = time.time()
            #     print(f"10 signs executed in {self.end_time - self.start_time:.2f} seconds")
            #     self.counterCollected = 0
            #     self.start_time = 0
            #     self.end_time = 0
            #     # Set the stop button event to stop the execution
            #     self.stop_button_event.set()
           
            collected_char = self.media_pipe_detection()
            print(collected_char)
            if collected_char == "Empty":
                # UDP thread to send 
                self.udp_send_thread = Thread(target=self.udp_send, args=(self.word_var.get(),))
                self.udp_send_thread.start()
                self.word_var.set("")
            else:
                self.word_var.set(self.word_var.get() + str(collected_char))
            
            
            self.next_char_collect_event.set()
            time.sleep(0.3)

    def udp_send(self, word_to_send):
        if word_to_send:
            bytesToSend= str.encode(word_to_send)
            self.udp_client_socket.sendto(bytesToSend, (self.server_ip.get(), int(self.server_port.get())))
            print("Word send")
        else:
            print("Nothing to send, empty cache")

    def stop_button_pressed(self):
        self.stop_button_event.set()
        self.start_button['state'] = tk.NORMAL
        self.stop_button['state'] = tk.DISABLED
        self.server_ip_entry['state'] = tk.NORMAL
        self.server_port_entry['state'] = tk.NORMAL
        self.root.after_cancel(self.after_id)
        self.camera.grid_forget()
        self.char.grid_forget()
        self.char_moving.grid_forget()
        self.word_text.grid_forget()
        self.word_text_moving.grid_forget()
        self.progessbar.grid_forget()
        self.word_var.set("")
       
    def show_frame(self,):
        # get frame
        ret, self.frame = self.cap.read()
        hands, frame = self.detector.findHands(self.frame)
        if ret:
            # cv2 uses `BGR` but `GUI` needs `RGB`
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

            # convert to PIL image
            img = Image.fromarray(frame).resize((550, 400))

            # convert to Tkinter image
            photo = ImageTk.PhotoImage(image=img)
            
            # solution for bug in `PhotoImage`
            self.camera.photo = photo
            
            # replace image in label
            self.camera.configure(image=photo)  
        
        # run again after 20ms (0.02s)
        self.after_id  = self.root.after(1, self.show_frame)
            
Client()