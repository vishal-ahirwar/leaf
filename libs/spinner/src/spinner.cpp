#include "../include/spinner.h"

#include <fmt/color.h>
#include <fmt/core.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#endif
fmt::color spinner_color;

Spinner::Spinner(std::string message, Color color)
    : mMessage(std::move(message)), mRunning(false), mColor(color) {
  setColor();
}

void Spinner::start() {
  if (mRunning.exchange(true)) return;
  setupConsole();
  mThread = std::thread([this] { run(); });
};

void Spinner::stop() {
  if (!mRunning.exchange(false)) return;
  if (mThread.joinable()) mThread.join();
  fmt::print(fmt::fg(spinner_color), "\n");
  std::cout.flush();
}

void Spinner::run() {
  static const char* frames[] = {"⠋", "⠙", "⠹", "⠸", "⠼",
                                 "⠴", "⠦", "⠧", "⠇", "⠏"};
  size_t i = 0;
  while (mRunning.load()) {
    fmt::print(fmt::fg(spinner_color), "\r{} {:<30}", frames[i], mMessage);
    std::cout.flush();
    i = (i + 1) % (sizeof(frames) / sizeof(frames[0]));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
void Spinner::setupConsole() {
#if defined(_WIN32)
  SetConsoleOutputCP(CP_UTF8);
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut != INVALID_HANDLE_VALUE) {
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode)) {
      mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(hOut, mode);
    }
  }
#endif
}

void Spinner::setDisplayMessage(std::string message, Color color) {
  this->mMessage = std::move(message);
  if (color != Color::none) {
    mColor = color;
    setColor();
  }
}

void Spinner::setColor() {
  switch (mColor) {
    case Color::black:
      spinner_color = fmt::color::black;
      break;
    case Color::blue:
      spinner_color = fmt::color::blue;
      break;
    case Color::crimson:
      spinner_color = fmt::color::crimson;
      break;
    case Color::green:
      spinner_color = fmt::color::green;
      break;
    case Color::white:
      spinner_color = fmt::color::white;
      break;
    case Color::yellow:
      spinner_color = fmt::color::yellow;
      break;
    case Color::gray:
      spinner_color = fmt::color::gray;
      break;
    case Color::orange:
      spinner_color = fmt::color::orange;
      break;
    case Color::sky_blue:
      spinner_color = fmt::color::sky_blue;
      break;
    case Color::none:
      spinner_color=fmt::color::dark_gray;
      break;
  }
}