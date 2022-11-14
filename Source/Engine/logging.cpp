#include "logging.h"

namespace {

void spawn_console() {
  AllocConsole();
  FILE *fDummy;
  freopen_s(&fDummy, "CONOUT$", "w", stdout);
}

void enable_console_colors() {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

  if (hOut == INVALID_HANDLE_VALUE) {
  }

  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

  if (hIn == INVALID_HANDLE_VALUE) {
  }

  DWORD dwOriginalOutMode = 0;
  DWORD dwOriginalInMode = 0;

  if (!GetConsoleMode(hOut, &dwOriginalOutMode)) {
  }

  if (!GetConsoleMode(hIn, &dwOriginalInMode)) {
  }

  DWORD dwRequestedOutModes =
      ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
  DWORD dwRequestedInModes = ENABLE_VIRTUAL_TERMINAL_INPUT;

  DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;

  if (!SetConsoleMode(hOut, dwOutMode)) {
  }

  DWORD dwInMode = dwOriginalInMode | ENABLE_VIRTUAL_TERMINAL_INPUT;

  if (!SetConsoleMode(hIn, dwInMode)) {
  }
}
} // namespace

void logging_start() {
#if LOG_ENABLE == 1
  spawn_console();
  enable_console_colors();
#endif
}
