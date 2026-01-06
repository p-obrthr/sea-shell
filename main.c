#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_SIZE 1024

void free_words(char **words, size_t count) {
  for (int i = 0; i < count; i++) {
    free(words[i]);
  }
  free(words);
}

char **getWords(const char *input, size_t *count) {
  size_t cap = 4;
  size_t i = 0;
  size_t j = 0;

  char **words = malloc(cap * sizeof(char *));
  words[*count] = malloc(CMD_SIZE);

  while (input[i] != '\0') {
    if (input[i] == ' ') {
      while (input[i + 1] == ' ') {
        i++;
      }
      words[*count][j] = '\0';
      if (*count == cap) {
        cap *= 2;
        char *(*words_tmp) = realloc(words, cap * sizeof(char *));
        if (words_tmp == NULL) {
          free(words);
          return NULL;
        }
        words = words_tmp;
      }
      (*count)++;
      words[*count] = malloc(CMD_SIZE);
      j = 0;
    } else {
      words[*count][j] = input[i];
      j++;
    }
    i++;
  }

  words[*count][j] = '\0';
  (*count)++;

  return words;
}

void echo(char **words, size_t count) {
  for (int i = 1; i < count; i++) {
    printf("%s%s", words[i], (i + 1 < count) ? " " : "\n");
  }
}

void type(const char *cmd) {
  if (strcmp("echo", cmd) == 0 || strcmp("type", cmd) == 0 ||
      strcmp("exit", cmd) == 0) {
    printf("%s is a shell builtin\n", cmd);
  } else {
    printf("%s: not found\n", cmd);
  }
}

int main(int argc, char *argv[]) {
  char *cmd = malloc(CMD_SIZE);
  setbuf(stdout, NULL);

  while (1) {
    printf("$ ");

    fgets(cmd, CMD_SIZE, stdin);
    cmd[strcspn(cmd, "\n")] = '\0';

    size_t count = 0;
    char *(*words) = getWords(cmd, &count);

    if (strcmp("", words[0]) == 0) {
      free_words(words, count);
      continue;
    } else if (strcmp("exit", words[0]) == 0) {
      free_words(words, count);
      break;
    }

    if (strcmp("echo", words[0]) == 0) {
      echo(words, count);
    } else if (strcmp("type", words[0]) == 0) {
      type(words[1]);
    } else {
      printf("%s: command not found\n", cmd);
    }

    free_words(words, count);
  }

  free(cmd);
  return 0;
}
