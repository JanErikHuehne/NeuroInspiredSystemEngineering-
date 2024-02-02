#include <Arduino.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <random>
#include <actuator.h>
#include <Dynamixel2Arduino.h>

#include <DynamixelSDK.h>


#define NEURON_COUNT 3      // Each Servo Motor is controlled by 3 neuron network
#define ASSEMBLE_COUNT 7    // Number of Servo Motors
#define INIT_DELAY 3000
#define SIMULATION_STEPS 2000
#define UPDATE_INTERVAL 100
#define DT 0.1
#define INPUT_SIGNAL 2.5
#define SCALING_FACTOR1 100.0
#define SCALING_FACTOR2 150.0
#define SCALING_FACTOR3 200.0
#define OFFSET 510.0
#define QUEUE_SIZE 100
#define ADDR_AX_ID           3                 
#define ADDR_AX_MAX_TORQUE              34 
#define ADDR_AX_TORQUE_ENABLE           24                 // Control table address is different in Dynamixel model
#define ADDR_AX_GOAL_POSITION           30
#define ADDR_AX_PRESENT_POSITION        36
#define ADDR_AX_MOVING_SPEED            32
#define MOVING                          46

// Protocol version
#define PROTOCOL_VERSION                1.0   

#define BAUDRATE                        1000000
#define DEVICENAME                      "1"   

int sensorValues[QUEUE_SIZE] = {0};
int sensorValues1[QUEUE_SIZE] = {0};
int currentIndex = 0;
float input_signal = 5;      // u_0 to change the number of s shapes smoothly
float curr_input_signal = 5; // to keep the old input signal when the signal starts changing after sensory trigger to change behavior for new section

// Create PortHandler instance
dynamixel::PortHandler *portHandler;

// Create PacketHandler instance
dynamixel::PacketHandler *packetHandler;

int dxl_comm_result = COMM_TX_FAIL;             // Communication result
uint8_t dxl_error = 0;                          // Dynamixel error
int16_t dxl_present_position = 0;               // Present position
bool initialized;
int stopFlag =0;

struct MatsuokaNetwork {
    struct MatsuokaAssemble *assemble;
    int assembleNumber;
    void (*ode_solver)(struct MatsuokaNetwork *, double, double);
};

struct MatsuokaAssemble {
    struct MatsuokaNeuron *neurons;
    int neuronNumber;
    double *y;
    double *w;
};

struct MatsuokaNeuron {
    // Function pointers
    double (*neuronDynamics)(MatsuokaNetwork *, MatsuokaAssemble *, double, double, double, double, double *, double *, double, int, int);
    double (*neuronOutput)(double);
    double (*adaptation)(double, double, double);
    void (*ode_solver)(MatsuokaNetwork *, MatsuokaAssemble *, struct MatsuokaNeuron *, double, double, double *, int);

    // Properties
    double tau, x, x_prime, b, T;
    double *a;
    int id, n;
};

double matsuoka_neuron_dynamics(MatsuokaNetwork *network, MatsuokaAssemble *neuronsAssemble, double tau, double s_i, double x_i_prime, double x_i, double *y, double *a_i, double b, int y_length, int neuron_id) {
    // y is the output of each neuron of the assembly the neuron is associated with 

    bool debug = false;
    double result = 0.0; 
    result = s_i - b * x_i_prime - x_i;
    // INTER-ASSEMBLY CONNECTIVITY
    for (int j = 0; j < y_length; ++j) {
        result -= a_i[j] * y[j];
    }
    // BETWEEN ASSEMBLY CONNECTIVITY
    double add_term = 0.0;
    for (int i = 0; i < ASSEMBLE_COUNT; i++) {
        add_term += network->assemble[i].y[neuron_id] * neuronsAssemble->w[i];
    }

    if  (debug) {
        printf("X derivative for neuron %d : %lf \n",neuron_id, result);
    }
    result = (result + add_term) / tau;
    //Serial.println(result);
    return result;

}

double matsuoka_neuron_output(double x) {
  if (x > 0) {
    return x;
  }
  else {
    return 0; 
  }
}

double matsouka_adaptation(double T, double y_i, double x_prime) {
  double result = (1/T) * (y_i - x_prime);
  return result;
}


