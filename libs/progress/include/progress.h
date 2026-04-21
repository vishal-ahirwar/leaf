#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

namespace progress
{

enum class Color
{
    none, white, black, crimson, blue, green, yellow, gray, orange, sky_blue
};

class Spinner
{
  public:
    explicit Spinner(std::string message = "", Color color = Color::white);
    ~Spinner();

    void start();
    void stop();
    void setDisplayMessage(std::string msg, Color color = Color::none);
    
    bool isRunning() const { return mRunning.load(); }

    static void suspendCurrent();
    static void resumeCurrent();

  private:
    void run();
    void setColor();
    static void setupConsole();

    std::string mMessage;
    std::atomic<bool> mRunning;
    std::atomic<bool> mSuspended{false};
    std::thread mThread;
    Color mColor;
    std::chrono::steady_clock::time_point mStartTime;
    static std::mutex sMutex;
    static Spinner* s_ActiveSpinner;
};

class ProgressBar
{
  public:
    explicit ProgressBar(std::string message = "", Color color = Color::white);
    ~ProgressBar();

    void start();
    void stop();
    void setProgress(float percentage); // 0.0 to 100.0
    void setDisplayMessage(std::string msg);

  private:
    void draw();
    void setColor();
    
    std::string mMessage;
    float mProgress{0.0f};
    Color mColor;
    std::chrono::steady_clock::time_point mStartTime;
    static std::mutex sMutex;
};

} // namespace progress
