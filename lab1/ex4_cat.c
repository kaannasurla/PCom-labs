#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


// ===== MACRO DIE — esențial de înțeles =====
#define DIE(assertion, call_description)   \
  do {                                     \
    if (assertion) {                       \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);  \
      perror(call_description);            \
      exit(errno);                         \
    }                                      \
  } while (0)
// Echivalent cu: if(eroare) { printează + ieși }
// Studenții îl vor folosi la TOATE temele lor!

int main(int argc, char *argv[]) {
    // 1. Verifică că a primit un argument (argc == 2)
    DIE (argc != 2, "Not enough arguments");

    // 2. Deschide fișierul cu open(argv[1], O_RDONLY)
    int source_fd = open(argv[1], O_RDONLY);
    DIE (source_fd < 0, "Can't open file");

    // 3. Citește în buclă cu read() până returnează 0
    int bytes_count;
    char buf[1024];
    while((bytes_count = read(source_fd, buf, sizeof(buf)))) {
        
    // 4. Scrie la stdout (fd = 1) cu write()
        bytes_count = write(1, buf, bytes_count);

    // 5. Verifică valorile de retur (< 0 = eroare)
        DIE (bytes_count < 0, "Eroare la scriere");
    }

    // 6. Închide fișierul cu close()
    close(source_fd);
    return 0;
}
