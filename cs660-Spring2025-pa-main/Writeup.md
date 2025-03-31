
---

## BTreeFile::insertTuple

Handles insertion of a new tuple into the B+ Tree.

- Traverses from the root to the appropriate leaf page using key comparison.
- If the leaf has space, inserts the tuple in sorted order.
- If the leaf is full, splits the leaf page and propagates the split up the tree.
- If the root splits, a new root is created, increasing the tree height.
- Keeps track of the path for upward propagation using a stack (`path`).

This method ensures the B+ Tree remains balanced and maintains sorted order, with all tuples residing in the leaves.

---

## BTreeFile::getTuple

Retrieves a tuple from a specific leaf page using the given iterator (page ID and slot).

- Fetches the page from the buffer pool.
- Constructs a `LeafPage` and uses `getTuple(slot)` to retrieve the tuple.

Efficient tuple access relies on accurate page-slot addressing.

---

## BTreeFile::next

Advances an iterator to the next tuple in logical order.

- If the current slot is not the last in the leaf, increments the slot.
- Otherwise, moves to the `next_leaf` and resets the slot to 0.

This enables in-order scanning across leaves by chaining through leaf-level pointers.

---

## BTreeFile::begin

Finds the first tuple in the B+ Tree.

- Traverses from the root down the leftmost children until reaching a leaf.
- Returns an iterator pointing to the first slot of the first leaf page.

Supports ordered traversal starting from the smallest key.

---

## BTreeFile::end

Returns a sentinel iterator indicating the end of the tuple sequence.

- Represents an invalid or terminal position in tree traversal.

---

## IndexPage::insert

Inserts a key-child pair into the index node while maintaining sorted order.

- Finds the correct position by comparing keys.
- Shifts existing entries to make room.
- Updates the key and child arrays.
- Returns true if the page is full after insertion (to trigger split).

Essential for keeping index pages balanced and ordered.

---

## IndexPage::split

Splits the current index page into two.

- Chooses the median key as the split point.
- Transfers the right half of the keys and children to a new `IndexPage`.
- Updates the sizes of both pages.
- Returns the median key to be inserted into the parent node.

This operation is key to preserving the B+ Tree’s height and balance properties during overflow.

---

## LeafPage::insertTuple

Inserts a tuple into the leaf page while keeping it sorted.

- Scans existing tuples to detect duplicates (based on key).
- Updates if the key already exists.
- Otherwise, finds insertion position and shifts tuples to make space.
- Inserts the new tuple in sorted order.
- Returns whether the page is now full.

Enables efficient range queries and fast lookups by maintaining order.

---

## LeafPage::split

Splits a full leaf page into two.

- Copies the second half of the tuples to a new page.
- Updates `next_leaf` pointer to link the new page.
- Returns the first key of the new page as the split key.

Leaf-level splitting ensures the tree remains balanced and allows continued range scanning.

---

## LeafPage::getTuple

Deserializes and returns the tuple at a specific slot in the leaf.

- Computes offset and uses `TupleDesc` to deserialize the binary data.

Simple and efficient, this function enables direct access to stored data.

---

## LeafPage::clear

Resets the leaf page by clearing its size field.

- Can be used during reinitialization or tree rebuild.
- Optionally leaves `next_leaf` unchanged for continuity.

Useful in testing, rebuilding, or repurposing a page without reallocating it.

---

