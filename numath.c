#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "numath.h"

inline int col_headers(SparseMatrix *const matrix) {
    return matrix->ncols < matrix->nrows;
}

inline unsigned headers_len(SparseMatrix *const matrix) {
    return (col_headers(matrix)) ? matrix->ncols : matrix->nrows;
}

inline void prep_access(SparseMatrix *const matrix, unsigned *row, unsigned *col) {
    if (col_headers(matrix)) NUMATH_SWAP(*row, *col, unsigned);
}

SparseMatrix *createMatrix(unsigned nrows, unsigned ncols) {
    SparseMatrix *matrix = (SparseMatrix *) malloc(1 * sizeof(SparseMatrix));
    if (!matrix) {
        fprintf(stderr, "Couldn't allocate memory (Buy more RAM)\n");
        exit(1);
    }
    matrix->nrows = nrows;
    matrix->ncols = ncols;
    matrix->headers = (SparseHeader *) calloc(headers_len(matrix), sizeof(SparseHeader));
    return matrix;
}

SparseCell *createCell(unsigned index, element_t element) {
    SparseCell *cell = (SparseCell *) calloc(1, sizeof(SparseCell));
    if (!cell) {
        fprintf(stderr, "Couldn't allocate memory (Buy more RAM)\n");
        exit(1);
    }
    cell->index = index;
    cell->element = element;
    return cell;
}

element_t getElement(SparseMatrix *const matrix, unsigned int row, unsigned int col) {
    assert(matrix && 0 <= row && row < matrix->nrows && 0 <= col && col < matrix->ncols);
    prep_access(matrix, &row, &col);
    SparseCell *cell = matrix->headers[row];
    while (cell && cell->index < col) cell = cell->next;
    return (cell && cell->index == col) ? cell->element : 0;
}

void setElement(SparseMatrix *const matrix, unsigned int row, unsigned int col, element_t element) {
    assert(matrix && 0 <= row && row < matrix->nrows && 0 <= col && col < matrix->ncols);
    prep_access(matrix, &row, &col);
    SparseCell *cell = matrix->headers[row];
    SparseCell *prev = NULL;
    while (cell && cell->index < col) {
        prev = cell;
        cell = cell->next;
    }
    int cellFound = (cell && cell->index == col);
    if (cellFound && element) cell->element = element;
    else {
        SparseCell *newCell;
        if (!cellFound) {
            newCell = createCell(col, element);
            newCell->next = cell;
        } else {
            newCell = cell->next;
            freeCell(cell);
        }
        if (prev) prev->next = newCell;
        else matrix->headers[row] = newCell;
    }
}

void addToElement(SparseMatrix *const matrix, unsigned int row, unsigned int col, element_t element) {
    assert(matrix && 0 <= row && row < matrix->nrows && 0 <= col && col < matrix->ncols);
    prep_access(matrix, &row, &col);
    SparseCell *cell = matrix->headers[row];
    SparseCell *prev = NULL;
    while (cell && cell->index < col) {
        prev = cell;
        cell = cell->next;
    }
    int cellFound = (cell && cell->index == col);
    if (cellFound && cell->element + element) cell->element += element;
    else {
        SparseCell *newCell;
        if (!cellFound) {
            newCell = createCell(col, element);
            newCell->next = cell;
        } else {
            newCell = cell->next;
            freeCell(cell);
        }
        if (prev) prev->next = newCell;
        else matrix->headers[row] = newCell;
    }
}

void subElement(SparseMatrix *const matrix, unsigned int row, unsigned int col, element_t element) {
    addToElement(matrix, row, col, -element);
}

void mulElement(SparseMatrix *const matrix, unsigned int row, unsigned int col, element_t element) {
    assert(matrix && 0 <= row && row < matrix->nrows && 0 <= col && col < matrix->ncols);
    if (element == 1.0f) return;
    prep_access(matrix, &row, &col);
    SparseCell *cell = matrix->headers[row];
    SparseCell *prev = NULL;
    while (cell && cell->index < col) {
        prev = cell;
        cell = cell->next;
    }
    if (cell && cell->index == col) {
        if (element) cell->element *= element;
        else {
            freeCell(cell);
            if (prev) prev->next = cell->next;
            else matrix->headers[row] = cell->next;
        }
    }
}

void divElement(SparseMatrix *const matrix, unsigned int row, unsigned int col, element_t element) {
    assert(matrix && 0 <= row && row < matrix->nrows && 0 <= col && col < matrix->ncols);
    assert(element);
    if (element == 1.0f) return;
    prep_access(matrix, &row, &col);
    SparseCell *cell = matrix->headers[row];
    while (cell && cell->index < col) {
        cell = cell->next;
    }
    if (cell && cell->index == col) {
        cell->element /= element;
    }
}

void removeElement(SparseMatrix *const matrix, unsigned int row, unsigned int col) {
    setElement(matrix, row, col, 0);
}

