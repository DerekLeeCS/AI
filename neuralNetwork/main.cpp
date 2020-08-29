#include <iostream>
#include <list>
#include <string>


using std::cin;
using std::cout;
using std::list;
using std::string;


void returnInputs( list<string> &, int &, double & );
string filePrompt( int );


int main() {

    // List contains neuralFile, trainFile, outFile
    list<string> fileNames;
    int epochs;
    double learnRate;

    returnInputs( fileNames, epochs, learnRate );

    return 0;
}


// Handles processing required inputs from user
void returnInputs( list<string> &fileNames, int &epochs, double &learnRate ) {

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
