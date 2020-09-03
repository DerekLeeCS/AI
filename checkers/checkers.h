#include <string>
#include <list>
#include <vector>
#include <unordered_set>
#include <tuple>
#include <memory>
#include <chrono>

using std::string;
using std::list;
using std::vector;
using std::unordered_set;
using std::tuple;
using std::shared_ptr;


namespace pieceVals {

    // Minimax Search
    #define REMAINING_TIME_LIMIT        0.1
    #define TIME_LIMIT_EXCEEDED         10101
    #define SINGLE_MOVE                 11111
    #define VAL_MIN                     -99999.0f
    #define VAL_MAX                     99999.0f

    #define VICTORY_RED_PIECE           10000
    #define VICTORY_RED_MOVE            9999
    #define VICTORY_WHITE_PIECE         -10000
    #define VICTORY_WHITE_MOVE          -9999


    const bool COLOR_RED_VAL = 0;     // Red
    const bool COLOR_WHITE_VAL = 1;   // White

    const int TYPE_MAN_VAL = 0;       // Man
    const int TYPE_KING_VAL = 1;      // King
    const int TYPE_EMPTY_VAL = 2;     // Empty Square

    const bool FILLER_FALSE = 0;      // Piece
    const bool FILLER_TRUE = 1;       // Filler (squares that pieces cannot move on)

};


class board {

public:
    //////////////////// Member Functions ////////////////////

    // Constructor
    // Creates the default board
    board();

    class piece; // Object class for pieces

    void specialBoard(); // For testing boards

    void playGame();
    void playerMove();
    void computerMove();

    void endTurn(); // Series of actions to be taken at the end of a turn

    // Returns a pointer to the set of pieces that can be moved
    unordered_set< shared_ptr<piece> >* returnPieces();

    // Performs a specified move
    // If another jump is possible, returns true; otherwise, returns false
    bool moveResult( tuple<int,int>, tuple<int,int> );

    // Returns a list of pointers to pieces affected by a move
    list< shared_ptr< piece > > affectedPieces( tuple<int,int>, tuple<int,int> );

    // Isolates a board for iterative deepening
    // Used in minimax
    void isolateBoard( tuple<int,int>, tuple<int,int> );

    // Checks moves of a specific piece
    void checkMoves( shared_ptr<piece> & );

    // Checks moves of pieces affected by curPiece's move
    void checkDiagMoves( shared_ptr<piece> &, tuple<int,int>, bool );

    // Checks if a row/column is within the board
    // If valid, returns true; otherwise, returns false
    bool validLoc( int );

    // Updates the current score of the board
    void heuristic();

    // Checks the closest enemy piece to a king
    // Returns an int representing the number of moves needed to reach that piece
    int kingDistance( shared_ptr<piece> & );

    // Checks if the game is at a terminal state
    bool terminalState( float );

    float minimax( board &, int, bool, float, float );


    class piece {

    public:
        piece();
        piece( bool color, int type );  // Regular constructor
        piece( bool filler );           // Filler constructor
        piece( const piece & );         // Copy constructor

        void updateCount( board &, bool );    // Increments piece counts
        void clearPiece( board & );           // Removes piece from board vectors and decrements piece counts
        void resetPiece();                  // Resets moves/jumps of the piece

        // Checks if a piece should be promoted
        // If successful, returns true; else, returns false
        bool checkPromotion(board &);

        // Returns a pointer to the list of actions that can be taken by the piece
        list< tuple<int,int> >* returnActions();

        void insertMove( board & );
        void removeMove( board & );
        void insertJump( board & );
        void removeJump( board & );

        bool validDirection(int);

        ////////// Data Members //////////
        bool color;
        int type;
        bool filler;

        // Bool for if piece can move / jump
        bool validMove = false;
        bool validJump = false;

        list< tuple<int,int> > moves; // Represents possible squares to move to
        list< tuple<int,int> > jumps; // Represents possible squares to jump to

        tuple<int,int> loc; // Represents the location of the piece in the board

    };


private:
    //////////////////// Data Members ////////////////////

    shared_ptr< piece > gameboard[8][8]; // The board
    shared_ptr< piece > emptyPiece;      // A pointer to a piece representing an empty square
    shared_ptr< piece > fillerPiece;     // A pointer to a piece representing a filler square

    float score;            // Score determined by the heuristic
    int turnCount = 1;      // Current turn
    int computerTime;       // Amount of time in seconds computer has to calculate move
    bool redTurn = false;   // If true, red has current move; else, white has current move
    bool AIvsAI = false;    // If true, computer plays itself; else, computer plays against player
    int maxDepth;           // Maximum depth set by iterative deepening

    // Keeps track of time taken during minimax search
    std::chrono::time_point<std::chrono::system_clock> startTime, endTime;
    std::chrono::duration<double> elapsed_seconds;

    // Stores a list of moves to get to current position
    list< tuple< tuple<int,int>, tuple<int,int> > > moves;

    // Stores a list of the best moves available
    list< tuple< tuple<int,int>, tuple<int,int> > > bestMoves;

    // Stores a shared_ptr to all of the pieces
    unordered_set< shared_ptr<piece> > redPieces;
    unordered_set< shared_ptr<piece> > whitePieces;

    // Stores a shared_ptr to all of pieces that are available to move/jump
    unordered_set< shared_ptr<piece> > redMoves;
    unordered_set< shared_ptr<piece> > redJumps;
    unordered_set< shared_ptr<piece> > whiteMoves;
    unordered_set< shared_ptr<piece> > whiteJumps;
    unordered_set< shared_ptr<piece> > multiJumps;


    ////////// Count of pieces //////////

    // Number of men and kings currently on the board
    int redMen = 0;
    int redKings = 0;
    int whiteMen = 0;
    int whiteKings = 0;
    // Number of men on the last row
    int redLast = 0;
    int whiteLast = 0;


    ////////// Display Functions //////////

    void printVictory( bool, bool );

    //// Menu and Sub-Menus ////
    void printStart();
    void printSettings();
    void printPlayerSettings();
    void printPieceSettings();  // Change starting board
    void printTimeSettings();   // Change computing time
    void printAddPiece();
    void printRemovePiece();
    bool validateInput();       // Validates user input; if input is invalid, returns true; otherwise, returns false


    void printBoard();      // Prints the current board
    void printMoves();      // Prints the available moves for the player
    void printHelp();       // Prints a list of commands
    void printError();      // Prints an error message
    void printMoveError();  // Prints a move error message

};