void network_ode_solver(struct MatsuokaNetwork *network, double dt, double input)
{
    // We update the outputs of each neuron
    for (int i = 0; i < network->assembleNumber; ++i) {
        // Access each neuron using the index i
        struct MatsuokaAssemble *currentAssemble = &(network->assemble[i]);
       
        for(int k = 0; k < currentAssemble->neuronNumber; k++){
            double *y_i = &(currentAssemble->y[k]);
            *y_i = currentAssemble->neurons[k].neuronOutput(currentAssemble->neurons[k].x);
        }
    }
    
    for (int i = 0; i < network->assembleNumber; ++i) {
        // Access each neuron using the index i
        struct MatsuokaAssemble *currentAssemble = &(network->assemble[i]);
       
        for(int k = 0; k < currentAssemble->neuronNumber; k++){
          currentAssemble->neurons[k].ode_solver(network, currentAssemble, &(currentAssemble->neurons[k]), dt, input, currentAssemble->y, currentAssemble->neuronNumber);
        }
    }

}
void test_ode_solver(MatsuokaNetwork *network, MatsuokaAssemble *assemble, MatsuokaNeuron *neuron, double dt, double input, double *y, int numberNeurons) {
    bool debug = false; 
    if (debug) {
         printf("ODE SOLVER CALLED FOR NEURON %d \n", neuron->id);
    }
    double k1, k2, k3, k4;
    double l1, l2, l3, l4;
   
    k1 = neuron->neuronDynamics(network,
                                assemble, 
                                neuron->tau,
                                input, // s_i
                                neuron->x_prime, // x_i_prime
                                neuron->x,
                                y, // y*
                                neuron->a, // *a_i
                                neuron->b, // b
                                numberNeurons, // y_length
                                neuron->id // neuron_id
                               );
    l1 = neuron->adaptation(neuron->T, y[neuron->id], neuron->x_prime);
    if (debug) {
         printf("k1: %lf\n", k1);
    printf("l1: %lf\n", l1);
    }
   
    
    double x_half, x_prime_half;
    x_half = neuron->x +  0.5* dt * k1;
    x_prime_half = neuron->x_prime +  0.5* dt * l1;
    y[neuron->id] = neuron->neuronOutput(x_half);
    k2 = neuron->neuronDynamics(network,
                                assemble, 
                                neuron->tau,
                                input, // s_i
                                x_prime_half, // x_i_prime
                                x_half,
                                y, // y*
                                neuron->a, // *a_i
                                neuron->b, // b
                                numberNeurons, // y_length
                                neuron->id // neuron_id
                               );
    l2 = neuron->adaptation(neuron->T, y[neuron->id], x_prime_half);
    if (debug) {
        printf("k2: %lf\n", k2);
        printf("l2: %lf\n", l2);  
    }
    x_half = neuron->x + 0.5* dt * k2;
    x_prime_half = neuron->x_prime +  0.5* dt * l2;
    y[neuron->id] = neuron->neuronOutput(x_half);
    k3 = neuron->neuronDynamics(network,
                                assemble, 
                                neuron->tau,
                                input, // s_i
                                x_prime_half, // x_i_prime
                                x_half,
                                y, // y*
                                neuron->a, // *a_i
                                neuron->b, // b
                                numberNeurons, // y_length
                                neuron->id // neuron_id
                               );
    l3 = neuron->adaptation(neuron->T, y[neuron->id], x_prime_half);
    if (debug) {
        printf("k3: %lf\n", k3);
        printf("l3: %lf\n", l3); 
    }
  
    x_half = neuron->x + dt * k3;
    x_prime_half = neuron->x_prime +  dt * l3;
    y[neuron->id] = neuron->neuronOutput(x_half);
    k4 = neuron->neuronDynamics(network,
                                assemble, 
                                neuron->tau,
                                input, // s_i
                                x_prime_half, // x_i_prime
                                x_half,
                                y, // y*
                                neuron->a, // *a_i
                                neuron->b, // b
                                numberNeurons, // y_length
                                neuron->id // neuron_id
                               );
    l4 = neuron->adaptation(neuron->T, y[neuron->id], x_prime_half);

    if (debug) {
        printf("k44: %lf\n", k4);
        printf("l: %lf\n", l4);   
    }

    neuron->x += (k1 + 2*k2 + 2*k3 + k4) * (dt/6);
    neuron->x_prime += (l1 + 2*l2 + 2*l3 + l4) * (dt/6);
}

