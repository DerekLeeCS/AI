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


// Used for metric locations
namespace metricVals {

   const int METRIC_A = 0;
   const int METRIC_B = 1;
   const int METRIC_C = 2;
   const int METRIC_D = 3;

}


class NeuralNetwork {

public:

    // Reads a file representing the initial neural network
    NeuralNetwork( string );

    // Reads a file representing the training / testing data
    void loadData( string );

    // Writes a file representing the weights of the neural network
    void writeWeights( string );

    // Writes a file representing metrics for each output class
    void writeMetrics( string );

    // Trains the network according to number of epochs and learning rate
    // Based on Figure 18.24 in the textbook
    void train( int, double );

    // Tests the network on a given test file
    void test();


private:

    // Computes activation of a neuron given the activations of the previous layer and weights of connections
    double activation( vector<double>, vector<double> );

    // Prints Overall Accuracy, Precision, Recall, F1
    void otherMetrics( ofstream &, int, int, int, int, bool );
    void printMetrics( ofstream &output, double accuracy, double precision, double recall, double F1 );

    // Trims floating-points to 3 digits after the decimal
    void trimPrecision( ofstream &output, double number );

    // Sigmoid function and derivative of Sigmoid function
    double sig( double );
    double sigDeriv( double );

    // Neural Network Representation
    int numInNodes, numHidNodes, numOutNodes;               // Number of nodes per layer
    vector< vector< double > > weightsInHid, weightsHidOut; // Matrix of weights

    // Dataset
    int numEx;
    vector< vector< double > > inputAttributes;
    vector< vector< int > > output;

    // Calculates Activations
    //   Stores weighted sum of inputs to each layer
    vector< double > inputToHid;
    vector< double > inputToOut;
    //   Stores activations of each layer
    vector< double > activationsInput;
    vector< double > activationsHid;
    vector< double > activationsOutput;

    // Keeps track of metrics for each output class
    //   Stores A,B,C,D
    vector< vector< int > > metric;
    double globAcc=0, globPrecision=0, globRecall=0, globF1;

};
