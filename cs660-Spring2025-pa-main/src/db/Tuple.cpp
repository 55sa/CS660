#include <db/Tuple.hpp>
#include <stdexcept>
#include <cstring>
#include <algorithm> // for std::min
#include <unordered_set>

using namespace db;

/**
 * @brief Construct a new Tuple object
 */
Tuple::Tuple(const std::vector<field_t> &fields) : fields(fields) {}

/**
 * @brief Returns the type of the i-th field
 */
type_t Tuple::field_type(size_t i) const {
    const field_t &field = fields.at(i);
    if (std::holds_alternative<int>(field)) {
        return type_t::INT;
    }
    if (std::holds_alternative<double>(field)) {
        return type_t::DOUBLE;
    }
    if (std::holds_alternative<std::string>(field)) {
        return type_t::CHAR;
    }
    throw std::logic_error("Unknown field type");
}

/**
 * @brief Returns the number of fields
 */
size_t Tuple::size() const {
    return fields.size();
}

/**
 * @brief Returns the field at the given index
 */
const field_t &Tuple::get_field(size_t i) const {
    return fields.at(i);
}

// ---------------------------------------------------------------------------
// Helper function for TupleDesc
// ---------------------------------------------------------------------------
static inline size_t fixedTypeSize(type_t t) {
    switch (t) {
        case type_t::INT:    return INT_SIZE;
        case type_t::DOUBLE: return DOUBLE_SIZE;
        case type_t::CHAR:   return CHAR_SIZE;
    }
    throw std::logic_error("Unknown type in fixedTypeSize()");
}

// ---------------------------------------------------------------------------
// Implementation for TupleDesc
// ---------------------------------------------------------------------------

/**
 * @brief Construct a new Tuple Desc object
 * @details Construct a new TupleDesc object with the provided types and names
 */
TupleDesc::TupleDesc(const std::vector<type_t> &types, const std::vector<std::string> &names) {
    if (types.size() != names.size()) {
        throw std::logic_error("TupleDesc: types and names have different lengths");
    }
    std::unordered_set<std::string> used;
    used.reserve(types.size());
    size_t offset = 0;
    for (size_t i = 0; i < types.size(); i++) {
        // throw if repeated
        if (!used.insert(names[i]).second) {
            throw std::logic_error("TupleDesc: repeated field name: " + names[i]);
        }
        field_types.push_back(types[i]);
        field_names.push_back(names[i]);
        field_offsets.push_back(offset);
        offset += fixedTypeSize(types[i]);
    }
    total_length = offset;
}


/**
 * @brief Check if the provided Tuple is compatible with this TupleDesc
 */
bool TupleDesc::compatible(const Tuple &tuple) const {
    // TODO pa1
    if (tuple.size() != field_types.size()) {
        return false;
    }
    for (size_t i = 0; i < field_types.size(); i++) {
        if (tuple.field_type(i) != field_types[i]) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Get offset of the field
 */
size_t TupleDesc::offset_of(const size_t &index) const {
    // TODO pa1
    if (index >= field_types.size()) {
        throw std::runtime_error("TupleDesc::offset_of: invalid field index");
    }
    return field_offsets[index];
}

/**
 * @brief Get the index of the field
 */
size_t TupleDesc::index_of(const std::string &name) const {
    // TODO pa1
    for (size_t i = 0; i < field_names.size(); i++) {
        if (field_names[i] == name) {
            return i;
        }
    }
    throw std::runtime_error("TupleDesc::index_of: no field named '" + name + "'");
}

/**
 * @brief Get the number of fields in the TupleDesc
 */
size_t TupleDesc::size() const {
    // TODO pa1
    return field_types.size();
}

/**
 * @brief Get the length of the TupleDesc
 */
size_t TupleDesc::length() const {
    // TODO pa1
    return total_length;
}

/**
 * @brief Deserialize a Tuple
 */
Tuple TupleDesc::deserialize(const uint8_t *data) const {
    // TODO pa1
    std::vector<field_t> fields;
    fields.reserve(field_types.size());
    for (size_t i = 0; i < field_types.size(); i++) {
        size_t off = field_offsets[i];
        switch (field_types[i]) {
            case type_t::INT: {
                int val;
                std::memcpy(&val, data + off, INT_SIZE);
                fields.emplace_back(val);
                break;
            }
            case type_t::DOUBLE: {
                double d;
                std::memcpy(&d, data + off, DOUBLE_SIZE);
                fields.emplace_back(d);
                break;
            }
            case type_t::CHAR: {
                char buffer[CHAR_SIZE + 1];
                std::memcpy(buffer, data + off, CHAR_SIZE);
                buffer[CHAR_SIZE] = '\0';
                fields.emplace_back(std::string(buffer));
                break;
            }
        }
    }
    return Tuple(fields);
}

/**
 * @brief Serialize a Tuple
 */
void TupleDesc::serialize(uint8_t *data, const Tuple &t) const {
    // TODO pa1
    if (!compatible(t)) {
        throw std::runtime_error("TupleDesc::serialize: incompatible tuple");
    }
    for (size_t i = 0; i < field_types.size(); i++) {
        size_t off = field_offsets[i];
        const field_t &f = t.get_field(i);

        switch (field_types[i]) {
            case type_t::INT: {
                int val = std::get<int>(f);
                std::memcpy(data + off, &val, INT_SIZE);
                break;
            }
            case type_t::DOUBLE: {
                double d = std::get<double>(f);
                std::memcpy(data + off, &d, DOUBLE_SIZE);
                break;
            }
            case type_t::CHAR: {
                const std::string &s = std::get<std::string>(f);
                char buffer[CHAR_SIZE];
                std::memset(buffer, 0, CHAR_SIZE);
                std::memcpy(buffer, s.data(), std::min(s.size(), (size_t)CHAR_SIZE));
                std::memcpy(data + off, buffer, CHAR_SIZE);
                break;
            }
        }
    }
}

/**
 * @brief Merge two TupleDescs
 */
TupleDesc TupleDesc::merge(const TupleDesc &td1, const TupleDesc &td2) {
    std::vector<type_t> newTypes;
    std::vector<std::string> newNames;
    newTypes.reserve(td1.field_types.size() + td2.field_types.size());
    newNames.reserve(td1.field_names.size() + td2.field_names.size());

    // copy td1
    for (size_t i = 0; i < td1.field_types.size(); i++) {
        newTypes.push_back(td1.field_types[i]);
        newNames.push_back(td1.field_names[i]);
    }
    // copy td2
    for (size_t i = 0; i < td2.field_types.size(); i++) {
        newTypes.push_back(td2.field_types[i]);
        newNames.push_back(td2.field_names[i]);
    }
    // check for duplicates
    std::unordered_set<std::string> used;
    used.reserve(newNames.size());
    for (auto &name : newNames) {
        if (!used.insert(name).second) {
            throw std::logic_error("TupleDesc::merge: repeated field name: " + name);
        }
    }
    return TupleDesc(newTypes, newNames);
}