void printMatrix(SparseMatrix *const matrix) {
    if (!matrix) {
        fprintf(stderr, "NULL pointer past to printMatrix");
        return;
    }
    if (col_headers(matrix)) {
        SparseCell **cols = (SparseCell **) malloc(matrix->ncols * sizeof(SparseCell *));
        memcpy(cols, matrix->headers, matrix->ncols * sizeof(SparseCell *));
        for (int row = 0; row < matrix->nrows; ++row) {
            printf("| ");
            for (int col = 0; col < matrix->ncols; ++col) {
                SparseCell *cell = cols[col];
                if (cell && cell->index == row) {
                    printf(NUMATH_MATRIX_FORMAT, cell->element);
                    cols[col] = cell->next;
                } else {
                    printf(NUMATH_MATRIX_FORMAT, 0.0f);
                }
            }
            printf("|\n");
        }
        free(cols);
    } else {
        for (unsigned row = 0; row < matrix->nrows; ++row) {
            printf("| ");
            unsigned col = 0;
            for (SparseCell *cell = matrix->headers[row]; cell; cell = cell->next) {
                for (unsigned i = col; i < cell->index; ++i) {
                    printf(NUMATH_MATRIX_FORMAT, 0.0f);
                }
                printf(NUMATH_MATRIX_FORMAT, cell->element);
                col = cell->index + 1;
            }
            for (unsigned i = col; i < matrix->ncols; ++i) {
                printf(NUMATH_MATRIX_FORMAT, 0.0f);
            }
            printf("|\n");
        }
    }
    printf("\n");
}

SparseMatrix *transpose(SparseMatrix *matrix) {
    if (!matrix) return NULL;
    NUMATH_SWAP(matrix->ncols, matrix->nrows, unsigned);
    return matrix;
}

void freeMatrix(SparseMatrix *matrix) {
    if (!matrix) return;
    unsigned len = headers_len(matrix);
    for (int i = 0; i < len; ++i) {
        SparseCell *next;
        for (SparseCell *cell = matrix->headers[i]; cell; cell = next) {
            next = cell->next;
            freeCell(cell);
        }
    }
    free(matrix->headers);
    free(matrix);
}

void freeCell(SparseCell *cell) { free(cell); }

SparseMatrix *decompLU(SparseMatrix *const matrix) {
    SparseMatrix *LU = copyMatrix(matrix);
    unsigned int n = LU->nrows;
    for (int k = 0; k < n - 1; ++k) {
        element_t diag = getElement(LU, k, k);
        assert(diag);
        for (int row = k + 1; row < n; ++row) {
            divElement(LU, row, k, diag);
        }
        for (int j = k + 1; j < n; ++j) {
            element_t LUkj = getElement(LU, k, j);
            for (int i = k + 1; i < n; ++i) {
                subElement(LU, i, j, getElement(LU, i, k) * LUkj);
            }
        }
        printMatrix(LU);
    }
    return LU;
}

SparseCell *copyCell(SparseCell *cell) {
    if (!cell) return NULL;
    SparseCell *copy = createCell(cell->index, cell->element);
    copy->next = copyCell(cell->next);
    return copy;
}

SparseMatrix *copyMatrix(SparseMatrix *const matrix) {
    SparseMatrix *copy = createMatrix(matrix->nrows, matrix->ncols);
    for (int i = 0; i < headers_len(matrix); ++i) {
        copy->headers[i] = copyCell(matrix->headers[i]);
    }
    return copy;
}

SparseMatrix *matrixFromArray(element_t **array, unsigned int nrows, unsigned int ncols) {
    SparseMatrix *matrix = createMatrix(nrows, ncols);
    if (col_headers(matrix)) prep_access(matrix, &nrows, &ncols);
    for (int head = 0; head < nrows; ++head) {
        SparseCell *cell = NULL;
        for (int index = ncols - 1; index >= 0; --index) {
            element_t element = array[head][index];
            if (element) {
                SparseCell *newCell = createCell(index, element);
                newCell->next = cell;
                cell = newCell;
            }
        }
        matrix->headers[head] = cell;
    }
    return matrix;
}

int main(void) {
    SparseMatrix *matrix = createMatrix(6, 5);
    setElement(matrix, 1, 2, 15);
    setElement(matrix, 1, 1, 34);
    setElement(matrix, 1, 3, 20);
    printMatrix(matrix);
    setElement(matrix, 2, 2, 14);
    printMatrix(matrix);
    transpose(matrix);
    printMatrix(matrix);
    addToElement(matrix, 2, 2, 5);
    addToElement(matrix, 3, 4, 2);
    printMatrix(matrix);
    removeElement(matrix, 2, 1);
    printMatrix(matrix);
    printf("Element (3, 1): %f\n", getElement(matrix, 3, 1));
    printf("Element (2, 2): %f\n", getElement(matrix, 2, 2));
    printf("Element (2, 1): %f\n", getElement(matrix, 2, 1));
    mulElement(matrix, 3, 4, 5.2);
    printMatrix(matrix);
    freeMatrix(matrix);
    //////////////////////////////////////
    element_t row1[] = {1, 2, 3, 4},
                row2[] = {1, 4, 9, 16},
                row3[] = {1, 8, 27, 64},
                row4[] = {1, 16, 81, 256};
    element_t *array[] = {
            row1,
            row2,
            row3,
            row4,
    };
    matrix = matrixFromArray(array, 4, 4);
    printMatrix(matrix);
    SparseMatrix *LU = decompLU(matrix);
    printMatrix(LU);
    freeMatrix(LU);
    freeMatrix(matrix);
}
