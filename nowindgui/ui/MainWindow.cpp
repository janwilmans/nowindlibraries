#include "MainWindow.h"

#include "Theme.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <algorithm>
#include <cctype>
#include <utility>

namespace nowindgui {

namespace {

constexpr double activityLightSeconds = 0.12;

struct ScreenTab {
    Screen screen;
    ImGuiKey key;
    const char* keyName;
    const char* label;
};

constexpr ScreenTab screenTabs[] = {
    {Screen::Host, ImGuiKey_F1, "F1", "HOST"},
    {Screen::Library, ImGuiKey_F2, "F2", "LIBRARY"},
    {Screen::Log, ImGuiKey_F3, "F3", "LOG"},
    {Screen::Settings, ImGuiKey_F4, "F4", "SETTINGS"},
};

[[nodiscard]] const char* describe(SessionState state)
{
    switch (state)
    {
    case SessionState::Stopped: return "STOPPED";
    case SessionState::WaitingForInterface: return "WAITING FOR NOWIND INTERFACE";
    case SessionState::Hosting: return "HOSTING";
    case SessionState::Failed: return "FAILED";
    }
    return "";
}

[[nodiscard]] ImVec4 colorOf(SessionState state)
{
    switch (state)
    {
    case SessionState::Stopped: return toColor(role::disabledText);
    case SessionState::WaitingForInterface: return toColor(role::warning);
    case SessionState::Hosting: return toColor(role::good);
    case SessionState::Failed: return toColor(role::error);
    }
    return toColor(role::text);
}

[[nodiscard]] const char* describe(DriveKind kind)
{
    switch (kind)
    {
    case DriveKind::FloppyImage: return "FLOPPY";
    case DriveKind::HarddiskImage: return "HARDDISK";
    case DriveKind::RomDisk: return "ROM DISK";
    }
    return "";
}

[[nodiscard]] std::string formatSize(std::uintmax_t bytes)
{
    if (bytes >= 1024 * 1024)
    {
        return std::to_string(bytes / (1024 * 1024)) + " MB";
    }
    return std::to_string(bytes / 1024) + " KB";
}

[[nodiscard]] bool containsIgnoringCase(const std::string& text, const std::string& part)
{
    const auto lower = [](unsigned char character) { return static_cast<char>(std::tolower(character)); };
    std::string haystack(text.size(), ' ');
    std::string needle(part.size(), ' ');
    std::transform(text.begin(), text.end(), haystack.begin(), lower);
    std::transform(part.begin(), part.end(), needle.begin(), lower);
    return haystack.find(needle) != std::string::npos;
}

// A square block LED, lit while data is moving
void drawLed(bool lit)
{
    const float size = ImGui::GetTextLineHeight();
    const ImVec2 position = ImGui::GetCursorScreenPos();
    const ImVec4 color = lit ? toColor(role::good) : toColor(msx::darkGreen);
    ImGui::GetWindowDrawList()->AddRectFilled(position, ImVec2(position.x + size, position.y + size), ImGui::ColorConvertFloat4ToU32(color));
    ImGui::Dummy(ImVec2(size, size));
}

} // namespace

MainWindow::MainWindow(Settings settings, std::filesystem::path settingsFile)
    : mSettings(std::move(settings))
    , mSettingsFile(std::move(settingsFile))
{
    mLibraryItems = scanLibrary(mSettings.libraryFolder);
}

MainWindow::~MainWindow()
{
    mSession.stop();
    save();
}

const Settings& MainWindow::settings() const
{
    return mSettings;
}

void MainWindow::save() const
{
    saveSettings(mSettingsFile, mSettings);
}

void MainWindow::addDroppedFile(const std::string& path)
{
    std::error_code error;
    const std::uintmax_t size = std::filesystem::file_size(path, error);
    addDrive(path, error ? DriveKind::FloppyImage : guessDriveKind(size));
    changeScreen(Screen::Host);
}

void MainWindow::addDrive(std::string path, DriveKind kind)
{
    mConfiguration.drives.push_back({std::move(path), kind});
}

void MainWindow::changeScreen(Screen screen)
{
    mScreen = screen;
}

void MainWindow::pollSession()
{
    for (std::string& line : mSession.drainLog())
    {
        mLog.push_back(std::move(line));
    }
    while (mLog.size() > maximumLogLines)
    {
        mLog.pop_front();
    }

    const unsigned long long bytes = mSession.transferredBytes();
    if (bytes != mLastTransferredBytes)
    {
        mLastTransferredBytes = bytes;
        mActivityUntil = ImGui::GetTime() + activityLightSeconds;
    }
}

void MainWindow::draw()
{
    pollSession();
    for (const ScreenTab& tab : screenTabs)
    {
        if (ImGui::IsKeyPressed(tab.key, false))
        {
            changeScreen(tab.screen);
        }
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (ImGui::Begin("nowind", nullptr, flags))
    {
        drawHeader();
        const float footerHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().WindowPadding.y;
        if (ImGui::BeginChild("screen", ImVec2(0.0F, -footerHeight), ImGuiChildFlags_Borders))
        {
            switch (mScreen)
            {
            case Screen::Host: drawHostScreen(); break;
            case Screen::Library: drawLibraryScreen(); break;
            case Screen::Log: drawLogScreen(); break;
            case Screen::Settings: drawSettingsScreen(); break;
            }
        }
        ImGui::EndChild();
        drawFooter();
    }
    ImGui::End();
}

void MainWindow::drawHeader()
{
    const SessionState state = mSession.state();
    ImGui::TextColored(toColor(role::accent), "NOWIND INTERFACE HOST");
    ImGui::SameLine();
    ImGui::TextDisabled("MSX2");
    ImGui::SameLine(0.0F, ImGui::GetFontSize() * 2.0F);
    drawLed(ImGui::GetTime() < mActivityUntil);
    ImGui::SameLine();
    ImGui::TextColored(colorOf(state), "%s", describe(state));
    if (state == SessionState::Failed)
    {
        ImGui::SameLine();
        ImGui::TextColored(toColor(role::error), "- %s", mSession.lastError().c_str());
    }
}

void MainWindow::drawFooter()
{
    for (const ScreenTab& tab : screenTabs)
    {
        const bool selected = tab.screen == mScreen;
        if (selected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, toColor(role::controlActive));
        }
        if (ImGui::Button((std::string(tab.keyName) + " " + tab.label).c_str()))
        {
            changeScreen(tab.screen);
        }
        if (selected)
        {
            ImGui::PopStyleColor();
        }
        ImGui::SameLine();
    }
    ImGui::NewLine();
}

void MainWindow::drawHostScreen()
{
    const bool locked = mSession.isActive();
    ImGui::BeginDisabled(locked);
    drawDriveList();
    drawHostOptions();
    ImGui::EndDisabled();
    drawHostControls();
}

void MainWindow::drawDriveList()
{
    ImGui::TextColored(toColor(role::accent), "DRIVES");
    ImGui::SameLine();
    ImGui::TextDisabled("(drop .dsk files on the window, or add them from the library)");

    std::size_t removeIndex = mConfiguration.drives.size();
    for (std::size_t index = 0; index < mConfiguration.drives.size(); ++index)
    {
        DriveEntry& drive = mConfiguration.drives[index];
        ImGui::PushID(static_cast<int>(index));
        if (ImGui::Button("X"))
        {
            removeIndex = index;
        }
        ImGui::SameLine();
        ImGui::Text("%c:", static_cast<char>('A' + index % 26));
        ImGui::SameLine();
        ImGui::Text("%-8s", describe(drive.kind));
        ImGui::SameLine();
        ImGui::TextUnformatted(drive.kind == DriveKind::RomDisk ? "(in the interface)" : drive.path.c_str());
        ImGui::PopID();
    }
    if (removeIndex < mConfiguration.drives.size())
    {
        mConfiguration.drives.erase(mConfiguration.drives.begin() + static_cast<std::ptrdiff_t>(removeIndex));
    }
    if (mConfiguration.drives.empty())
    {
        ImGui::TextDisabled("no drives, the MSX will see an empty interface");
    }

    ImGui::SetNextItemWidth(-ImGui::CalcTextSize("ADD FLOPPY HARDDISK ROM DISK").x - ImGui::GetStyle().ItemSpacing.x * 12.0F);
    ImGui::InputTextWithHint("##path", "path to a disk image", &mDrivePathInput);
    const std::string path = mDrivePathInput;
    const auto addInput = [&](DriveKind kind) {
        if (!path.empty())
        {
            addDrive(path, kind);
            mDrivePathInput.clear();
        }
    };
    ImGui::SameLine();
    if (ImGui::Button("ADD FLOPPY")) addInput(DriveKind::FloppyImage);
    ImGui::SameLine();
    if (ImGui::Button("HARDDISK")) addInput(DriveKind::HarddiskImage);
    ImGui::SameLine();
    if (ImGui::Button("ROM DISK")) addDrive(std::string(), DriveKind::RomDisk);
    ImGui::Separator();
}

void MainWindow::drawHostOptions()
{
    ImGui::TextColored(toColor(role::accent), "OPTIONS");
    ImGui::Checkbox("Prefer MSX-DOS2 (MSX2 only)", &mSettings.preferDos2);
    ImGui::Checkbox("Allow other disk ROMs to initialize", &mSettings.allowOtherDiskRoms);
    ImGui::Checkbox("Phantom drives (as when not holding CTRL)", &mSettings.phantomDrives);
    ImGui::Separator();
}

void MainWindow::drawHostControls()
{
    const bool active = mSession.isActive();
    if (active)
    {
        if (ImGui::Button("STOP HOSTING"))
        {
            mSession.stop();
        }
    }
    else if (ImGui::Button("START HOSTING"))
    {
        startHosting();
    }
    ImGui::SameLine();
    ImGui::Text("%llu bytes transferred", mSession.transferredBytes());
    if (!mNotice.empty())
    {
        ImGui::TextColored(toColor(role::warning), "%s", mNotice.c_str());
    }
}

void MainWindow::startHosting()
{
    mConfiguration.preferDos2 = mSettings.preferDos2;
    mConfiguration.allowOtherDiskRoms = mSettings.allowOtherDiskRoms;
    mConfiguration.phantomDrives = mSettings.phantomDrives;
    mNotice.clear();
    mLastTransferredBytes = 0;
    save();
    mSession.start(mConfiguration);
}

void MainWindow::drawLibraryScreen()
{
    ImGui::TextColored(toColor(role::accent), "LIBRARY FOLDER");
    ImGui::SetNextItemWidth(-ImGui::CalcTextSize("SCAN").x - ImGui::GetStyle().ItemSpacing.x * 6.0F);
    const bool entered = ImGui::InputText("##folder", &mSettings.libraryFolder, ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if (ImGui::Button("SCAN") || entered)
    {
        mLibraryItems = scanLibrary(mSettings.libraryFolder);
        save();
    }
    ImGui::InputTextWithHint("##filter", "filter", &mLibraryFilter);
    ImGui::Separator();

    if (mLibraryItems.empty())
    {
        ImGui::TextDisabled("no disk images (*.dsk) found");
    }
    for (std::size_t index = 0; index < mLibraryItems.size(); ++index)
    {
        const LibraryItem& item = mLibraryItems[index];
        if (!mLibraryFilter.empty() && !containsIgnoringCase(item.name, mLibraryFilter))
        {
            continue;
        }
        ImGui::PushID(static_cast<int>(index));
        if (ImGui::Button("ADD"))
        {
            addDrive(item.path, item.kind);
            mNotice = "Added " + item.name + " to the drives";
        }
        ImGui::SameLine();
        ImGui::Text("%-8s %8s  %s", describe(item.kind), formatSize(item.sizeBytes).c_str(), item.name.c_str());
        ImGui::PopID();
    }
}

void MainWindow::drawLogScreen()
{
    if (ImGui::Button("CLEAR"))
    {
        mLog.clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Follow", &mLogScrollToEnd);
    ImGui::Separator();
    for (const std::string& line : mLog)
    {
        ImGui::TextUnformatted(line.c_str());
    }
    if (mLogScrollToEnd && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0F)
    {
        ImGui::SetScrollHereY(1.0F);
    }
}

void MainWindow::drawSettingsScreen()
{
    ImGui::TextColored(toColor(role::accent), "DISPLAY");
    ImGui::SliderInt("Size", &mSettings.uiZoom, 1, 3);
    ImGui::BeginDisabled(mSettings.reduceEffects);
    ImGui::SliderInt("Scanlines %", &mSettings.scanlineIntensity, 0, 100);
    ImGui::EndDisabled();
    ImGui::Checkbox("Reduce effects (no scanlines)", &mSettings.reduceEffects);
    ImGui::Separator();
    if (ImGui::Button("SAVE SETTINGS"))
    {
        save();
    }
}

} // namespace nowindgui
