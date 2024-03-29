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

inline SparseHeader *copyHeaders(SparseMatrix *matrix) {
    unsigned int len = headers_len(matrix);
    SparseCell **copy = (SparseCell **) malloc(len * sizeof(SparseCell *));
    memcpy(copy, matrix->headers, len * sizeof(SparseCell *));
    return copy;
}

SparseCell *createCell(unsigned index, element_t element, SparseCell *next) {
    SparseCell *cell = (SparseCell *) calloc(1, sizeof(SparseCell));
    if (!cell) {
        fprintf(stderr, "Couldn't allocate memory (Buy more RAM)\n");
        exit(1);
    }
    cell->index = index;
    cell->element = element;
    cell->next = next;
    return cell;
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

SparseCell *copyCell(SparseCell *cell) {
    if (!cell) return NULL;
    SparseCell *copy = createCell(cell->index, cell->element, copyCell(cell->next));
    return copy;
}

SparseMatrix *copyMatrix(SparseMatrix *const matrix) {
    SparseMatrix *copy = createMatrix(matrix->nrows, matrix->ncols);
    for (unsigned i = 0; i < headers_len(matrix); ++i) {
        copy->headers[i] = copyCell(matrix->headers[i]);
    }
    return copy;
}

SparseMatrix *matrixFromArray(const element_t *array, unsigned int nrows, unsigned int ncols) {
    SparseMatrix *matrix = createMatrix(nrows, ncols);
    if (col_headers(matrix)) prep_access(matrix, &nrows, &ncols);
    for (unsigned row = 0; row < nrows; ++row) {
        SparseCell *cell = NULL;
        for (int col = (int) ncols - 1; col >= 0; --col) {
            element_t element = (col_headers(matrix)) ? array[col * nrows + row] : array[row * ncols + col];
            if (element) {
                SparseCell *newCell = createCell(col, element, cell);
                cell = newCell;
            }
        }
        matrix->headers[row] = cell;
    }
    return matrix;
}

element_t getElement(SparseMatrix *const matrix, unsigned int row, unsigned int col) {
    assert(matrix && row < matrix->nrows && col < matrix->ncols);
    prep_access(matrix, &row, &col);
    SparseCell *cell = matrix->headers[row];
    while (cell && cell->index < col) cell = cell->next;
    return (cell && cell->index == col) ? cell->element : 0;
}

element_t setElement(SparseMatrix *const matrix, unsigned int row, unsigned int col, element_t element) {
    assert(matrix && row < matrix->nrows && col < matrix->ncols);
    prep_access(matrix, &row, &col);
    SparseCell *cell = matrix->headers[row];
    SparseCell *prev = NULL;
    while (cell && cell->index < col) {
        prev = cell;
        cell = cell->next;
    }
    int cellFound = (cell && cell->index == col);
    element_t oldValue;
    if (cellFound && element) {
        oldValue = cell->element;
        cell->element = element;
    } else {
        SparseCell *newCell;
        if (!cellFound) {
            oldValue = 0;
            newCell = createCell(col, element, cell);
        } else {
            oldValue = cell->element;
            newCell = cell->next;
            freeCell(cell);
        }
        if (prev) prev->next = newCell;
        else matrix->headers[row] = newCell;
    }
    return oldValue;
}

element_t removeElement(SparseMatrix *const matrix, unsigned int row, unsigned int col) {
    return setElement(matrix, row, col, 0);
}

element_t addToElement(SparseMatrix *const matrix, unsigned int row, unsigned int col, element_t element) {
    assert(matrix && row < matrix->nrows && col < matrix->ncols);
    if (!element) return getElement(matrix, row, col);
    prep_access(matrix, &row, &col);
    SparseCell *cell = matrix->headers[row];
    SparseCell *prev = NULL;
    while (cell && cell->index < col) {
        prev = cell;
        cell = cell->next;
    }
    int cellFound = (cell && cell->index == col);
    if (cellFound && cell->element + element) {
        cell->element += element;
        return cell->element;
    } else {
        SparseCell *newCell;
        element_t newValue;
        if (!cellFound) {
            newCell = createCell(col, element, cell);
            newValue = element;
        } else {
            newCell = cell->next;
            freeCell(cell);
            newValue = 0;
        }
        if (prev) prev->next = newCell;
        else matrix->headers[row] = newCell;
        return newValue;
    }
}

element_t subElement(SparseMatrix *const matrix, unsigned int row, unsigned int col, element_t element) {
    return addToElement(matrix, row, col, -element);
}

element_t mulElement(SparseMatrix *const matrix, unsigned int row, unsigned int col, element_t element) {
    assert(matrix && row < matrix->nrows && col < matrix->ncols);
    if (element == 1.0f) return getElement(matrix, row, col);
    prep_access(matrix, &row, &col);
    SparseCell *cell = matrix->headers[row];
    SparseCell *prev = NULL;
    while (cell && cell->index < col) {
        prev = cell;
        cell = cell->next;
    }
    if (cell && cell->index == col) {
        if (element) {
            cell->element *= element;
            return cell->element;
        } else {
            freeCell(cell);
            if (prev) prev->next = cell->next;
            else matrix->headers[row] = cell->next;
        }
    }
    return 0;
}

element_t divElement(SparseMatrix *const matrix, unsigned int row, unsigned int col, element_t element) {
    assert(matrix && row < matrix->nrows && col < matrix->ncols);
    assert(element);
    if (element == 1.0f) return getElement(matrix, row, col);
    prep_access(matrix, &row, &col);
    SparseCell *cell = matrix->headers[row];
    while (cell && cell->index < col) {
        cell = cell->next;
    }
    if (cell && cell->index == col) {
        cell->element /= element;
        return cell->element;
    }
    return 0;
}

SparseMatrix *switch_header_position(SparseMatrix *matrix, int switched) {
    if (!matrix) return NULL;
    unsigned oldlen = (switched) ? matrix->nrows + matrix->ncols - headers_len(matrix) : headers_len(matrix);
    SparseHeader *headers = (SparseHeader *) calloc(matrix->nrows + matrix->ncols - oldlen, sizeof(SparseHeader));
    for (int head = (int) oldlen - 1; head >= 0; --head) {
        SparseCell *cell = matrix->headers[head];
        while (cell) {
            SparseCell *next = cell->next;
            cell->next = headers[cell->index];
            headers[cell->index] = cell;
            cell->index = head;
            cell = next;
        }
    }
    free(matrix->headers);
    matrix->headers = headers;
    return matrix;
}

SparseMatrix *transpose(SparseMatrix *matrix) {
    if (!matrix) return NULL;
    if (matrix->nrows != matrix->ncols) {
        NUMATH_SWAP(matrix->ncols, matrix->nrows, unsigned);
    } else {
        switch_header_position(matrix, 0);
    }
    return matrix;
}

#if 0
SparseMatrix *decompLU(SparseMatrix *const matrix) {
    assert(matrix->ncols == matrix->nrows);
    SparseMatrix *LU = copyMatrix(matrix);
    unsigned int n = LU->nrows;
    for (int k = 0; k < n - 1; ++k) {
        element_t diag = getElement(LU, k, k);
        assert(diag);
        for (int i = k + 1; i < n; ++i) {
            element_t LUik = divElement(LU, i, k, diag);
            if (!LUik) continue;
            for (int j = k + 1; j < n; ++j) {
                subElement(LU, i, j, getElement(LU, k, j) * LUik);
            }
        }
//        printMatrix(LU);
    }
    return LU;
}
#else

SparseMatrix *decompLU(SparseMatrix *const matrix) {
    assert(matrix->ncols == matrix->nrows);
    SparseMatrix *LU = copyMatrix(matrix);
    unsigned int n = LU->nrows;
    SparseCell **rows = copyHeaders(LU);
    for (unsigned k = 0; k < n - 1; ++k) {
        SparseCell *topCell = rows[k];
        assert(topCell && topCell->index == k);
        element_t diag = topCell->element;
        assert(diag);

        for (unsigned r = k + 1; r < n; ++r) {
            topCell = rows[k]->next;
            SparseCell *curCell = rows[r];
            if (!curCell || curCell->index != k) continue;
            curCell->element /= diag;
            element_t factor = curCell->element;
            SparseCell *prev = curCell;
            curCell = curCell->next;
            rows[r] = curCell;
            while (topCell) {
                while (curCell && curCell->index < topCell->index) {
                    prev = curCell;
                    curCell = curCell->next;
                }
                if (curCell && curCell->index == topCell->index) {
                    curCell->element -= topCell->element * factor;
                    if (!curCell->element) {
                        prev->next = curCell->next;
                        free(curCell);
                        curCell = prev->next;
                    }
                } else {
                    prev->next = createCell(topCell->index, -topCell->element * factor, curCell);
                    prev = prev->next;
                }
                topCell = topCell->next;
            }
        }
    }
    free(rows);
    return LU;
}

#endif

inline element_t mulHeaders(SparseCell *one, SparseCell *other) {
    element_t result = 0;
    while (one && other) {
        if (one->index == other->index) {
            result += one->element * other->element;
            other = other->next;
        }
        while (one && other && one->index < other->index) one = one->next;
        while (one && other && other->index < one->index) other = other->next;
    }
    return result;
}

SparseMatrix *mulMatrix(SparseMatrix *one, SparseMatrix *other) {
    assert(one->ncols == other->nrows);
    SparseMatrix *result = createMatrix(one->nrows, other->ncols);
    int t1 = col_headers(one), t2 = !col_headers(other), tr = col_headers(result);
    if (t1) switch_header_position(one, 0);
    if (t2) switch_header_position(other, 0);
    unsigned r, c;
    for (int row = (int) result->nrows - 1; row >= 0; --row) {
        for (int col = (int) result->ncols - 1; col >= 0; --col) {
            r = tr ? col : row, c = row + col - r;
            element_t element = mulHeaders(one->headers[row], other->headers[col]);
            if (element) {
                SparseCell *cell = createCell(c, element, result->headers[r]);
                result->headers[r] = cell;
            }
        }
    }
    if (t1) switch_header_position(one, 1);
    if (t2) switch_header_position(other, 1);
    return result;
}

void printMatrix(SparseMatrix *const matrix) {
    if (!matrix) {
        fprintf(stderr, "NULL pointer past to printMatrix");
        return;
    }
    printf("\n");
    if (col_headers(matrix)) {
        SparseCell **cols = copyHeaders(matrix);
        for (unsigned row = 0; row < matrix->nrows; ++row) {
            printf("| ");
            for (unsigned col = 0; col < matrix->ncols; ++col) {
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
}

void freeMatrix(SparseMatrix *matrix) {
    if (!matrix) return;
    unsigned len = headers_len(matrix);
    for (unsigned i = 0; i < len; ++i) {
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

void testBasic() {
    SparseMatrix *matrix;
    matrix = createMatrix(6, 5);
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
    printf("////////////////////////////////////////////\n");
}

void testLU() {
    element_t array[] = {1, 2, 3, 4,
                         0, 4, 9, 0,
                         1, 8, 0, 0,
                         0, 0, 81, 256};
    SparseMatrix *matrix = matrixFromArray(array, 4, 4);
    printMatrix(matrix);
    SparseMatrix *LU = decompLU(matrix);
    printMatrix(LU);
    freeMatrix(LU);
    freeMatrix(matrix);
    printf("////////////////////////////////////////////\n");
}

void testTranspose() {
    element_t array[] = {1, 2, 3,
                         0, 4, 9,
                         1, 8, 0,
                         0, 0, 81};
    SparseMatrix *matrix = matrixFromArray(array, 4, 3);
    printMatrix(matrix);
    printMatrix(transpose(matrix));
    freeMatrix(matrix);
    element_t array2[] = {1, 2, 3, 4,
                          0, 4, 9, 0,
                          1, 8, 0, 0,
                          0, 0, 81, 256};
    matrix = matrixFromArray(array2, 4, 4);
    printMatrix(matrix);
    printMatrix(transpose(matrix));
    printf("////////////////////////////////////////////\n");
}

int main(void) {
    testBasic();
    testLU();
    testTranspose();
    element_t m1[] = {
            1, 2, //7, 6,
            32, 5,// 7, 4,
            6, 8, //3, 6,
    };
    element_t m2[] = {
            5, 7,
            3, 4,
            //9, 4,
            //7, 2,
    };
    SparseMatrix *one = matrixFromArray(m1, 3, 2);
    SparseMatrix *other = matrixFromArray(m2, 2, 2);
    SparseMatrix *result = mulMatrix(one, other);
    printMatrix(one);
    printMatrix(other);
    printMatrix(result);
    freeMatrix(one);
    freeMatrix(other);
    freeMatrix(result);
}
