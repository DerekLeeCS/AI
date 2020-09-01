#include "neuralNetwork.h"
#include <iomanip>
#include <cmath>

using namespace metricVals;
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

    // Initializes size of vectors
    this->inputToHid = vector< double >( this->numHidNodes );
    this->inputToOut = vector< double >( this->numOutNodes );
    this->activationsInput = vector< double >( this->numInNodes+1 );
    this->activationsHid = vector< double >( this->numHidNodes+1 );
    this->activationsOutput = vector< double >( this->numOutNodes );

}


// Reads a file representing the training / testing data
void NeuralNetwork::loadData( string fileName ) {

    ifstream input( fileName );

    int _checkInNodes, _checkOutNodes;
    input >> this->numEx >> _checkInNodes >> _checkOutNodes;

    // Initializes size of matrices
    this->inputAttributes = vector< vector<double> >( this->numEx, vector<double>( this->numInNodes ) );
    this->output = vector< vector<int> >( this->numEx, vector<int>( this->numOutNodes ) );

    // Verify weight file matches training file
    if ( this->numInNodes != _checkInNodes || this->numOutNodes != _checkOutNodes ) {

        std::cerr << "Error: Number of nodes in weight file does not match number of nodes in training file! ("
                  << this->numInNodes << "," << _checkInNodes << ") ("
                  << this->numOutNodes << "," << _checkOutNodes << ")" << "\n";
        return;

    }

    // Reads input and output data
    for ( int i=0; i<this->numEx; i++ ) {

        for ( int j=0; j<this->numInNodes; j++ )
            input >> this->inputAttributes[i][j];

        for ( int j=0; j<this->numOutNodes; j++ )
            input >> this->output[i][j];

    }

    input.close();

}


// Writes a file representing the weights of the neural network
void NeuralNetwork::writeWeights( string fileName ) {

    ofstream outputWeights( fileName );
    stringstream stream;

    outputWeights << this->numInNodes << " " << this->numHidNodes << " " << this->numOutNodes << "\n";

    for ( int i=0; i<this->numHidNodes; i++ ) {

        for ( int j=0; j<this->numInNodes+1; j++ )
            trimPrecision( outputWeights, this->weightsInHid[i][j] );

        outputWeights << "\n";

    }

    for ( int i=0; i<this->numOutNodes; i++ ) {

        for ( int j=0; j<this->numHidNodes+1; j++ )
            trimPrecision( outputWeights, this->weightsHidOut[i][j] );

        outputWeights << "\n";

    }

    outputWeights.close();

}


void NeuralNetwork::writeMetrics( string fileName ) {

    int A,B,C,D;
    int globA=0, globB=0, globC=0, globD=0; // Used for micro-averaging

    ofstream outputMetrics( fileName );

    for ( int nodeNum=0; nodeNum<this->numOutNodes; nodeNum++) {

        A = metric[ nodeNum ][ METRIC_A ];
        B = metric[ nodeNum ][ METRIC_B ];
        C = metric[ nodeNum ][ METRIC_C ];
        D = metric[ nodeNum ][ METRIC_D ];

        globA += A;
        globB += B;
        globC += C;
        globD += D;

        outputMetrics << A << " " << B << " " << C << " " << D << " ";

        otherMetrics( outputMetrics, A, B, C, D, false );

    }

    // Micro-averages
    otherMetrics( outputMetrics, globA, globB, globC, globD, true );

    // Macro-averages
    this->globAcc /= numOutNodes;
    this->globPrecision /= numOutNodes;
    this->globRecall /= numOutNodes;
    this->globF1 = ( 2 * this->globPrecision * this->globRecall ) / ( this->globPrecision + this->globRecall );

    printMetrics( outputMetrics, this->globAcc, this->globPrecision, this->globRecall, this->globF1 );

    return;

}


void NeuralNetwork::otherMetrics( ofstream &output, int A, int B, int C, int D, bool microAvg ) {

    double overallAcc, precision, recall, F1;

    overallAcc = ( A + D ) / double( A + B + C + D );
    precision = A / double( A + B );
    recall = A / double( A + C );
    F1 = ( 2 * precision * recall ) / ( precision + recall );

    if ( !microAvg ) {

        this->globAcc += overallAcc;
        this->globPrecision += precision;
        this->globRecall += recall;

    }

    printMetrics( output, overallAcc, precision, recall, F1 );

}


void NeuralNetwork::printMetrics( ofstream &output, double accuracy, double precision, double recall, double F1 ) {

    trimPrecision( output, accuracy );
    trimPrecision( output, precision );
    trimPrecision( output, recall );
    trimPrecision( output, F1 );
    output << "\n";

}


