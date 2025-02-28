# HeapPage

## 1. HeapPage Constructor

### Responsibilities:
- **Tuple Size Calculation:**  
  - Compute the total size of a tuple using the tuple descriptor (`td.length()`).

- **Capacity Calculation:**  
  - Determine the total number of bits available in the page:  
    `page_bits = DEFAULT_PAGE_SIZE * 8`.
  - Calculate bits required per tuple:  
    `bits_per_tuple = (tuple_size * 8) + 1` (the extra bit is for the header).
  - Use integer division to compute the maximum number of tuples.

- **Header Size Computation:**  
  - Each tuple slot is tracked using one bit.
  - The header size in bytes is given by:  
    `header_size = (capacity + 7) / 8` (rounding up to ensure full bytes).

- **Data Area and Leftover Computation:**  
  - Compute the total data area size for tuples:  
    `tuples_data_size = capacity * tuple_size`.
  - Determine any leftover space by subtracting the combined size of the header and the tuple data area from the total page size.

- **Pointer Initialization:**  
  - **Header Pointer:** Set to the beginning of the page buffer.
  - **Data Pointer:** Set after the header and any leftover space, ensuring no overlap between header and data.

---

## 2. Iterator Functions

### `begin()`:

  - Iterates from `0` to `capacity - 1`.
  - Returns the first index where `is_slot_used` is true.
  - Returns `end()` if no occupied slot is found.

### `end()`:
 
  - Represents one past the last valid slot, equivalent to `capacity`.

---

## 3. Tuple Insertion (`insertTuple`)

- **Compatibility Check:**  
  - Verifies that the tuple is compatible with the page’s tuple descriptor.
  - Throws an exception if incompatible.

- **Empty Slot Search:**  
  - Iterates through the slots to find the first an empty one.
  
- **Tuple Serialization:**  
  - Calculates the data area offset based on the slot index (`offset = i * td.length()`).
  - Serializes the tuple into the computed offset.

- **Slot Update:**  
  - Marks the slot as occupied in the header.
  - Returns `true` if insertion is successful; `false` if the page is full.

---

## 4. Tuple Deletion (`deleteTuple`)

- **Validation:**  
  - Checks that the slot index is within bounds.
  - Throws an exception if the slot is already empty.

- **Update Operations:**  
  - Marks the slot as free in the header.
  - Optionally clears the tuple data using `std::memset`.

---

## 5. Tuple Retrieval (`getTuple`)

- **Validation:**  
  - Ensures that the slot index is valid and that the slot is occupied.
  - Throws exceptions for invalid or empty slots.

- **Deserialization:**  
  - Computes the offset for the tuple.
  - Uses the tuple descriptor’s `deserialize` function to reconstruct the tuple from the data area.

---

## 6. Advancing to the Next Occupied Slot (`next`)

- **Implementation:**  
  - Increments the slot index in a loop.
  - Stops when an occupied slot is found or when the end of the page is reached.
  - Sets the slot index to `end()` if no further occupied slot exists.

---

## 7. Checking if a Slot is Empty (`empty`)

- **Validation:**  
  - If the slot index is out of range (greater than or equal to `capacity`), it is considered empty.
- **Check:**  
  - Returns the negation of `is_slot_used` for the given slot.

---

# DbFile

## 1. Opening the File and Initializing `numPages`

- **File Opening:**
  - The constructor uses the system call `open()` with the flags `O_RDWR | O_CREAT`. This ensures the file is opened for reading and writing, and if it does not exist, it is created.
  - A permission mode of `0666` is provided, allowing read and write permissions for the owner, group, and others.
  - If the call to `open()` fails (i.e., returns a negative file descriptor), a runtime exception is thrown with a descriptive error message that includes the `errno` value.

- **Determining the File Size:**
  - The `fstat()` system call is used to retrieve metadata about the file, specifically its size (`st.st_size`).
  - If `fstat()` fails, the file descriptor is closed and a runtime exception is thrown.

- **Initializing `numPages`:**
  - If the file size is zero (indicating an empty file), the file is extended to one default page size using `ftruncate()`, and `numPages` is set to 1.
  - If the file contains data, the implementation verifies that the file size is a multiple of `DEFAULT_PAGE_SIZE`. If it is not, an exception is thrown.
  - Otherwise, `numPages` is computed as the file size divided by `DEFAULT_PAGE_SIZE`. A safeguard ensures that `numPages` is at least 1.

