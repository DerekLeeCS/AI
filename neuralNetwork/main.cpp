#include "neuralNetwork.h"





using std::tie;


void trainProgram();
void testProgram();
void returnInputs( vector<string> &, int &, double & );
string filePrompt( int );


int main() {

    bool test;

    cout << "Enter 0 for the training program, or 1 for the testing program:" << "\n";
    cin >> test;
    cout << "\n"
         << "---------------------------------------------------------------" << "\n";

    if ( !test )
        trainProgram();
    else
        testProgram();

    return 0;
}


// Handles training the neural network
void trainProgram() {

    // Input variables
    vector<string> fileNames;   // Vector contains name of weight file, training file, output file
    int epochs;
    double learnRate;

    // Gets inputs from user
    returnInputs( fileNames, epochs, learnRate );

    // Reads the file representing the initial neural network
    NeuralNetwork newNetwork = NeuralNetwork( fileNames[0] );



    /*
    ifstream inputWeights( fileNames[0] );

    int numInNodes, numHidNodes, numOutNodes;
    inputWeights >> numInNodes >> numHidNodes >> numOutNodes;

    double inHidWeights[ numHidNodes ][ numInNodes + 1 ], hidOutWeights[ numOutNodes ][ numHidNodes + 1 ];   // Stores the weights of edges

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
    */

    // Reads the file representing the training set
    newNetwork.loadTrainData( fileNames[1] );

    /*
    ifstream inputTrain( fileNames[1] );

    int numTrainEx, _checkInNodes, _checkOutNodes;
    inputTrain >> numTrainEx >> _checkInNodes >> _checkOutNodes;

    // Verify weight file matches training file
    if ( numInNodes != _checkInNodes || numOutNodes != _checkOutNodes ) {

        std::cerr << "Error: Number of nodes in weight file does not match number of nodes in training file! ("
                  << numInNodes << "," << _checkInNodes << ") ("
                  << numOutNodes << "," << _checkOutNodes << ")" << "\n";
        return;

    }

    int inputAttributes[ numTrainEx ][ numInNodes ];
    bool output[ numTrainEx ][ numOutNodes ];

    // Reads input and output data
    for ( int i=0; i<numTrainEx; i++ ) {

        for ( int j=0; j<numInNodes; j++ )
            inputTrain >> inputAttributes[i][j];

        for ( int j=0; j<numOutNodes; j++ )
            inputTrain >> output[i][j];

    }

    inputTrain.close();
    */

    // Trains the network
    newNetwork.train( epochs, learnRate );


    return;
}


// Handles testing the neural network
void testProgram() {



}








// Handles processing required inputs from user
void returnInputs( vector<string> &fileNames, int &epochs, double &learnRate ) {

    // Append filenames to list
    for ( int i=0; i<3; i++ )
        fileNames.push_back( filePrompt(i) );

    cout << "Enter the number of epochs:" << "\n";
    cin >> epochs;
    cout << "\n";

    cout << "Enter the learning rate:" << "\n";
    cin >> learnRate;
    cout << "\n";

    return;

}

// Prompts the user for input files
// Intended to iterate through caseNum from 0 to 2
string filePrompt( int caseNum ) {

    string userPrompt, userInput;

    switch ( caseNum ) {

        case 0:
            userPrompt = "initial neural network";
            break;

        case 1:
            userPrompt = "training set";
            break;

        case 2:
            userPrompt = "output file";
            break;

    }

    cout << "Enter the name of the file representing the " << userPrompt << ":" << "\n";
    cin >> userInput;
    cout << "\n";

    return userInput;

}



