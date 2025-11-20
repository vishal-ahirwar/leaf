#ifndef SPINNER_H
#define SPINNER_H

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

class Spinner {
 public:
  enum class Color {
    none,
    white,
    black,
    crimson,
    blue,
    green,
    yellow,
    gray,
    orange,
    sky_blue
  };

 public:
  Spinner(std::string message = "", Color color = Color::white);

  void start();

  void stop();

  void setDisplayMessage(std::string, Color color = Color::none);

  ~Spinner() { stop(); }

 private:
  void run();
  void setColor();
  static void setupConsole();

  std::string mMessage;
  std::atomic<bool> mRunning;
  std::thread mThread;
  Color mColor{};
};

#endif  // SPINNER_H