// Function declarations
double matsuoka_neuron_dynamics(MatsuokaNetwork *, MatsuokaAssemble *, double, double, double, double, double *, double *, double, int, int);
double matsuoka_neuron_output(double);
double matsouka_adaptation(double, double, double);
void network_ode_solver(struct MatsuokaNetwork *, double, double);
void test_ode_solver(MatsuokaNetwork *, MatsuokaAssemble *, MatsuokaNeuron *neuron, double, double, double *, int);

void initializeNeuron(struct MatsuokaNeuron *neuron, int id) {
    float w = 2.7;
    neuron->a = (double *)malloc(NEURON_COUNT * sizeof(double));
    for (int k = 0; k < NEURON_COUNT; k++) {
        neuron->a[k] = 0.0;

        if(id == 0){
            if(k == NEURON_COUNT - 1){
               neuron->a[k] = w;  
            } 
        }                                                                                                           
        else{
            if(k == id-1){
                neuron->a[k] = w;
            }
        } 
    }

    neuron->n = NEURON_COUNT;
    neuron->tau = 2.0;
    neuron->T = 6;
    neuron->b = 2.5;
    neuron->x = (double) (rand() %100 )*0.01 ;      // Random initialization
    neuron->x_prime = (double) (rand() %100 )*0.01; // Random initialization
    neuron->id = id;
    neuron->neuronDynamics = matsuoka_neuron_dynamics;
    neuron->neuronOutput = matsuoka_neuron_output;
    neuron->adaptation = matsouka_adaptation;
    neuron->ode_solver = test_ode_solver;
} 

float calculateAverage(int queue[], int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += queue[i];
    }
    return (float)sum / size;
}



void setup() {
  /* set the configuration of the RS neuron to match a desired output: OSCILLATORY, QUIESCENT, PLATEAU, ALMOSTOSC */
  // put your setup code here, to run once:
  Serial.begin(115200);
  initialized = false;
  // Initialize portHandler. Set the port path
  // Get methods and members of PortHandlerLinux or PortHandlerWindows
  portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);

  // Initialize packetHandler. Set the protocol version
  // Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
  packetHandler= dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

  if (portHandler->openPort())
  {
    Serial.print("Succeeded to open the port!\n");
  }
  else
  {
    Serial.print("Failed to open the port!\n");
    Serial.print("Press any key to terminate...\n");
    return;
  }

  // Set port baudrate
  if (portHandler->setBaudRate(BAUDRATE))
  {
    Serial.print("Succeeded to change the baudrate!\n");
  }
  else
  {
    Serial.print("Failed to change the baudrate!\n");
    Serial.print("Press any key to terminate...\n");
    return;
  }



}


