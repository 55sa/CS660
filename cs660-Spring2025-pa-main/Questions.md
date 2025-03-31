

## 1. Minimizing Impact of Index Maintenance on Bulk Insertions and Deletions

To reduce index maintenance overhead during bulk operations, strategies like deferred index updates, batching changes, and bulk loading can be used. These methods avoid frequent tree modifications by temporarily buffering operations or building the tree in a more efficient bottom-up manner. Implementation changes include adding support for buffering, deferred execution, and optimized tree construction logic.

## 2. Optimizing BTreeFile for Auto-Incrementing Key Insertions

Since auto-incrementing keys always insert into the rightmost leaf, we can optimize by keeping that leaf node pinned in memory, reducing disk I/O. Pre-allocating pages and optimizing right-side splits also help maintain efficiency. This requires tracking the rightmost leaf, modifying split logic, and using the buffer pool to pin hot nodes.

## 3. Pinning Internal Index Nodes in Memory

Pinning internal index nodes in memory improves performance by avoiding repeated disk reads during lookups and updates. Since these nodes are accessed frequently, keeping them in memory reduces latency and improves concurrency. However, it increases memory usage and requires careful buffer pool management to avoid evicting important data pages.
