#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <sstream>
#include <iostream>


using std::vector;
using std::string;
using std::tuple;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::cin;
using std::cout;


class NeuralNetwork {

public:

    // Reads a file representing the initial neural network
    NeuralNetwork( string );

    // Reads a file representing the training data
    void loadTrainData( string );

    // Writes a file representing the weights of the neural network
    void writeWeights( string );

    // Trains the network according to number of epochs and learning rate
    // Based on Figure 18.24 in the textbook
    void train( int, double );


private:

    // Computes activation of a neuron given the activations of the previous layer and weights of connections
    double activation( vector<double>, vector<double> );

    // Sigmoid function and derivative of Sigmoid function
    double sig( double );
    double sigDeriv( double );

    // Neural Network Representation
    int numInNodes, numHidNodes, numOutNodes;               // Number of nodes per layer
    vector< vector< double > > weightsInHid, weightsHidOut; // Matrix of weights

    // Training Data
    int numTrainEx;
    vector< vector< double > > inputAttributes;
    vector< vector< int > > output;

};
