#include <vector>
#include <string>
#include <tuple>
#include <fstream>


using std::vector;
using std::string;
using std::tuple;
using std::ifstream;
using std::ofstream;


// Returns tuple containing 2 matrices and 3 numbers
    // Matrices represent weights
    // Numbers represent number of neurons
tuple< vector< vector<double> >, vector< vector<double> >, int, int, int > loadNetwork( string );
