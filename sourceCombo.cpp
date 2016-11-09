#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <cstring>

const int  MAXV = 9;									//max value in a sudoku
const int  MAXSTR = 200;								//max length of string (e.g read from file)
const int  OFFSET = 1;                                  //offset by "1" as an array starts at 0, but lowest sudoku value = 1 
      int  testing = 0;									//enables testoutput (1 = changed value | 2 = current board progression)
      char filename[MAXSTR] = "BOARD.DTA";				//edit filename to apply own specific file (in acceptable format)
                                                        //		or add filename directly into console (also works for testing)
void print();
void printAlts();
void solveUnique();
void fillAlts();
bool possAdd(int i, int k, int j);
bool testX(int i, int j);
bool testY(int k, int j);
bool testRegion(int i, int k, int j);
bool searchUnique(int i, int k, int j);
bool searchX(int i, int j);
bool searchY(int k, int j);
bool searchRegion(int i, int k, int j);
bool noSolutionUnique();
void solveBackTrack();
bool passedTests(int i, int k, int j);
void backtrack(int &i, int &k, int &j);
void abortBacktrackSolution(int &i, int &k);
void displayTesting(int i, int k, int j);
void stats(bool &changedPath);
bool fileExists();
void readFromFile();

void setValue(bool m, int &i, int &k, int &j);
                                                        
struct Tal {
    int value;											//current value in cell (0 == emty)
    bool alts[MAXV];									//boolean with possible numbers to add in a cell
};

Tal sudoku[MAXV][MAXV];									//sudoku board
bool  emty[MAXV][MAXV];									//boolean sudoku board with "original" emty or not status for  each cell

long long   changedValue = 0,							//stats
            numChangedPath = 0;
int			uniqueNumber = 0,
            uniqueCell = 0;

bool noSolutionBackTrack = false;						//if true, the current board has no solution

int main(int argc, char *argv[]) {

    for (int i = 1; i < argc; i++) {						//console input
        if (atoi(argv[i]) >= 1 && atoi(argv[i]) <= 2) {		//possible inout for testing variable
            testing = atoi(argv[i]);
            std::cout << "\n\tActivated testing\n";
            std::cout << "\tTesting = " << testing << std::endl;
        }
        else { 												//possible input for new filename (new sudoku)
            strcpy(filename, argv[i]);
        }
    }

    if (fileExists()) {

        time_t now = time(0);
        tm* localtm = localtime(&now);
        std::cout << "\n\tStarting calculating at: " << asctime(localtm) << std::endl;

        std::cout << "\tReading from file \"" << filename << "\"" << std::endl;
        readFromFile();

        std::cout << "\n\tORIGINAL BOARD:\n\n";
        print();
        std::chrono::steady_clock::time_point startCalk = std::chrono::steady_clock::now();
        solveUnique();
        std::chrono::steady_clock::time_point endCalk = std::chrono::steady_clock::now();

        if (noSolutionUnique()) {

            for (int i = 0; i < MAXV; i++) {
                for (int k = 0; k < MAXV; k++) {
                    if (sudoku[i][k].value == 0)
                        emty[i][k] = 1;
                    else
                        emty[i][k] = 0;
                }
            }

            solveBackTrack();
            std::chrono::steady_clock::time_point endCalk = std::chrono::steady_clock::now();

            if (!noSolutionBackTrack) {
                std::cout << "\n\n\tSOLVED BOARD:\n\n";
                print();

                time_t now = time(0);
                tm* localtm = localtime(&now);
                std::cout << "\n\tFinnished calculating at: " << asctime(localtm);

                std::chrono::steady_clock::duration diff = endCalk - startCalk;

                float milli = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();

                std::cout << "\n\tUsed " << milli / 1000 << " second(s) to calculate given board."
                          << "\n\n\tChanged path   "     << numChangedPath << " time(s)\n"
                          << "\tChanged value  "         << changedValue   << " time(s)\n"
                          << "\tUnique numbers "         << uniqueNumber
                          << "\n\tUnique cells   "       << uniqueCell << std::endl;

            }
        }
        else {
            std::cout << "\n\n\tSOLVED BOARD:\n\n";
            print();

            time_t now = time(0);
            tm* localtm = localtime(&now);
            std::cout << "\n\tFinnished calculating at: " << asctime(localtm);

            std::chrono::steady_clock::duration diff = endCalk - startCalk;

            float milli = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();

            std::cout << "\n\tUsed " << milli / 1000 << " second(s) to calculate given board."
                      << "\n\n\tUnique numbers " << uniqueNumber
                      << "\n\tUnique cells   "   << uniqueCell
                      << "\n\tChanged value  "   << changedValue << " time(s)\n";
        }
    }

    return 0;
}

