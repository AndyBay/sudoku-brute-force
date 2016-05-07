#include <iostream>
#include <fstream>
#include <ctime>

const int MAXV = 9;
const int testing = 0;                              //enables testoutput (1 = changed value | 2 = current board progression)
const char filename[15] = "BOARD.DTA";              //edit filename to apply own specific file (in acceptable format)

void print();
void solve();
void backtrackEmtyCell(int &i, int &k);
bool passedTests(int i, int k, int tempValue);
bool testRegion(int i, int k, int tempValue);
bool testX(int i, int tempValue);
bool testY(int k, int tempValue);
void resetPostValues(int i, int k, int iPost, int kPost);
void displayTesting(int i, int k, int tempValue);
void stats(bool &changedPath);
bool fileExists();
void readFromFile();

int  sudoku[MAXV + 1][MAXV + 1];                    //array with all values (starting at 1)
bool    emty[(MAXV * MAXV) + 1];                    //array with free cell / not free cell bool (starting at 1)

long long numChangedPath = 0;                       //stats
long   changedValue = 0;                            //stats
bool noSolution = false;

int main() {

  if (fileExists()) {


    time_t startTime = time(0);                     //starttime
    char currentTime[26];
    ctime_s(currentTime, sizeof currentTime, &startTime);
    std::cout << "\n\tStarting calculating at: " << currentTime << std::endl;

    std::cout << "\tReading from file \"" << filename << "\"" << std::endl;
    readFromFile();

    std::cout << "\n\tORIGINAL BOARD:\n\n";
    print();
    solve();

    if (!noSolution) {
      std::cout << "\n\n\tSOLVED BOARD:\n\n";
      print();

      time_t endTime = time(0);                     //endtime
      ctime_s(currentTime, sizeof currentTime, &endTime);

      std::cout << "\n\tFinnished calculating at: " << currentTime;

      int usedTime = (endTime - startTime);         //amount of time used
      if (usedTime >= 60) {                         //with minutes
        int min = usedTime / 60;
        int sec = usedTime - (min * 60);
        std::cout << "\n\tUsed " << min
                  << " min " << sec;
      }
      else {                                        //seconds (whole)
        std::cout << "\n\tUsed " << endTime - startTime;
      }
      std::cout << " second(s) to calculate given board."
                << "\n\n\tChanged path  " << numChangedPath << " time(s)\n"
                << "\tChanged value " << changedValue << " time(s)\n";
    }
  }
  std::cout << "\n\tEnter to quit: ";
  std::cin.ignore();

  return 0;
}

void print() {

  for (int i = 1; i <= MAXV; i++) {
    for (int k = 1; k <= MAXV; k++) {
      if (k == 1)                                   //start of row
        std::cout << '\t' << sudoku[i][k];
      else                                          //normal
        std::cout << ' ' << sudoku[i][k];
      if (k % 3 == 0)                               //end of region in row
        std::cout << '\t';
      if (k == MAXV)                                //end of row
        std::cout << std::endl;   
      if (i % 3 == 0 && k == MAXV)                  //end of region at end of row
        std::cout << std::endl;
    }
  }

}

void solve() {
  int  tempValue;                                   //tempvalue
  int  attempted = 0;                               //testing variable
  bool through = false;                             //succeeded through the current tests
  bool changedPath = false;                         //stats variable

  for (int i = 1; i <= MAXV; i++) {                 //for all rows

    if (i > attempted)                              //progression message
      std::cout << "\n\tATTEMPTING ROW NR: " << ++attempted;

    for (int k = 1; k <= MAXV; k++) {               //for all cells in row

      tempValue = sudoku[i][k];                     //tempvalue = current value in cell

      if (emty[(MAXV * i) - (MAXV - k)]) {
        do {
          through = false;
          tempValue++;
          if (tempValue <= MAXV) {                  //legal value not overreached
            if (passedTests(i, k, tempValue)) {     //goes through all tests
              sudoku[i][k] = tempValue;             //sets new value
              displayTesting(i, k, tempValue);      //displays selected testing
              stats(changedPath);                   //+1 to correct stats
              through = true;                       //successfully changed cell to valid value
            }
          }
          else {
            int iPost = i;                          //rownumber to reset to
            int kPost = k;                          //columnnumber to reset to (in correct row)

            backtrackEmtyCell(i, k);                //finds nearest cell applicable for change
            tempValue = sudoku[i][k];               //new temp variable to try finding higher legal number
            resetPostValues(i, k, iPost, kPost);    //resets changed values up to specific point
            changedPath = true;                     //stats
          }
        } while (!through);                         //not yet successfully changed to legal value
      }
    }
  }
}

