#include <db/BufferPool.hpp>
#include <db/Database.hpp>
#include <stdexcept>
#include <algorithm>

namespace db {


struct Frame {
    bool inUse = false;
    bool dirty = false;
    PageId pid{};
    Page page{};
};

class BufferPool::Impl {
public:

    std::array<Frame, DEFAULT_NUM_PAGES> frames;


    std::list<size_t> lruList;


    std::unordered_map<PageId, size_t, std::hash<PageId>> pageTable;

    Impl() {

    }


    void moveToFrontOfLRU(size_t idx) {

        lruList.remove(idx);
        lruList.push_front(idx);
    }


    size_t findFreeSlot() const {
        for (size_t i = 0; i < DEFAULT_NUM_PAGES; i++) {
            if (!frames[i].inUse) {
                return i;
            }
        }
        return npos;
    }


    void evictOnePage() {
        if (lruList.empty()) {
            throw std::runtime_error("No page to evict, but buffer is full. Something is wrong!");
        }

        size_t victim = lruList.back();
        Frame &frm = frames[victim];

        if (frm.dirty) {
            flushOnePage(frm.pid, frm);
        }

        pageTable.erase(frm.pid);


        frm.inUse = false;
        frm.dirty = false;
        frm.pid = PageId{};


        lruList.pop_back();
    }


    void flushOnePage(const PageId &pid, Frame &frm) {
        auto &dbFile = getDatabase().get(pid.file);
        dbFile.writePage(frm.page, pid.page);
        frm.dirty = false;
    }


    static constexpr size_t npos = static_cast<size_t>(-1);
};

BufferPool::BufferPool()
    : pImpl(std::make_unique<Impl>())
{

}

BufferPool::~BufferPool() {
    // TODO pa0:
    for (size_t i = 0; i < DEFAULT_NUM_PAGES; i++) {
        Frame &frm = pImpl->frames[i];
        if (frm.inUse && frm.dirty) {
            pImpl->flushOnePage(frm.pid, frm);
        }
    }
}

Page &BufferPool::getPage(const PageId &pid) {
    // TODO pa0:


    auto it = pImpl->pageTable.find(pid);
    if (it != pImpl->pageTable.end()) {

        size_t idx = it->second;

        pImpl->moveToFrontOfLRU(idx);
        return pImpl->frames[idx].page;
    }


    size_t freeIdx = pImpl->findFreeSlot();
    if (freeIdx == Impl::npos) {

        pImpl->evictOnePage();

        freeIdx = pImpl->findFreeSlot();
        if (freeIdx == Impl::npos) {

            throw std::runtime_error("Cannot find free slot after eviction");
        }
    }


    Frame &frm = pImpl->frames[freeIdx];
    frm.inUse = true;
    frm.dirty = false;
    frm.pid = pid;


    {
        auto &dbFile = getDatabase().get(pid.file);
        dbFile.readPage(frm.page, pid.page);
    }


    pImpl->pageTable[pid] = freeIdx;

    pImpl->moveToFrontOfLRU(freeIdx);

    return frm.page;
}

void BufferPool::markDirty(const PageId &pid) {
    // TODO pa0
    auto it = pImpl->pageTable.find(pid);
    if (it == pImpl->pageTable.end()) {

        return;
    }
    size_t idx = it->second;
    pImpl->frames[idx].dirty = true;
}

    bool BufferPool::isDirty(const PageId &pid) const {

    auto it = pImpl->pageTable.find(pid);
    if (it == pImpl->pageTable.end()) {

        throw std::runtime_error("Page not found in BufferPool::isDirty()");
    }

    return pImpl->frames[it->second].dirty;
}


bool BufferPool::contains(const PageId &pid) const {
    // TODO pa0
    return (pImpl->pageTable.find(pid) != pImpl->pageTable.end());
}

void BufferPool::discardPage(const PageId &pid) {
    // TODO pa0
    auto it = pImpl->pageTable.find(pid);
    if (it == pImpl->pageTable.end()) {

        return;
    }
    size_t idx = it->second;


    pImpl->pageTable.erase(it);


    pImpl->lruList.remove(idx);


    Frame &frm = pImpl->frames[idx];
    frm.inUse = false;
    frm.dirty = false;
    frm.pid = PageId{};
}

void BufferPool::flushPage(const PageId &pid) {
    // TODO pa0
    auto it = pImpl->pageTable.find(pid);
    if (it == pImpl->pageTable.end()) {

        return;
    }
    size_t idx = it->second;
    Frame &frm = pImpl->frames[idx];

    if (frm.dirty) {
        pImpl->flushOnePage(pid, frm);
    }
}

void BufferPool::flushFile(const std::string &file) {
    // TODO pa0

    for (size_t i = 0; i < DEFAULT_NUM_PAGES; i++) {
        Frame &frm = pImpl->frames[i];
        if (frm.inUse && frm.pid.file == file && frm.dirty) {

            pImpl->flushOnePage(frm.pid, frm);
        }
    }
}

}
