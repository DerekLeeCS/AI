#include "neuralNetwork.h"
#include <iomanip>
#include <cmath>

using std::make_tuple;


// Reads a file representing the initial neural network
NeuralNetwork::NeuralNetwork( string fileName ) {

    ifstream inputWeights( fileName );

    inputWeights >> this->numInNodes >> this->numHidNodes >> this->numOutNodes;

    // Initializes matrix of weights
    // Stores the weights of edges
    this->weightsInHid = vector< vector<double> >( this->numHidNodes, vector<double>( this->numInNodes+1 ) );
    this->weightsHidOut = vector< vector<double> >( this->numOutNodes, vector<double>( this->numHidNodes+1 ) );

    // Reads weights of edges from input layer to hidden layer
    for ( int i=0; i<this->numHidNodes; i++ ) {
        for ( int j=0; j<this->numInNodes+1; j++ )
            inputWeights >> this->weightsInHid[i][j];
    }

    // Reads weights of edges from hidden layer to output layer
    for ( int i=0; i<this->numOutNodes; i++ ) {
        for ( int j=0; j<this->numHidNodes+1; j++ )
            inputWeights >> this->weightsHidOut[i][j];
    }

    inputWeights.close();

}


// Reads a file representing the training data
void NeuralNetwork::loadTrainData( string fileName ) {

    ifstream inputTrain( fileName );

    int _checkInNodes, _checkOutNodes;
    inputTrain >> this->numTrainEx >> _checkInNodes >> _checkOutNodes;

    // Verify weight file matches training file
    if ( this->numInNodes != _checkInNodes || this->numOutNodes != _checkOutNodes ) {

        std::cerr << "Error: Number of nodes in weight file does not match number of nodes in training file! ("
                  << this->numInNodes << "," << _checkInNodes << ") ("
                  << this->numOutNodes << "," << _checkOutNodes << ")" << "\n";
        return;

    }

    // Initializes size of matrices
    this->inputAttributes = vector< vector<double> >( numTrainEx, vector<double>( this->numInNodes ) );
    this->output = vector< vector<int> >( numTrainEx, vector<int>( this->numOutNodes ) );

    // Reads input and output data
    for ( int i=0; i<this->numTrainEx; i++ ) {

        for ( int j=0; j<this->numInNodes; j++ )
            inputTrain >> this->inputAttributes[i][j];

        for ( int j=0; j<this->numOutNodes; j++ )
            inputTrain >> this->output[i][j];

    }

    inputTrain.close();

}


// Writes a file representing the weights of the neural network
void NeuralNetwork::writeWeights( string fileName ) {

    ofstream outputWeights( fileName );
    stringstream stream;

    outputWeights << this->numInNodes << " " << this->numHidNodes << " " << this->numOutNodes << "\n";

    for ( int i=0; i<numHidNodes; i++ ) {

        for ( int j=0; j<numInNodes+1; j++ ) {

            stream << std::fixed << std::setprecision(3) << this->weightsInHid[i][j];
            outputWeights << stream.str() << " ";
            stream.str(""); // Reset the stream

        }

        outputWeights << "\n";

    }

    for ( int i=0; i<numOutNodes; i++ ) {

        for ( int j=0; j<numHidNodes+1; j++ ) {

            stream << std::fixed << std::setprecision(3) << this->weightsHidOut[i][j];
            outputWeights << stream.str() << " ";
            stream.str(""); // Reset the stream

        }

        outputWeights << "\n";

    }

    outputWeights.close();

}


