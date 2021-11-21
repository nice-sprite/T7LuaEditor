//
// Created by coxtr on 11/20/2021.
//

#ifndef T7LUAEDITOR_GFXEXCEPTION_H
#define T7LUAEDITOR_GFXEXCEPTION_H

#include <exception>
#include <string>
class GfxException : public std::exception {
    GfxException(int line, const char* file) noexcept;
    const char* what() const noexcept override;
    virtual const char* GetType() const;
    int GetLine() const noexcept;
    std::string GetFile() const noexcept;
    const char* GetOriginString() const noexcept;
private:
    int line;
    std::string file;
protected:
    mutable std::wstring whatBuffer;
};


#endif //T7LUAEDITOR_GFXEXCEPTION_H
