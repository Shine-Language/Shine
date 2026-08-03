#pragma once
#include <stdexcept>
#include "shine/token.h"

namespace shine {

class CompileError : public std::runtime_error {
public:
    CompileError(SourceLoc loc, std::string msg)
        : std::runtime_error(loc.file + ":" + std::to_string(loc.line) + ":" +
                              std::to_string(loc.col) + ": error: " + msg),
          loc_(loc) {}
    const SourceLoc& loc() const { return loc_; }
private:
    SourceLoc loc_;
};

}
