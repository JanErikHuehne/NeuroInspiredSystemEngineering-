#include <Arduino.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <random>

#define NEURON_COUNT 3      // Each Servo Motor is controlled by 3 neuron network
#define ASSEMBLE_COUNT 7    // Number of Servo Motors
#define INIT_DELAY 3000
#define SIMULATION_STEPS 2000
#define UPDATE_INTERVAL 100
#define DT 0.1
#define INPUT_SIGNAL 2.5

bool initialized;

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

void setup() {
    Serial.begin(9600);
    delay(INIT_DELAY);
    initialized = false;
}

void loop() {
    static struct MatsuokaNetwork myNetwork;

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

    for (int i = 0; i < SIMULATION_STEPS; i++) {
        myNetwork.ode_solver(&myNetwork, DT, INPUT_SIGNAL);
        for (int j = 0; j < ASSEMBLE_COUNT; j++) {
            Serial.print("ID");
            Serial.print(j);
            Serial.print(": ");
            Serial.print(myNetwork.assemble[j].neurons[0].neuronOutput(myNetwork.assemble[j].neurons[0].x) - myNetwork.assemble[j].neurons[1].neuronOutput(myNetwork.assemble[j].neurons[1].x) );
            Serial.print(" ");
        }
        Serial.println();
        delay(UPDATE_INTERVAL);
    }
    // Right now it takes 1.5 seconds for the network to converge to stable traveling wave pattern. 
}
