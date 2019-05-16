#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <sys/param.h>

#define BLOCK 1
#define INTERLEAVED 2

struct Image_Transform {
    long size;
    double **values;
};

struct Image {
    char name[256];
    long width;
    long height;
    long colors;
    unsigned char **pixels;
};

struct Thread {
    struct Image *image;
    struct Image_Transform *trans;
    long start_num;
    long type;
    long thread_count;
};

int load_transform(struct Image_Transform *buf, const char *path) {
    if (buf == NULL)
        return 1;

    FILE *fp;
    char *transform;
    long file_size;

    if ((fp = fopen(path, "r")) == NULL)
        return 1;

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    transform = calloc(sizeof(char), (size_t) file_size);
    if (fread(transform, sizeof(char), (size_t) file_size, fp) != file_size)
        return 1;

    char *sep = "\r\n ";
    char *size_str = strtok(transform, sep);
    long transform_size = strtol(size_str, NULL, 10);
    buf->size = transform_size;

    double **values = calloc((size_t) transform_size, sizeof(double *));
    for (long i = 0; i < transform_size; i++)
        values[i] = calloc((size_t) transform_size, sizeof(double));
    buf->values = values;

    long row = 0;
    long col = 0;
    char *token = strtok(NULL, sep);
    while (token != NULL) {
        if (col >= buf->size) {
            col = 0;
            row++;
        }
        if (row >= buf->size)
            break;

        double value = atof(token);
        values[row][col] = value;
        col++;
        token = strtok(NULL, sep);
    }

    fclose(fp);
    return 0;
}

int load_image(struct Image *buf, const char *path) {
    if (buf == NULL)
        return 1;

    FILE *fp;
    char *pgma;
    long file_size;

    if ((fp = fopen(path, "r")) == NULL)
        return 1;

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    pgma = calloc(sizeof(char), (size_t) file_size);
    if (fread(pgma, sizeof(char), (size_t) file_size, fp) != file_size)
        return 1;

    char *header_sep = "\t\r\n";
    char *body_sep = "\t\r\n ";

    char *magic_byte = strtok(pgma, header_sep);
    if (magic_byte == NULL || strcmp(magic_byte, "P2") != 0)
        return 1;

    char *name = strtok(NULL, header_sep);
    if (name == NULL)
        return 1;
    strcpy(buf->name, name);

    char *width = strtok(NULL, body_sep);
    if (width == NULL)
        return 1;
    buf->width = strtol(width, NULL, 10);

    char *height = strtok(NULL, header_sep);
    if (height == NULL)
        return 1;
    buf->height = strtol(height, NULL, 10);

    char *colors = strtok(NULL, header_sep);
    if (colors == NULL)
        return 1;
    buf->colors = strtol(colors, NULL, 10);

    unsigned char **pixels = calloc((size_t) buf->height, sizeof(char *));
    for (long i = 0; i < buf->height; i++)
        pixels[i] = calloc((size_t) buf->width, sizeof(char));
    buf->pixels = pixels;

    long row = 0;
    long col = 0;
    char *token;
    while (1) {
        token = strtok(NULL, body_sep);
        if (token == NULL)
            break;
        if (col >= buf->width) {
            col = 0;
            row++;
        }
        if (row >= buf->height)
            break;

        unsigned long grayscale = strtoul(token, NULL, 10);
        pixels[row][col] = (unsigned char) grayscale;
        col++;
    }

    fclose(fp);
    return 0;
}

__time_t get_timestamp() {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    return current_time.tv_sec * (int) 1e6 + current_time.tv_usec;
}

int apply_on_pixel(struct Image *image, struct Image_Transform *transform, long row, long col) {
    if (image == NULL || transform == NULL)
        return 1;

    double new_value = 0;
    long size = transform->size;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            long img_col = MAX(0, col - (long) ceil(size / 2) + i);
            long img_row = MAX(0, row - (long) ceil(size / 2) + j);
            if (img_row < image->height && img_col < image->width)
                new_value += image->pixels[img_row][img_col] * transform->values[j][i];
        }
    }

    image->pixels[row][col] = (unsigned char) round(new_value);
    return 0;
}

