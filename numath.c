#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "numath.h"

SparseMatrix *createMatrix(unsigned int nrows, unsigned int ncols) {
    SparseMatrix *matrix = (SparseMatrix*) malloc(1*sizeof(SparseMatrix));
    matrix->nrows = nrows;
    matrix->ncols = ncols;
    matrix->headers = (SparseHeader*) calloc((COL_HEADERS(matrix)) ? ncols : nrows, sizeof(SparseHeader));
    return matrix;
}

element_t get(SparseMatrix *matrix, unsigned int row, unsigned int col) {
    assert(0 <= row && row < matrix->nrows && 0 <= col && col < matrix->ncols);
    PREP_ACCESS(matrix, row, col);
    SparseCell *cell = matrix->headers[row];
    while (cell && cell->col < col) cell = cell->next;
    return (cell && cell->col == col) ? cell->element : 0;
}

void add(SparseMatrix *matrix, unsigned int row, unsigned int col, element_t element) {
    assert(0 <= row && row < matrix->nrows && 0 <= col && col < matrix->ncols);
    unsigned r = row, c = col;
    PREP_ACCESS(matrix, row, col);
    SparseCell *cell = matrix->headers[row];
    if (!cell) matrix->headers[row] = createCell(r, c, element);
    else {
        SparseCell *prev = NULL;
        while (cell && cell->row < row) {
            prev = cell;
            cell = cell->next;
        }
        if (cell->row == row) cell->element = element;
        else {
            prev->next = createCell(r, c, element);
            prev->next->next = cell;
        }
    }
}

SparseCell *createCell(unsigned int row, unsigned int col, element_t element) {
    SparseCell *cell = (SparseCell*) calloc(1, sizeof(SparseCell));
    cell->row = row;
    cell->col = col;
    cell->element = element;
    return cell;
}

void printMatrix(SparseMatrix *matrix) {
    if (COL_HEADERS(matrix)) {
        SparseCell* cols[matrix->ncols];
    } else {
        for (unsigned row = 0; row < matrix->nrows; ++row) {
            printf("| ");
            unsigned col = 0;
            for (SparseCell *cell = matrix->headers[row]; cell ; cell = cell->next) {
                for (unsigned i = col; i < cell->col; ++i) {
                    printf("%-4d ", 0);
                }
                printf("%-4d ", cell->element);
                col = cell->col + 1;
            }
            for (unsigned i = col; i < matrix->ncols; ++i) {
                printf("%-4d ", 0);
            }
            printf("|\n");
        }
    }
}

void freeMatrix(SparseMatrix *matrix) {
    unsigned len = (COL_HEADERS(matrix)) ? matrix->ncols : matrix->nrows;
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

int main(void) {
    SparseMatrix *matrix = createMatrix(5,6);
    add(matrix, 1, 2, 15);
    add(matrix, 2, 2, 104);
    add(matrix, 3, 4, 2);
    printMatrix(matrix);
    freeMatrix(matrix);
}
