
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

    myNeuron.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron.a[0] = 0.0;
    myNeuron.a[1] = 0.0;
    myNeuron.a[2] = 0.0;
    myNeuron.a[3] = 0.0;
    myNeuron.a[4] = 0.0;
    myNeuron.a[5] = 0.0;
    myNeuron.a[6] = 0.0;
    myNeuron.a[7] = 1.5;
    myNeuron.a[8] = 1.5;
    myNeuron.a[9] = 1.5;
    for (int k = 10; k < 20; k++) {
        myNeuron.a[k] = 0.0;
    }
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
    myNeuron2.a[0] = 1.5;
    myNeuron2.a[1] = 0.0;
    myNeuron2.a[2] = 0.0;
    myNeuron2.a[3] = 0.0;
    myNeuron2.a[4] = 0.0;
    myNeuron2.a[5] = 0.0;
    myNeuron2.a[6] = 0.0;
    myNeuron2.a[7] = 0.0;
    myNeuron2.a[8] = 1.5;
    myNeuron2.a[9] = 1.5;
    myNeuron2.T = 12;
    myNeuron2.b = 2.5;
    myNeuron2.x = 0.0;
    myNeuron2.id = 1;
    myNeuron2.x_prime = 0.0;
    myNeuron2.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron2.neuronOutput = matsuoka_neuron_output;
    myNeuron2.adaptation = matsouka_adaptation;
    myNeuron2.ode_solver = test_ode_solver;
    for (int k = 10; k < 20; k++) {
        myNeuron2.a[k] = 0.0;
    }
  

    myNeuron3.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron3.n = numberNeurons;
    myNeuron3.a[0] = 1.5;
    myNeuron3.a[1] = 1.5;
    myNeuron3.a[2] = 0.0;
    myNeuron3.a[3] = 0.0;
    myNeuron3.a[4] = 0.0;
    myNeuron3.a[5] = 0.0;
    myNeuron3.a[6] = 0.0;
    myNeuron3.a[7] = 0.0;
    myNeuron3.a[8] = 0.0;
    myNeuron3.a[9] = 1.5;
    myNeuron3.T = 12;
    myNeuron3.b = 2.5;
    myNeuron3.x = 0.0;
    myNeuron3.id = 2;
    myNeuron3.x_prime = 0.0;
    myNeuron3.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron3.neuronOutput = matsuoka_neuron_output;
    myNeuron3.adaptation = matsouka_adaptation;
    myNeuron3.ode_solver = test_ode_solver;
     for (int k = 10; k < 20; k++) {
        myNeuron3.a[k] = 0.0;
    }
   
    myNeuron4.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron4.n = numberNeurons;
    myNeuron4.a[0] = 1.5;
    myNeuron4.a[1] = 1.5;
    myNeuron4.a[2] = 1.5;
    myNeuron4.a[3] = 0.0;
    myNeuron4.a[4] = 0.0;
    myNeuron4.a[5] = 0.0;
    myNeuron4.a[6] = 0.0;
    myNeuron4.a[7] = 0.0;
    myNeuron4.a[8] = 0.0;
    myNeuron4.a[9] = 0.0;
    myNeuron4.T = 12;
    myNeuron4.b = 2.5;
    myNeuron4.x = 0.0;
    myNeuron4.id = 3;
    myNeuron4.x_prime = 0.0;
    myNeuron4.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron4.neuronOutput = matsuoka_neuron_output;
    myNeuron4.adaptation = matsouka_adaptation;
    myNeuron4.ode_solver = test_ode_solver;
     for (int k = 10; k < 20; k++) {
        myNeuron4.a[k] = 0.0;
    }
   

    myNeuron5.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron5.n = numberNeurons;
    myNeuron5.a[0] = 0.0;
    myNeuron5.a[1] = 1.5;
    myNeuron5.a[2] = 1.5;
    myNeuron5.a[3] = 1.5;
    myNeuron5.a[4] = 0.0;
    myNeuron5.a[5] = 0.0;
    myNeuron5.a[6] = 0.0;
    myNeuron5.a[7] = 0.0;
    myNeuron5.a[8] = 0.0;
    myNeuron5.a[9] = 0.0;
    myNeuron5.T = 12;
    myNeuron5.b = 2.5;
    myNeuron5.x = 0.0;
    myNeuron5.id = 4;
    myNeuron5.x_prime = 0.0;
    myNeuron5.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron5.neuronOutput = matsuoka_neuron_output;
    myNeuron5.adaptation = matsouka_adaptation;
    myNeuron5.ode_solver = test_ode_solver;
    for (int k = 10; k < 20; k++) {
        myNeuron5.a[k] = 0.0;
    }
   

    myNeuron6.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron6.n = numberNeurons;
    myNeuron6.a[0] = 0.0;
    myNeuron6.a[1] = 0.0;
    myNeuron6.a[2] = 1.5;
    myNeuron6.a[3] = 1.5;
    myNeuron6.a[4] = 1.5;
    myNeuron6.a[5] = 0.0;
    myNeuron6.a[6] = 0.0;
    myNeuron6.a[7] = 0.0;
    myNeuron6.a[8] = 0.0;
    myNeuron6.a[9] = 0.0;
    myNeuron6.T = 12;
    myNeuron6.b = 2.5;
    myNeuron6.x = 0.0;
    myNeuron6.id = 5;
    myNeuron6.x_prime = 0.0;
    myNeuron6.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron6.neuronOutput = matsuoka_neuron_output;
    myNeuron6.adaptation = matsouka_adaptation;
    myNeuron6.ode_solver = test_ode_solver;
    for (int k = 10; k < 20; k++) {
        myNeuron6.a[k] = 0.0;
    }
   

    myNeuron7.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron7.n = numberNeurons;
    myNeuron7.a[0] = 0.0;
    myNeuron7.a[1] = 0.0;
    myNeuron7.a[2] = 0.0;
    myNeuron7.a[3] = 1.5;
    myNeuron7.a[4] = 1.5;
    myNeuron7.a[5] = 1.5;
    myNeuron7.a[6] = 0.0;
    myNeuron7.a[7] = 0.0;
    myNeuron7.a[8] = 0.0;
    myNeuron7.a[9] = 0.0;
    myNeuron7.T = 12;
    myNeuron7.b = 2.5;
    myNeuron7.x = 0.0;
    myNeuron7.id = 6;
    myNeuron7.x_prime = 0.0;
    myNeuron7.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron7.neuronOutput = matsuoka_neuron_output;
    myNeuron7.adaptation = matsouka_adaptation;
    myNeuron7.ode_solver = test_ode_solver;
    for (int k = 10; k < 20; k++) {
        myNeuron7.a[k] = 0.0;
    }
   

    myNeuron8.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron8.n = numberNeurons;
    myNeuron8.a[0] = 0.0;
    myNeuron8.a[1] = 0.0;
    myNeuron8.a[2] = 0.0;
    myNeuron8.a[3] = 0.0;
    myNeuron8.a[4] = 1.5;
    myNeuron8.a[5] = 1.5;
    myNeuron8.a[6] = 1.5;
    myNeuron8.a[7] = 0.0;
    myNeuron8.a[8] = 0.0;
    myNeuron8.a[9] = 0.0;
    myNeuron8.T = 12;
    myNeuron8.b = 2.5;
    myNeuron8.x = 0.0;
    myNeuron8.id = 7;
    myNeuron8.x_prime = 0.0;
    myNeuron8.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron8.neuronOutput = matsuoka_neuron_output;
    myNeuron8.adaptation = matsouka_adaptation;
    myNeuron8.ode_solver = test_ode_solver;
    for (int k = 10; k < 20; k++) {
        myNeuron8.a[k] = 0.0;
    }
  

    myNeuron9.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron9.n = numberNeurons;
    myNeuron9.a[0] = 0.0;
    myNeuron9.a[1] = 0.0;
    myNeuron9.a[2] = 0.0;
    myNeuron9.a[3] = 0.0;
    myNeuron9.a[4] = 0.0;
    myNeuron9.a[5] = 1.5;
    myNeuron9.a[6] = 1.5;
    myNeuron9.a[7] = 1.5;
    myNeuron9.a[8] = 0.0;
    myNeuron9.a[9] = 0.0;
    myNeuron9.T = 12;
    myNeuron9.b = 2.5;
    myNeuron9.x = 0.0;
    myNeuron9.id = 8;
    myNeuron9.x_prime = 0.0;
    myNeuron9.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron9.neuronOutput = matsuoka_neuron_output;
    myNeuron9.adaptation = matsouka_adaptation;
    myNeuron9.ode_solver = test_ode_solver;
    for (int k = 10; k < 20; k++) {
        myNeuron9.a[k] = 0.0;
    }
   

    myNeuron10.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron10.n = numberNeurons;
    myNeuron10.a[0] = 0.0;
    myNeuron10.a[1] = 0.0;
    myNeuron10.a[2] = 0.0;
    myNeuron10.a[3] = 0.0;
    myNeuron10.a[4] = 0.0;
    myNeuron10.a[5] = 0.0;
    myNeuron10.a[6] = 1.5;
    myNeuron10.a[7] = 1.5;
    myNeuron10.a[8] = 1.5;
    myNeuron10.a[9] = 0.0;
    myNeuron10.T = 12;
    myNeuron10.b = 2.5;
    myNeuron10.x = 0.0;
    myNeuron10.id = 9;
    myNeuron10.x_prime = 0.0;
    myNeuron10.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron10.neuronOutput = matsuoka_neuron_output;
    myNeuron10.adaptation = matsouka_adaptation;
    myNeuron10.ode_solver = test_ode_solver;
    for (int k = 10; k < 20; k++) {
        myNeuron10.a[k] = 0.0;
    }
  

    myNeuron11.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron11.n = numberNeurons;
    for (int k = 1; k < 20; k++) {
        myNeuron11.a[k] = 0.0;
    }
    myNeuron11.a[0] = 2.5;
    myNeuron11.T = 12;
    myNeuron11.b = 2.5;
    myNeuron11.x = 0.0;
    myNeuron11.id = 10;
    myNeuron11.x_prime = 0.0;
    myNeuron11.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron11.neuronOutput = matsuoka_neuron_output;
    myNeuron11.adaptation = matsouka_adaptation;
    myNeuron11.ode_solver = test_ode_solver;
  

    myNeuron12.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron12.n = numberNeurons;
    for (int k = 0; k < 20; k++) {
        myNeuron12.a[k] = 0.0;
    }
    myNeuron12.a[1] = 2.5;
    myNeuron12.T = 12;
    myNeuron12.b = 2.5;
    myNeuron12.x = 0.0;
    myNeuron12.id = 11;
    myNeuron12.x_prime = 0.0;
    myNeuron12.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron12.neuronOutput = matsuoka_neuron_output;
    myNeuron12.adaptation = matsouka_adaptation;
    myNeuron12.ode_solver = test_ode_solver;
  

    myNeuron13.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron13.n = numberNeurons;
    for (int k = 0; k < 20; k++) {
        myNeuron13.a[k] = 0.0;
    }    
    myNeuron13.a[2] = 2.5;
    myNeuron13.T = 12;
    myNeuron13.b = 2.5;
    myNeuron13.x = 0.0;
    myNeuron13.id = 12;
    myNeuron13.x_prime = 0.0;
    myNeuron13.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron13.neuronOutput = matsuoka_neuron_output;
    myNeuron13.adaptation = matsouka_adaptation;
    myNeuron13.ode_solver = test_ode_solver;
    

    myNeuron14.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron14.n = numberNeurons;
    for (int k = 0; k < 20; k++) {
        myNeuron14.a[k] = 0.0;
    }    
    myNeuron14.a[3] = 2.5;
    myNeuron14.T = 12;
    myNeuron14.b = 2.5;
    myNeuron14.x = 0.0;
    myNeuron14.id = 13;
    myNeuron14.x_prime = 0.0;
    myNeuron14.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron14.neuronOutput = matsuoka_neuron_output;
    myNeuron14.adaptation = matsouka_adaptation;
    myNeuron14.ode_solver = test_ode_solver;
   

    myNeuron15.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron15.n = numberNeurons;
    for (int k = 0; k < 20; k++) {
        myNeuron15.a[k] = 0.0;
    }    
    myNeuron15.a[4] = 2.5;
    myNeuron15.T = 12;
    myNeuron15.b = 2.5;
    myNeuron15.x = 0.0;
    myNeuron15.id = 14;
    myNeuron15.x_prime = 0.0;
    myNeuron15.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron15.neuronOutput = matsuoka_neuron_output;
    myNeuron15.adaptation = matsouka_adaptation;
    myNeuron15.ode_solver = test_ode_solver;
   

    myNeuron16.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron16.n = numberNeurons;
    for (int k = 0; k < 20; k++) {
        myNeuron16.a[k] = 0.0;
    }    
    myNeuron16.a[5] = 2.5;
    myNeuron16.T = 12;
    myNeuron16.b = 2.5;
    myNeuron16.x = 0.0;
    myNeuron16.id = 15;
    myNeuron16.x_prime = 0.0;
    myNeuron16.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron16.neuronOutput = matsuoka_neuron_output;
    myNeuron16.adaptation = matsouka_adaptation;
    myNeuron16.ode_solver = test_ode_solver;
   


    myNeuron17.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron17.n = numberNeurons;
    for (int k = 0; k < 20; k++) {
        myNeuron17.a[k] = 0.0;
    }    
    myNeuron17.a[6] = 2.5;
    myNeuron17.T = 12;
    myNeuron17.b = 2.5;
    myNeuron17.x = 0.0;
    myNeuron17.id = 16;
    myNeuron17.x_prime = 0.0;
    myNeuron17.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron17.neuronOutput = matsuoka_neuron_output;
    myNeuron17.adaptation = matsouka_adaptation;
    myNeuron17.ode_solver = test_ode_solver;
   

    myNeuron18.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron18.n = numberNeurons;
    for (int k = 0; k < 20; k++) {
        myNeuron18.a[k] = 0.0;
    }    
    myNeuron18.a[7] = 2.5;
    myNeuron18.T = 12;
    myNeuron18.b = 2.5;
    myNeuron18.x = 0.0;
    myNeuron18.id = 17;
    myNeuron18.x_prime = 0.0;
    myNeuron18.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron18.neuronOutput = matsuoka_neuron_output;
    myNeuron18.adaptation = matsouka_adaptation;
    myNeuron18.ode_solver = test_ode_solver;
   

    myNeuron19.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron19.n = numberNeurons;
    for (int k = 0; k < 20; k++) {
        myNeuron19.a[k] = 0.0;
    }    
    myNeuron19.a[8] = 2.5;
    myNeuron19.T = 12;
    myNeuron19.b = 2.5;
    myNeuron19.x = 0.0;
    myNeuron19.id = 18;
    myNeuron19.x_prime = 0.0;
    myNeuron19.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron19.neuronOutput = matsuoka_neuron_output;
    myNeuron19.adaptation = matsouka_adaptation;
    myNeuron19.ode_solver = test_ode_solver;

    myNeuron20.a = (double *)malloc(numberNeurons * sizeof(double));
    myNeuron20.n = numberNeurons;
    for (int k = 0; k < 20; k++) {
        myNeuron20.a[k] = 0.0;
    }    
    myNeuron20.a[9] = 2.5;
    myNeuron20.T = 12;
    myNeuron20.b = 2.5;
    myNeuron20.x = 0.0;
    myNeuron20.id = 19;
    myNeuron20.x_prime = 0.0;
    myNeuron20.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron20.neuronOutput = matsuoka_neuron_output;
    myNeuron20.adaptation = matsouka_adaptation;
    myNeuron20.ode_solver = test_ode_solver;

    myNetwork.neuronNumber = 20;
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
        delay(3000);
        initialized = true;
    }    

    for (int i = 0; i < 2000; i++) {
        myNetwork.ode_solver(&myNetwork, 0.1, 1);
        /*
        struct MatsuokaNeuron *currentNeuron = &(myNetwork.neurons[0]);
        Serial.print("ID");
        Serial.print(currentNeuron->id);
        Serial.print(": ");
        Serial.print(myNetwork.y[0]);
        Serial.print(" ");
        currentNeuron = &(myNetwork.neurons[10]);
        Serial.print("ID");
        Serial.print(currentNeuron->id);
        Serial.print(": ");
        Serial.print(myNetwork.y[10]);
        Serial.print(" ");
        currentNeuron = &(myNetwork.neurons[1]);
        Serial.print("ID");
        Serial.print(currentNeuron->id);
        Serial.print(": ");
        Serial.print(myNetwork.y[1] + 1 );
        Serial.print(" ");
        currentNeuron = &(myNetwork.neurons[11]);
        Serial.print("ID");
        Serial.print(currentNeuron->id);
        Serial.print(": ");
        Serial.print(myNetwork.y[11]+ 1);
        Serial.print(" ");
        currentNeuron = &(myNetwork.neurons[2]);
        Serial.print("ID");
        Serial.print(currentNeuron->id);
        Serial.print(": ");
        Serial.print(myNetwork.y[2]+ 2);
        Serial.print(" ");
        currentNeuron = &(myNetwork.neurons[12]);
        Serial.print("ID");
        Serial.print(currentNeuron->id);
        Serial.print(": ");
        Serial.print(myNetwork.y[12]+ 2);
        Serial.print(" ");
        */
        for (int j = 0; j < 10; ++j) {
            // Access each neuron using the index i
            struct MatsuokaNeuron *currentNeuron = &(myNetwork.neurons[j]);
            Serial.print("ID");
            Serial.print(currentNeuron->id);
            Serial.print(": ");
            Serial.print(myNetwork.y[j]  - myNetwork.y[j+10] + j);
            Serial.print(" ");
        }
        Serial.println();
        delay(100);
    }
}
