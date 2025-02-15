#include <db/Database.hpp>
#include <db/HeapFile.hpp>
#include <db/HeapPage.hpp>
#include <stdexcept>
#include <cstring>

using namespace db;

HeapFile::HeapFile(const std::string &name, const TupleDesc &td)
    : DbFile(name, td) {}

/**
 * @brief Insert a tuple to the database file.
 * @details Insert a tuple to the first available slot of the last page. If the last page is full, create a new page.
 */
// TODO pa1
void HeapFile::insertTuple(const Tuple &t) {
    if (getNumPages() == 0) {
        throw std::runtime_error("HeapFile has 0 pages, which should not happen if DbFile ensures 1+ page.");
    }

    size_t lastPageIdx = getNumPages() - 1;
    Page page;
    readPage(page, lastPageIdx);

    HeapPage hp(page, getTupleDesc());
    if (!hp.insertTuple(t)) {
        // last page is full => create a new page
        size_t newPageIdx = getNumPages();

        // create blank page
        Page blank;
        std::memset(blank.data(), 0, blank.size());
        writePage(blank, newPageIdx);
        numPages++;

        // insert into the new page
        readPage(blank, newPageIdx);
        HeapPage hp2(blank, getTupleDesc());
        if (!hp2.insertTuple(t)) {
            throw std::runtime_error("Could not insert tuple into brand-new page");
        }
        writePage(blank, newPageIdx);
    } else {
        // insertion succeeded in last page
        writePage(page, lastPageIdx);
    }
}

/**
 * @brief Delete a tuple from the database file.
 * @details Delete a tuple from the database file by marking the slot unused.
 */
// TODO pa1
void HeapFile::deleteTuple(const Iterator &it) {
    if (it.page >= getNumPages()) {
        throw std::runtime_error("deleteTuple: invalid iterator page");
    }
    Page page;
    readPage(page, it.page);
    HeapPage hp(page, getTupleDesc());
    hp.deleteTuple(it.slot);
    writePage(page, it.page);
}

/**
 * @brief Get a tuple from the database file.
 * @details Get a tuple from the database file by reading the tuple from the page.
 */
// TODO pa1
Tuple HeapFile::getTuple(const Iterator &it) const {
    if (it.page >= getNumPages()) {
        throw std::runtime_error("getTuple: invalid page");
    }
    Page page;
    readPage(page, it.page);
    HeapPage hp(page, getTupleDesc());
    return hp.getTuple(it.slot);
}

/**
 * @brief Advance the iterator to the next tuple.
 * @details Advance the iterator to the next tuple by moving to the next slot of the page.
 */
// TODO pa1
void HeapFile::next(Iterator &it) const {
    if (it.page >= getNumPages()) {
        // instead of doing: it = end();
        // we directly set it.page / it.slot to the "end" sentinel
        it.page = getNumPages();
        it.slot = 0;
        return;
    }

    Page page;
    readPage(page, it.page);
    HeapPage hp(page, getTupleDesc());
    hp.next(it.slot);
    if (it.slot != hp.end()) {
        // still in the same page
        return;
    }
    // move to next page
    size_t p = it.page + 1;
    while (p < getNumPages()) {
        Page page2;
        readPage(page2, p);
        HeapPage hp2(page2, getTupleDesc());
        size_t s = hp2.begin();
        if (s != hp2.end()) {
            it.page = p;
            it.slot = s;
            return;
        }
        p++;
    }

    // same as "it = end()" but done by direct assignment
    it.page = getNumPages();
    it.slot = 0;
}

/**
 * @brief Get the iterator to the first tuple.
 * @details Get the iterator to the first tuple by finding the first occupied slot.
 */
// TODO pa1
Iterator HeapFile::begin() const {
    for (size_t i = 0; i < getNumPages(); i++) {
        Page page;
        readPage(page, i);
        HeapPage hp(page, getTupleDesc());
        size_t s = hp.begin();
        if (s != hp.end()) {
            return Iterator(*this, i, s);
        }
    }
    return end();
}

/**
 * @brief Get the iterator to the end of the file.
 * @details Return a sentinel value indicating there are no more tuples.
 */
// TODO pa1
Iterator HeapFile::end() const {
    // returning a brand-new Iterator object is not an assignment;
    // no copy assignment is invoked at the call site
    return Iterator(*this, getNumPages(), 0);
}
