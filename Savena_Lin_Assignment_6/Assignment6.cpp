/*
Name of program: EECS 348 Assignment 6
Description: Sudoku solver
Inputs: puzzles with clues
Output: completed sudoku puzzle, unless there's no solutions, put into 1 file
All collaborators: None
Other sources for the code: Lumo, Leo, ChatGPT, comment lines from Leo were kept
Author’s full name: Savena Lin
Creation date: 14 October 2026
Revised by: Jeff Liang
Revision date: 29 October 2026
Re-revises by: Savena Lin
Revision date: 13 November 2026
Original author and source information above are preserved.
*/


#include <iostream>  //Importing the library for terminal input and output
#include <fstream>   //Importing the library for reading and writing files
#include <sstream>   //Importing the library for building text
#include <string>    //Importing the library for text
#include <vector>    //Importing the library for storing a list of messages
#include <cstdlib>   //Importing the library for general C utilities

using namespace std;  //Use standard library names without writing std::

// Board stores the 9x9 grid. 0 means an empty cell. leo comment
class Board {  //This class stores one Sudoku board
public:  //These members can be used outside Board
    int grid[9][9]; //2d array that represents the sudoku grid

    // IMPROVED (maintainability): New shared helper. The row/column/3x3-block rules were
    // previously written twice — once in isValid and once in findContradictions — so a rule
    // change would have to be made in two places and could drift out of sync. The logic now
    // lives in exactly one function that both callers use.
    // Note: both callers hide the tested cell before calling this helper (the cell is empty
    // in isValid, and zeroed in findContradictions), so g[r][c] can never shadow v itself.
    // Returns true when value v already appears in the same row, column, or 3x3 block.
    static bool valueClashes(const int g[9][9], int r, int c, int v) {  //This checks whether v already shares a row, column, or block with cell (r, c)
        for (int i = 0; i < 9; i++) {  //Loop through the row and the column
            if (g[r][i] == v) return true; // same row *leo comment*
            //checks if the value already exists in the same row

            if (g[i][c] == v) return true; // same column *leo comment*
            //checks if the value already exists in the same column
        }

        int br = (r / 3) * 3; //creates the top left corner row of the 3x3 block
        int bc = (c / 3) * 3; //creates the top left corner column of the 3x3 block

        for (int i = 0; i < 3; i++) { //loops thru the 3x3 block's rows
            for (int j = 0; j < 3; j++) { //loops thru the 3x3 block's column
                if (g[br + i][bc + j] == v) return true; // same block *leo comment*
                //checks if theres a dupe in that 3x3 block
            }
        }

        return false; //returns false when no duplicate was found
    }

    // Load the grid from a file. Each row has 9 tokens separated by whitespace. *leo comment*
    // '_' becomes 0; digits become their integer value. *leo comment*
    bool load(const string& filename) { //creates the input file steam, puzzle files are put thru this
        ifstream infile(filename); //checks if successful file open

        if (!infile.is_open()) {  //Check if the file did not open
            cerr << "Could not open " << filename << endl;  //Print that the file could not be opened
            return false;  //Stop because there is no file to read
            //This block checks to make sure the file is successfully opened, returns false if it cannot
        }

        // Read every token first. The grid is filled only after all 81 are valid,
        // so an incomplete or badly formatted file is never solved.
        //This block was revised by ChatGPT (listed in Other sources in the prologue)
        int values[81];  //This stores the 81 cell values before they are copied
        int count = 0;   //This stores how many valid values have been read
        string token;    //This stores one value from the file

        while (count < 81 && infile >> token) {  //Read values until 81 are stored or the file ends
            if (token == "_") {  //Check if the token is an underscore
                values[count] = 0;  //Store an empty cell as 0
                //An underscore is an empty cell
            } else if (token.size() == 1 && token[0] >= '1' && token[0] <= '9') {  //Check if the token is a digit from 1 to 9
                values[count] = token[0] - '0';  //Store the digit as a number
                //A single digit from 1 to 9 is a clue
            } else {  //The token is not a valid cell value
                cerr << filename << " does not contain exactly 81 valid values." << endl;  //Print that the file is invalid
                infile.close();  //Close the file
                return false;  //Stop because this puzzle cannot be solved
                //Reject letters, 0, and numbers outside 1-9
            }

            count++;  //Count this valid value
        }

        //81 valid tokens, and nothing after them. Extra text makes the file invalid.
        string extra;  //This stores a token past the first 81

        if (count != 81 || infile >> extra) {  //Check if the file does not have exactly 81 values
            cerr << filename << " does not contain exactly 81 valid values." << endl;  //Print that the file is invalid
            infile.close();  //Close the file
            return false;  //Stop because this puzzle cannot be solved
        }

        for (int i = 0; i < 81; i++) {  //Copy each checked value into the grid
            grid[i / 9][i % 9] = values[i];  //Place this value in its row and column
            //Copy the checked values into the 9x9 grid
        }

        infile.close(); //closes the file
        return true; //indicates the file was successfully opened and valid
    }

