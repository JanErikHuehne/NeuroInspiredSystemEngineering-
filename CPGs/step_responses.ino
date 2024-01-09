#include <Arduino.h>

struct MatsuokaNeuron {
    double (*neuronDynamics)(double s_i, double x_i_prime, double x_i, double *y, double *a_i, double b, int y_length, bool use_adaption);
    double (*neuronOutput)(double x);
    double (*adaptation)(double T, double y_i, double x_prime);
    void (*ode_solver)(struct MatsuokaNeuron *neuron, double dt, double input);
    double x;
    double x_prime;
    double *a;
    double b;
    double T;
    int n;
};

double matsuoka_neuron_dynamics(double s_i, double x_i_prime, double x_i, double *y, double *a_i, double b, int y_length, bool use_adaption) {
    double result = 0.0;
    if (use_adaption) {
        result = s_i - b * x_i_prime - x_i;
    } else {
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
    } else {
        return 0;
    }
}

double matsouka_adaptation(double T, double y_i, double x_prime) {
    return (1 / T) * (y_i - x_prime);
}

void test_ode_solver(struct MatsuokaNeuron *neuron, double dt, double input) {
    bool debug = false;
    double k1, k2, k3, k4;
    double l1, l2, l3, l4;
    double a = 0.0;
    double y = neuron->neuronOutput(neuron->x);
    k1 = neuron->neuronDynamics(input, neuron->x_prime, neuron->x, &y, &a, neuron->b, 1, true);
    l1 = neuron->adaptation(neuron->T, y, neuron->x_prime);

    double x_half, x_prime_half;
    x_half = neuron->x + 0.5 * dt * k1;
    x_prime_half = neuron->x_prime + 0.5 * dt * l1;
    y = neuron->neuronOutput(x_half);
    k2 = neuron->neuronDynamics(input, x_prime_half, x_half, &y, &a, neuron->b, 1, true);
    l2 = neuron->adaptation(neuron->T, y, x_prime_half);

    x_half = neuron->x + 0.5 * dt * k2;
    x_prime_half = neuron->x_prime + 0.5 * dt * l2;
    y = neuron->neuronOutput(x_half);
    k3 = neuron->neuronDynamics(input, x_prime_half, x_half, &y, &a, neuron->b, 1, true);
    l3 = neuron->adaptation(neuron->T, y, x_prime_half);

    x_half = neuron->x + dt * k3;
    x_prime_half = neuron->x_prime + dt * l3;
    y = neuron->neuronOutput(x_half);
    k4 = neuron->neuronDynamics(input, x_prime_half, x_half, &y, &a, neuron->b, 1, true);
    l4 = neuron->adaptation(neuron->T, y, x_prime_half);

    neuron->x += (k1 + 2 * k2 + 2 * k3 + k4) * (dt / 6);
    neuron->x_prime += (l1 + 2 * l2 + 2 * l3 + l4) * (dt / 6);
}

// Define the MatsuokaNeuron instance
struct MatsuokaNeuron myNeuron;
struct MatsuokaNeuron myNeuron2;
struct MatsuokaNeuron myNeuron3;

void setup() {
    // Start the Serial communication
    Serial.begin(9600);

    // Initialize the MatsuokaNeuron instance
    myNeuron.T = 12;
    myNeuron.b = 2.5;
    myNeuron.x = 0.0;
    myNeuron.x_prime = 0.0;
    myNeuron.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron.neuronOutput = matsuoka_neuron_output;
    myNeuron.adaptation = matsouka_adaptation;
    myNeuron.ode_solver = test_ode_solver;


    myNeuron2.T = 12;
    myNeuron2.b = 0.0;
    myNeuron2.x = 0.0;
    myNeuron2.x_prime = 0.0;
    myNeuron2.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron2.neuronOutput = matsuoka_neuron_output;
    myNeuron2.adaptation = matsouka_adaptation;
    myNeuron2.ode_solver = test_ode_solver;

   
    myNeuron3.b = 8;
    myNeuron3.T = 12;
    myNeuron3.x = 0.0;
    myNeuron3.x_prime = 0.0;
    myNeuron3.neuronDynamics = matsuoka_neuron_dynamics;
    myNeuron3.neuronOutput = matsuoka_neuron_output;
    myNeuron3.adaptation = matsouka_adaptation;
    myNeuron3.ode_solver = test_ode_solver;
}

void loop() {
  delay(3000);
    // Run the ode_solver function in a loop and print x to Serial Monitor
    for (int i = 0; i < 1000; ++i) {
        // We rescale each output of a factor of 10 for better visualization on 
        // the serial plotter
        Serial.print("x1:");
        Serial.print(10*myNeuron.x);
        Serial.print(",");
        Serial.print("x2:");
        Serial.print(10*myNeuron2.x);
        Serial.print(",");
        Serial.print("x3:");
        Serial.print(10*myNeuron3.x);
        Serial.println();

        // Call ode_solver function with dt = 0.1 and input = 1.0
        myNeuron.ode_solver(&myNeuron, 0.1, 1.0);
        myNeuron2.ode_solver(&myNeuron2, 0.1, 1.0);
        myNeuron3.ode_solver(&myNeuron3, 0.1, 1.0);
        delay(100);
    }

    // Stop further execution
    while (true) {
        // Do nothing, loop indefinitely
    }
}