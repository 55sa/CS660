#include <db/LeafPage.hpp>
#include <stdexcept>

using namespace db;

LeafPage::LeafPage(Page &page, const TupleDesc &td, size_t key_index) : td(td), key_index(key_index) {
    // TODO pa2
}

bool LeafPage::insertTuple(const Tuple &t) {
    // TODO pa2
}

int LeafPage::split(LeafPage &new_page) {
    // TODO pa2
}

Tuple LeafPage::getTuple(size_t slot) const {
    // TODO pa2
}
