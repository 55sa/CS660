#include <cstring>
 #include <stdexcept>
 #include <vector>
 #include "db/BTreeFile.hpp"
 #include "db/Database.hpp"
 #include "db/IndexPage.hpp"
 #include "db/LeafPage.hpp"

 using namespace db;

 BTreeFile::BTreeFile(const std::string &name, const TupleDesc &td, size_t key_index)
     : DbFile(name, td), key_index(key_index) {}

 void BTreeFile::insertTuple(const Tuple &t) {
     int key = std::get<int>(t.get_field(key_index));
     BufferPool &bufferPool = getDatabase().getBufferPool();
     PageId rootPid{name, 0};
     Page &rootPage = bufferPool.getPage(rootPid);
     IndexPage rootIndex(rootPage);

     // 如果根页为空（树为空），则创建第一个叶页
     if (rootIndex.header->size == 0) {
         PageId leafPid{name, getNumPages()};
         numPages++;
         Page &leafPage = bufferPool.getPage(leafPid);
         LeafPage leaf(leafPage, td, key_index);
         // 调用插入，返回值表示是否需要分裂
         bool needsSplit = leaf.insertTuple(t);
         bufferPool.markDirty(leafPid);
         rootIndex.header->index_children = false;  // 表示叶页
         rootIndex.header->size = 0; // 没有分隔键
         rootIndex.children[0] = leafPid.page;
         bufferPool.markDirty(rootPid);
         // 对于第一个叶页，通常不会满，因此不需要分裂
         if (!needsSplit)
             return;
         // 如果意外满了（极不可能），则后续分裂逻辑会处理
     }

     // 记录从根到叶页的路径
     std::vector<PageId> parentStack;
     PageId currPid = rootPid;
     bool atIndex = true;
     while (true) {
         if (atIndex) {
             Page &page = bufferPool.getPage(currPid);
             IndexPage ip(page);
             int pos = 0;
             while (pos < ip.header->size && key >= ip.keys[pos])
                 pos++;
             parentStack.push_back(currPid);
             if (!ip.header->index_children) {
                 currPid.page = ip.children[pos];
                 atIndex = false;
                 break;
             } else {
                 currPid.page = ip.children[pos];
             }
         } else {
             break;
         }
     }

     // 尝试在叶页中插入
     Page &leafPg = bufferPool.getPage(currPid);
     LeafPage leaf(leafPg, td, key_index);
     bool needsSplit = leaf.insertTuple(t);
     bufferPool.markDirty(currPid);
     if (!needsSplit) {
         // 如果插入后页面未满，则直接返回
         return;
     }

     // 否则，执行分裂逻辑……
     // （后续分裂代码保持不变）
     // 1. 把当前叶页所有元组复制到临时 vector 中，再加入新元组，排序后重分配两半
     std::vector<Tuple> allTuples;
     for (size_t i = 0; i < leaf.header->size; i++) {
         allTuples.push_back(leaf.getTuple(i));
     }
     allTuples.push_back(t);
     std::sort(allTuples.begin(), allTuples.end(), [this](const Tuple &a, const Tuple &b) {
         return std::get<int>(a.get_field(key_index)) < std::get<int>(b.get_field(key_index));
     });

     // 2. 清空当前叶页，并将前半部分重新插入
     leaf.clear();
     size_t splitIndex = allTuples.size() / 2;
     for (size_t i = 0; i < splitIndex; i++) {
         if (!leaf.insertTuple(allTuples[i]))
             throw std::runtime_error("Reinsertion failed in left leaf");
     }

     // 3. 分配新叶页，将后半部分插入
     PageId newLeafPid{name, getNumPages()};
     numPages++;
     Page &newLeafPg = bufferPool.getPage(newLeafPid);
     LeafPage newLeaf(newLeafPg, td, key_index);
     newLeaf.clear();
     for (size_t i = splitIndex; i < allTuples.size(); i++) {
         if (!newLeaf.insertTuple(allTuples[i]))
             throw std::runtime_error("Reinsertion failed in right leaf");
     }

     // 4. 更新叶页链表
     newLeaf.header->next_leaf = leaf.header->next_leaf;
     leaf.header->next_leaf = newLeafPid.page;
     bufferPool.markDirty(currPid);
     bufferPool.markDirty(newLeafPid);

     // 5. 设定分裂关键字为新叶页中第一个元组的 key
     int splitKey = std::get<int>(newLeaf.getTuple(0).get_field(key_index));

     // 6. 向上级传播分裂（省略细节，此处不再赘述）
     int separator = splitKey;
     size_t childPage = newLeafPid.page;
     while (!parentStack.empty()) {
         PageId parentPid = parentStack.back();
         parentStack.pop_back();
         Page &parentPg = bufferPool.getPage(parentPid);
         IndexPage parentIdx(parentPg);
         bool needSplit = parentIdx.insert(separator, childPage);
         bufferPool.markDirty(parentPid);
         if (!needSplit)
             return;
         PageId newIndexPid{name, getNumPages()};
         numPages++;
         Page &newIndexPg = bufferPool.getPage(newIndexPid);
         IndexPage newIndex(newIndexPg);
         separator = parentIdx.split(newIndex);
         bufferPool.markDirty(newIndexPid);
         childPage = newIndexPid.page;
     }

     // 根分裂：按照要求第 0 页始终为根
     PageId leftChildPid{name, getNumPages()};
     numPages++;
     Page &leftChildPg = bufferPool.getPage(leftChildPid);
     IndexPage leftChild(leftChildPg);
     leftChild = rootIndex; // 复制原根
     rootIndex.header->size = 1;
     rootIndex.header->index_children = true;
     rootIndex.keys[0] = separator;
     rootIndex.children[0] = leftChildPid.page;
     rootIndex.children[1] = childPage;
     bufferPool.markDirty(rootPid);
 }


 Tuple BTreeFile::getTuple(const Iterator &it) const {
     BufferPool &bufferPool = getDatabase().getBufferPool();
     PageId pid{name, it.page};
     Page &page = bufferPool.getPage(pid);
     LeafPage leaf(page, td, key_index);
     return leaf.getTuple(it.slot);
 }

 void BTreeFile::deleteTuple(const Iterator &it) {
     throw std::runtime_error("deleteTuple not implemented for BTreeFile");
 }


 void BTreeFile::next(Iterator &it) const {
     BufferPool &bufferPool = getDatabase().getBufferPool();
     PageId pid{name, it.page};
     Page &page = bufferPool.getPage(pid);
     LeafPage leaf(page, td, key_index);
     // If there is another tuple in the current leaf, just advance the slot.
     if (it.slot + 1 < leaf.header->size) {
         it.slot++;
         return;
     }
     // Otherwise, follow the next_leaf pointer.
     if (leaf.header->next_leaf != 0) {
         it.page = leaf.header->next_leaf;
         it.slot = 0;
     } else {
         // End of file indicated by page number equal to numPages.
         it.page = numPages;
         it.slot = 0;
     }
 }

 Iterator BTreeFile::begin() const {
     // 如果文件只有1页（即只有根页），则认为为空
     if (getNumPages() <= 1)
         return end();

     BufferPool &bufferPool = getDatabase().getBufferPool();
     // 从根页（页号0）开始
     PageId currPid{name, 0};
     Page &rootPage = bufferPool.getPage(currPid);
     IndexPage rootIndex(rootPage);

     // 如果根页为空但文件页数 > 1，则必然有子页
     if (rootIndex.header->size == 0) {
         currPid.page = rootIndex.children[0];
     } else {
         // 否则沿着最左侧分支遍历
         while (true) {
             Page &currPage = bufferPool.getPage(currPid);
             IndexPage ip(currPage);
             if (!ip.header->index_children) {
                 // 到达内部节点，其子节点就是叶页
                 currPid.page = ip.children[0];
                 break;
             }
             currPid.page = ip.children[0];
         }
     }

     // currPid 应该指向一个叶页
     Page &leafPage = bufferPool.getPage(currPid);
     LeafPage leaf(leafPage, td, key_index);
     if (leaf.header->size > 0)
         return Iterator(*this, currPid.page, 0);
     else
         return end();
 }


 Iterator BTreeFile::end() const {
     return Iterator(*this, numPages, 0);
 }