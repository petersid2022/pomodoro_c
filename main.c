#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

#define GRN "\033[0;32m"
#define RED "\033[0;31m"
#define BOLD "\033[1m"
#define ITALIC "\033[3m"
#define RESET "\033[0m"

typedef struct appData appData;
struct appData {
  int break_len;
  int break_amount;
  int pomodoro_len;
  int pomodoro_amount;
  bool ispomodoro;
};

void help() {
  // clang-format off
  printf("Pomodoro CLI Help\n"
         "--------------------\n"
         "Usage: pomodoro [flags]\n\n"
         "Flags:\n"
         "\t--help\t\tDisplay this help message\n"
         "\t--num-pomos\tSet the number of Pomodoros of the session / Defaults to 4\n"
         "\t--pomo-length\tSet the length of each Pomodoro (in minutes) / Defaults to 35\n"
         "\t--break-length\tSet the length of each in-between break (in minutes) / Defaults to 5\n\n"
         "Examples:\n"
         "\tpomodoro --pomo-length 25 --break-length 5 --num-pomos 4\n"
         "\tpomodoro --help\n");
  // clang-format on
}

void initAppData(appData *app) {
  app->pomodoro_len = 35;
  app->break_len = 5;
  app->pomodoro_amount = 4;
  app->break_amount = app->pomodoro_amount - 1;
  app->ispomodoro = true;
}

char *getTimeFormat(int x) {
  static char buf[50];
  int hours = x / 3600;
  int minutes = (x - (hours * 3600)) / 60;
  int seconds = (x - (hours * 3600)) - (minutes * 60);
  sprintf(buf, GRN "%.2d:%.2d:%.2d" RESET, hours, minutes, seconds);
  return buf;
}

void clear(void) {
  while (getchar() != '\n')
    ;
}

void waitForEnter(void) {
  char buffer[2];
  fgets(buffer, sizeof(buffer), stdin);
}

void sendNotification(const char *message) {
  char command[256];
  snprintf(command, sizeof(command), "dunstify -u low -t 5000 '%s'", message);
  system(command);
}

void startPomodoro(appData *app) {
  const int p_start = app->pomodoro_amount;

  for (int i = app->pomodoro_len * 10; i >= 0; --i) {
    int t1 = p_start - app->pomodoro_amount + 1;
    fflush(stdout);
    printf("\r\033[0K");
    printf(BOLD "Pomodoro (%d/%d) \t%s" RESET, t1, p_start, getTimeFormat(i));
    sleep(1);
  }

  app->pomodoro_amount--;
  sendNotification("Your Pomodoro session is over!");
  clear();

  fflush(stdout);
  printf("\nPress ENTER to continue...");
  waitForEnter();

  app->ispomodoro = false;
}

void startBreak(appData *app) {
  const int b_start = app->break_amount;

  for (int i = app->break_len * 10; i >= 0; --i) {
    int t1 = b_start - app->break_amount + 1;
    fflush(stdout);
    printf("\r\033[0K");
    printf(BOLD "Break (%d/%d) \t%s" RESET, t1, b_start, getTimeFormat(i));
    sleep(1);
  }

  app->break_amount--;
  sendNotification("Your Break session is over!");
  clear();

  fflush(stdout);
  printf("\nPress ENTER to continue...");
  waitForEnter();

  app->ispomodoro = true;
}

void runGameState(appData *app) {
  while (app->ispomodoro) {
    startPomodoro(app);
  }
  startBreak(app);
}

// TODO: parse only integers and possibly have better argument naming
int getTerminalArguments(appData *app, int argc, char **argv) {
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
      app->pomodoro_amount = out;
      app->break_amount = app->pomodoro_amount - 1;
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
      app->pomodoro_len = out;
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
      app->break_len = out;
      continue;
    } else if (strcmp("--help", argv[i]) == 0) {
      help();
      return -1;
    } else {
      fprintf(stderr,
              RED "ERROR:" RESET " '%s' is not a valid option, try again!\n",
              argv[i]);
      return -1;
    }
  }

  return 0;
}

int main(int argc, char *argv[]) {
  appData app;
  initAppData(&app);

  if (getTerminalArguments(&app, argc, argv) != 0)
    return -1;

  // Move the cursor to the home position
  printf("\033[H");

  // Clear the screen from the cursor to the end
  printf("\033[J");

  // clang-format off
  printf(GRN BOLD "\n"
    "+-----------+-------------------+\n"
    "|           | %d # of pomodoros\t|\n"
    "| Pomodoros +-------------------+\n"
    "|           | %d minutes each\t|\n"
    "+-----------+-------------------+\n"
    "|           | %d # of breaks\t|\n"
    "| Breaks    +-------------------+\n"
    "|           | %d minutes each\t|\n"
    "+-----------+-------------------+\n" RESET
    "\nConfirm? (y/n) ", app.pomodoro_amount, app.pomodoro_len, app.break_amount, app.break_len);
  // clang-format on

  char input[2];
  fgets(input, sizeof(input), stdin);
  if (input[0] != 'y')
    return -1;

  bool running = true;

  while (running) {
    runGameState(&app);
    if (app.pomodoro_amount == 0)
      running = false;
  }

  return 0;
}
