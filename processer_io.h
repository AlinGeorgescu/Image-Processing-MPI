/**
 * (C) Copyright 2020
 */

#ifndef PROCESSER_IO_H_
#define PROCESSER_IO_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "./processer_utils.h"

#define MAX_LEN  255

/**
 * Functia de citire din fisier a unei imagini.
 * 
 * @param image_in = denumirea fisierului
 * @param type     = adresa tipului imaginii (PNM / PGM)
 * @param channels = adresa numarului de canale al imaginii
 * @param width    = adresa latimii imaginii
 * @param height   = adresa inaltimii imaginii
 * @param maxval   = adresa valorii maxime a unui pixel
 * @param image    = adresa matricei imaginii
 */
void read_image(char *image_in,
                uint8_t *type, uint8_t *channels,
                int *width, int *height,
                uint8_t *maxval, uint8_t ***image) {
    FILE *in;
    char comment_line[MAX_LEN];
    char c;
    int i;

    in = fopen(image_in, "rb");
    check_file(in);

    fscanf(in, "%c %hhu", &c, type);
    fgets(comment_line, MAX_LEN, in);
    fgets(comment_line, MAX_LEN, in);
    fscanf(in, "%d %d %hhu", width, height, maxval);
    fgets(comment_line, MAX_LEN, in);

    if ((*type) == 5) {
        *channels = 1;
    } else {
        *channels = 3;
    }

    *image = (uint8_t **) malloc(((*height) + 2) * sizeof(uint8_t *));
    check_container(*image, in);

    for (i = 0; i < (*height) + 2; ++i) {
        (*image)[i] =
            (uint8_t *) calloc((*channels) * ((*width) + 2), sizeof(uint8_t));
        check_container((*image)[i], in);
    }

    for (i = 1; i < (*height) + 1; ++i) {
        fread((*image)[i] + (*channels), sizeof(uint8_t),
              (*width) * (*channels), in);
    }

    fclose(in);
}

/**
 * Functia de scriere in fisier a unei imagini.
 * 
 * @param image_out = denumirea fisierului
 * @param type      = adresa tipului imaginii (PNM / PGM)
 * @param channels  = adresa numarului de canale al imaginii
 * @param width     = adresa latimii imaginii
 * @param height    = adresa inaltimii imaginii
 * @param maxval    = adresa valorii maxime a unui pixel
 * @param image     = matricea imaginii
 */
void print_image(char *image_out,
                uint8_t *type, uint8_t *channels,
                int *width, int *height,
                uint8_t *maxval, uint8_t **image) {
    FILE *out;
    int i;

    out = fopen(image_out, "wb");
    check_file(out);

    fprintf(out, "P%u\n", *type);
    fprintf(out, "%d %d\n", *width, *height);
    fprintf(out, "%d\n", *maxval);

    for (i = 1; i < (*height) + 1; ++i) {
        fwrite(image[i] + (*channels), sizeof(uint8_t),
               (*width) * (*channels), out);
    }

    fclose(out);
}

#endif  // PROCESSER_IO_H_