// Trains the network according to number of epochs and learning rate
// Based on Figure 18.24 in the textbook
void NeuralNetwork::train( int epochs, double learnRate ) {

    // Stores weighted sum of inputs to each layer
    vector< double > inputToHid = vector< double >( numHidNodes );
    vector< double > inputToOut = vector< double >( numOutNodes );

    // Stores activations of each layer
    vector< double > activationsInput = vector< double >( numInNodes+1 );
    vector< double > activationsHid = vector< double >( numHidNodes+1 );
    vector< double > activationsOutput = vector< double >( numOutNodes );

    // Stores error
    vector< double > deltaOut = vector< double >( numOutNodes );
    vector< double > deltaHid = vector< double >( numHidNodes );

    int nodeNum;

    // Train for specified number of epochs
    for ( int iteration=0; iteration<epochs; iteration++ ) {

        for ( int trainEx=0; trainEx<this->numTrainEx; trainEx++ ) {

            // Propagate the inputs forward to compute the outputs
            for ( nodeNum=0; nodeNum<this->numInNodes; nodeNum++ )
                activationsInput[ nodeNum+1 ] = this->inputAttributes[ trainEx ][ nodeNum ];

            // Fixed input of -1
            activationsInput[0] = -1;
            activationsHid[0] = -1;

            for ( nodeNum=0; nodeNum<this->numHidNodes; nodeNum++ ) {

                inputToHid[ nodeNum ] = this->activation( activationsInput, this->weightsInHid[ nodeNum ] );
                activationsHid[ nodeNum+1 ] = this->sig( inputToHid[ nodeNum ] );

            }

            for ( nodeNum=0; nodeNum<this->numOutNodes; nodeNum++ ) {

                inputToOut[ nodeNum ] = this->activation( activationsHid, this->weightsHidOut[ nodeNum ] );
                activationsOutput[ nodeNum ] = this->sig( inputToOut[ nodeNum ] );

            }

            // Reset error vectors
            std::fill( deltaOut.begin(), deltaOut.end(), 0 );
            std::fill( deltaHid.begin(), deltaHid.end(), 0 );

            // Propagate deltas backward from output layer to input layer
            for ( nodeNum=0; nodeNum<this->numOutNodes; nodeNum++ )
                deltaOut[ nodeNum ] = sigDeriv( inputToOut[ nodeNum ] ) * ( this->output[ trainEx ][ nodeNum ] - activationsOutput[ nodeNum ] );

            for ( nodeNum=0; nodeNum<this->numHidNodes; nodeNum++ ) {

                // Uses nodeNum+1 in weightsHidOut[j][ nodeNum+1 ]
                //   because there is no input to the dummy node (activation -1) to backpropagate to
                for ( int j=0; j<this->numOutNodes; j++ )
                    deltaHid[ nodeNum ] += ( weightsHidOut[j][ nodeNum+1 ] * deltaOut[j] );

                deltaHid[ nodeNum ] *= sigDeriv( inputToHid[ nodeNum ] );

            }

            // Update every weight in network using deltas
            for ( int i=0; i<this->numHidNodes+1; i++ ) {

                for ( int j=0; j<this->numOutNodes; j++ )
                    this->weightsHidOut[j][i] += learnRate * activationsHid[i] * deltaOut[j];

            }

            for ( int i=0; i<this->numInNodes+1; i++ ) {

                for ( int j=0; j<this->numHidNodes; j++ )
                    this->weightsInHid[j][i] += learnRate * activationsInput[i] * deltaHid[j];

            }

        }

    }

    return;

}


// Computes activation of a neuron given the activations of the previous layer and weights of connections
double NeuralNetwork::activation( vector<double> prevActivations, vector<double> weights ) {

    // Make sure both arrays are the same size
    int numPrevNeurons = prevActivations.size();
    int _numWeights = weights.size();

    if ( numPrevNeurons != _numWeights ) {

        std::cerr << "Error: Size of previous layer and weights is not the same! ("
                  << numPrevNeurons << "," << _numWeights << ")" << "\n";
        exit(EXIT_FAILURE);

    }

    double neuronActivation = 0;

    for ( int i=0; i<numPrevNeurons; i++ )
        neuronActivation += prevActivations[i] * weights[i];

    return neuronActivation;

}



// Sigmoid function to calculate output of perceptron
double NeuralNetwork::sig( double x ) {

    return 1 / ( 1 + exp(-x) );

}

// Derivative of Sigmoid function used for updating weights
double NeuralNetwork::sigDeriv( double x ) {

    return sig(x) * ( 1 - sig(x) );

}
