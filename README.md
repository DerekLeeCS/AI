# AI

Contains the assignments for ECE 469 Artificial Intelligence in Fall 2020.


## Checkers

The goal of the project is to implement a Checkers-playing AI. The implementation uses Alpha-Beta Pruning and Iterative Deepening to determine the best possible move. The performance of the program depends on its efficiency (which affects how many depths the minimax search can fully explore), and to a lesser extent, the heuristic used to evaluate each game state. For more details on the implementation, refer to the Write-Up in the directory.

The user can change settings, such as which player goes first, whether the program plays itself, and has the ability to alter the initial board. At the start of the game, the user is prompted for a time limit, representing how long the program has to perform Iterative Deepening.

## Neural Network

The goal of the project is to implement a neural network from scratch. The neural network is designed for Boolean classification. The program handles the training and the testing of the network. The neural network is relatively simple, containing only one hidden layer. The training utilizes backpropagation to update the weights between the layers. 

For training, the user specifies the file containing the initial network to be trained, the file containing the training set, and the output file to store the trained network, along with the number of epochs and the learning rate for training.

For testing, the user specifies the file containing the trained network, the file containing the test set, and the output file to store metrics representing the network's performance on the test set.
