#include <iostream>
#include <omp.h>
#include <pthread.h>
#include <time.h>

using namespace std;
class Grid;

class Position {
public:
  int line;
  int column;
  Position(int line, int column) : line(line), column(column) {}
  ~Position() {
    delete &line;
    delete &column;
  }
};

class Cell {
public:
  Cell(int N, int line, int column) : N(N), line(line), column(column) {}
  ~Cell() {
    delete &N;
    delete &line;
    delete &column;
  }

  Position *getLeft() {
    int _column = column == 0 ? (N - 1) : (column - 1);
    return new Position(this->line, _column);
  }

  Position *getRight() {
    int _column = column == (N - 1) ? 0 : (column + 1);
    return new Position(this->line, _column);
  }

  Position *getUpper() {
    int _line = line == 0 ? (N - 1) : (line - 1);
    return new Position(_line, this->column);
  }

  Position *getLower() {
    int _line = line == (N - 1) ? 0 : (line + 1);
    return new Position(_line, this->column);
  }

private:
  int N;
  int line;
  int column;
};

class Grid {
public:
  float **grid;
  int N;
  Grid(int N) : N(N) {
    grid = (float **)malloc(N * sizeof(float *));
    for (int lineIndex = 0; lineIndex < N; lineIndex++) {
      float *_line = (float *)malloc(N * sizeof(float));
      for (int columnIndex = 0; columnIndex < N; columnIndex++) {
        _line[columnIndex] = 0;
      }
      grid[lineIndex] = _line;
    }
  }
  ~Grid() {
    for (int lineIndex = 0; lineIndex < N; lineIndex++) {
      free(grid[lineIndex]);
    }
    free(grid);
    // delete &N;
  }

  float getCellValue(int line, int column) { return this->grid[line][column]; }

  Cell *getCell(int line, int column) { return new Cell(N, line, column); }

  float getCellNeighboursAverage(int line, int column) {
    Cell *cell = this->getCell(line, column);
    Position *upper = cell->getUpper();
    Position *lower = cell->getLower();
    Position *left = cell->getLeft();
    Position *right = cell->getRight();
    Position *postions[] = {
      upper,
      lower,
      left,
      right,
      new Position(upper->line, left->column),
      new Position(upper->line, right->column),
      new Position(lower->line, left->column),
      new Position(lower->line, right->column)
    };
    int sum = 0;
    #pragma omp parallel for reduction (+:sum)
    for (int index = 0; index < 8; index++) {
      Position *_position = postions[index];
      sum += this->getCellValue(_position->line, _position->column);
      free(postions[index]);
    }
    free(cell);
    return sum / 8.;
  }

  int countCellNeighbours(int line, int column) {
    Cell *cell = this->getCell(line, column);
    Position *upper = cell->getUpper();
    Position *lower = cell->getLower();
    Position *left = cell->getLeft();
    Position *right = cell->getRight();
    Position *postions[] = {
      upper,
      lower,
      left,
      right,
      new Position(upper->line, left->column),
      new Position(upper->line, right->column),
      new Position(lower->line, left->column),
      new Position(lower->line, right->column)
    };
    int count = 0;
    #pragma omp parallel for reduction (+:count)
    for (int index = 0; index < 8; index++) {
      Position *_position = postions[index];
      if (this->getCellValue(_position->line, _position->column))
        count++;
      free(postions[index]);
    }
    free(cell);
    return count;
  }

  float getNextCellState(int line, int column) {
    int neighbours = this->countCellNeighbours(line, column);
    float cellValue = this->getCellValue(line, column);
    if (cellValue > 0 && (neighbours == 2 || neighbours == 3))
      return cellValue; // caso 1
    if (cellValue == 0 && neighbours == 3) return 1; // caso 2
    return 0;                                              // caso 3
  }