void backtrackEmtyCell(int &i, int &k) {

  do {
    k--;                                            //reduces columnnumber
    if (k < 1) {                                    //if transgressed cellnumber
      k = MAXV;                                     //changes columnnumber to last (9)
      i--;                                          //reduces rownumber
    }
    if (i < 1) {                                    //NO SOLUTION
      std::cout << "\n\n\tERROR: no solution" << std::endl;
      i = k = MAXV;                                 //changes row/columnnumber to last to end quickly
      noSolution = true;
    }                                               //backtracs until emty cell is found
  } while (!emty[(MAXV * i) - (MAXV - k)]);
}

bool passedTests(int i, int k, int tempValue) {

  if (testRegion(i, k, tempValue))                  //tests current region
    if (testX(i, tempValue))                        //tests current row
      if (testY(k, tempValue))                      //tests current column
        return 1;

  return 0;
}

bool testRegion(int i, int k, int tempValue) {

  while (i % 3 != 1)  i--;                          //finds top row in region
  while (k % 3 != 1)  k--;                          //finds first cell/columnnumber in region

  for (int j = i; j <= i + 2; j++)                  //for all rows in region
    for (int h = k; h <= k + 2; h++)                //for all columns in region
      if (tempValue == sudoku[j][h])                //if same value
        return 0;

  return 1;
}

bool testX(int i, int tempValue) {                  //testing row

  for (int k = 1; k <= MAXV; k++)                   //for all columns in row
    if (tempValue == sudoku[i][k])                  //if same value
      return 0;

  return 1;
}

bool testY(int k, int tempValue) {                  //testing column

  for (int i = 1; i <= MAXV; i++)                   //for all rows in column
    if (tempValue == sudoku[i][k])                  //if same value
      return 0;

  return 1;
}

void resetPostValues(int iBack, int kBack, int iPost, int kPost) {

  for (int i = iBack; i <= iPost; i++)              //for all rows between starting and endpoint
    for (int k = 1; k <= MAXV; k++) {               //for all columns between starting and endpoint
      if (i == iPost && k == kPost + 1) {           //startingpoint for backtracking reached
        break;
      }
      if (i == iBack && k == 1)                     //startingpoint
        k = kBack + 1;
      if (k > MAXV) {                               //if transgressed cellnumber
        k = 1;                                      //changes columnnumber to first (1)
        i++;                                        //increases rownumber
      }
      if (emty[(MAXV * i) - (MAXV - k)])            //emty cell
        sudoku[i][k] = 0;                           //resets previously emty cell
    }
}

void displayTesting(int i, int k, int tempValue) {

  if (testing > 0)                                  //testing[1] message
    std::cout << "\n\tModified [" << i << "][" << k << "] to: " << tempValue << std::endl;
  if (testing > 1)                                  //testing[2] print
    print();
}

void stats(bool &changedPath) {

  if (changedPath) {                                //changed path to cell with new lwgal value
    changedPath = false;                            //resets
    numChangedPath++;                               //+1
  }
  changedValue++;                                   //+1
}

bool fileExists() {
  std::ifstream infile(filename);

  if (infile) {
    return true;
  }
  else {                                            //message with possibly helpful direcions
    std::cout << "\n\n\tERROR: no file by name \"" << filename << "\" found."
              << "\n\n\tPlease add said file to same directory using"
              << "\n\tcorrect format & restart current program.\n";
    return false;
  }
}

void readFromFile() {
  std::ifstream infile(filename);

  for (int i = 1; i <= MAXV; i++) {
    for (int k = 1; k <= MAXV; k++) {
      char value;
      infile >> value;                              //reads in character from file
      value = value - '0';                          //converts to integer
      sudoku[i][k] = value;                         //sets value
      if (value == 0)                               //sets emty/ not emty cell
        emty[(MAXV * i) - (MAXV - k)] = 1;
      else
        emty[(MAXV * i) - (MAXV - k)] = 0;
    }
  }
}