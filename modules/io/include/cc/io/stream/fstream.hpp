#pragma once
#include "../base/sbase.hpp"
#include <fstream>
#include <filesystem>

namespace cc::io {

enum class mode {
    read,
    write,
    read_write,
};

class fstream : public sbase<fstream> {

public:
    explicit fstream(const std::filesystem::path& path, mode mode = mode::read);

    ~fstream();

    fstream(const fstream&) = delete;
    fstream& operator=(const fstream&) = delete;

    fstream(fstream&& other) noexcept;
    fstream& operator=(fstream&& other) noexcept;

    [[nodiscard]] size_t read_impl(std::span<std::byte> buffer);
    [[nodiscard]] size_t write_impl(std::span<const std::byte> data);

    void flush_impl();

    [[nodiscard]] bool is_open_impl() const noexcept;
    [[nodiscard]] cc::result<std::string> read_line();

    void seek(size_t pos);
    [[nodiscard]] size_t tell() const;
    [[nodiscard]] const std::filesystem::path& path() const noexcept;


private:
    std::fstream stream_;
    std::filesystem::path path_;

};

} // namespace cc::io