void *thread_logic(void *arg) {
    struct Thread *thread = (struct Thread *) arg;
    struct Image *img = thread->image;
    struct Image_Transform *trans = thread->trans;
    __time_t start_time = get_timestamp();

    long k = thread->start_num;
    if (thread->type == BLOCK) {
        long x0 = k * (long) ceil(img->width / thread->thread_count);
        long x1 = (k + 1 == thread->thread_count) ? img->width - 1 :
                  (k + 1) * (long) ceil(img->width / thread->thread_count) - 1;

        while (x0 <= x1) {
            for (long row = 0; row < img->height; row++)
                apply_on_pixel(img, trans, row, x0);
            x0++;
        }
    } else if (thread->type == INTERLEAVED) {
        for (long row = 0; row < img->height; row++) {
            long x0 = k;
            while (x0 < img->width) {
                apply_on_pixel(img, trans, row, x0);
                x0 += thread->thread_count;
            }
        }
    }

    __time_t *timestamp = malloc(sizeof(unsigned long));
    *timestamp = get_timestamp() - start_time;
    free(thread);
    pthread_exit(timestamp);
}

int save_image(struct Image *img, const char *path) {
    if (img == NULL)
        return 1;

    FILE *fp;
    if ((fp = fopen(path, "w")) == NULL)
        return 1;

    fprintf(fp,
            "P2\n%s\n%ld %ld\n%ld\n",
            img->name, img->width, img->height, img->colors);

    int split_cnt = 0;
    for (int row = 0; row < img->height; row++) {
        for (int col = 0; col < img->width; col++) {
            if (split_cnt + 1 == img->width) {
                fprintf(fp, " %3d\n", img->pixels[row][col]);
                split_cnt = 0;
            } else {
                fprintf(fp, " %3d", img->pixels[row][col]);
                split_cnt += 1;
            }
        }
    }

    fclose(fp);
    return 0;
}


int main(int argc, char **argv) {
    if (argc != 6) {
        printf("Invalid number of arguments.\n");
        return 1;
    }

    long thread_count = strtol(argv[1], NULL, 10);
    long split_type;
    if (strcmp(argv[2], "interleaved") == 0)
        split_type = INTERLEAVED;
    else if (strcmp(argv[2], "block") == 0)
        split_type = BLOCK;
    else {
        printf("Run with interleaved or block flag.\n");
        return 1;
    }

    char *input_path = argv[3];
    char *filter_path = argv[4];
    char *output_path = argv[5];
    pthread_t *threads;
    struct Image_Transform *transform = malloc(sizeof(struct Image_Transform));
    if (load_transform(transform, filter_path) != 0) {
        printf("Error while reading the filer file.\n");
        return 1;
    }

    struct Image *image = malloc(sizeof(struct Image));
    if (load_image(image, input_path) != 0) {
        printf("Error while loading the image.\n");
        return 1;
    }

    printf("IMAGE INFO\n");
    printf("Image name: %s\n", image->name);
    printf("Dimension: %ldx%ld\n", image->width, image->height);
    printf("Colors: %ld\n", image->colors);
    printf("Filter size: %ldx%ld\n", transform->size, transform->size);
    printf("Split type: %s\n", argv[2]);

    __time_t start_time = get_timestamp();

    printf("THREAD COUNT: %ld\n", thread_count);
    threads = calloc((size_t) thread_count, sizeof(pthread_t));
    for (int i = 0; i < thread_count; i++) {
        pthread_t tid;
        struct Thread *thread = malloc(sizeof(struct Thread));
        thread->image = image;
        thread->trans = transform;
        thread->start_num = i;
        thread->type = split_type;
        thread->thread_count = thread_count;

        pthread_create(&tid, NULL, thread_logic, thread);
        threads[i] = tid;
    }

    for (int i = 0; i < thread_count; i++) {
        void *ret;
        if (pthread_join(threads[i], &ret) != 0) {
            printf("Error while joining %d thread.\n", i);
            return 1;
        }

        unsigned long time = *((unsigned long *) ret);
        printf("Thread %d took %ldus.\n", i, time);
        free(ret);
    }

    printf("Overall time: %ldus.\n\n\n\n", get_timestamp() - start_time);

    if (save_image(image, output_path) != 0) {
        printf("Error while saving output image to %s.\n", output_path);
        return 1;
    }

    for (int i = 0; i < transform->size; i++)
        free(transform->values[i]);
    free(transform->values);
    free(transform);

    for (int i = 0; i < image->height; i++)
        free(image->pixels[i]);
    free(image->pixels);
    free(image);
    free(threads);

    return 0;
}