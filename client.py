import tkinter as tk  # PEP8: `import *` is not preferred
from PIL import Image, ImageTk
import cv2
from threading import Thread, Event
import tkinter.ttk as ttk
import time
"""
import socket
serverAddress = input("Enter server address:")
serverAddressPort = (serverAddress, 20001)


msgFromClient       = "Hello UDP Server"
bytesToSend         = str.encode(msgFromClient)

bufferSize = 1024
while True:

    UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    message = input("Enter message to send:")
    bytesToSend         = str.encode(message)
    UDPClientSocket.sendto(bytesToSend, serverAddressPort)
    msgFromServer = UDPClientSocket.recvfrom(bufferSize)
    msg = "Message from Server {}".format(msgFromServer[0])
    print(msg)

"""



class Client:

    def __init__(self,):
        # Set up main window
        self.root = tk.Tk()
        self.root.geometry("920x650")
        self.root.resizable(False, False)
        self.root.configure(background='white')
        # Set up window widgets
        self.camera = tk.Label(self.root)
        self.camera.grid(row=0, column=2, padx=(50, 0), pady=(50,0), rowspan=100, columnspan=200)
        self.address_label = tk.Label(self.root, text="Server IP Adress ", bd=0, bg="white").grid(row=0, column=0, pady=(50,0))
        self.port_label = tk.Label(self.root, text="Server Port ", bd=0, bg="white").grid(row=1, column=0) 
        self.server_ip = ""
        self.server_port = ""
        self.current_word_text = ""
        # Ip and port entry fields 
        self.server_ip_entry = tk.Entry(self.root, textvariable=self.server_ip, ).grid(row=0, column=1, pady=(50,0))
        self.server_port_entry = tk.Entry(self.root, textvariable=self.server_port).grid(row=1, column=1)
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
    
        # next char bar
        self.var  = tk.IntVar()
        self.progessbar = ttk.Progressbar(self.root, variable=self.var, orient=tk.HORIZONTAL, length=250)  
       
        self.root.mainloop()
  
        

    def add_values(self):
        variable = self.var
        for x in range(100):
            if self.stop_button_event.is_set():
                return 
            time.sleep(0.02)
            variable.set(x)
        self.collect_next_char_event.set()
        self.next_char_collect_event.wait()
        self.collect_next_char_event.clear()

    def add_bar(self,):
        while not self.stop_button_event.is_set():
            self.var.set(0)
            self.add_values()

    def start_button_pressed(self):
        self.stop_button_event.clear()
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

    def collect_char(self,):
        while not self.stop_button_event.is_set():
            self.next_char_collect_event.clear()
            self.collect_next_char_event.wait()
            # Do something with same frame, pass it to mediapipe in this thread
            print("collected")
            self.word_var.set(self.word_var.get()+  "T")
            self.next_char_collect_event.set()
            time.sleep(0.1)

    def stop_button_pressed(self):
        self.stop_button_event.set()
        self.start_button['state'] = tk.NORMAL
        self.stop_button['state'] = tk.DISABLED
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
   
        if ret:
            # cv2 uses `BGR` but `GUI` needs `RGB`
            frame = cv2.cvtColor(self.frame, cv2.COLOR_BGR2RGB)

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