#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define GRN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

typedef struct appData appData;
struct appData {
  int break_len;
  int break_amount;
  int pomodoro_len;
  int pomodoro_amount;
  bool break_state;
  bool pomodoro_state;
};

char *getTimeFormat(int x) {
  static char buf[50];
  int hours = x / 3600;
  int minutes = (x - (hours * 3600)) / 60;
  int seconds = (x - (hours * 3600)) - (minutes * 60);
  sprintf(buf, GRN "%.2d:%.2d:%.2d" RESET, hours, minutes, seconds);
  return buf;
}

void newSession(appData *app) {
  app->pomodoro_state = true;

  for (int i = app->pomodoro_len * 60; i >= 0; --i) {
    printf("\r");
    printf("\033[0K");
    printf("%s", getTimeFormat(i));
    fflush(stdout);
    sleep(1);
  }

  // apparently this rings the bell
  printf("\a");

  app->pomodoro_state = false;
  app->break_state = true;

  for (int i = app->break_len * 60; i >= 0; --i) {
    printf("\r");
    printf("\033[0K");
    printf("%s", getTimeFormat(i));
    fflush(stdout);
    sleep(1);
  }
}

void initApp(appData *app) {
  app->pomodoro_len = 35;
  app->break_len = 5;
  app->pomodoro_amount = 4;
  app->break_amount = app->pomodoro_amount - 1;
  app->pomodoro_state = false;
  app->break_state = false;
}

void help() {
  // clang-format off
  printf("Pomodoro CLI Help\n");
  printf("--------------------\n");
  printf("Usage: pomodoro [flags]\n\n");
  printf("Flags:\n");
  printf("\t--help          Display this help message\n");
  printf("\t--num-pomos     Set the number of Pomodoros of the session / Defaults to 4\n");
  printf("\t--pomo-length   Set the length of each Pomodoro (in minutes) / Defaults to 35\n");
  printf("\t--break-length  Set the length of each in-between break (in minutes) / Defaults to 5\n");
  printf("\nExamples:\n");
  printf("\tpomodoro --pomo-length 25 --break-length 5 --num-pomos 4\n");
  printf("\tpomodoro --help\n");
  // clang-format on
}

int main(int argc, char *argv[]) {
  appData app;

  initApp(&app);

  for (int i = 1; i < argc; i += 2) {
    if (strcmp("--num-pomos", argv[i]) == 0) {
      char *num = argv[i + 1];
      int out = atoi(num);
      if (out == 0) {
        fprintf(stderr,
                RED "ERROR:" RESET " '%s' is not a number, try again!\n",
                argv[i + 1]);
        return -1;
      }
      app.pomodoro_amount = out;
      app.break_amount = app.pomodoro_amount - 1;
      continue;
    } else if (strcmp("--pomo-length", argv[i]) == 0) {
      char *num = argv[i + 1];
      int out = atoi(num);
      if (out == 0) {
        fprintf(stderr,
                RED "ERROR:" RESET " '%s' is not a number, try again!\n",
                argv[i + 1]);
        return -1;
      }
      app.pomodoro_len = out;
      continue;
    } else if (strcmp("--break-length", argv[i]) == 0) {
      char *num = argv[i + 1];
      int out = atoi(num);
      if (out == 0) {
        fprintf(stderr,
                RED "ERROR:" RESET " '%s' is not a number, try again!\n",
                argv[i + 1]);
        return -1;
      }
      app.break_len = out;
      continue;
    } else if (strcmp("--help", argv[i]) == 0) {
      help();
      return -1;
    } else {
      fprintf(stderr, "ERROR: Not a valid option, try again!\n");
      return -1;
    }
  }

  printf("\n");
  printf("+-----------+-------------------+\n");
  printf("|           | %d # of pomodoros\t|\n", app.pomodoro_amount);
  printf("| Pomodoros +-------------------+\n");
  printf("|           | %d minutes each\t|\n", app.pomodoro_len);
  printf("+-----------+-------------------+\n");
  printf("|           | %d # of breaks\t|\n", app.break_amount);
  printf("| Breaks    +-------------------+\n");
  printf("|           | %d minutes each\t|\n", app.break_len);
  printf("+-----------+-------------------+\n");
  printf("\nConfirm? (y/n) ");

  if (getchar() != 'y')
    return -1;

  newSession(&app);

  return 0;
}
