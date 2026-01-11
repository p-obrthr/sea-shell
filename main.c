#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZE 1024

void freeStrs(char **strs, size_t count) {
  for (int i = 0; i < count; i++) {
    free(strs[i]);
  }
  free(strs);
}

char *allocStr(int size) {
  char *str = malloc(size);
  if (str != NULL) {
    str[0] = '\0';
  }
  return str;
}

bool isCmd(const char *input, const char *cmd) {
  return strcmp(input, cmd) == 0;
}

char **split(const char *str, size_t *count, char c) {
  size_t cap = 4;
  size_t i = 0;
  size_t j = 0;
  size_t dir_i = 0;

  char **splits = malloc(cap * sizeof(char *));
  splits[dir_i] = allocStr(SIZE);

  while (str[i] == c) {
    i++;
  }

  while (str[i] != '\0') {
    if (str[i] != c) {
      splits[dir_i][j] = str[i];
      j++;
    } else {
      while (str[i + 1] == c) {
        i++;
      }
      splits[dir_i][j] = '\0';
      dir_i++;
      j = 0;

      if (dir_i + 1 > cap) {
        cap *= 2;
        char **splits_tmp = realloc(splits, cap * sizeof(char *));
        if (splits_tmp == NULL) {
          free(splits);
        }
        splits = splits_tmp;
      }

      splits[dir_i] = allocStr(SIZE);
    }
    i++;
  }
  splits[dir_i][j] = '\0';
  *count = dir_i + 1;

  return splits;
}

char *findFilePathExec(const char *cmd) {
  char *paths = getenv("PATH");
  if (paths == NULL) {
    return NULL;
  }
  size_t count = 0;
  char **dirs = split(paths, &count, ':');

  for (size_t i = 0; i < count; i++) {
    size_t len = strlen(dirs[i]) + 1 + strlen(cmd) + 1;
    char *file_path = allocStr(len);
    if (file_path == NULL) {
      return NULL;
    }

    snprintf(file_path, len, "%s/%s", dirs[i], cmd);
    FILE *fp = fopen(file_path, "r");
    if (fp != NULL) {
      fclose(fp);
      struct stat sb;
      if (stat(file_path, &sb) == 0 && sb.st_mode & S_IXUSR) {
        freeStrs(dirs, count);
        return file_path;
      }
    } else {
      free(file_path);
    }
  }

  freeStrs(dirs, count);
  return NULL;
}

char *echo(char **words, size_t count) {
  char *output = allocStr(SIZE * 2);
  if (output == NULL) {
    return NULL;
  }
  for (int i = 1; i < count; i++) {
    strcat(output, words[i]);
    strcat(output, (i + 1 < count) ? " " : "\n");
  }
  return output;
}

bool isBuiltIn(const char *toCompare, const char *builtIns[]) {
  for (size_t i = 0; builtIns[i] != NULL; i++) {
    if (strcmp(builtIns[i], toCompare) == 0) {
      return true;
    }
  }
  return false;
}

char *type(const char *cmd, const char *builtIns[]) {
  if (cmd == NULL || strcmp("", cmd) == 0) {
    return NULL;
  }

  char *output = allocStr(SIZE);
  if (output == NULL) {
    return NULL;
  }

  strcat(output, cmd);

  if (isBuiltIn(cmd, builtIns)) {
    strcat(output, " is a shell builtin\n");
    return output;
  }

  char *filePath = findFilePathExec(cmd);
  if (filePath != NULL) {
    strcat(output, " is ");
    strcat(output, filePath);
    free(filePath);
  } else {
    strcat(output, ": not found");
  }

  strcat(output, "\n");

  return output;
}

char *pwd() {
  char *output = allocStr(SIZE);
  if (output == NULL) {
    return NULL;
  }
  if (getcwd(output, SIZE) != NULL) {
    strcat(output, "\n");
  }
  return output;
}

char *cd(const char *target) {
  char *output = allocStr(SIZE);
  if (target != NULL && output != NULL) {
    if (strcmp(target, "~") == 0) {
      chdir(getenv("HOME"));
    } else if (chdir(target) != 0) {
      snprintf(output, SIZE, "cd: %s: No such file or directory\n", target);
    }
  }
  return output;
}

char *tryExecExternal(const char *type, const char *cmd) {
  char *output = allocStr(SIZE);
  if (output == NULL) {
    return NULL;
  }
  char *path = findFilePathExec(type);
  if (path == NULL) {
    snprintf(output, SIZE, "%s: command not found\n", cmd);
    return output;
  }

  free(path);
  system(cmd);
  return NULL;
}

int main(int argc, char *argv[]) {
  bool running = true;
  char *cmd = allocStr(SIZE);
  if (cmd == NULL) {
    return 1;
  }
  setbuf(stdout, NULL);

  const char *builtIns[] = {"exit", "echo", "type", "pwd", "cd", NULL};

  while (running) {
    printf("$ ");

    fgets(cmd, SIZE, stdin);
    cmd[strcspn(cmd, "\n")] = '\0';

    size_t count = 0;
    char **words = split(cmd, &count, ' ');

    char *output = NULL;

    if (isCmd(words[0], "exit")) {
      running = false;
    } else if (isCmd(words[0], "echo")) {
      output = echo(words, count);
    } else if (isCmd(words[0], "type")) {
      output = type(words[1], builtIns);
    } else if (isCmd(words[0], "pwd")) {
      output = pwd();
    } else if (isCmd(words[0], "cd")) {
      output = cd(words[1]);
    } else {
      output = tryExecExternal(words[0], cmd);
    }

    freeStrs(words, count);
    if (output != NULL) {
      printf("%s", output);
      free(output);
    }
  }

  free(cmd);
  return 0;
}
