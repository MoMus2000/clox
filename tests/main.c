#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    const char *cmd;
    const char* value;
} ResultMapEntry;

ResultMapEntry resultmapper[] = {
  {"./build/clox_test ./tests/scripts/test_1.clox", "10"}
};

int main(int argc, char** argv) {
  char buffer[1024];

  size_t len = sizeof(resultmapper) / sizeof(resultmapper[0]);

  for(int i=0; i<len; i++) {
    FILE* pipe = popen(resultmapper[i].cmd, "r");

    if(!pipe){
      perror("popen failed");
      return 1;
    }

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("Output: %s", buffer);
    }

    size_t val_len = strlen(resultmapper[i].value);
    char *output = malloc(val_len + 2); // +1 for '\n', +1 for '\0'
    strcpy(output, resultmapper[i].value);
    output[val_len] = '\n';
    output[val_len + 1] = '\0';

    if (strcmp(buffer, output) != 0){
      printf("Output Mismatch:\n");
      printf("Expected: %sbut got: %s", buffer, output);
      return 1;
    }

    pclose(pipe);

  }

}

