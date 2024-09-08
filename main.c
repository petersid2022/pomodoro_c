#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

#define ANSI_GREEN "\033[0;32m"
#define ANSI_RED "\033[0;31m"
#define ANSI_BOLD "\033[1m"
#define ANSI_ITALIC "\033[3m"
#define ANSI_RESET "\033[0m"

typedef struct appData appData;
struct appData {
  int break_len;
  int break_amount;
  int pomodoro_len;
  int pomodoro_amount;
  bool pomodoro_state;
  bool break_state;
};

void initAppData(appData *app) {
  app->pomodoro_len = 35;
  app->break_len = 5;
  app->pomodoro_amount = 4;
  app->break_amount = app->pomodoro_amount - 1;
  app->pomodoro_state = false;
  app->break_state = false;
}

char *getTimeFormat(int x) {
  static char buf[50];
  int hours = x / 3600;
  int minutes = (x - (hours * 3600)) / 60;
  int seconds = (x - (hours * 3600)) - (minutes * 60);
  sprintf(buf, ANSI_GREEN "%.2d:%.2d:%.2d" ANSI_RESET, hours, minutes, seconds);
  return buf;
}

void help() {
  printf("Pomodoro CLI Help\n"
         "--------------------\n"
         "Usage: pomodoro [flags]\n\n"
         "Flags:\n"
         "\t--help\t\tDisplay this help message\n"
         "\t--num-pomos\tSet the number of Pomodoros of the session / Defaults "
         "to 4\n"
         "\t--pomo-length\tSet the length of each Pomodoro (in minutes) / "
         "Defaults to 35\n"
         "\t--break-length\tSet the length of each in-between break (in "
         "minutes) / Defaults to 5\n\n"
         "Examples:\n"
         "\tpomodoro --pomo-length 25 --break-length 5 --num-pomos 4\n"
         "\tpomodoro --help\n");
}

void sendNotification(const char *message) {
  char command[256];
  snprintf(
      command, sizeof(command),
      "dunstify -u low -t 5000 '%s'; mpv assets/music/alarm.ogg > /dev/null",
      message);
  system(command);
}

void flushStdin() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

bool pressEnterBlocker() {
  flushStdin();
  printf("\r\033[0K");
  printf("Press Enter to start the new session.");
  fflush(stdout);
  char ch = getchar();
  return ch == '\n';
}

void runGameState(appData *app) {
  int totalPomodoros = app->pomodoro_amount;
  int totalBreaks = app->break_amount;

  while (app->pomodoro_amount > 0) {
    int pomo_start_amount = app->pomodoro_amount;
    for (int i = app->pomodoro_len * 60; i >= 0; --i) {
      fflush(stdout);
      printf("\r\033[0K");
      printf(ANSI_BOLD "Pomodoro Session (%d/%d) \t%s" ANSI_RESET,
             totalPomodoros - app->pomodoro_amount + 1, totalPomodoros,
             getTimeFormat(i));
      sleep(1);
    }

    sendNotification("Your pomodoro session is done!");
    app->pomodoro_amount--;

    fflush(stdout);

    if (app->break_amount > 0 && pressEnterBlocker()) {
      int break_start_amount = app->break_amount;
      for (int i = app->break_len * 60; i >= 0; --i) {
        fflush(stdout);
        printf("\r\033[0K");
        printf(ANSI_BOLD "Break Session (%d/%d) \t%s" ANSI_RESET,
               totalBreaks - app->break_amount + 1, totalBreaks,
               getTimeFormat(i));
        fflush(stdout);
        sleep(1);
      }

      sendNotification("Your break session is done!");
      app->break_amount--;
    }

    fflush(stdout);
  }

  if (app->break_amount == 0 && app->pomodoro_amount == 0)
    exit(0);
}

// TODO: parse only integers
// TODO: possibly have better argument naming
int getTerminalArguments(appData *app, int argc, char **argv) {
  for (int i = 1; i < argc; i += 2) {
    if (strcmp("--num-pomos", argv[i]) == 0) {
      char *num = argv[i + 1];
      int out = atoi(num);
      if (out == 0) {
        fprintf(stderr,
                ANSI_RED "ERROR:" ANSI_RESET
                         " '%s' is not a number, try again!\n",
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
                ANSI_RED "ERROR:" ANSI_RESET
                         " '%s' is not a number, try again!\n",
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
                ANSI_RED "ERROR:" ANSI_RESET
                         " '%s' is not a number, try again!\n",
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
              ANSI_RED "ERROR:" ANSI_RESET
                       " '%s' is not a valid option, try again!\n",
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

  printf(ANSI_GREEN ANSI_BOLD
         "\n"
         "+-----------+-------------------+\n"
         "|           | %d # of pomodoros\t|\n"
         "| Pomodoros +-------------------+\n"
         "|           | %d minutes each\t|\n"
         "+-----------+-------------------+\n"
         "|           | %d # of breaks\t|\n"
         "| Breaks    +-------------------+\n"
         "|           | %d minutes each\t|\n"
         "+-----------+-------------------+\n" ANSI_RESET ANSI_ITALIC
         "\nConfirm? (y/n): " ANSI_RESET,
         app.pomodoro_amount, app.pomodoro_len, app.break_amount,
         app.break_len);

  if (getchar() != 'y')
    return -1;

  for (;;) {
    runGameState(&app);
  }

  return 0;
}