    // Print the grid to stdout in a human-readable format. *leo comment*
    void print() const {  //This prints the grid on the terminal
        for (int r = 0; r < 9; r++) {  //Loop through every row
            for (int c = 0; c < 9; c++) {  //Loop through every column
                //looping thru all rows and columns

                if (c == 0) cout << "  "; //visual, adds indentation for alignment
                if (c == 3 || c == 6) cout << " "; //adds a space for clarity between every 3rd column

                if (grid[r][c] == 0) {  //Check if this cell is empty
                    cout << "_";  //Print an underscore for an empty cell
                    //checks if the cell is empty, if so, prints an underscore
                } else {  //This cell holds a number
                    cout << grid[r][c];  //Print the number in this cell
                    //otherwise, it prints the int of the clue
                }

                if (c < 8) cout << " "; //places a space between all the cells, but not for the last column
            }

            cout << endl; //prints a new line after each row
        }
    }

    //Write the grid in the same space-separated form as the puzzle file
    //This block was obtained from ChatGPT (listed in Other sources in the prologue)
    void write(ostream& out) const {  //This writes the grid to a file
        for (int r = 0; r < 9; r++) {  //Loop through every row
            for (int c = 0; c < 9; c++) {  //Loop through every column
                if (c > 0) out << ' ';  //Place a space before every cell except the first

                if (grid[r][c] == 0) {  //Check if this cell is empty
                    out << '_';  //Write an underscore for an empty cell
                } else {  //This cell holds a number
                    out << grid[r][c];  //Write the number in this cell
                }
            }

            out << '\n';  //Write a new line after each row
        }
    }

    // Check whether placing value v at row r, column c is legal *leo comment*
    // under Sudoku rules (row, column, and 3x3 block uniqueness). *leo comment*
    bool isValid(int r, int c, int v) const {  //This checks whether a number can be placed
        // IMPROVED (maintainability): This function now delegates to the shared helper
        // valueClashes instead of containing its own copy of the row/column/block loops.
        // Behavior is identical, but the Sudoku rules exist in only one place in the file.
        return !valueClashes(grid, r, c, v); //legal only when nothing clashes *leo comment*
        //returns true if all placements are allowed
    }

    //added, checks to see if the clues given are contradictory/invalid
    vector<string> findContradictions() const {  //This finds clues that contradict each other
        vector<string> conflicts; //stores conflict messages
        int tmp[9][9]; //Temp 2d array that stores the grid while it's checking for contradictions

        for (int r = 0; r < 9; r++)  //Loop through every row
            for (int c = 0; c < 9; c++)  //Loop through every column
                tmp[r][c] = grid[r][c];  //Copy this cell into the temporary grid
                //stores the original grid

        for (int r = 0; r < 9; r++) {  //Loop through every row
            for (int c = 0; c < 9; c++) {  //Loop through every column
                //loops thru the array

                if (tmp[r][c] == 0) continue; //skips all cells w 0

                int v = tmp[r][c]; //stores the value in a temp variable
                tmp[r][c] = 0; // hide this clue before testing it

                // IMPROVED (maintainability): The duplicated row/column/block scanning loops
                // that followed here were replaced by a single call to the shared helper
                // valueClashes. Same behavior and same complexity, but the Sudoku rules are
                // no longer maintained in two separate places.
                bool dup = valueClashes(tmp, r, c, v); //tracks if a dupe is found

                if (dup) {  //Check if this clue conflicts with another clue
                    ostringstream msg;  //This builds the conflict message

                    msg << "Duplicate " << v << " at row " << (r + 1)  //Start the message with the value and row
                        << ", col " << (c + 1);  //Add the column to the message

                    conflicts.push_back(msg.str());  //Save the conflict message
                    //creates the conflict message
                }

                tmp[r][c] = v; // restores the temp array to its original value
            }
        }

        return conflicts;  //Return every conflict that was found
    }
};

