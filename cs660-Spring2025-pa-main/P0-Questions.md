# P0 Questions

## Question 1: Strategy to Reduce Dirty Page Evictions
The main challenge here is balancing memory efficiency with disk I/O. Based on my understanding, we could implement a smarter eviction strategy that minimizes expensive disk writes. Here's what I propose:

First, we could add a background process that monitors the buffer pool usage. When it reaches around 80% capacity, it starts writing back some dirty pages to disk - focusing on those that haven't been accessed recently. This way, when we actually need to evict pages, we're more likely to find clean pages available.

We could also split our LRU implementation into two queues - one for dirty pages and one for clean pages. When looking for pages to evict, we'd prefer the clean queue first. This approach is a bit more complex to implement, but it could significantly reduce the number of forced write-backs during eviction.

## Question 2: Implications of Not Flushing Pages on File Removal
This is actually a tricky issue that could cause some serious problems. Think about what happens when you delete a file but keep its pages in the buffer pool:

The most obvious issue is data loss - if those pages were modified (dirty), those changes are just gone. This breaks one of the fundamental promises of a database: durability.

But there's an even more subtle problem. What if someone creates a new file with the same name? The buffer pool still has pages it thinks belong to that file name, but they're actually from the old, deleted file. This could lead to returning completely wrong data to queries.

Plus, you're basically leaking memory - keeping around pages that can never be used again. Not great for performance.

## Question 3: Discarding Pages Without Writing Back
There are actually several legitimate cases where we'd want to just discard pages rather than writing them back:

The most obvious case is for read-only operations - if we haven't modified the page, there's nothing to write back. Similarly, for temporary tables, there's no need to persist any changes.

A more interesting case is during transaction rollback. If a transaction modified some pages and then needs to be rolled back, we can just discard its dirty pages instead of writing them - the original data is still safe on disk.

Also, after a system crash, any pages left in the buffer pool might be in an inconsistent state, so we're better off discarding them and starting fresh.