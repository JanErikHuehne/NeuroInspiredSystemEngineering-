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
        if ((j+1)!= neuron_id) {
           
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


MatsuokaNeuron generateNeuron(int numberNeurons, int id) {
    struct MatsuokaNeuron newNeuron;
    newNeuron.a = (double *)malloc(numberNeurons * sizeof(double));
     for (int i = 0; i < numberNeurons; i++) {
            newNeuron.a[i] = 0;
        }
    newNeuron.n = numberNeurons;
    newNeuron.T = 12;
    newNeuron.b = 2.5;
    newNeuron.x = 0.0;
    newNeuron.id = id;
    newNeuron.x_prime = 0.0;
    newNeuron.neuronDynamics = matsuoka_neuron_dynamics;
    newNeuron.neuronOutput = matsuoka_neuron_output;
    newNeuron.adaptation = matsouka_adaptation;
    newNeuron.ode_solver = test_ode_solver;
    return newNeuron;
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
    myNeuron =  generateNeuron(numberNeurons, 0);
    myNeuron2 = generateNeuron(numberNeurons, 1);
    myNeuron3 =  generateNeuron(numberNeurons, 2);
    myNeuron4 = generateNeuron(numberNeurons, 3);
    myNeuron5 =  generateNeuron(numberNeurons, 4);
    myNeuron6 = generateNeuron(numberNeurons, 5);
    myNeuron7 =  generateNeuron(numberNeurons, 6);
    myNeuron8 = generateNeuron(numberNeurons, 7);
    myNeuron9 =  generateNeuron(numberNeurons, 8);
    myNeuron10 = generateNeuron(numberNeurons, 9);
    myNeuron11 =  generateNeuron(numberNeurons, 10);
    myNeuron12 = generateNeuron(numberNeurons, 11);
    myNeuron13 =  generateNeuron(numberNeurons, 12);
    myNeuron14 = generateNeuron(numberNeurons, 13);
    myNeuron15 =  generateNeuron(numberNeurons, 14);
    myNeuron16 = generateNeuron(numberNeurons, 15);
    myNeuron17 =  generateNeuron(numberNeurons, 16);
    myNeuron18 = generateNeuron(numberNeurons, 17);
    myNeuron19 =  generateNeuron(numberNeurons, 18);
    myNeuron20 = generateNeuron(numberNeurons, 19);

    float network_weights = 1.5;
    float inhibitor_weights = 2.5;

    // First Neuron 
    myNeuron.a[9] = network_weights;
    myNeuron.a[8] = network_weights;
    myNeuron.a[7] = network_weights;

    // Second Neuron 
    myNeuron2.a[0] = network_weights;
    myNeuron2.a[9] = network_weights;
    myNeuron2.a[8] = network_weights;
    
    // Third Neuron 
    myNeuron3.a[0] = network_weights;
    myNeuron3.a[1] = network_weights;
    myNeuron3.a[9] = network_weights;
    
    // Fourth Neuron 
    myNeuron4.a[2] = network_weights;
    myNeuron4.a[1] = network_weights;
    myNeuron4.a[0] = network_weights;

    // Fifth Neuron
    myNeuron5.a[3] = network_weights;
    myNeuron5.a[2] = network_weights;
    myNeuron5.a[1] = network_weights;
    
    // Sixth Neuron 
    myNeuron6.a[2] = network_weights;
    myNeuron6.a[3] = network_weights;
    myNeuron6.a[4] = network_weights;
  
    // Seventh Neuron   
    myNeuron7.a[3] = network_weights;
    myNeuron7.a[4] = network_weights;
    myNeuron7.a[5] = network_weights;

    // Eighth Neuron    
    myNeuron8.a[4] = network_weights;
    myNeuron8.a[5] = network_weights;
    myNeuron8.a[6] = network_weights;
    
    // Ninth Neuron
    myNeuron9.a[5] = network_weights;
    myNeuron9.a[6] = network_weights;
    myNeuron9.a[7] = network_weights;
    
    // Tenth Neuron
    myNeuron10.a[6] = network_weights;
    myNeuron10.a[7] = network_weights;
    myNeuron10.a[8] = network_weights;
    
    // The first 10 neuron inhibit their counterpart

    myNeuron11.a[0] = inhibitor_weights;
    myNeuron12.a[1] = inhibitor_weights;
    myNeuron13.a[2] = inhibitor_weights;
    myNeuron14.a[3] = inhibitor_weights;
    myNeuron15.a[4] = inhibitor_weights;
    myNeuron16.a[5] = inhibitor_weights;
    myNeuron17.a[6] = inhibitor_weights;
    myNeuron18.a[7] = inhibitor_weights;
    myNeuron19.a[8] = inhibitor_weights;
    myNeuron20.a[9] = inhibitor_weights;

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
        for (int j = 0; j < 20; ++j) {
            // Access each neuron using the index i
            struct MatsuokaNeuron *currentNeuron = &(myNetwork.neurons[j]);
            if(j<10 && j>=0){
              Serial.print("ID");
              Serial.print(currentNeuron->id);
              Serial.print(": ");
              Serial.print(myNetwork.y[j] + j );
              Serial.print(" ");
              //j = j + 1;
            }
            //j = j + 9;
        }
        Serial.println();
        delay(100);
    }
    unsigned long endTime = millis(); // Get the current time in milliseconds
    unsigned long elapsedTime = endTime - startTime; // Calculate the elapsed time in milliseconds

}