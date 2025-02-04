#include <db/Database.hpp>
#include <stdexcept>

namespace db {


    class Database::Impl {
    public:
        std::unordered_map<std::string, std::unique_ptr<DbFile>> files;
    };

    Database &getDatabase() {
        static Database instance;
        return instance;
    }


    BufferPool &Database::getBufferPool() {
        return bufferPool;
    }


    Database::Database() : pImpl(std::make_unique<Impl>()) {
    }

    // add
    void Database::add(std::unique_ptr<DbFile> file) {
        // TODO pa0
        const std::string &name = file->getName();
        if (pImpl->files.find(name) != pImpl->files.end()) {

            throw std::logic_error("File name already exists: " + name);
        }

        pImpl->files[name] = std::move(file);
    }

    // remove
    std::unique_ptr<DbFile> Database::remove(const std::string &name) {
        // TODO pa0
        auto it = pImpl->files.find(name);
        if (it == pImpl->files.end()) {
            throw std::logic_error("No such file: " + name);
        }

        bufferPool.flushFile(name);


        std::unique_ptr<DbFile> removed = std::move(it->second);
        pImpl->files.erase(it);
        return removed;
    }

    // get
    DbFile &Database::get(const std::string &name) const {
        // TODO pa0
        auto it = pImpl->files.find(name);
        if (it == pImpl->files.end()) {
            throw std::logic_error("No such file: " + name);
        }
        return *(it->second);
    }


    Database::~Database() = default;

} // namespace db