void print() {											//prints out current state of sudoku board
                                                        
    for (int i = 0; i < MAXV; i++) {
        for (int k = 0; k < MAXV; k++) {
            if ((k + OFFSET) == 1)								//start of row
                std::cout << '\t' << sudoku[i][k].value;
            else												//normal
                std::cout << ' ' << sudoku[i][k].value;
            if ((k + OFFSET) % 3 == 0)							//end of region in row
                std::cout << '\t';
            if ((k + OFFSET) == MAXV)							//end of row
                std::cout << std::endl;
            if ((i + OFFSET) % 3 == 0 && (k + OFFSET) == MAXV)	//end of region at end of row
                std::cout << std::endl;
        }
    }
}

void printAlts() {										//prints out current states of alternatives for each cell
                                                        //		for testing purposes only
    for (int i = 0; i < MAXV; i++) {
        std::cout << '\n';
        for (int k = 0; k < MAXV; k++) {
            std::cout << '\t';
            for (int j = 0; j < MAXV; j++) {
                if (sudoku[i][k].alts[j] == true) {
                    std::cout << j;
                }
                else {
                    std::cout << '-';
                }
            }
        }
    }
    std::cout << std::endl;
}

void solveUnique() {									//finds unique numbers and cells

    fillAlts();											//fills "new" alternative numbers for each cell

    for (int i = 0; i < MAXV; i++) {					//for all columns
        for (int k = 0; k < MAXV; k++) {				//for all rows

            if (sudoku[i][k].value == 0) {				//if emty cell

                int num, count = 0;						//variables for finding unique cell

                for (int j = 0; j < MAXV; j++) {		//for all values
                    if (searchUnique(i, k, j)) {		//if unique number
                        setValue(1, i, k, j);
                    }
                    if (sudoku[i][k].alts[j]) {			//if current value is an alternative, but not unique
                        num = j; count++;				//prepares for finding unique cell
                    }
                }
                if (count == 1 && possAdd(i, k, num)) {		//if unique cell
                    setValue(0, i, k, num);
                }
            }
        }
    }
}

void setValue(bool m, int &i, int &k, int &j) {

    sudoku[i][k].value = (j + OFFSET);					//sets new value
    changedValue++;										//+1
    displayTesting(i, k, j + OFFSET);					//stats function
    fillAlts();											//change alts as new value is set

    if (m) {
        uniqueNumber++;									//+1
        i = k = 0;										//resets for loop properly?
        j = -1;
    }
    else {
        uniqueCell++;									//+1
        i = 0;											//resets for loop properly?
        k = -1;
    }
}

void fillAlts() {										//fills "new" alternative numbers for each cell

    for (int i = 0; i < MAXV; i++) {
        for (int k = 0; k < MAXV; k++) {
            for (int j = 0; j < MAXV; j++) {
                if (sudoku[i][k].alts[j] == 1) {		//if alternative not changed from default (true)
                    if (possAdd(i, k, j) == 0) {		//check if alternative should be false
                        sudoku[i][k].alts[j] = 0;		//set alternative to false
                    }
                }
            }
        }
    }
}

