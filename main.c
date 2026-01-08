#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define CMD_SIZE 1024
#define DIRECTORY_SIZE 1024

void freeStrs(char **strs, size_t count) {
  for (int i = 0; i < count; i++) {
    free(strs[i]);
  }
  free(strs);
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
        char **words_tmp = realloc(words, cap * sizeof(char *));
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

char *findFilePathExec(const char *cmd) {
  char *paths = getenv("PATH");
  if (paths == NULL) {
    return NULL;
  }

  size_t cap = 4;
  size_t i = 0;
  size_t j = 0;
  size_t dir_i = 0;

  char **dirs = malloc(cap * sizeof(char *));
  dirs[dir_i] = malloc(DIRECTORY_SIZE);

  if (paths[i] == ':') {
    i++;
  }

  while (paths[i] != '\0') {
    if (paths[i] != ':') {
      dirs[dir_i][j] = paths[i];
      j++;
    } else {
      dirs[dir_i][j] = '\0';
      dir_i++;
      j = 0;

      if (dir_i + 1 > cap) {
        cap *= 2;
        char **dirs_tmp = realloc(dirs, cap * sizeof(char *));
        if (dirs_tmp == NULL) {
          free(dirs);
        }
        dirs = dirs_tmp;
      }

      dirs[dir_i] = malloc(DIRECTORY_SIZE);
    }
    i++;
  }

  dirs[dir_i][j] = '\0';

  for (size_t i = 0; i <= dir_i; i++) {
    size_t len = strlen(dirs[i]) + 1 + strlen(cmd) + 1;
    char *file_path = malloc(len);
    if (file_path == NULL) {
      continue;
    }

    snprintf(file_path, len, "%s/%s", dirs[i], cmd);
    FILE *fp = fopen(file_path, "r");
    if (fp != NULL) {
      fclose(fp);
      struct stat sb;
      if (stat(file_path, &sb) == 0 && sb.st_mode & S_IXUSR) {
        freeStrs(dirs, dir_i + 1);
        return file_path;
      }
    } else {
      free(file_path);
    }
  }

  freeStrs(dirs, dir_i + 1);
  return NULL;
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
    return;
  }

  char *filePath = findFilePathExec(cmd);
  if (filePath != NULL) {
    printf("%s is %s\n", cmd, filePath);
    free(filePath);
  } else {
    printf("%s: not found\n", cmd);
  }
}

bool tryExecExternal(const char *type, const char *cmd) {
  char *path = findFilePathExec(type);
  if (path == NULL) {
    return false;
  }

  free(path);
  system(cmd);
  return true;
}

int main(int argc, char *argv[]) {
  bool running = true;
  char *cmd = malloc(CMD_SIZE);
  setbuf(stdout, NULL);

  while (running) {
    printf("$ ");

    fgets(cmd, CMD_SIZE, stdin);
    cmd[strcspn(cmd, "\n")] = '\0';

    size_t count = 0;
    char **words = getWords(cmd, &count);

    if (strcmp("exit", words[0]) == 0) {
      running = false;
    } else if (strcmp("echo", words[0]) == 0) {
      echo(words, count);
    } else if (strcmp("type", words[0]) == 0) {
      type(words[1]);
    } else if (!tryExecExternal(words[0], cmd)) {
      printf("%s: command not found\n", cmd);
    }

    freeStrs(words, count);
  }

  free(cmd);
  return 0;
}