void loop() { 
  /* Read my program running time in milliseconds */
    static struct MatsuokaNetwork myNetwork;
    unsigned long mytime = millis();
    float w_0 = 0.1;    // Weights between the assembles

    if (!initialized) {
        myNetwork.assembleNumber = ASSEMBLE_COUNT;
        myNetwork.assemble  =  (struct MatsuokaAssemble *)malloc(ASSEMBLE_COUNT * sizeof(struct MatsuokaAssemble));

        for (int i = 0; i < ASSEMBLE_COUNT; i++) {
            myNetwork.assemble[i].neurons = (struct MatsuokaNeuron *)malloc(NEURON_COUNT * sizeof(struct MatsuokaNeuron));
            myNetwork.assemble[i].neuronNumber = NEURON_COUNT;
            myNetwork.assemble[i].y = (double *)malloc(ASSEMBLE_COUNT * sizeof(double));
            myNetwork.assemble[i].w = (double *)malloc(ASSEMBLE_COUNT * sizeof(double));
            for (int j = 0; j < ASSEMBLE_COUNT; j++) {
              myNetwork.assemble[i].w[j] = 0;
            }
            if(i==0){
                // Feedback from the last assemble to the first: CHANGE HERE TO CHANGE THE NUMBER OF S-SHAPES
                myNetwork.assemble[i].w[ASSEMBLE_COUNT - 1] = w_0;  
            }
            else{
                myNetwork.assemble[i].w[i - 1] = w_0;
            }
            Serial.print("After assembly");
            for (int j = 0; j < ASSEMBLE_COUNT; j++) {
              Serial.print(myNetwork.assemble[i].w[j]);
              Serial.print(" ");
            }
            Serial.println(" ");

            for (int j = 0; j < NEURON_COUNT; j++) {
                initializeNeuron(&myNetwork.assemble[i].neurons[j], j);
            }
           
        }
        myNetwork.ode_solver = network_ode_solver;
        initialized = true;
    }
    
    float pin0 = analogRead(0);
    sensorValues[currentIndex] = pin0;
    float pin1 = analogRead(1);
    sensorValues1[currentIndex] = pin1;
    currentIndex = (currentIndex + 1) % QUEUE_SIZE;
    float averageleft = calculateAverage(sensorValues, QUEUE_SIZE);
    float averageright = calculateAverage(sensorValues1, QUEUE_SIZE);
    float myval = averageleft+527.0-averageright; // both sensor inactive, myval =0; left sensor active --> bend right: myvalue positive, right sensor active --> bend left: myvalue negativem amplitude f
    //----------------------------------
    float sensoryTrigger = 0.0;     // Sensory trigger is zero for keeping movement with the same amplitude, if positive change to bigger amplitude, if negative to smaller amplitude
    bool sensory_input = false;     // sensory input flag
    double sensory_start_time = 0;  // sensory trigger start time point
    double timeAfterInput = 0;      // passed time after sensory input arrived for the first time -> used to change the input signal smoothly
    int changeOscillation = 0;      // specify the direction of oscillation change, -1 for lower amplitude, 1 for higher
    // ----------------------------------
    if (fabs(myval) <= 5.0) {
      myval = 0.0;
    }
    if (myval <0.0) {
      myval = myval / 6.0;
    }  

    // ---------NEW CODE PART----------------------
    if (sensoryTrigger != 0.0) {
        sensory_input = true;
    }
                
    if(sensory_input){

        timeAfterInput = mytime - sensory_start_time;

        if(changeOscillation > 0){ // change to higher amplitude S-shapes
            input_signal = (timeAfterInput/400) + curr_input_signal;
    
            if(input_signal > curr_input_signal*3){
                sensory_input = false;
            }
        }
        else if(changeOscillation < 0){ // change to lower amplitude S-shapes
            input_signal = -(timeAfterInput/1000)/1.5 + curr_input_signal;

            if(input_signal < (curr_input_signal/3)){
                sensory_input = false;
            }
        }
        
    }
    if (sensoryTrigger != 0.0){ // There is a trigger to change the behavior for another section
        if(sensory_input == false){ // First time triggered
            if(sensoryTrigger > 0.0){ // Positive behavior change -> change to higher amplitudes
                changeOscillation = 1;
            }
            if(sensoryTrigger < 0.0){ // Negative behavior change -> change to lower amplitudes
                changeOscillation = -1;
            }
            curr_input_signal = input_signal; // Save the current input signal as it will change starting from trigger point
            sensory_start_time = millis(); // save the trigger time to change behavior as the input signal will start changing from this point
            sensory_input = true; // sensory input arrived flag
        }

    }
    // ---------NEW CODE PART-------------------------------------------------------------------

    myNetwork.ode_solver(&myNetwork, DT, input_signal);
    if (mytime > 20000){
        for (int j = 0; j < ASSEMBLE_COUNT; j++) { // THIS IS LINE 391 IN REPOSITORY

            float result = myNetwork.assemble[j].neurons[0].neuronOutput(myNetwork.assemble[j].neurons[0].x) - myNetwork.assemble[j].neurons[1].neuronOutput(myNetwork.assemble[j].neurons[1].x);
            // ------ NOW THIS IS THE ONLY THING WE NEED -> TO CHANGE AMLPITUDE OF MOTOR 4 -> FOR A HIGHER AMPLITUDE IN THE TAIL-------
            if(j == 3){
                result = (result*SCALING_FACTOR1) + OFFSET + 100.0;
            }
            else{
                result = (result*SCALING_FACTOR1) + OFFSET;
            }
            // ---------------------------------------------------------------------
            dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, j+1, ADDR_AX_GOAL_POSITION, result, &dxl_error);
            if(j == 0){
                Serial.println(j);
                Serial.println(result);
            }

        }
    }
    else{
      Serial.println(mytime);
      delay(300);
    }
 
}    