---

## 2. Closing the File

- **Destructor Handling:**
  - The destructor checks if the file descriptor (`fd`) is valid (i.e., `fd >= 0`).
  - If valid, the file descriptor is closed using the `close()` system call.

---

## 3. Reading a Page

- **Offset Calculation:**
  - The byte offset is computed by multiplying the page number (`id`) by `DEFAULT_PAGE_SIZE`.

- **Reading with `pread()`:**
  - The `pread()` system call reads data from the file into the page buffer at the calculated offset.
  - `pread()` is used because it does not change the file descriptor’s offset, making it thread-safe and allowing for concurrent read operations.

- **Error Checking:**
  - If `pread()` returns a negative value, an exception is thrown with a message that includes the `errno` value.
  - An exception is also thrown if the number of bytes read does not equal `DEFAULT_PAGE_SIZE`, indicating an incomplete read.

---

## 4. Writing a Page

- **Offset Calculation:**
  - Similar to reading, the offset is calculated by multiplying the page number (`id`) by `DEFAULT_PAGE_SIZE`.

- **Writing with `pwrite()`:**
  - The `pwrite()` system call writes data from the page buffer to the file at the calculated offset.
  - `pwrite()` maintains the current file offset, ensuring that write operations are thread-safe.

- **Error Checking:**
  - If `pwrite()` returns a negative value, an exception is thrown with details that include the `errno` value.
  - An exception is also thrown if the number of bytes written does not match `DEFAULT_PAGE_SIZE`, indicating an incomplete write.

---

# Tuple

## 1. `TupleDesc::compatible`

- **Field Count Check:**  
  The method checks if the number of fields in the tuple matches the number of types in the descriptor.
- **Field Type Check:**  
  It iterates over each field and compares the tuple's field type with the corresponding type stored in the descriptor.
- **Outcome:**  
  Returns `true` if both the count and types match; otherwise, returns `false`.

---

## 2. `TupleDesc::offset_of`

- **Index Validation:**  
  Validates the index to ensure it is within bounds. If not, a `std::runtime_error` is thrown.
- **Offset Retrieval:**  
  Returns the precomputed offset from the `field_offsets` vector, which was calculated during object construction.

---

## 3. `TupleDesc::index_of`

- **Iteration:**  
  The function loops through the `field_names` vector to find a match.
- **Error Handling:**  
  If the field name is not found, it throws a `std::runtime_error` with an appropriate error message.

---

## 4. `TupleDesc::size`

- **Simple Retrieval:**  
  It returns the size of the `field_types` vector, which represents the total number of fields.

---

## 5. `TupleDesc::length`

- **Precomputed Total:**  
  The `total_length` variable, calculated during construction by summing up the sizes of each field type, is returned.

---

## 6. `TupleDesc::deserialize`

- **Field Iteration:**  
  Iterates over each field defined in the descriptor.
- **Primitive Types:**  
  - For `INT`, copies `INT_SIZE` bytes into an `int` variable.
  - For `DOUBLE`, copies `DOUBLE_SIZE` bytes into a `double` variable.
- **CHAR Type Handling:**  
  - Copies `CHAR_SIZE` bytes into a buffer.
  - Ensures the buffer is null-terminated before converting it into a `std::string`.
- **Tuple Assembly:**  
  Constructs a vector of fields which is then used to create and return a new `Tuple`.

---

## 7. `TupleDesc::serialize`

- **Compatibility Check:**  
  Uses the `compatible` method to ensure that the tuple's schema matches the descriptor. Throws an exception if they are incompatible.
- **Field Serialization:**  
  - For `INT` and `DOUBLE`, uses `std::memcpy` to write the data at the correct offset.
  - For `CHAR` fields, creates a temporary buffer, zeroes it out, copies the string data up to `CHAR_SIZE`, and then writes the buffer to the output array.

---

## 8. `TupleDesc::merge`

- **Combining Fields:**  
  New vectors for types and names are created by first inserting all fields from the first descriptor and then from the second.
- **Duplicate Check:**  
  Uses an `unordered_set` to ensure that field names do not duplicate across the merged schema. Throws a `std::logic_error` if a duplicate is detected.
- **Creation of Merged Descriptor:**  
  Constructs and returns a new `TupleDesc` object based on the combined types and names.

---
