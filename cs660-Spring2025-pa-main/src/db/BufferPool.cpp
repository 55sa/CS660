#include <db/BufferPool.hpp>
#include <db/Database.hpp>
#include <numeric>

using namespace db;

BufferPool::BufferPool() {
    // TODO pa0
}

BufferPool::~BufferPool() {
    // TODO pa0
}

Page &BufferPool::getPage(const PageId &pid) {
    // TODO pa0
}

void BufferPool::markDirty(const PageId &pid) {
    // TODO pa0
}

bool BufferPool::isDirty(const PageId &pid) const {
    // TODO pa0
}

bool BufferPool::contains(const PageId &pid) const {
    // TODO pa0
}

void BufferPool::discardPage(const PageId &pid) {
    // TODO pa0
}

void BufferPool::flushPage(const PageId &pid) {
    // TODO pa0
}

void BufferPool::flushFile(const std::string &file) {
    // TODO pa0
}
