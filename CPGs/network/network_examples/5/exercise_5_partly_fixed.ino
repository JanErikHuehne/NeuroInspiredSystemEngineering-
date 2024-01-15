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

void test_ode_solver_euler(struct MatsuokaNeuron *neuron, double dt, double input, double *y, int numberNeurons) {
    bool debug = false; 
    if (debug) {
        printf("ODE SOLVER CALLED FOR NEURON %d \n", neuron->id);
    }

    double k1, l1;

    k1 = neuron->neuronDynamics(input, neuron->x_prime, neuron->x, y, neuron->a, neuron->b, numberNeurons, neuron->id);
    l1 = neuron->adaptation(neuron->T, y[neuron->id], neuron->x_prime);

    if (debug) {
        printf("k1: %lf\n", k1);
        printf("l1: %lf\n", l1);
    }

    neuron->x += dt * k1;
    neuron->x_prime += dt * l1;
}

void test_ode_solver_RK2(struct MatsuokaNeuron *neuron, double dt, double input, double *y, int numberNeurons) {
    bool debug = false; 
    if (debug) {
        printf("ODE SOLVER CALLED FOR NEURON %d \n", neuron->id);
    }

    double k1, k2;
    double l1, l2;
   
    k1 = neuron->neuronDynamics(input, neuron->x_prime, neuron->x, y, neuron->a, neuron->b, numberNeurons, neuron->id);
    l1 = neuron->adaptation(neuron->T, y[neuron->id], neuron->x_prime);

    if (debug) {
        printf("k1: %lf\n", k1);
        printf("l1: %lf\n", l1);
    }

    double x_half, x_prime_half;
    x_half = neuron->x + 0.5 * dt * k1;
    x_prime_half = neuron->x_prime + 0.5 * dt * l1;
    y[neuron->id] = neuron->neuronOutput(x_half);

    k2 = neuron->neuronDynamics(input, x_prime_half, x_half, y, neuron->a, neuron->b, numberNeurons, neuron->id);
    l2 = neuron->adaptation(neuron->T, y[neuron->id], x_prime_half);

    if (debug) {
        printf("k2: %lf\n", k2);
        printf("l2: %lf\n", l2);
    }

    neuron->x += dt * (k1 + k2) / 2;
    neuron->x_prime += dt * (l1 +l2) / 2;
}

void setup() {
    Serial.begin(9600);  // Initialize serial communication with baud rate 9600


    
}


void initializeNeuron(struct MatsuokaNeuron *neuron, int numberNeurons, int id) {
        neuron->a = (double *)malloc(numberNeurons * sizeof(double));
        for (int i = 0; i < numberNeurons; i++) {
            neuron->a[i] = 0;
        }
        Serial.print(id);
        Serial.print("\n");
        neuron->n = numberNeurons;
        neuron->T = 12;
        neuron->b = 2.5;
        neuron->x = 0.0;
        neuron->id = 0;
        neuron->x_prime = 0.0;
        neuron->neuronDynamics = matsuoka_neuron_dynamics;
        neuron->neuronOutput = matsuoka_neuron_output;
        neuron->adaptation = matsouka_adaptation;
        neuron->ode_solver = test_ode_solver;
    }

