#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    const char* key;
    const char** values;  // pointer to array of strings
    size_t count;         // number of strings in values
} ResultMapEntry;

const char* results1[] = {"10"};
const char* results2[] = {"Breakky This is the good life", "Hola Como Estas ?"};

ResultMapEntry resultmapper[] = {
    {"./build/clox_test ./tests/scripts/test_1.clox", results1, 1},
    {"./build/clox_test ./tests/scripts/test_2.clox", results2, 3}
};

int main(int argc, char** argv) {
  char buffer[1024];

  size_t len = sizeof(resultmapper) / sizeof(resultmapper[0]);

  for(int i=0; i<len; i++) {
    FILE* pipe = popen(resultmapper[i].key, "r");

    if(!pipe){
      perror("popen failed");
      return 1;
    }

    int j = 0;

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t val_len = strlen(resultmapper[i].values[j]);
        char *output = malloc(val_len + 2); // +1 for '\n', +1 for '\0'
        strcpy(output, resultmapper[i].values[j]);
        output[val_len] = '\n';
        output[val_len + 1] = '\0';

        if (strcmp(buffer, output) != 0){
          printf("Output Mismatch:\n");
          printf("Expected: %sbut got: %s", buffer, output);
          return 1;
        }

        free(output);
        
        j++;
    }

    pclose(pipe);

    printf("[Test #%d] PASS\n", i);

  }

}

