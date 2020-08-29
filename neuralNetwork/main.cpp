#include <iostream>
#include <list>
#include <string>


using std::cin;
using std::cout;
using std::list;
using std::string;


string filePrompt( int );


int main() {

    // List contains neuralFile, trainFile, outFile
    list<string> fileNames;
    int epochs;
    double learnRate;

    // Append filenames to list
    for ( int i=0; i<3; i++ )
        fileNames.push_back( filePrompt(i) );

    cout << "Enter the number of epochs:" << "\n";
    cin >> epochs;
    cout << "\n";

    cout << "Enter the learning rate:" << "\n";
    cin >> learnRate;
    cout << "\n";

    return 0;
}


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