// Trims floating-points to 3 digits after the decimal
void NeuralNetwork::trimPrecision( ofstream &output, double number ) {

    stringstream stream;
    stream << std::fixed << std::setprecision(3) << number;
    output << stream.str() << " ";

}


// Trains the network according to number of epochs and learning rate
// Based on Figure 18.24 in the textbook
void NeuralNetwork::train( int epochs, double learnRate ) {

    // Stores error
    vector< double > deltaOut = vector< double >( this->numOutNodes );
    vector< double > deltaHid = vector< double >( this->numHidNodes );

    int nodeNum;

    // Train for specified number of epochs
    for ( int iteration=0; iteration<epochs; iteration++ ) {

        for ( int trainEx=0; trainEx<this->numEx; trainEx++ ) {

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

            //// Propagate deltas backward from output layer to input layer ////

            // Output Layer
            for ( nodeNum=0; nodeNum<this->numOutNodes; nodeNum++ )
                deltaOut[ nodeNum ] = sigDeriv( inputToOut[ nodeNum ] ) * ( this->output[ trainEx ][ nodeNum ] - activationsOutput[ nodeNum ] );

            // Hidden Layer
            for ( nodeNum=0; nodeNum<this->numHidNodes; nodeNum++ ) {

                // Uses nodeNum+1 in weightsHidOut[j][ nodeNum+1 ]
                //   because there is no input to the dummy node (activation -1) to backpropagate to
                for ( int j=0; j<this->numOutNodes; j++ )
                    deltaHid[ nodeNum ] += ( weightsHidOut[j][ nodeNum+1 ] * deltaOut[j] );

                deltaHid[ nodeNum ] *= sigDeriv( inputToHid[ nodeNum ] );

            }

            //// Update every weight in network using deltas ////

            // Hidden to Output Weights
            for ( int i=0; i<this->numHidNodes+1; i++ ) {

                for ( int j=0; j<this->numOutNodes; j++ )
                    this->weightsHidOut[j][i] += learnRate * activationsHid[i] * deltaOut[j];

            }

            // Input to Hidden Weights
            for ( int i=0; i<this->numInNodes+1; i++ ) {

                for ( int j=0; j<this->numHidNodes; j++ )
                    this->weightsInHid[j][i] += learnRate * activationsInput[i] * deltaHid[j];

            }

        }

    }

    return;

}


// Calculates outputs based on input data and the pre-trained weights
void NeuralNetwork::test() {

    // Initializes metric vector
    this->metric = vector< vector<int> >( this->numOutNodes, vector<int>( 4, 0 ) );

    int nodeNum;

    for ( int testEx=0; testEx<this->numEx; testEx++ ) {

        // Propagate the inputs forward to compute the outputs
        for ( nodeNum=0; nodeNum<this->numInNodes; nodeNum++ )
            activationsInput[ nodeNum+1 ] = this->inputAttributes[ testEx ][ nodeNum ];

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


        // Check which metric to increment based on predicted output and actual output
        for ( nodeNum=0; nodeNum<this->numOutNodes; nodeNum++ ) {

            bool predictedVal;
            bool expectedVal = output[ testEx ][ nodeNum ];

            // If activations of output neurons are >= 0.5, round up to 1
            //   Otherwise, round down to 0
            if ( activationsOutput[ nodeNum ] >= 0.5 )
                predictedVal = 1;
            else
                predictedVal = 0;

            if ( predictedVal == 1 && expectedVal == 1 )
                metric[ nodeNum ][ METRIC_A ]++;
            else if ( predictedVal == 1 && expectedVal == 0 )
                metric[ nodeNum ][ METRIC_B ]++;
            else if ( predictedVal == 0 && expectedVal == 1 )
                metric[ nodeNum ][ METRIC_C ]++;
            else if ( predictedVal == 0 && expectedVal == 0 )
                metric[ nodeNum ][ METRIC_D ]++;

        }

    }

}


// Computes activation of a neuron given the activations of the previous layer and weights of connections
double NeuralNetwork::activation( vector<double> prevActivations, vector<double> weights ) {

    // Make sure both arrays are the same size
    int numPrevNeurons = prevActivations.size();
    int _numWeights = weights.size();

    if ( numPrevNeurons != _numWeights ) {

        std::cerr << "Error: Size of previous layer and weights is not the same! ("
                  << numPrevNeurons << "," << _numWeights << ")" << "\n";

        exit( EXIT_FAILURE );

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
