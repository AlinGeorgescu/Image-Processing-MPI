/**
 * (C) Copyright 2020
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "/usr/include/mpi/mpi.h"
#include "./processer_utils.h"
#include "./processer_io.h"

#define MASTER    0
#define SOME_TAG  0

int main(int argc, char *argv[]) {
    int numtasks, rank;
    int width, height;
    int start, end;
    int i, j;
    int master_fragment;
    uint8_t type, channels, maxval;
    uint8_t **image;
    char *image_in = argv[1];
    char *image_out = argv[2];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 4) {
        if (rank == MASTER) {
            fprintf(stderr, "Eroare! Numar de argumente invalid!\n");
        }
        MPI_Finalize();
        return 0;
    }

    if (rank == MASTER) {
        // Citirea imaginii de intrare din fisier.
        read_image(image_in, &type, &channels, &width, &height,
                   &maxval, &image);

        // Eventuala trimitere a informatiilor catre celelalte procese.
        if (numtasks > 1) {
            MPI_Bcast(&width, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
            MPI_Bcast(&height, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
            MPI_Bcast(&channels, 1, MPI_UINT8_T, MASTER, MPI_COMM_WORLD);
            MPI_Bcast(&maxval, 1, MPI_UINT8_T, MASTER, MPI_COMM_WORLD);
        }

        for (i = 1; i < numtasks; ++i) {
            start = i * ceil((double) height / numtasks);
            end = MIN(height, (i + 1) * ceil((double) height / numtasks)) + 2;
            for (j = start; j < end; ++j) {
                MPI_Send(
                    image[j],
                    channels * (width + 2),
                    MPI_UINT8_T,
                    i,
                    SOME_TAG,
                    MPI_COMM_WORLD);
            }
        }

        // Prelucrarea bucatii de imagine arondata procesului MASTER.
        master_fragment = ceil((double) height / numtasks);
        for (i = 3; i < argc; ++i) {
            apply_filter(master_fragment, width, channels, maxval,
                         image, argv[i]);
            if (i != argc - 1 && numtasks > 1) {
                MPI_Recv(
                    image[master_fragment + 1],
                    channels * (width + 2),
                    MPI_UINT8_T,
                    MASTER + 1,
                    MPI_ANY_TAG,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE);
                MPI_Send(
                    image[master_fragment],
                    channels * (width + 2),
                    MPI_UINT8_T,
                    MASTER + 1,
                    SOME_TAG,
                    MPI_COMM_WORLD);
            }
        }

        // Eventuala primire a prelucrarilor de la celelalte procese.
        for (i = 1; i < numtasks; ++i) {
            start = i * ceil((double) height / numtasks) + 1;
            end = MIN(height, (i + 1) * ceil((double) height / numtasks)) + 1;
            for (j = start; j < end; ++j) {
                MPI_Recv(
                    image[j],
                    channels * (width + 2),
                    MPI_UINT8_T,
                    i,
                    MPI_ANY_TAG,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE);
            }
        }

        // Scrierea in fisier a imaginii.
        print_image(image_out, &type, &channels,
                    &width, &height, &maxval, image);
    } else {
        // Primirea informatiilor de la procesul MASTER.
        MPI_Bcast(&width, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
        MPI_Bcast(&height, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
        MPI_Bcast(&channels, 1, MPI_UINT8_T, MASTER, MPI_COMM_WORLD);
        MPI_Bcast(&maxval, 1, MPI_UINT8_T, MASTER, MPI_COMM_WORLD);

        start = rank * ceil((double) height / numtasks);
        end = MIN(height, (rank + 1) * ceil((double) height / numtasks));
        height = end - start;

        image = (uint8_t **) malloc((height + 2) * sizeof(uint8_t *));
        check_container(image, NULL);

        for (i = 0; i < height + 2; ++i) {
            image[i] =
                (uint8_t *) malloc(channels * (width + 2) * sizeof(uint8_t));
            check_container(image[i], NULL);
            MPI_Recv(
                image[i],
                channels * (width + 2),
                MPI_UINT8_T,
                MASTER,
                MPI_ANY_TAG,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);
        }

        // Prelucrarea bucatii de imagine arondata procesului curent.
        for (i = 3; i < argc; ++i) {
            apply_filter(height, width, channels, maxval,
                         image, argv[i]);

            if (i != argc - 1) {
                if (rank & 1) {
                    MPI_Send(
                        image[1],
                        channels * (width + 2),
                        MPI_UINT8_T,
                        rank - 1,
                        SOME_TAG,
                        MPI_COMM_WORLD);
                    if (rank != numtasks - 1) {
                        MPI_Send(
                            image[height],
                            channels * (width + 2),
                            MPI_UINT8_T,
                            rank + 1,
                            SOME_TAG,
                            MPI_COMM_WORLD);

                        MPI_Recv(
                            image[height + 1],
                            channels * (width + 2),
                            MPI_UINT8_T,
                            rank + 1,
                            MPI_ANY_TAG,
                            MPI_COMM_WORLD,
                            MPI_STATUS_IGNORE);
                    }

                    MPI_Recv(
                        image[0],
                        channels * (width + 2),
                        MPI_UINT8_T,
                        rank - 1,
                        MPI_ANY_TAG,
                        MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE);
                } else {
                    MPI_Recv(
                        image[0],
                        channels * (width + 2),
                        MPI_UINT8_T,
                        rank - 1,
                        MPI_ANY_TAG,
                        MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE);
                    if (rank != numtasks - 1) {
                        MPI_Recv(
                            image[height + 1],
                            channels * (width + 2),
                            MPI_UINT8_T,
                            rank + 1,
                            MPI_ANY_TAG,
                            MPI_COMM_WORLD,
                            MPI_STATUS_IGNORE);

                        MPI_Send(
                            image[height],
                            channels * (width + 2),
                            MPI_UINT8_T,
                            rank + 1,
                            SOME_TAG,
                            MPI_COMM_WORLD);
                    }

                    MPI_Send(
                        image[1],
                        channels * (width + 2),
                        MPI_UINT8_T,
                        rank - 1,
                        SOME_TAG,
                        MPI_COMM_WORLD);
                }
            }
        }

        // Trimiterea inapoi a prelucrarilor realizate.
        for (i = 1; i < height + 1; ++i) {
            MPI_Send(
                image[i],
                channels * (width + 2),
                MPI_UINT8_T,
                MASTER,
                SOME_TAG,
                MPI_COMM_WORLD);
        }
    }

    for (i = 0; i < height + 2; ++i) {
        free(image[i]);
    }
    free(image);

    MPI_Finalize();
    return 0;
}
