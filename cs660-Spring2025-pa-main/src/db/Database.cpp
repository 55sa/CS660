#include <db/Database.hpp>

using namespace db;

BufferPool &Database::getBufferPool() { return bufferPool; }

Database &db::getDatabase() {
    static Database instance;
    return instance;
}

void Database::add(std::unique_ptr<DbFile> file) {
    // TODO pa0
}

std::unique_ptr<DbFile> Database::remove(const std::string &name) {
    // TODO pa0
}

DbFile &Database::get(const std::string &name) const {
    // TODO pa0
}