void loop() {

// Fill the remaining elements with desired values
// ...
    static bool initialized = false;
    struct MatsuokaNeuron myNeuron;
    struct MatsuokaNeuron myNeuron2;
    struct MatsuokaNeuron myNeuron3;
    struct MatsuokaNeuron myNeuron4;
    struct MatsuokaNeuron myNeuron5;
    struct MatsuokaNeuron myNeuron6;
    struct MatsuokaNeuron myNeuron7;
    struct MatsuokaNeuron myNeuron8;
    struct MatsuokaNeuron myNeuron9;
    struct MatsuokaNeuron myNeuron10;

    struct MatsuokaNeuron myNeuron11;
    struct MatsuokaNeuron myNeuron12;
    struct MatsuokaNeuron myNeuron13;
    struct MatsuokaNeuron myNeuron14;
    struct MatsuokaNeuron myNeuron15;
    struct MatsuokaNeuron myNeuron16;
    struct MatsuokaNeuron myNeuron17;
    struct MatsuokaNeuron myNeuron18;
    struct MatsuokaNeuron myNeuron19;
    struct MatsuokaNeuron myNeuron20;
    struct MatsuokaNetwork myNetwork;

    if (!initialized) {
        // Initial delay of 3 seconds
    Serial.println(("INITALIZING"));

    int numberNeurons = 20;
    initializeNeuron(&myNeuron, numberNeurons, 1);
    initializeNeuron(&myNeuron2, numberNeurons, 2);
    initializeNeuron(&myNeuron3, numberNeurons, 3);
    initializeNeuron(&myNeuron4, numberNeurons, 4);
    initializeNeuron(&myNeuron5, numberNeurons, 5);
    initializeNeuron(&myNeuron6, numberNeurons, 6);
    initializeNeuron(&myNeuron7, numberNeurons, 7);
    initializeNeuron(&myNeuron8, numberNeurons, 8);
    initializeNeuron(&myNeuron9, numberNeurons, 9);
    initializeNeuron(&myNeuron10, numberNeurons, 10);
    initializeNeuron(&myNeuron11, numberNeurons, 11);
    initializeNeuron(&myNeuron12, numberNeurons, 12);
    initializeNeuron(&myNeuron13, numberNeurons,13);
    initializeNeuron(&myNeuron14, numberNeurons, 14);
    initializeNeuron(&myNeuron15, numberNeurons, 15);
    initializeNeuron(&myNeuron16, numberNeurons, 16);
    initializeNeuron(&myNeuron17, numberNeurons, 17);
    initializeNeuron(&myNeuron18, numberNeurons, 18);
    initializeNeuron(&myNeuron19, numberNeurons, 19);
    initializeNeuron(&myNeuron20, numberNeurons, 20);

    // First Neuron 

    myNeuron.a[9] = 1.5;
    myNeuron.a[8] = 1.5;
    myNeuron.a[7] = 1.5;

    // Second Neuron 
   
    myNeuron2.a[0] = 1.5;
    myNeuron2.a[9] = 1.5;
    myNeuron2.a[8] = 1.5;
    
    // Third Neuron 
    myNeuron3.a[0] = 1.5;
    myNeuron3.a[1] = 1.5;
    myNeuron3.a[9] = 1.5;
    
    
    // Fourth Neuron 
    
    myNeuron4.a[2] = 1.5;
    myNeuron4.a[1] = 1.5;
    myNeuron4.a[0] = 1.5;

    // Fifth Neuron
    myNeuron5.a[3] = 1.5;
    myNeuron5.a[2] = 1.5;
    myNeuron5.a[1] = 1.5;
    

    // Sixth Neuron 
    myNeuron6.a[2] = 1.5;
    myNeuron6.a[3] = 1.5;
    myNeuron6.a[4] = 1.5;
  
    
    // Seventh Neuron   
    myNeuron7.a[3] = 1.5;
    myNeuron7.a[4] = 1.5;
    myNeuron7.a[5] = 1.5;
   

    // Eighth Neuron    
    myNeuron8.a[4] = 1.5;
    myNeuron8.a[5] = 1.5;
    myNeuron8.a[6] = 1.5;
    
    // Ninth Neuron
    myNeuron9.a[5] = 1.5;
    myNeuron9.a[6] = 1.5;
    myNeuron9.a[7] = 1.5;
    
    // Tenth Neuron
    myNeuron10.a[6] = 1.5;
    myNeuron10.a[7] = 1.5;
    myNeuron10.a[8] = 1.5;
    

    // The first 10 neuron inhibit their counterpart
     //myNeuron11.a[0] = 2.5;
     //myNeuron.a[10] = 2.5;
     //myNeuron1.a[11] = 2.5;
     //myNeuron13.a[2] = 2.5;
     //myNeuron2.a[12] = 2.5;
     //myNeuron14.a[3] = 2.5;
     //myNeuron15.a[4] = 2.5;
     //myNeuron16.a[5] = 2.5;
     //myNeuron17.a[6] = 2.5;
     //myNeuron18.a[7] = 2.5;
     //myNeuron19.a[8] = 2.5;
     //myNeuron20.a[9] = 2.5;
    myNetwork.neuronNumber = numberNeurons;

    myNetwork.neurons = (struct MatsuokaNeuron *)malloc(myNetwork.neuronNumber * sizeof(struct MatsuokaNeuron));

    if (myNetwork.neurons == NULL) {
        Serial.println("Memory allocation failed");
        while (1)

            ;  // Stop execution if memory allocation fails
    }

    myNetwork.neurons[0] = myNeuron;
    myNetwork.neurons[1] = myNeuron2;
    myNetwork.neurons[2] = myNeuron3;
    myNetwork.neurons[3] = myNeuron4;
    myNetwork.neurons[4] = myNeuron5;
    myNetwork.neurons[5] = myNeuron6;
    myNetwork.neurons[6] = myNeuron7;
    myNetwork.neurons[7] = myNeuron8;
    myNetwork.neurons[8] = myNeuron9;
    myNetwork.neurons[9] = myNeuron10;
    myNetwork.neurons[10] = myNeuron11;
    myNetwork.neurons[11] = myNeuron12;
    myNetwork.neurons[12] = myNeuron13;
    myNetwork.neurons[13] = myNeuron14;
    myNetwork.neurons[14] = myNeuron15;
    myNetwork.neurons[15] = myNeuron16;
    myNetwork.neurons[16] = myNeuron17;
    myNetwork.neurons[17] = myNeuron18;
    myNetwork.neurons[18] = myNeuron19;
    myNetwork.neurons[19] = myNeuron20;
    myNetwork.y = (double *)malloc(myNetwork.neuronNumber * sizeof(double));

    myNetwork.ode_solver = network_ode_solver;
        delay(300);
        initialized = true;
    }    
    unsigned long startTime = millis();

    for (int i = 0; i < 500; i++) {
        myNetwork.ode_solver(&myNetwork, 0.1, 1);
        for (int j = 0; j < 10; ++j) {
            // Access each neuron using the index i
            struct MatsuokaNeuron *currentNeuron = &(myNetwork.neurons[j]);
            Serial.print("ID");
            Serial.print(currentNeuron->id);
            Serial.print(": ");
            Serial.print(myNetwork.y[j] + j + 0.1);
            Serial.print(" ");
        }
        Serial.println();
        delay(100);
    }
    unsigned long endTime = millis(); // Get the current time in milliseconds
    unsigned long elapsedTime = endTime - startTime; // Calculate the elapsed time in milliseconds


}