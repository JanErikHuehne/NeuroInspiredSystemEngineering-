
// Online C compiler to run C program online

#include <Arduino.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

struct MatsuokaNeuron {
    // Function to calculate neuron dynamics
    double (*neuronDynamics)(double s_i, double x_i_prime, double x_i, double *y, double *a_i, double b, int y_length, int neuron_id);

    // Function to calculate neuron output
    double (*neuronOutput)(double x);

    // Function to calculate neuron adaptation
    double (*adaptation)(double T, double y_i, double x_prime);
  
    void (*ode_solver)(struct MatsuokaNeuron *neuron, double dt, double input, double *y, int numberNeurons);
    double x; 
    double x_prime; 
    // Hyperparameters
    double *a; // Array with length n
    double b;
    double T;
    int id;
    int n;
};


struct MatsuokaNetwork {
    struct MatsuokaNeuron *neurons;
    int neuronNumber;
    double *y;
    void (*ode_solver)(struct MatsuokaNetwork *network, double dt, double input);
};

double matsuoka_neuron_dynamics(double s_i, double x_i_prime, double x_i, double *y, double *a_i, double b, int y_length, int neuron_id) {
    bool debug = false;
    
    double result = 0.0; 
    result = s_i - b * x_i_prime - x_i;
    for (int j = 0; j < y_length; ++j) {
        if (j != neuron_id) {
           
            result -= a_i[j] * y[j];
        }
    }
    if  (debug) {
        printf("X derivative for neuron %d : %lf \n",neuron_id, result);
    }
  
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
    // We update the outputs of each neuron here
    for (int i = 0; i < network->neuronNumber; ++i) {
        // Access each neuron using the index i
        struct MatsuokaNeuron *currentNeuron = &(network->neurons[i]);
        double *y_i = &(network->y[i]);
        *y_i = currentNeuron->neuronOutput(currentNeuron->x);
    }

   
    
    // Now we go through every neuron to perform Runge-Kutta udpate
    for (int i = 0; i < network->neuronNumber; ++i) {
        struct MatsuokaNeuron *currentNeuron = &(network->neurons[i]);
        // printf("Neuron ID %d \n", currentNeuron->id),
        currentNeuron->ode_solver(currentNeuron, dt, input, network->y, network->neuronNumber);
    }
 

}
void test_ode_solver(struct MatsuokaNeuron *neuron, double dt, double input, double *y, int numberNeurons) {
    
   
    bool debug = false; 
    if (debug) {
         printf("ODE SOLVER CALLED FOR NEURON %d \n", neuron->id);
    }
    double k1, k2, k3, k4;
    double l1, l2, l3, l4;
   
    k1 = neuron->neuronDynamics(input, // s_i
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
    k2 = neuron->neuronDynamics(input, // s_i
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
    k3 = neuron->neuronDynamics(input, // s_i
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
    k4 = neuron->neuronDynamics(input, // s_i
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

void setup() {
    Serial.begin(9600);  // Initialize serial communication with baud rate 9600


    
}


void loop() {
    static bool initialized = false;
    struct MatsuokaNeuron myNeuron;
    struct MatsuokaNeuron myNeuron2;
    struct MatsuokaNeuron myNeuron3;
    struct MatsuokaNetwork myNetwork;

    if (!initialized) {
        // Initial delay of 3 seconds
    Serial.println(("INITALIZING"));
    int numberNeurons = 3;

    myNeuron.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron.a[0] = 0.0;
    myNeuron.a[1] = 2.5;
    myNeuron.a[2] = 2.5;
    myNeuron.n = numberNeurons;
    myNeuron.T = 12;
    myNeuron.b = 2.5;
    myNeuron.x = 0.0;
    myNeuron.id = 0;
    myNeuron.x_prime = 0.0;
    myNeuron.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron.neuronOutput = matsuoka_neuron_output;
    myNeuron.adaptation = matsouka_adaptation;
    myNeuron.ode_solver = test_ode_solver;

    myNeuron2.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron2.n = numberNeurons;
    myNeuron2.a[0] = 2.5;
    myNeuron2.a[1] = 0.0;
    myNeuron2.a[2] = 2.5;
    myNeuron2.T = 12;
    myNeuron2.b = 2.5;
    myNeuron2.x = 0.0;
    myNeuron2.id = 1;
    myNeuron2.x_prime = 0.0;
    myNeuron2.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron2.neuronOutput = matsuoka_neuron_output;
    myNeuron2.adaptation = matsouka_adaptation;
    myNeuron2.ode_solver = test_ode_solver;


    myNeuron3.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron3.n = numberNeurons;
    myNeuron3.a[0] = 2.5;
    myNeuron3.a[1] = 2.5;
    myNeuron3.a[2] = 0.0;
    myNeuron3.T = 12;
    myNeuron3.b = 2.5;
    myNeuron3.x = 0.0;
    myNeuron3.id = 2;
    myNeuron3.x_prime = 0.0;
    myNeuron3.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron3.neuronOutput = matsuoka_neuron_output;
    myNeuron3.adaptation = matsouka_adaptation;
    myNeuron3.ode_solver = test_ode_solver;

    myNetwork.neuronNumber = 3;

    myNetwork.neurons = (struct MatsuokaNeuron *)malloc(myNetwork.neuronNumber * sizeof(struct MatsuokaNeuron));

    if (myNetwork.neurons == NULL) {
        Serial.println("Memory allocation failed");
        while (1)

            ;  // Stop execution if memory allocation fails
    }

    myNetwork.neurons[0] = myNeuron;
    myNetwork.neurons[1] = myNeuron2;
    myNetwork.neurons[2] = myNeuron3;

    myNetwork.y = (double *)malloc(myNetwork.neuronNumber * sizeof(double));

    myNetwork.ode_solver = network_ode_solver;
        delay(3000);
        initialized = true;
    }    

    for (int i = 0; i < 500; i++) {
        myNetwork.ode_solver(&myNetwork, 0.1, 1);
        for (int j = 0; j < myNetwork.neuronNumber; ++j) {
            // Access each neuron using the index i
            struct MatsuokaNeuron *currentNeuron = &(myNetwork.neurons[j]);
            Serial.print("ID");
            Serial.print(currentNeuron->id);
            Serial.print(": ");
            Serial.print(myNetwork.y[j]);
            Serial.print(" ");
        }
        Serial.println();
        delay(100);
    }
}
