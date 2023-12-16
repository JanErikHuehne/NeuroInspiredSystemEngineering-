// Online C compiler to run C program online
#include <stdio.h>
#include <stdbool.h>


struct MatsuokaNeuron {
    // Function to calculate neuron dynamics
    double (*neuronDynamics)(double s_i, double x_i_prime, double x_i, double *y, double *a_i, double b, int y_length, bool use_adaption);

    // Function to calculate neuron output
    double (*neuronOutput)(double x);

    // Function to calculate neuron adaptation
    double (*adaptation)(double T, double y_i, double x_prime);
  
    void (*ode_solver)(struct MatsuokaNeuron *neuron, double dt, double input);
    double x; 
    double x_prime; 
    // Hyperparameters
    double *a; // Array with length n
    double b;
    double T;
    int n;
};

double matsuoka_neuron_dynamics(double s_i, double x_i_prime, double x_i, double *y, double *a_i, double b, int y_length, bool use_adaption) {
    double result = 0.0; 
    if (use_adaption) {
         result = s_i - b * x_i_prime - x_i;
    }
    
    else {
        result = s_i - x_i;
    }
    for (int j = 0; j < y_length; ++j) {
        result -= a_i[j] * y[j];
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

void test_ode_solver(struct MatsuokaNeuron *neuron, double dt, double input) {
    bool debug = false; 
    double k1, k2, k3, k4;
    double l1, l2, l3, l4;
    double a = 0.0;
    double y = neuron->neuronOutput(neuron->x);
    k1 = neuron->neuronDynamics(input, // s_i
                               neuron->x_prime, // x_i_prime
                               neuron->x,
                               &y, // y*
                               &a, // *a_i
                               neuron->b, // b
                               1, // y_length
                               true // use_adaption
                               );
    l1 = neuron->adaptation(neuron->T, y, neuron->x_prime);
    if (debug) {
         printf("k1: %lf\n", k1);
    printf("l1: %lf\n", l1);
    }
   
    
    double x_half, x_prime_half;
    x_half = neuron->x +  0.5* dt * k1;
    x_prime_half = neuron->x_prime +  0.5* dt * l1;
    y = neuron->neuronOutput(x_half);
    k2 = neuron->neuronDynamics(input, // s_i
                               x_prime_half, // x_i_prime
                               x_half,
                               &y, // y*
                               &a, // *a_i
                               neuron->b, // b
                               1, // y_length
                               true // use_adaption
                               );
    l2 = neuron->adaptation(neuron->T, y, x_prime_half);
    if (debug) {
        printf("k2: %lf\n", k2);
        printf("l2: %lf\n", l2);  
    }
    x_half = neuron->x + 0.5* dt * k2;
    x_prime_half = neuron->x_prime +  0.5* dt * l2;
    y = neuron->neuronOutput(x_half);
    k3 = neuron->neuronDynamics(input, // s_i
                               x_prime_half, // x_i_prime
                               x_half,
                               &y, // y*
                               &a, // *a_i
                               neuron->b, // b
                               1, // y_length
                               true // use_adaption
                               );
    l3 = neuron->adaptation(neuron->T, y, x_prime_half);
    if (debug) {
        printf("k3: %lf\n", k3);
        printf("l3: %lf\n", l3); 
    }
  
    x_half = neuron->x + dt * k3;
    x_prime_half = neuron->x_prime +  dt * l3;
    y = neuron->neuronOutput(x_half);
    k4 = neuron->neuronDynamics(input, // s_i
                               x_prime_half, // x_i_prime
                               x_half,
                               &y, // y*
                               &a, // *a_i
                               neuron->b, // b
                               1, // y_length
                               true // use_adaption
                               );
    l4 = neuron->adaptation(neuron->T, y, x_prime_half);
    if (debug) {
        printf("k4: %lf\n", k4);
        printf("l4: %lf\n", l4);   
    }
   
    
    neuron->x += (k1 + 2*k2 + 2*k3 + k4) * (dt/6);
    neuron->x_prime += (l1 + 2*l2 + 2*l3 + l4) * (dt/6);
}
int main() {
    // Write C code here
    struct MatsuokaNeuron myNeuron;
    myNeuron.T = 12;
    myNeuron.b = 10;
    myNeuron.x = 0.0;
    myNeuron.x_prime = 0.0;
    myNeuron.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron.neuronOutput = matsuoka_neuron_output;
    myNeuron.adaptation = matsouka_adaptation;
    myNeuron.ode_solver = test_ode_solver;
     for (int i = 0; i < 1000; ++i) {
        printf("x: %lf\n", myNeuron.x);
        printf("x_prime: %lf\n", myNeuron.x_prime);
        myNeuron.ode_solver(&myNeuron, 0.1 , 1.0);
    }
    return 0;
}