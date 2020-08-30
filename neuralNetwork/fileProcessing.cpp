#include "fileProcessing.h"



using std::make_tuple;




// Returns tuple containing 2 matrices and 3 numbers
    // Matrices represent weights
    // Numbers represent number of neurons
tuple< vector< vector<double> >, vector< vector<double> >, int, int, int > loadNetwork( string fileName ) {

    ifstream inputWeights( fileName );

    int numInNodes, numHidNodes, numOutNodes;
    inputWeights >> numInNodes >> numHidNodes >> numOutNodes;

    // Initializes matrix of weights
    // Stores the weights of edges
    vector< vector<double> > inHidWeights( numHidNodes, vector<double>( numInNodes+1 ) );
    vector< vector<double> > hidOutWeights( numOutNodes, vector<double>( numHidNodes+1 ) );

    // Reads weights of edges from input layer to hidden layer
    for ( int i=0; i<numHidNodes; i++ ) {
        for ( int j=0; j<numInNodes+1; j++ )
            inputWeights >> inHidWeights[i][j];
    }

    // Reads weights of edges from hidden layer to output layer
    for ( int i=0; i<numOutNodes; i++ ) {
        for ( int j=0; j<numHidNodes+1; j++ )
            inputWeights >> hidOutWeights[i][j];
    }

    inputWeights.close();

    return make_tuple( inHidWeights, hidOutWeights, numInNodes, numHidNodes, numOutNodes );

}
