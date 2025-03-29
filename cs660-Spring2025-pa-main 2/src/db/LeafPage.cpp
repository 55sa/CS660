#include <cstring>
#include <stdexcept>
#include "db/LeafPage.hpp"
#include "db/Tuple.hpp"

#include <algorithm>
#include <vector>

using namespace db;

LeafPage::LeafPage(Page &page, const TupleDesc &td, size_t key_index)
    : td(td), key_index(key_index) {
    // 页面布局: [LeafPageHeader | tuple data...]
    header = reinterpret_cast<LeafPageHeader*>(page.data());
    data = page.data() + sizeof(LeafPageHeader);
    // 计算页面可容纳的元组数量
    capacity = (DEFAULT_PAGE_SIZE - sizeof(LeafPageHeader)) / td.length();
    if (header->size > capacity) {
        header->size = 0;
        header->next_leaf = 0;
    }
}

/*
 * 插入元组：
 *  - 如果页面已经满（即 header->size >= capacity），则返回 false 表示不能插入，需要在上层触发分裂。
 *  - 否则，按 key 的升序插入新元组：
 *      - 若 key 已存在，则更新对应元组，并返回 false（不触发分裂）。
 *      - 若插入后页面未满，则返回 false；
 *      - 若插入后页面恰好满了，则返回 true，告知上层需要分裂该叶页。
 */
bool LeafPage::insertTuple(const Tuple &t) {
    int key = std::get<int>(t.get_field(key_index));
    // 如果页面已经满，则不能插入
    if (header->size >= capacity)
         return false;
    int pos = 0;
    for (; pos < header->size; pos++) {
         Tuple curr = getTuple(pos);
         int currKey = std::get<int>(curr.get_field(key_index));
         if (key < currKey)
              break;
         if (key == currKey) {
              // 更新已有元组，不触发分裂
              td.serialize(data + pos * td.length(), t);
              return false;
         }
    }
    // 后移数据，为新元组腾出位置
    for (int i = header->size; i > pos; i--) {
         std::memmove(data + i * td.length(),
                      data + (i - 1) * td.length(),
                      td.length());
    }
    td.serialize(data + pos * td.length(), t);
    header->size++;
    // 如果刚好填满，返回 true（告知需要分裂），否则返回 false
    return (header->size == capacity);
}

/*
 * 分裂叶页：
 *  - 将当前页后半部分的元组复制到 new_page 中，
 *  - 更新 new_page 的 header->size 与 next_leaf，
 *  - 当前页的 size 调整为前半部分，
 *  - 返回 new_page 中第一个元组的 key 作为分隔键。
 */
int LeafPage::split(LeafPage &new_page) {
    int total = header->size;
    int mid = total / 2;
    new_page.header->size = total - mid;
    std::memcpy(new_page.data,
                data + mid * td.length(),
                (total - mid) * td.length());
    // 新页继承原页的 next_leaf 指针
    new_page.header->next_leaf = header->next_leaf;
    header->size = mid;
    Tuple firstTuple = new_page.getTuple(0);
    int splitKey = std::get<int>(firstTuple.get_field(key_index));
    return splitKey;
}

/*
 * 反序列化指定 slot 处的元组。
 */
Tuple LeafPage::getTuple(size_t slot) const {
    if (slot >= header->size)
         throw std::runtime_error("Slot not occupied");
    return td.deserialize(data + slot * td.length());
}

/*
 * 清空叶页，用于分裂时先清空页面后重新插入排好序的元组。
 */
void LeafPage::clear() {
    header->size = 0;
    // 注意：next_leaf 可以保留原值（由上层更新链表）或置 0，视具体设计而定
}
