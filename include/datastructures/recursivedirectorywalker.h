#include <filesystem>
#include <uv.h>

namespace fs = std::filesystem;

struct RecursiveDirectoryWalker {
    fs::recursive_directory_iterator ix_it; // iterator to index walker.
					    //
    RecursiveDirectoryWalker(fs::path path) {
      ix_it = fs::recursive_directory_iterator(path);
    }

    bool done() const {
        return this->ix_it == std::filesystem::end(this->ix_it);
    }

    std::optional<fs::path> tick() {
        if (done()) {
            return {};
        }
        this->ix_it++; // next file.
        const fs::path curp = *this->ix_it;
        if (!std::filesystem::is_regular_file(curp)) {
            return {};
        }
        assert(std::filesystem::is_regular_file(curp));
        return curp;
    }
};
