#include <cstring>
 #include <stdexcept>
 #include "db/IndexPage.hpp"
 #include "db/types.hpp"

 using namespace db;

 IndexPage::IndexPage(Page &page) {
     // The page layout: [IndexPageHeader | keys[] | children[]]
     header = reinterpret_cast<IndexPageHeader*>(page.data());
     // Compute capacity based on available space.
     capacity = (DEFAULT_PAGE_SIZE - sizeof(IndexPageHeader) - sizeof(size_t)) /
                (sizeof(int) + sizeof(size_t));
     keys = reinterpret_cast<int*>(page.data() + sizeof(IndexPageHeader));
     children = reinterpret_cast<size_t*>(page.data() + sizeof(IndexPageHeader) + capacity * sizeof(int));
     // If header data is uninitialized (or corrupt), reinitialize.
     if (header->size > capacity)
         header->size = 0;
 }

 bool IndexPage::insert(int key, size_t child) {
     // If page is already full, signal that a split is needed.
     if (header->size >= capacity)
         return true;
     int pos = 0;
     // Find insertion point so that keys remain in sorted order.
     while (pos < header->size && keys[pos] < key)
         pos++;
     // Shift keys and children right to make room.
     for (int i = header->size; i > pos; i--) {
         keys[i] = keys[i-1];
     }
     for (int i = header->size + 1; i > pos + 1; i--) {
         children[i] = children[i-1];
     }
     keys[pos] = key;
     children[pos+1] = child;
     header->size++;
     // Signal that the page is full if insertion causes size to equal capacity.
     return header->size == capacity;
 }

 int IndexPage::split(IndexPage &new_page) {
     int n = header->size;
     int median_index = n / 2;
     int median_key = keys[median_index];
     int new_count = n - (median_index + 1);
     new_page.header->size = new_count;
     // Copy keys and children from the right half to the new page.
     for (int i = 0; i < new_count; i++) {
         new_page.keys[i] = keys[median_index + 1 + i];
     }
     for (int i = 0; i < new_count + 1; i++) {
         new_page.children[i] = children[median_index + 1 + i];
     }
     // Adjust the current page’s size.
     header->size = median_index;
     return median_key;
 }