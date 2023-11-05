using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.SceneManagement;
using System;
using FMODUnity;

// Enum for different hand movements
public enum HandMovements
{
    rest = 0,
    thumb2Tip_1 = 1,
    thumb2Tip_2 = 2,
    thumb2Tip_3 = 3,
    thumb2Tip_4 = 4
}

public class HandTracking : MonoBehaviour
{
    public string scene;                    // Current scene name
    public static HandTracking instance;    // Singleton instance of the HandTracking class
    public Text ChordInfoText;              // UI Text component to display chord information
    public GameObject[] handPoints;         // Array of hand points game objects
    public int position = 0;                // Current hand position

    // Start is called before the first frame update
    void Start()
    {
        instance = this;            // Set the instance to this script, making it a singleton
        ChordInfoText.text = " ";   // Initialize chord info text to empty
    }

    // Update is called once per frame
    void Update()
    {
        // Get the current scene name
        scene = LoadNextScene.instance.scene.name;

        // Check if data is received via UDP communication
        string data = UDPReceive.udpReceive.data;

        // Check if the first character is '[' before removing it
        if (!string.IsNullOrEmpty(data) && data.Length > 1)
        {
            // Process the received data (remove brackets and split values)
            data = data.Remove(0, 1);               // Removing the first element '['
            data = data.Remove(data.Length - 1, 1); // removing last element ']'
            string[] points = data.Split(',');      // Splitting all the data to remove commas

            // Update the positions of hand points based on the received data
            for (int i = 0; i < 21; i++ )
            {
                float x = 6 - float.Parse(points[i*3]) / 100; // accessing every x value because the data is arranged as x1,y1,z1,x2,y2,z2,...
                float y = float.Parse(points[i*3 + 1]) / 100; // dividing by 100 because the values of positions in unity are low
                float z = float.Parse(points[i*3 + 2]) / 100;

                // Update the position of hand points
                handPoints[i].transform.localPosition = new Vector3(x, y, z); // Property "Transform" on our points -> by accessing Transform we can change the position of points
            }
            
            // Check if the current scene is a rhythm game or free mode
            if((scene == "RhythmGame_LetItBe_Easy") || (scene == "RhythmGame_LetItBe_Medium") || (scene == "RhythmGame_LetItBe_Fast") || (scene == "FreeMode"))
            {
                // Detect hand movements and play corresponding chords
                detectHandMovement(handPoints);
            }
        }   
    }

    // Detect hand movements and play corresponding chords
    void detectHandMovement(GameObject[] handPoints)
    {
        // Calculate distances between hand points
        float[] eu = calculateDistances();

        // Check the distances, compare with threshold, detect the hand movement
        //and play corresponding chords based on thresholds
        if (eu[0] < Calibration.instance.th_index)
        {
            if(position != 1)
            {
                stopChords();
                FMODEvents.instance.instance_chord_C_ARP_140.start();
                ChordInfoText.text = "You are playing C major chord: this chord consists of the notes, C - E - G";
            }  
            position = 1; // index 
        }
        else if (eu[1] < Calibration.instance.th_middle)
        {
            if(position != 2)
            {
                stopChords();
                FMODEvents.instance.instance_chord_G_ARP_140.start();
                ChordInfoText.text = "You are playing G major chord: this chord consists of the notes, G - B - D";
            }
            position = 2; // middle
        }
        else if (eu[2] < Calibration.instance.th_ring)
        {
            if(position != 3)
            {
                stopChords();
                FMODEvents.instance.instance_chord_Am_ARP_140.start();
                ChordInfoText.text = "You are playing A minor chord: this chord consists of the notes, A - C - E";
            }
            position = 3; // ring
        }
        else if (eu[3] < Calibration.instance.th_pinky)
        {
            if(position != 4)
            {
                stopChords();
                FMODEvents.instance.instance_chord_F_ARP_140.start();
                ChordInfoText.text = "You are playing F major chord: this chord consists of the notes, F - A - C";
            }
            position = 4; // pinky
        }
        else
        {
            if(position != 0)
            {
                stopChords();
                ChordInfoText.text = " ";
            }
            position = 0;
        }
    }

    // Calculate distances between hand points
    public float[] calculateDistances()
    {
        float[] eu = new float[4];
        // four different Euclidian distance calculation for four different hand points
        eu[0] = Vector3.Distance(handPoints[4].transform.localPosition, handPoints[8].transform.localPosition);
        eu[1] = Vector3.Distance(handPoints[4].transform.localPosition, handPoints[12].transform.localPosition);
        eu[2] = Vector3.Distance(handPoints[4].transform.localPosition, handPoints[16].transform.localPosition);
        eu[3] = Vector3.Distance(handPoints[4].transform.localPosition, handPoints[20].transform.localPosition);

        return eu;
    }

    // Stop playing all chord audio events
     public void stopChords()
     {
        FMODEvents.instance.instance_chord_G_ARP_140.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT);
        FMODEvents.instance.instance_chord_C_ARP_140.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT);
        FMODEvents.instance.instance_chord_F_ARP_140.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT);
        FMODEvents.instance.instance_chord_Am_ARP_140.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT);
     }

    // Check if the hand is in a specific movement state
    public bool returnState(HandMovements movementName)
    {
        int enumValue = (int)movementName;  // Convert the enum value to an integer
        if(position == enumValue)           
        {
            return true;                    // Return true if the hand is in the specified movement state
        }
        else
        {
            return false;                   // Return false if the hand is not in the specified movement state
        }
    }
}