bool possAdd(int i, int k, int j) {						//if possible to add current value

    if (sudoku[i][k].value == 0)						//if emty cell
        if (passedTests(i, k, (j + OFFSET)))			//goes through all tests
            return 1;									//value is possible to add

    return 0;
}

bool testX(int i, int j) {                             	//checks row for matching value

    for (int k = 0; k < MAXV; k++)                     	//for all cells in current row
        if (sudoku[i][k].value == j)                   	//if same value
            return 0;

    return 1;
}

bool testY(int k, int j) {								//checks column for matching value

    for (int i = 0; i < MAXV; i++)						//for all cels in current column
        if (sudoku[i][k].value == j)					//if same value
            return 0;

    return 1;
}

bool testRegion(int i, int k, int j) {					//checks region for matching value

    while ((i + OFFSET) % 3 != 1)  i--;					//finds top cell/row in region
    while ((k + OFFSET) % 3 != 1)  k--;					//finds first cell/column in region

    for (int g = i; g <= i + 2; g++)					//for all rows in region
        for (int h = k; h <= k + 2; h++)				//for all columns in region
            if (sudoku[g][h].value == j)				//if same value
                return 0;

    return 1;
}

bool searchUnique(int i, int k, int j) {				//if unique number
                                                        
    if (possAdd(i, k, j)) {								//if value is an alternative

        if (searchX(i, j))								//check row
            return 1;
        if (searchY(k, j))								//check column
            return 1;
        if (searchRegion(i, k, j))						//check region
            return 1;									//return true if value is unique
    }

  return 0;
}

bool searchX(int i, int j) {							//checks row for unique value

    int count = 0;										//variable for checking the amount of cells 
                                                        //		that has value as an alternative
    for (int k = 0; k < MAXV; k++) {					//for all cells in current row
        if (sudoku[i][k].alts[j] == true) {				//if value is an alternative
            count++;
            if (count > 1) break;						//if more than one cell has value as an alternative
        }
    }
    if (count == 1)										//if only one cell has value as an alternative
        return 1;										//return true
    return 0;
}

bool searchY(int k, int j) {							//checks column for unique value

    int count = 0;										//variable for checking the amount of cells
                                                        //		that has value as an alternative
    for (int i = 0; i < MAXV; i++) {					//for all cells in current column
        if (sudoku[i][k].alts[j]) {						//if value is an alternative
            count++;
            if (count > 1) break;						//if more than one cell has value as an alternative
        }
    }
    if (count == 1)										//if only one cell has value as an alternative
        return 1;										//return true
    return 0;
}

bool searchRegion(int i, int k, int j) {				//checks region for unique value

    int count = 0;										//variable for checking the amount of cells
                                                        //		that has value as an alternative
    while ((i + OFFSET) % 3 != 1)  i--;					//finds top cell/row in region
     while ((k + OFFSET) % 3 != 1)  k--;				//finds first cell/column in region

    for (int g = i; g <= i + 2; g++) {					//for all rows in region
        for (int h = k; h <= k + 2; h++) {				//for all columns in region

            if (sudoku[g][h].alts[j] == true) {			//if value is an alternative
                count++;
                if (count > 1) break;					//if more than one cell has value as an alternative
            }
        }
    }
    if (count == 1)										//if only one cell has value as an alternative
        return 1;										//return true
    return 0;
}

bool noSolutionUnique() {								//if board not solved after solveUnique()

    for (int i = 0; i < MAXV; i++) {					//for all columns
        for (int k = 0; k < MAXV; k++) {				//for all rows
            for (int j = 0; j < MAXV; j++) {			//for all cells and values

                if (sudoku[i][k].alts[j]) {				//if cell has an alternative (i.e not solved)
                    return 1;							//return true (board was not solved by solveUnique())
                }
            }
        }
    }
    return 0;											//return false (board has been solved)
}

