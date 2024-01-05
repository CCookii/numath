#ifndef NUMATH_NUMATH_H
#define NUMATH_NUMATH_H

#ifndef NUMATH_MATRIX_TYPE
#define NUMATH_MATRIX_TYPE double
#endif
typedef NUMATH_MATRIX_TYPE element_t;

#ifndef NUMATH_MATRIX_FORMAT
#define NUMATH_MATRIX_FORMAT "%-8.3f "
#endif

typedef struct SparseCell {
    unsigned index;
    struct SparseCell *next;
    element_t element;
} SparseCell;

typedef SparseCell *SparseHeader;

typedef struct SparseMatrix {
    unsigned nrows;
    unsigned ncols;
    SparseHeader *headers;
} SparseMatrix;

#define NUMATH_SWAP(a, b, T) { T t = a; a = b; b = t; } while (0)

int col_headers(SparseMatrix *matrix);

unsigned headers_len(SparseMatrix *matrix);

void prep_access(SparseMatrix *matrix, unsigned *row, unsigned *col);

SparseHeader *copyHeaders(SparseMatrix *matrix);

SparseCell *createCell(unsigned index, element_t element);

SparseMatrix *createMatrix(unsigned nrows, unsigned ncols);

SparseMatrix *matrixFromArray(const element_t *array, unsigned nrows, unsigned ncols);

SparseCell *copyCell(SparseCell *cell);

SparseMatrix *copyMatrix(SparseMatrix *matrix);

element_t getElement(SparseMatrix *matrix, unsigned row, unsigned col);

element_t setElement(SparseMatrix *matrix, unsigned row, unsigned col, element_t element);

element_t removeElement(SparseMatrix *matrix, unsigned row, unsigned col);

element_t addToElement(SparseMatrix *matrix, unsigned row, unsigned col, element_t element);

element_t subElement(SparseMatrix *matrix, unsigned row, unsigned col, element_t element);

element_t mulElement(SparseMatrix *matrix, unsigned row, unsigned col, element_t element);

element_t divElement(SparseMatrix *matrix, unsigned row, unsigned col, element_t element);

SparseMatrix *transpose(SparseMatrix *matrix);

SparseMatrix *decompLU(SparseMatrix *matrix);

void printMatrix(SparseMatrix *matrix);

void freeCell(SparseCell *cell);

void freeMatrix(SparseMatrix *matrix);

#endif //NUMATH_NUMATH_H
