#pragma once

#include "../app/Library.h"
#include "../app/Settings.h"
#include "../host/HostConfiguration.h"
#include "../host/HostSession.h"

#include <deque>
#include <filesystem>
#include <string>
#include <vector>

namespace nowindgui {

enum class Screen {
    Host,
    Library,
    Log,
    Settings
};

// The one window of the application: a header with the connection state, the current screen and the function key strip
class MainWindow {
public:
    MainWindow(Settings settings, std::filesystem::path settingsFile);
    ~MainWindow();
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

    // Call once per frame between ImGui::NewFrame() and ImGui::Render()
    void draw();

    void addDroppedFile(const std::string& path);

    [[nodiscard]] const Settings& settings() const;

private:
    void drawHeader();
    void drawFooter();
    void drawHostScreen();
    void drawDriveList();
    void drawHostOptions();
    void drawHostControls();
    void drawLibraryScreen();
    void drawLogScreen();
    void drawSettingsScreen();

    void pollSession();
    void startHosting();
    void addDrive(std::string path, DriveKind kind);
    void changeScreen(Screen screen);
    void save() const;

    static constexpr std::size_t maximumLogLines = 2000;

    Settings mSettings;
    std::filesystem::path mSettingsFile;
    HostSession mSession;
    HostConfiguration mConfiguration;
    std::vector<LibraryItem> mLibraryItems;
    std::deque<std::string> mLog;
    std::string mDrivePathInput;
    std::string mLibraryFilter;
    std::string mNotice;
    unsigned long long mLastTransferredBytes = 0;
    double mActivityUntil = 0.0;
    Screen mScreen = Screen::Host;
    bool mLogScrollToEnd = true;
};

} // namespace nowindgui
