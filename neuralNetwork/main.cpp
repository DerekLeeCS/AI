#include "neuralNetwork.h"


void trainProgram();
void testProgram();
void returnInputs( vector<string> &, int &, double & );
string filePrompt( int );
bool test;

int main() {

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

    // Trains the network
    newNetwork.loadData( fileNames[1] );
    newNetwork.train( epochs, learnRate );

    // Writes weights to output file
    newNetwork.writeWeights( fileNames[2] );

    return;

}


// Handles testing the neural network
void testProgram() {

    vector<string> fileNames;

    // Gets filenames from user and appends to list
    for ( int i=0; i<3; i++ )
        fileNames.push_back( filePrompt(i) );

    // Reads the file representing the initial neural network
    NeuralNetwork newNetwork = NeuralNetwork( fileNames[0] );

    // Tests the network
    newNetwork.loadData( fileNames[1] );
    newNetwork.test();

    // Writes metrics to output file
    newNetwork.writeMetrics( fileNames[2] );

    return;

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

            // Checks global variable
            if ( !test )
                userPrompt = "training set";
            else
                userPrompt = "test set";

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