void solveBackTrack() {									//solves board by trying all possible values

    int  j;                                   			//temp variable
    int  attempted = 0;                               	//testing variable
    bool through = false;                             	//succeeded through the current tests
    bool changedPath = false;                         	//stats variable

    for (int i = 0; i < MAXV; i++) {                 	//for all columns

        if ((i + OFFSET) > attempted)                  	//progression message
            std::cout << "\n\tATTEMPTING ROW NR: " << ++attempted;

        for (int k = 0; k < MAXV; k++) {               	//for all rows

            if (emty[i][k]) {							//if cell is emty

                j = 0;									//resets temp variable
                do {
                    through = false;					//resets variable if changed
                    j++;								//+1 for temp variable
                                                        //		(starts at 0 and becomes "1" (lowest legal value))

                    if (j <= MAXV) {                   	//legal value not overreached
                        if (passedTests(i, k, j)) {    	//goes through all tests
                            sudoku[i][k].value = j;    	//sets new value
                            displayTesting(i, k, j);   	//displays selected testing
                            stats(changedPath);        	//+1 to correct stats
                            through = true;            	//successfully changed cell to valid value
                        }
                    }
                    else {
                        backtrack(i, k, j);
                        changedPath = true;				//stats
                    }
                } while (!through);                    	//not yet successfully changed to legal value
            }
        }
    }
}

bool passedTests(int i, int k, int j) {

    if (testRegion(i, k, j))                           	//tests current region
        if (testX(i, j))                               	//tests current row
            if (testY(k, j))                           	//tests current column
                return 1;								//return true (tests passed)
                                                        
    return 0;
}

void backtrack(int &i, int &k, int &j) {

    do {

        k--;											//reduces columnnumber
        if (k < 0) {									//if transgressed cellnumber
            k = (MAXV - OFFSET);						//changes columnnumber to last
            i--;    									//reduces rownumber
        }
        if (i < 0)
            abortBacktrackSolution(i, k);				//no solution (ending function)

    } while (emty[i][k] == false);						//backtracs until emty cell is found				

    j = sudoku[i][k].value;								//new temp value to try finding higher legal number
    sudoku[i][k].value = 0;								//resets previously emty cell

}

void abortBacktrackSolution(int &i, int &k) {

    std::cout << "\n\n\tERROR: no solution" << std::endl;
    print();
    i = k = (MAXV - OFFSET);							//changes row/columnnumber to last to end quickly
    noSolutionBackTrack = true;

}

void displayTesting(int i, int k, int j) {

    if (testing > 0)									//testing[1] message
        std::cout << "\n\tModified [" << i + OFFSET << "][" << k + OFFSET << "] to: " << j << std::endl;
    if (testing > 1)									//testing[2] print
        print();
}

void stats(bool &changedPath) {

    if (changedPath) {									//changed path to cell with new lwgal value
        changedPath = false;							//resets
        numChangedPath++;								//+1
    }
    changedValue++;										//+1
}

bool fileExists() {
    std::ifstream infile(filename);

    if (infile) {										//if file exists
        return true;
    }
    else {                                             	//message with possibly helpful direcions
        std::cout << "\n\n\tERROR: no file by name \"" << filename << "\" found."
                  << "\n\n\tPlease add said file to same directory using"
                  << "\n\tcorrect format & restart current program.\n";
        return false;
    }
}

void readFromFile() {
    std::ifstream infile(filename);

    for (int i = 0; i < MAXV; i++) {					//for all columns
        for (int k = 0; k < MAXV; k++) {				//for all rows
            char value;
            infile >> value;							//reads in character from file
                                                        
            if (value == '#') {							//crappy way to "ignore" "comments"

                const int MAXTXT = 200;
                char buffer[MAXTXT];
                infile.getline(buffer, MAXTXT);
                k--;
            }
            else {

                value = value - '0';					//converts to integer
                sudoku[i][k].value = value;				//sets value

                for (int j = 0; j < MAXV; j++) {		//for all cells and values
                    sudoku[i][k].alts[j] = true;		//set value as alternative
                }																								
            }
        }
    }
}
