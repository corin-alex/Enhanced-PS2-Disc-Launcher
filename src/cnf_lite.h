#include <stdbool.h>
#include <stddef.h>

int Read_SYSTEM_CNF(char *boot_path, size_t boot_path_size, char *ver, size_t ver_size);
int Read_Launcher_CNF(const char *cnf_path, int *language, bool *autolaunch);
