/**
 * (C) Copyright 2020
 */

#ifndef PROCESSER_UTILS_H_
#define PROCESSER_UTILS_H_

#include <stdio.h>
#include "/usr/include/mpi/mpi.h"

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

// Smoothing filter
const float SMOOTHING[3][3] = {
    {1.f / 9, 1.f / 9, 1.f / 9},
    {1.f / 9, 1.f / 9, 1.f / 9},
    {1.f / 9, 1.f / 9, 1.f / 9}
};

// Approximative Gaussian Blur filter
const float GAUSSIAN[3][3] = {
    {1.f / 16, 2.f / 16, 1.f / 16},
    {2.f / 16, 4.f / 16, 2.f / 16},
    {1.f / 16, 2.f / 16, 1.f / 16}
};

// Sharpen filter
const float SHARPEN[3][3] = {
    {       0, -2.f / 3,        0},
    {-2.f / 3, 11.f / 3, -2.f / 3},
    {       0, -2.f / 3,        0}
};

// Mean removal filter
const float MEAN_REMOVAL[3][3] = {
    {-1.f, -1.f, -1.f},
    {-1.f,  9.f, -1.f},
    {-1.f, -1.f, -1.f}
};

// Emboss filter
const float EMBOSS[3][3] = {
    {0.f,  1.f, 0.f},
    {0.f,  0.f, 0.f},
    {0.f, -1.f, 0.f}
};

/**
 * Verifica daca fisierul s-a deschis cu succes sau nu.
 * 
 * @param file = descriptorul de fisier
 */
void check_file(FILE *file) {
    if (!file) {
        fprintf(stderr, "Eroare la deschiderea fisierelor!\n");
        MPI_Finalize();
        exit(-1);
    }
}

/**
 * Verifica daca s-a putut aloca memorie pentru structura de date sau nu.
 * 
 * @param ptr  = pointerul catre container
 * @param file = fisierul ce trebuie inschis inainte de inchiderea programului
 */
void check_container(void *ptr, FILE *file) {
    if (!ptr) {
        fprintf(stderr, "Eroare la alocarea memoriei!\n");
        if (file) {
            fclose(file);
        }
        MPI_Finalize();
        exit(-1);
    }
}

/**
 * Aplica un filtru pe o imagine.
 * 
 * @param height      = inaltimea fragmentului de imagine editat
 * @param width       = latimea fragmentului de imagine editat
 * @param channels    = numarul de canale al imaginii (color / alb-negru)
 * @param maxval      = valoarea maxima a unui pixel
 * @param image       = matricea imaginii
 * @param filter_name = filtrul ce trebuie aplicat
 */
void apply_filter(int height, int width, uint8_t channels, uint8_t maxval,
                  uint8_t **image, char *filter_name) {
    int i, j;
    float pixel;
    uint8_t **aux;
    const float (*filter_matrix)[3];

    if (!strcmp(filter_name, "smooth")) {
        filter_matrix = SMOOTHING;
    } else if (!strcmp(filter_name, "blur")) {
        filter_matrix = GAUSSIAN;
    } else if (!strcmp(filter_name, "sharpen")) {
        filter_matrix = SHARPEN;
    } else if (!strcmp(filter_name, "mean")) {
        filter_matrix = MEAN_REMOVAL;
    } else if (!strcmp(filter_name, "emboss")) {
        filter_matrix = EMBOSS;
    } else {
        fprintf(stderr, "Eroare! Filtru invalid!\n");
        return;
    }

    aux = (uint8_t **) malloc((height + 2) * sizeof(uint8_t *));
    check_container(aux, NULL);

    for (i = 0; i < height + 2; ++i) {
        aux[i] = (uint8_t *) malloc(channels * (width + 2) * sizeof(uint8_t));
        check_container(aux[i], NULL);
        memcpy(aux[i], image[i], channels * (width + 2) * sizeof(uint8_t));
    }

    for (i = 1; i < height + 1; ++i) {
        for (j = channels; j < channels * (width + 1); ++j) {
            pixel = filter_matrix[2][2] * aux[i - 1][j - channels] +
                    filter_matrix[2][1] * aux[i - 1][j]            +
                    filter_matrix[2][0] * aux[i - 1][j + channels] +
                    filter_matrix[1][2] * aux[i][j - channels]     +
                    filter_matrix[1][1] * aux[i][j]                +
                    filter_matrix[1][0] * aux[i][j + channels]     +
                    filter_matrix[0][2] * aux[i + 1][j - channels] +
                    filter_matrix[0][1] * aux[i + 1][j]            +
                    filter_matrix[0][0] * aux[i + 1][j + channels];
            pixel = MIN(maxval, pixel);
            pixel = MAX(0, pixel);
            image[i][j] = pixel;
        }
    }

    for (i = 0; i < height + 2; ++i) {
        free(aux[i]);
    }
    free(aux);
}

#endif  // PROCESSER_UTILS_H_