// Solver encapsulates the recursive backtracking search. leo comment
class Solver {  //This class searches for solutions
private:  //These members can only be used inside Solver
    Board& board; // reference to the puzzle board leo comment
    //This block was obtained from ChatGPT (listed in Other sources in the prologue)
    ostream& solutionOutput;  //This stores where each completed grid is written
    int solutionsFound; // total number of solutions discovered leo comment

    // Pick the empty cell with the fewest legal numbers (1-9).
    // row and col are the two coordinates; no vector is stored on the recursive calls.
    // Returns false when every cell is filled.
    //This block was revised by ChatGPT (listed in Other sources in the prologue)
    bool findEmpty(int& row, int& col) const {  //This chooses the empty cell with the fewest choices
        int fewest = 10; //more than any cell can have, so the first empty cell replaces it
        bool found = false;  //This tracks whether an empty cell has been found

        for (int r = 0; r < 9; r++) {  //Loop through every row
            for (int c = 0; c < 9; c++) {  //Loop through every column
                if (board.grid[r][c] != 0) continue;  //Skip a cell that is already filled
                //skip filled cells

                int options = 0;  //This counts how many numbers can go in this cell

                for (int v = 1; v <= 9; v++) {  //Try each number from 1 to 9
                    if (board.isValid(r, c, v)) options++;  //Count this number when it can be placed
                    //count how many numbers can legally go in this cell
                }

                if (options < fewest) {  //Check if this cell has fewer choices than the best so far
                    fewest = options;  //Remember this smaller number of choices
                    row = r;  //Remember this cell's row
                    col = c;  //Remember this cell's column
                    found = true;  //Remember that an empty cell was found
                    //keep the cell that has fewer choices than the best so far

                    if (options == 0) return true;  //Stop because nothing can be placed in this cell
                    //nothing fits here, so this branch cannot succeed
                }
            }
        }

        return found; //false only when the board has no empty cells
    }

    // The recursive depth-first search. *leo comment*
    // When the board is completely filled we've found a solution and print it. *leo comment*
    // Otherwise we pick the empty cell with the fewest possible numbers, try *leo comment*
    // values 1-9, and recurse. If no value works we backtrack. *leo comment*
    //This block was revised by ChatGPT (listed in Other sources in the prologue)
    void search() {  //This searches for every solution
        int row; //row of the chosen empty cell
        int col; //column of the chosen empty cell

        // Board is full — we found a solution. *leo comment*
        if (!findEmpty(row, col)) {  //Check if the board has no empty cells
            if (solutionsFound > 0) {  //Check if a solution was already written
                solutionOutput << '\n';  //Separate this solution from the previous one
            }

            board.write(solutionOutput);  //Write the solved grid to the output file

            solutionsFound++; //adds to the number of found solutions

            board.print(); //prints the solved puzzle grid
            cout << endl; //prints new line

            return; //continue with the recursive call, keeps backtracking
        }

        //Try each candidate value. *leo comment*
        for (int v = 1; v <= 9; v++) { //goes from 1->9 in every empty slot
            if (board.isValid(row, col, v)) {  //Check if this number can be placed here
                //checks if that int is valid in that position

                board.grid[row][col] = v; // place value *leo comment*

                search(); // recurse *leo comment*

                board.grid[row][col] = 0; // backtrack: undo placement *leo comment*
            }
        }
    }

public:  //These members can be used outside Solver
    // Construct a solver bound to a specific board. leo comment
    //This block was revised by ChatGPT (listed in Other sources in the prologue)
    explicit Solver(Board& b, ostream& out)  //This creates a solver for one board and one output
        : board(b), solutionOutput(out), solutionsFound(0) {}  //Save the board, the output, and a count of zero

    // Run the solver. Returns the number of solutions found. *leo comment*
    int solve() {  //This runs the search and returns how many solutions were found
        search();  //Start the recursive search

        return solutionsFound; //starts the recursive search and returns the amount of solutions found
    }
};

