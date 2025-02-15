#include <db/DbFile.hpp>
#include <stdexcept>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

using namespace db;

/**
 * @brief Returns the TupleDesc describing this file's schema.
 */
const TupleDesc &DbFile::getTupleDesc() const {
    return td;
}

/**
 * @brief DbFile constructor: opens/creates the file, calculates numPages.
 */
// TODO pa1: open file and initialize numPages
// Hint: use open, fstat
DbFile::DbFile(const std::string &name, const TupleDesc &td)
    : name(name), td(td), numPages(0), fd(-1)
{
    fd = ::open(name.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        throw std::runtime_error("DbFile: Could not open file " + name +
                                 ", errno=" + std::to_string(errno));
    }

    struct stat st{};
    if (::fstat(fd, &st) < 0) {
        ::close(fd);
        throw std::runtime_error("DbFile: fstat failed for file " + name);
    }

    size_t fileSize = static_cast<size_t>(st.st_size);
    if (fileSize == 0) {
        // if empty, create one page
        if (::ftruncate(fd, DEFAULT_PAGE_SIZE) < 0) {
            ::close(fd);
            throw std::runtime_error("DbFile: Could not truncate file to 1 page, errno=" +
                                     std::to_string(errno));
        }
        numPages = 1;
    } else {
        if (fileSize % DEFAULT_PAGE_SIZE != 0) {
            ::close(fd);
            throw std::runtime_error(
                "DbFile: File size not multiple of page size, file=" + name);
        }
        numPages = fileSize / DEFAULT_PAGE_SIZE;
        if (numPages == 0) {
            numPages = 1;
        }
    }
}

/**
 * @brief DbFile destructor: closes file.
 */
// TODO pa1: close file
// Hint: use close
DbFile::~DbFile() {
    if (fd >= 0) {
        ::close(fd);
    }
}

/**
 * @brief Returns the file name.
 */
const std::string &DbFile::getName() const {
    return name;
}

/**
 * @brief Reads a page from the file using pread.
 * @param page The page buffer
 * @param id The page number
 */
// TODO pa1: read page
// Hint: use pread
void DbFile::readPage(Page &page, const size_t id) const {
    reads.push_back(id);
    off_t offset = static_cast<off_t>(id) * DEFAULT_PAGE_SIZE;
    ssize_t bytesRead = ::pread(fd, page.data(), DEFAULT_PAGE_SIZE, offset);
    if (bytesRead < 0) {
        throw std::runtime_error("DbFile::readPage: pread failed, errno=" +
                                 std::to_string(errno));
    }
    if (bytesRead != static_cast<ssize_t>(DEFAULT_PAGE_SIZE)) {
        throw std::runtime_error("DbFile::readPage: incomplete read from file.");
    }
}

/**
 * @brief Writes a page to the file using pwrite.
 * @param page The page buffer
 * @param id The page number
 */
// TODO pa1: write page
// Hint: use pwrite
void DbFile::writePage(const Page &page, const size_t id) const {
    writes.push_back(id);
    off_t offset = static_cast<off_t>(id) * DEFAULT_PAGE_SIZE;
    ssize_t bytesWritten = ::pwrite(fd, page.data(), DEFAULT_PAGE_SIZE, offset);
    if (bytesWritten < 0) {
        throw std::runtime_error("DbFile::writePage: pwrite failed, errno=" +
                                 std::to_string(errno));
    }
    if (bytesWritten != static_cast<ssize_t>(DEFAULT_PAGE_SIZE)) {
        throw std::runtime_error("DbFile::writePage: incomplete write to file.");
    }
}

/**
 * @brief Returns the list of page IDs read so far.
 */
const std::vector<size_t> &DbFile::getReads() const {
    return reads;
}

/**
 * @brief Returns the list of page IDs written so far.
 */
const std::vector<size_t> &DbFile::getWrites() const {
    return writes;
}

/**
 * @brief Inserts a tuple; by default not implemented.
 */
void DbFile::insertTuple(const Tuple &t) {
    throw std::runtime_error("Not implemented");
}

/**
 * @brief Deletes a tuple; by default not implemented.
 */
void DbFile::deleteTuple(const Iterator &it) {
    throw std::runtime_error("Not implemented");
}

/**
 * @brief Gets a tuple; by default not implemented.
 */
Tuple DbFile::getTuple(const Iterator &it) const {
    throw std::runtime_error("Not implemented");
}

/**
 * @brief Moves the iterator to the next tuple; by default not implemented.
 */
void DbFile::next(Iterator &it) const {
    throw std::runtime_error("Not implemented");
}

/**
 * @brief Returns an iterator to the first tuple; by default not implemented.
 */
Iterator DbFile::begin() const {
    throw std::runtime_error("Not implemented");
}

/**
 * @brief Returns an iterator to the end of the file; by default not implemented.
 */
Iterator DbFile::end() const {
    throw std::runtime_error("Not implemented");
}

/**
 * @brief Returns the number of pages in the file.
 */
size_t DbFile::getNumPages() const {
    return numPages;
}
