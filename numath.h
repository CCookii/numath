#ifndef NUMATH_NUMATH_H
#define NUMATH_NUMATH_H

#ifndef NUMATH_TYPE
#define NUMATH_TYPE int
#endif
typedef NUMATH_TYPE element_t;

typedef struct SparseCell {
    unsigned row;
    unsigned col;
    struct SparseCell* next;
    element_t element;
} SparseCell;

typedef SparseCell* SparseHeader;

typedef struct SparseMatrix {
    unsigned nrows;
    unsigned ncols;
    SparseHeader *headers;
} SparseMatrix;

#define NUMATH_SWAP(a, b, T) { T t = a; a = b; b = t; } while (0)

#define COL_HEADERS(matrix) (matrix->ncols < matrix->nrows)

#define PREP_ACCESS(matrix, row, col) if (COL_HEADERS(matrix)) NUMATH_SWAP(row, col, unsigned)

SparseMatrix *createMatrix(unsigned nrows, unsigned ncols);

SparseCell *createCell(unsigned int row, unsigned int col, element_t element);

element_t get(SparseMatrix *matrix, unsigned row, unsigned col);

void add(SparseMatrix *matrix, unsigned row, unsigned col, element_t element);

void printMatrix(SparseMatrix *matrix);

void freeMatrix(SparseMatrix *matrix);

void freeCell(SparseCell *cell);

#endif //NUMATH_NUMATH_H
