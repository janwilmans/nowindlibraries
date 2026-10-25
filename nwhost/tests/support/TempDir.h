//! TempDir.h
#ifndef NWTEST_TEMPDIR_H
#define NWTEST_TEMPDIR_H

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

namespace nwtest {

// a fresh directory that is removed again with everything in it
class TempDir {
public:
    TempDir()
    {
        static std::atomic<unsigned> counter{0};
        auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path = std::filesystem::temp_directory_path() /
               ("nwtest-" + std::to_string(stamp) + "-" + std::to_string(counter++));
        std::filesystem::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ignored;
        std::filesystem::permissions(path, std::filesystem::perms::owner_all, std::filesystem::perm_options::add, ignored);
        for (auto& entry : std::filesystem::recursive_directory_iterator(path, ignored))
        {
            std::filesystem::permissions(entry.path(), std::filesystem::perms::owner_all,
                                         std::filesystem::perm_options::add, ignored);
        }
        std::filesystem::remove_all(path, ignored);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    std::string file(const std::string& name) const { return (path / name).string(); }

    std::string write(const std::string& name, const std::vector<std::uint8_t>& content) const
    {
        std::ofstream out(path / name, std::ios::binary);
        out.write(reinterpret_cast<const char*>(content.data()), std::streamsize(content.size()));
        return file(name);
    }

    std::vector<std::uint8_t> read(const std::string& name) const
    {
        std::ifstream in(path / name, std::ios::binary);
        return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }

    std::filesystem::path path;
};

// changes the current directory for the lifetime of this object (Device and BDOSProxy work relative to it)
class CurrentDirectory {
public:
    explicit CurrentDirectory(const std::filesystem::path& directory)
        : previous(std::filesystem::current_path())
    {
        std::filesystem::current_path(directory);
    }
    ~CurrentDirectory()
    {
        std::error_code ignored;
        std::filesystem::current_path(previous, ignored);
    }

private:
    std::filesystem::path previous;
};

} // namespace nwtest

#endif // NWTEST_TEMPDIR_H
