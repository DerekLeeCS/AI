#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>


using std::string;
using std::ifstream;
using std::ofstream;
using std::istringstream;
using std::cin;
using std::cout;


int main() {

    string inFile, outFile;

    cout << "Enter the name of the input file:" << "\n";
    cin >> inFile;
    cout << "\n";

    cout << "Enter the name of the output file:" << "\n";
    cin >> outFile;
    cout << "\n";

    ifstream input( inFile );
    ofstream output( outFile );
    string line;

    // Process the file
    while ( getline( input, line ) ) {

        std::replace( line.begin(), line.end(), ',', ' ' );   // Replace commas with spaces for stringstream

        istringstream iss( line );
        string var;

        while( iss >> var ) {

            // Inputs
            if ( var == "x" )
                output << 1 << " ";
            else if ( var == "o" )
                output << -1 << " ";
            else if ( var == "b" )
                output << 0 << " ";

            // Outputs
            else if ( var == "positive" )
                output << 1 << "\n";
            else if ( var == "negative" )
                output << 0 << "\n";

            // Should never happen
            else {

                std::cerr << "Error: Unknown variable";
                exit( EXIT_FAILURE );

            }

        }

    }

    return 0;

}
