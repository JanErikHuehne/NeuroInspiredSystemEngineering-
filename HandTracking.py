class HandMovements(self):
    rest = 0
    thumb2Tip_1 = 1
    thumb2Tip_2 = 2
    thumb2Tip_3 = 3
    thumb2Tip_4 = 4
    #different hand poisionts 

class HandTracking(MonoBehaviour):
    scene: str
    instance: HandTracking
    ChordInfoText: Text
    handPoints: ArrayList
    position: int

    def Start(self):
        self.instance = self  # Set the instance to this script, making it a singleton
        self.ChordInfoText.text = " "  # Initialize chord info text to empty

    def Update(self):
        # Get the current scene name
        self.scene = LoadNextScene.instance.scene.name

        # Check if data is received via UDP communication
        data = UDPReceive.udpReceive.data

        # Check if the first character is '[' before removing it
        if not String.IsNullOrEmpty(data) and len(data) > 1:
            # Process the received data (remove brackets and split values)
            data = data.Remove(0, 1)  # Removing the first element '['
            data = data.Remove(data.Length - 1, 1)  # Removing the last element ']'
            points = data.Split(',')

            # Update the positions of hand points based on the received data
            for i in range(21):
                x = 6 - float(points[i * 3]) / 100
                y = float(points[i * 3 + 1]) / 100
                z = float(points[i * 3 + 2]) / 100

                # Update the position of hand points
                handPoints[i].transform.localPosition = Vector3(x, y, z)

        
    def detectHandMovement(self, handPoints):
        # Calculate distances between hand points
        eu = self.calculateDistances()

        # Check the distances, compare with threshold, detect the hand movement
        # and play corresponding chords based on thresholds
        if eu[0] < Calibration.instance.th_index:
            if self.position != 1:
                self.stopChords()
                FMODEvents.instance.instance_chord_C_ARP_140.start()
                self.ChordInfoText.text = "You are playing C major chord: this chord consists of the notes, C - E - G"
            self.position = 1
        elif eu[1] < Calibration.instance.th_middle:
            if self.position != 2:
                self.stopChords()
                FMODEvents.instance.instance_chord_G_ARP_140.start()
                self.ChordInfoText.text = "You are playing G major chord: this chord consists of the notes, G - B - D"
            self.position = 2
        elif eu[2] < Calibration.instance.th_ring:
            if self.position != 3:
                self.stopChords()
                FMODEvents.instance.instance_chord_Am_ARP_140.start()
                self.ChordInfoText.text = "You are playing A minor chord: this chord consists of the notes, A - C - E"
            self.position = 3
        elif eu[3] < Calibration.instance.th_pinky:
            if self.position != 4:
                self.stopChords()
                FMODEvents.instance.instance_chord_F_ARP_140.start()
                self.ChordInfoText.text = "You are playing F major chord: this chord consists of the notes, F - A - C"
            self.position = 4
        else:
            if self.position != 0:
                self.stopChords()
                self.ChordInfoText.text = " "
            self.position = 0

    def calculateDistances(self):
        eu = [0.0] * 4
        eu[0] = Vector3.Distance(handPoints[4].transform.localPosition, handPoints[8].transform.localPosition)
        eu[1] = Vector3.Distance(handPoints[4].transform.localPosition, handPoints[12].transform.localPosition)
        eu[2] = Vector3.Distance(handPoints[4].transform.localPosition, handPoints[16].transform.localPosition)
        eu[3] = Vector3.Distance(handPoints[4].transform.localPosition, handPoints[20].transform.localPosition)
        #this is where the distance between the hand positions is calcualted
        return eu

    def stopChords(self):
        FMODEvents.instance.instance_chord_G_ARP_140.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT)
        FMODEvents.instance.instance_chord_C_ARP_140.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT)
        FMODEvents.instance.instance_chord_F_ARP_140.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT)
        FMODEvents.instance.instance_chord_Am_ARP_140.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT)

    def returnState(self, movementName):
        enumValue = int(movementName)
        return self.position == enumValue

