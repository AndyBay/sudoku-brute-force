#include <iostream>
#include <fstream>
#include <ctime>

const int MAXV = 9;
const int testing = 0;                              //enables testoutput (1 = changed value | 2 = current board progression)
const char filename[15] = "BOARD.DTA";              //edit filename to apply own specific file

void print();
void solve();
void backtrackEmtyCell(int i, int k);
bool testRegion(int i, int k, int tempValue);
bool testX(int i, int tempValue);
bool testY(int k, int tempValue);
void resetPostValues(int i, int k, int iPost, int kPost);
bool fileExists();
void readFromFile();

int  sudoku[MAXV + 1][MAXV + 1];                    //array with all values (starting at 1)
bool    emty[(MAXV * MAXV) + 1];                    //array with free cell / not free cell bool (starting at 1)

long numChangedPath = 0;                            //stats
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
  int  tempValue;
  int  attempted = 0;                               //testing variable

  bool through = false;                             //succeeded through the current tests

  bool changedPath = false;                         //stats variable

  for (int i = 1; i <= MAXV; i++) {

    if (i > attempted)                              //progression message
      std::cout << "\n\tATTEMPTING ROW NR: " << ++attempted;

    for (int k = 1; k <= MAXV; k++) {

      tempValue = sudoku[i][k];

      if (emty[(MAXV * i) - (MAXV - k)]) {
        do {
          through = false;
          tempValue++;
          if (tempValue <= MAXV) {                  //legal value not overreached
            if (testRegion(i, k, tempValue)) {      //tests region
              if (testX(i, tempValue)) {            //tests row
                if (testY(k, tempValue)) {          //tests column
                  sudoku[i][k] = tempValue;         //sets new value
                  if (testing > 0)                  //testing[1] message
                    std::cout << "\n\tModified [" << i << "][" << k << "] to: " << tempValue << std::endl;
                  if (testing > 1)                  //testing[2] print
                    print();
                  if (changedPath) {                //stats
                    changedPath = false;
                    numChangedPath++;
                  }
                  changedValue++;                   //stats
                  through = true;
                }
              }
            }
          }
          else {
            int iPost = i;                          //rownumber to reset to
            int kPost = k;                          //columnnumber to reset to (if correct rownumber)

            do {                                    //finds nearest cell applicable for change
              k--;                                  //changes columnnumber
              if (k < 1) {                          //changes rownumber
                k = MAXV;
                i--;
              }
              if (i < 1) {                          //NO SOLUTION
                std::cout << "\n\n\tERROR: no solution" << std::endl;
                i = k = MAXV;
                noSolution = true;
              }                                     //backtracks until emty cell is found
            } while (!emty[(MAXV * i) - (MAXV - k)]);

            tempValue = sudoku[i][k];               //new temp variable to try finding higher legal number
            resetPostValues(i, k, iPost, kPost);    //resets changed values up to specific point
            changedPath = true;                     //stats
          }
        } while (!through);
      }
    }
  }
}

void backtrackEmtyCell(int i, int k) {

  do {
    k--;                                            //changes columnnumber
    if (k < 1) {                                    //changes rownumber
      k = MAXV;
      i--;
    }
    if (i < 1) {                                    //NO SOLUTION
      std::cout << "\n\n\tERROR: no solution" << std::endl;
      i = k = MAXV;
      noSolution = true;
    }                                               //backtracs until emty cell is found
  } while (!emty[(MAXV * i) - (MAXV - k)]);
}

bool testRegion(int i, int k, int tempValue) {

  int i2 = i;                                       //copies value to change freely
  int k2 = k;                                       //copies value to change freely

  while (i2 % 3 != 1)  i2--;                        //finds top row in region
  while (k2 % 3 != 1)  k2--;                        //finds first cell/columnnumber in region

  for (int j = i2; j <= i2 + 2; j++) {
    for (int h = k2; h <= k2 + 2; h++) {
      if (tempValue == sudoku[j][h]) {
        return 0;
      } 
    }
  }
  return 1;
}

bool testX(int i, int tempValue) {                  //testing row

  for (int k = 1; k <= MAXV; k++) {
    if (tempValue == sudoku[i][k]) {
      return 0;
    }
  }
  return 1;
}

bool testY(int k, int tempValue) {                  //testing column

  for (int i = 1; i <= MAXV; i++) {
    if (tempValue == sudoku[i][k]) {
      return 0;
    }
  }
  return 1;
}

void resetPostValues(int iBack, int kBack, int iPost, int kPost) {

  for (int i = iBack; i <= iPost; i++) {
    for (int k = 1; k <= MAXV; k++) {
      if (i == iPost && k == kPost + 1) {           //startingpoint for backtracking reached
        break;
      }
      if (i == iBack && k == 1) {                   //row starting resetting from
        k = kBack + 1;
        if (k > MAXV) {                             //next row
          k = 1;
          i++;
        }
      }
      if (emty[(MAXV * i) - (MAXV - k)]) {
        sudoku[i][k] = 0;                           //resets previously emty cell
      }
    }
  }
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
      if (value == 0)                               //emty/ not emty cell
        emty[(MAXV * i) - (MAXV - k)] = 1;
      else
        emty[(MAXV * i) - (MAXV - k)] = 0;
    }
  }
}