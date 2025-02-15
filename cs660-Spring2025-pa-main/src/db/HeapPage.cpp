#include <db/HeapPage.hpp>
#include <stdexcept>
#include <cstring>

// TODO pa1

using namespace db;

/**
 * @brief Helper function to return the fixed byte size of each type
 */
static inline size_t field_size(type_t t) {
    switch (t) {
        case type_t::INT:    return INT_SIZE;
        case type_t::DOUBLE: return DOUBLE_SIZE;
        case type_t::CHAR:   return CHAR_SIZE;
    }
    throw std::logic_error("Unknown type in field_size()");
}

/**
 * @brief HeapPage constructor
 * @details header and data should point to locations inside the page buffer. Do not allocate extra memory.
 */
// TODO pa1
HeapPage::HeapPage(Page &page, const TupleDesc &td)
    : td(td), capacity(0), header(nullptr), data(nullptr)
{
    // 1) compute tuple_size
    size_t tuple_size = td.length();
    // 2) compute how many tuples fit: formula from assignment
    size_t page_bits = DEFAULT_PAGE_SIZE * 8;
    size_t bits_per_tuple = (tuple_size * 8) + 1;
    if (bits_per_tuple == 0) {
        throw std::runtime_error("Invalid tuple size or formula error");
    }
    capacity = page_bits / bits_per_tuple; // integer division

    // 3) compute header size (capacity bits => capacity/8 bytes, round up)
    size_t header_size = (capacity + 7) / 8;
    // 4) total data area for all tuples
    size_t tuples_data_size = capacity * tuple_size;
    // 5) leftover
    size_t leftover = DEFAULT_PAGE_SIZE - (header_size + tuples_data_size);
    if (header_size + leftover > DEFAULT_PAGE_SIZE) {
        throw std::runtime_error("Invalid leftover calculation.");
    }

    // set pointers
    header = page.data();
    data   = page.data() + header_size + leftover;
}

/**
 * The test code uses the leftmost bit (high bit) in each byte to represent slot0,
 * so we invert the bit index (7 - bit_idx).
 */
static bool is_slot_used(const uint8_t *header, size_t slot) {
    size_t byte_idx = slot / 8;
    size_t bit_idx = slot % 8;
    uint8_t mask = (1 << (7 - bit_idx));
    return (header[byte_idx] & mask) != 0;
}

static void set_slot_used(uint8_t *header, size_t slot, bool used) {
    size_t byte_idx = slot / 8;
    size_t bit_idx = slot % 8;
    uint8_t mask = (1 << (7 - bit_idx));
    if (used) {
        header[byte_idx] |= mask;
    } else {
        header[byte_idx] &= ~mask;
    }
}

/**
 * @brief Get the first occupied slot of the page
 * @return The first occupied slot of the page
 */
// TODO pa1
size_t HeapPage::begin() const {
    for (size_t i = 0; i < capacity; i++) {
        if (is_slot_used(header, i)) {
            return i;
        }
    }
    return end();
}

/**
 * @brief Get the end of the page (capacity)
 * @return capacity
 */
// TODO pa1
size_t HeapPage::end() const {
    return capacity;
}

/**
 * @brief Insert a tuple into the page
 * @param t The tuple to be inserted
 * @return True if successful, false otherwise (page full)
 */
// TODO pa1
bool HeapPage::insertTuple(const Tuple &t) {
    if (!td.compatible(t)) {
        throw std::runtime_error("HeapPage::insertTuple: incompatible tuple");
    }
    for (size_t i = 0; i < capacity; i++) {
        if (!is_slot_used(header, i)) {
            size_t offset = i * td.length();
            td.serialize(data + offset, t);
            set_slot_used(header, i, true);
            return true;
        }
    }
    return false;
}

/**
 * @brief Delete a tuple from the page
 * @param slot The slot of the tuple to be deleted
 * @details The test expects an exception if slot is invalid or already empty
 */
// TODO pa1
void HeapPage::deleteTuple(size_t slot) {
    if (slot >= capacity) {
        throw std::runtime_error("deleteTuple: invalid slot");
    }
    if (!is_slot_used(header, slot)) {
        throw std::runtime_error("deleteTuple: slot is already empty");
    }
    set_slot_used(header, slot, false);

    // optionally clear data
    size_t offset = slot * td.length();
    std::memset(data + offset, 0, td.length());
}

/**
 * @brief Get a tuple at the specified slot
 * @param slot The slot
 * @return The tuple
 */
// TODO pa1
Tuple HeapPage::getTuple(size_t slot) const {
    if (slot >= capacity) {
        throw std::runtime_error("getTuple: invalid slot");
    }
    if (!is_slot_used(header, slot)) {
        throw std::runtime_error("getTuple: slot is empty");
    }
    size_t offset = slot * td.length();
    return td.deserialize(data + offset);
}

/**
 * @brief Advance the slot to the next occupied slot
 * @param slot The slot
 */
// TODO pa1
void HeapPage::next(size_t &slot) const {
    while (++slot < capacity) {
        if (is_slot_used(header, slot)) {
            return;
        }
    }
    slot = end();
}

/**
 * @brief Check if the slot is empty
 * @param slot The slot
 * @return True if empty, false otherwise
 */
// TODO pa1
bool HeapPage::empty(size_t slot) const {
    if (slot >= capacity) {
        return true;
    }
    return !is_slot_used(header, slot);
}