// IMPROVED (maintainability): The body of the old per-puzzle loop in main() was extracted
// into its own function. main() is now a short, readable driver loop, and the per-puzzle
// logic is one function instead of one very long main() — lowering cyclomatic complexity
// and making each piece easier to review and modify. All original behavior and comments
// were kept.
// outputError is passed by reference so write failures detected inside this function are
// still reported by main()'s exit code, exactly as before.
void solveOnePuzzle(int p, ofstream& solutionsFile, bool& outputError) {  //This loads, validates, and solves one puzzle and writes the results
    Board board;  //This stores the puzzle being solved

    ostringstream oss;  //This builds the puzzle file name
    oss << "puzzle" << p << ".txt"; //builds the puzzle file name

    string filename = oss.str(); //stores the above as the file name

    cout << "Puzzle file: " << filename << endl;  //Print the puzzle file name
    cout << "Original puzzle:" << endl;  //Print the label before the original grid
    //prints the filename and the word "original puzzle"

    //Every puzzle gets a header line in solutions.txt, even if it cannot be loaded
    //This block was revised by Lumo (listed in Other sources in the prologue)
    solutionsFile << "Puzzle: " << filename << '\n';  //Write which puzzle the results belong to

    if (!board.load(filename)) {  //Check if the puzzle file could not be loaded
        return; //skips the rest of this puzzle and lets the caller move to the next one
        //if the board loads, keeps going, if it fails, skips this puzzle and goes to next puzzle
    }

    board.print(); //prints the original puzzle with the clues
    cout << endl; //For more clarity, adding the space between the clues and the solutions

    // Reject contradictory puzzles immediately instead of searching
    vector<string> conflicts = board.findContradictions();  //Find clues that contradict each other

    if (!conflicts.empty()) {  //Check if the clues contradict each other
        //checks for if there were issues with the clues

        cout << "Invalid puzzle - contradictory clues:" << endl;  //Print that the puzzle is invalid
        //prints that the puzzle is not possible

        for (const string& msg : conflicts) {  //Loop through every conflict message
            cout << "  " << msg << endl;  //Print that conflict message
        }

        //Print and write the same message when the clues contradict each other
        //This block was revised by ChatGPT (listed in Other sources in the prologue)
        cout << "No solution found." << endl;  //Print that no solution was found

        solutionsFile << "No solution found.\n";  //Write that no solution was found

        if (!solutionsFile) {  //Check if the solutions file was not written
            cerr << "Could not write solutions.txt" << endl;  //Print that the solutions file could not be written
            outputError = true;  //Remember that the file output failed
        }

        cout << endl;  //Print a blank line before the next puzzle
        return; //goes to the next puzzle
    }

    //Send each completed grid to the shared solutions file
    //This block was revised by ChatGPT (listed in Other sources in the prologue)
    Solver solver(board, solutionsFile); //goes to solve the puzzle

    int count = solver.solve(); //variable for how many solutions found

    //Print and write one summary. Zero solutions has no count line.
    //This block was revised by ChatGPT (listed in Other sources in the prologue)
    if (count == 0) {  //Check if the search found no solution
        cout << "No solution found." << endl;  //Print that no solution was found
        solutionsFile << "No solution found.\n";  //Write that no solution was found
    } else {  //The search found at least one solution
        cout << "Solutions found: " << count << endl;  //Print how many solutions were found
        solutionsFile << "Solutions found: " << count << '\n';  //Write how many solutions were found
    }

    if (!solutionsFile) {  //Check if the solutions file was not written
        cerr << "Could not write solutions.txt" << endl;  //Print that the solutions file could not be written
        outputError = true;  //Remember that the file output failed
    }

    cout << endl; //if there are solutions, prints it
}

// Main is now a simple driver: open the shared solutions file, run every puzzle, and
// report failure in the exit code.
// Revised for Assignment 5: all results are written to one shared file, solutions.txt.
// This block was revised by Lumo (listed in Other sources in the prologue)
int main() {  //This is the main method
    //This block was obtained from ChatGPT (listed in Other sources in the prologue)
    bool outputError = false;  //This tracks whether writing the solution file failed

    //One shared output file holds the results for every puzzle
    //This block was revised by Lumo (listed in Other sources in the prologue)
    ofstream solutionsFile("solutions.txt");  //Open the single solutions file

    if (!solutionsFile.is_open()) {  //Check if the solutions file did not open
        cerr << "Could not open solutions.txt" << endl;  //Print that the solutions file could not be opened
        return 1;  //Stop because there is nowhere to write the results
    }

    for (int p = 1; p <= 5; p++) {  //Solve puzzle1 through puzzle5
        // IMPROVED (maintainability): the entire per-puzzle block that used to sit here was
        // moved into solveOnePuzzle(), so main() reads as a plain list of work items.
        solveOnePuzzle(p, solutionsFile, outputError); //load, validate, and solve one puzzle
    }

    solutionsFile.close();  //Close the shared solutions file

    //This block was obtained from ChatGPT (listed in Other sources in the prologue)
    return outputError ? 1 : 0;  //Return 1 when the solutions file failed, otherwise return 0
}
