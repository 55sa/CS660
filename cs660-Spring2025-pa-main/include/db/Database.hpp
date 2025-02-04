#pragma once

#include <db/BufferPool.hpp>
#include <db/DbFile.hpp>
#include <memory>
#include <string>
#include <unordered_map>

/**
 * @brief A database is a collection of files and a BufferPool.
 * @details The Database class is responsible for managing the database files.
 * It provides functions to add new database files, retrieve them by name, and remove them.
 * The class also supports removing all files from the catalog.
 * @note A Database owns the DbFile objects that are added to it.
 */
namespace db {
    class Database {
        /// 私有实现类（PImpl），隐藏 catalog 等成员
        class Impl;
        std::unique_ptr<Impl> pImpl;

        /// 该 Database 自带一个 BufferPool
        BufferPool bufferPool;

        /// 构造函数私有化（Singleton）
        Database();

    public:
        /// 允许 getDatabase() 访问私有构造
        friend Database &getDatabase();

        Database(Database const &) = delete;
        void operator=(Database const &) = delete;
        Database(Database &&) = delete;
        void operator=(Database &&) = delete;

        /**
         * @brief 析构时可以做一些清理
         */
        ~Database();

        /**
         * @brief Provides access to the singleton instance of the BufferPool.
         * @return The buffer pool
         */
        BufferPool &getBufferPool();

        /**
         * @brief Adds a new file to the Database.
         * @param file The file to add.
         * @throws std::logic_error if the file name already exists.
         * @note This method takes ownership of the DbFile.
         */
        void add(std::unique_ptr<DbFile> file);

        /**
         * @brief Removes a file.
         * @param name The name of the file to remove.
         * @return The removed file (as a std::unique_ptr).
         * @throws std::logic_error if the name does not exist.
         * @note This method should call BufferPool::flushFile(name)
         * @note This method moves the DbFile ownership to the caller.
         */
        std::unique_ptr<DbFile> remove(const std::string &name);

        /**
         * @brief Returns the DbFile of the specified name.
         * @param name The name of the file.
         * @return A reference to the DbFile object.
         * @throws std::logic_error if the name does not exist.
         */
        DbFile &get(const std::string &name) const;
    };

    /**
     * @brief Returns the singleton instance of the Database.
     * @return The Database object.
     */
    Database &getDatabase();
} // namespace db
