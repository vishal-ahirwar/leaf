#include "../include/progress.h"
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace progress
{

std::mutex Spinner::sMutex;
Spinner* Spinner::s_ActiveSpinner = nullptr;
std::mutex ProgressBar::sMutex;

void Spinner::setupConsole()
{
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

Spinner::Spinner(std::string message, Color color)
    : mMessage(std::move(message)), mRunning(false), mColor(color)
{
    setupConsole();
}

Spinner::~Spinner()
{
    stop();
}

void Spinner::start()
{
    if (mRunning) return;
    s_ActiveSpinner = this;
    mRunning = true;
    mSuspended = false;
    mStartTime = std::chrono::steady_clock::now();
    mThread = std::thread(&Spinner::run, this);
}

void Spinner::stop()
{
    if (!mRunning) return;
    mRunning = false;
    if (mThread.joinable()) {
        mThread.join();
    }
    std::lock_guard<std::mutex> lock(sMutex);
    if (s_ActiveSpinner == this) s_ActiveSpinner = nullptr;
    std::cout << "\r\033[K"; 
    std::cout.flush();
}

void Spinner::suspendCurrent()
{
    std::lock_guard<std::mutex> lock(sMutex);
    if (s_ActiveSpinner) {
        s_ActiveSpinner->mSuspended = true;
        std::cout << "\r\033[K"; 
        std::cout.flush();
    }
}

void Spinner::resumeCurrent()
{
    std::lock_guard<std::mutex> lock(sMutex);
    if (s_ActiveSpinner) {
        s_ActiveSpinner->mSuspended = false;
    }
}

void Spinner::setDisplayMessage(std::string msg, Color color)
{
    std::lock_guard<std::mutex> lock(sMutex);
    mMessage = std::move(msg);
    if (color != Color::none) {
        mColor = color;
    }
}

void Spinner::setColor()
{
    switch (mColor)
    {
    case Color::green: fmt::print(fmt::fg(fmt::color::lime_green), ""); break;
    case Color::white: fmt::print(fmt::fg(fmt::color::white), ""); break;
    case Color::blue: fmt::print(fmt::fg(fmt::color::dodger_blue), ""); break;
    case Color::yellow: fmt::print(fmt::fg(fmt::color::yellow), ""); break;
    case Color::crimson: fmt::print(fmt::fg(fmt::color::crimson), ""); break;
    default: fmt::print(fmt::fg(fmt::color::white), ""); break;
    }
}

void Spinner::run()
{
    const char* chars = "|/-\\";
    int i = 0;
    while (mRunning)
    {
        {
            std::lock_guard<std::mutex> lock(sMutex);
            if (!mSuspended)
            {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - mStartTime).count();
                
                std::cout << "\r\033[K"; 
                setColor();
                fmt::print("{} {} [{:02d}:{:02d}]", chars[i % 4], mMessage, elapsed / 60, elapsed % 60);
                
                // reset color
                fmt::print("\033[0m");
                std::cout.flush();
                i++;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

ProgressBar::ProgressBar(std::string message, Color color)
    : mMessage(std::move(message)), mColor(color)
{
}

ProgressBar::~ProgressBar()
{
    stop();
}

void ProgressBar::start()
{
    mStartTime = std::chrono::steady_clock::now();
    draw();
}

void ProgressBar::stop()
{
    std::lock_guard<std::mutex> lock(sMutex);
    std::cout << "\r\033[K"; 
    std::cout.flush();
}

void ProgressBar::setProgress(float percentage)
{
    mProgress = std::max(0.0f, std::min(100.0f, percentage));
    draw();
}

void ProgressBar::setDisplayMessage(std::string msg)
{
    std::lock_guard<std::mutex> lock(sMutex);
    mMessage = std::move(msg);
    draw();
}

void ProgressBar::setColor()
{
    switch (mColor)
    {
    case Color::green: fmt::print(fmt::fg(fmt::color::lime_green), ""); break;
    case Color::white: fmt::print(fmt::fg(fmt::color::white), ""); break;
    case Color::blue: fmt::print(fmt::fg(fmt::color::dodger_blue), ""); break;
    case Color::yellow: fmt::print(fmt::fg(fmt::color::yellow), ""); break;
    case Color::crimson: fmt::print(fmt::fg(fmt::color::crimson), ""); break;
    default: fmt::print(fmt::fg(fmt::color::white), ""); break;
    }
}

void ProgressBar::draw()
{
    std::lock_guard<std::mutex> lock(sMutex);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - mStartTime).count();
    
    std::cout << "\r\033[K"; 
    setColor();
    
    int barWidth = 40;
    int pos = static_cast<int>(barWidth * (mProgress / 100.0f));
    
    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    
    fmt::print("] {:3.0f}% | {} [{:02d}:{:02d}]", mProgress, mMessage, elapsed / 60, elapsed % 60);
    fmt::print("\033[0m");
    std::cout.flush();
}

} // namespace progress
