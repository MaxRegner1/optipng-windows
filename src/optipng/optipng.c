#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

void copy_file(const char *src, const char *dest) {
    FILE *src_file = fopen(src, "rb");
    if (!src_file) {
        perror("Failed to open source file");
        return;
    }

    FILE *dest_file = fopen(dest, "wb");
    if (!dest_file) {
        perror("Failed to open destination file");
        fclose(src_file);
        return;
    }

    char buffer[8192];
    size_t n;

    while ((n = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        if (fwrite(buffer, 1, n, dest_file) != n) {
            perror("Failed to write to destination file");
            break;
        }
    }

    fclose(src_file);
    fclose(dest_file);
}

void process_folder(const char *source_dir, const char *result_dir) {
    struct dirent *entry;
    DIR *dir = opendir(source_dir);
    if (!dir) {
        perror("Failed to open source directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char src_path[512], dest_path[512];

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(src_path, sizeof(src_path), "%s/%s", source_dir, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", result_dir, entry->d_name);

        struct stat statbuf;
        if (stat(src_path, &statbuf) == -1) {
            perror("Failed to stat file");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            mkdir(dest_path, 0755);
            process_folder(src_path, dest_path);
        } else {
            copy_file(src_path, dest_path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <source_dir> <result_dir>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *source_dir = argv[1];
    char *result_dir = argv[2];

    process_folder(source_dir, result_dir);

    return EXIT_SUCCESS;
}
