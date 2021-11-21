
#include "GfxException.h"
#include <fmt/format.h>

GfxException::GfxException(int line, const char *file) noexcept
:   line(line), file(file)

{

}

const char *GfxException::what() const noexcept {

    return fmt::format("{},{}: {}", GetFile(), GetLine(), GetType()).c_str();
}

const char *GfxException::GetType() const {
    return "GfxException";
}

int GfxException::GetLine() const noexcept {
    return line;
}

std::string GfxException::GetFile() const noexcept {
    return file;
}

const char *GfxException::GetOriginString() const noexcept {
    return nullptr;
}