  Grid *getNewGrid() {
    Grid *newGrid = new Grid(N);
    for (int lineIndex = 0; lineIndex < N; lineIndex++) {
      for (int columnIndex = 0; columnIndex < N; columnIndex++) {
        newGrid->grid[lineIndex][columnIndex] = this->getNextCellState(lineIndex, columnIndex);
      }
    }
    return newGrid;
  }


  Grid *getNewGrid(int numThreads) {
    Grid *newGrid = new Grid(N);
#pragma omp parallel for num_threads(numThreads)
    for (int lineIndex = 0; lineIndex < N; lineIndex++) {
      for (int columnIndex = 0; columnIndex < N; columnIndex++) {
        newGrid->grid[lineIndex][columnIndex] = this->getNextCellState(lineIndex, columnIndex);
      }
    }
    return newGrid;
  }

  int countRemainingLivingCells() {
    int c, l, livingCells = 0;
    for (l = 0; l < N; l++) {
      for (c = 0; c < N; c++) {
        if (this->grid[l][c] > 0)
          livingCells++;
      }
    }
    return livingCells;
  }

  void printAll() {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        if (this->grid[i][j] == 0) {
          printf("□");
          continue;
        }
        printf("■");
        // printf("%.2f ", this->grid[i][j]);
      }
      cout << "\n";
    }
  }

  void print() {
    int max = N >= 50 ? 50 : N;
    printf("  ");
    for (int i = 0; i < max; i++) {
      if (i % 2 == 0) {
        printf("▬▬"); 
      }
      else {
        printf("▭▭");
      }
    }
    printf("\n");
    for (int i = 0; i < max; i++) {
      if (i % 2 == 0) {
        printf(" ▌");
      }
      else {
        printf("  ");
      }
      for (int j = 0; j < max; j++) {
        if (this->grid[i][j] == 0) {
          printf("  ");
        } else if (this->grid[i][j] <= .33) {
          printf("░░");
        } else if (this->grid[i][j] <= .66) {
          printf("▒▒");
        } else if (this->grid[i][j] <= .99) {
          printf("▓▓");
        } else {
          printf("██");
        }
      }
      if (i % 2 == 0) {
        printf("▌");
      }
      else {
        printf("  ");
      }
      printf("\n");
    }
    printf("  ");
    for (int i = 0; i < max; i++) {
      if (i % 2 == 0) {
        printf("▬▬"); 
      }
      else {
        printf("▭▭");
      }
    }
    printf("\n");
  }
};

Grid *play(int generations, int numThreads, Grid *grid) {
  int i;
  Grid *newGrid, *_grid=grid->getNewGrid();
  cout << "Geração: 1 - " << _grid->countRemainingLivingCells() << endl;
  for (i = 0; i < generations - 1; i++) {
    newGrid = _grid->getNewGrid();
    delete _grid;
    _grid = newGrid;
    cout << "Geração: " << i + 2 << " - " << _grid->countRemainingLivingCells() << " células vivas." << endl;
  }
  return _grid;
}

int main() {
  double start, end;
  start = omp_get_wtime();
  int numThreads = 8, generations = 3, N = 2048;
  Grid *grid = new Grid(N);
  int lin = 1, col = 1;
  grid->grid[lin  ][col+1] = 1.0;
  grid->grid[lin+1][col+2] = 1.0;
  grid->grid[lin+2][col  ] = 1.0;
  grid->grid[lin+2][col+1] = 1.0;
  grid->grid[lin+2][col+2] = 1.0;


  //R-pentomino
  lin =10; col = 30;
  grid->grid[lin  ][col+1] = 1.0;
  grid->grid[lin  ][col+2] = 1.0;
  grid->grid[lin+1][col  ] = 1.0;
  grid->grid[lin+1][col+1] = 1.0;
  grid->grid[lin+2][col+1] = 1.0;
  Grid *finalGrid = play(generations, numThreads, grid);
  printf("\nthere are still {%i} living cells\n",
         finalGrid->countRemainingLivingCells());
  end = omp_get_wtime();
  printf(" took %f seconds.\n", end - start);
}
// g++ main.cpp -fopenmp -o main