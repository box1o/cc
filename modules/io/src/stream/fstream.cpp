#include "cc/io/stream/fstream.hpp"

namespace cc::io {

fstream::fstream(const std::filesystem::path& path, mode mode) 
: path_(path) {
    std::ios_base::openmode open_mode = std::ios_base::binary;

    switch (mode) {
        case mode::read:
            open_mode |= std::ios_base::in;
            break;
        case mode::write:
            open_mode |= std::ios_base::out | std::ios_base::trunc;
            break;
        case mode::read_write:
            open_mode |= std::ios_base::in | std::ios_base::out;
            break;
    }

    stream_.open(path, open_mode);

    if (!stream_.is_open()) {
        cc::log::Error("Failed to open file: {}", path.string());
    }
}

fstream::~fstream() {
    if (stream_.is_open()) {
        stream_.close();
    }
}

fstream::fstream(fstream&& other) noexcept 
    : stream_(std::move(other.stream_))
    , path_(std::move(other.path_)) {}

fstream& fstream::operator=(fstream&& other) noexcept {
    if (this != &other) {
        stream_ = std::move(other.stream_);
        path_ = std::move(other.path_);
    }
    return *this;
}

size_t fstream::read_impl(std::span<std::byte> buffer) {
    stream_.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    return stream_.gcount();
}

size_t fstream::write_impl(std::span<const std::byte> data) {
    stream_.write(reinterpret_cast<const char*>(data.data()), data.size());
    return stream_.good() ? data.size() : 0;
}

void fstream::flush_impl() {
    stream_.flush();
}

bool fstream::is_open_impl() const noexcept {
    return stream_.is_open();
}

cc::result<std::string> fstream::read_line() {
    std::string line;
    if (!std::getline(stream_, line)) {
        if (stream_.eof()) {
            return cc::err(cc::error_code::file_eof, "End of file");
        }
        return cc::err(cc::error_code::file_read_error, "Read failed");
    }

    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    return line;
}

void fstream::seek(size_t pos) {
    stream_.seekg(pos);
}

size_t fstream::tell() const {
    return const_cast<std::fstream&>(stream_).tellg();
}

const std::filesystem::path& fstream::path() const noexcept {
    return path_;
}

} // namespace cc::io
