 **Q1**  
 Deleting tuples from a HeapFile might lead to having empty pages. 
 How can we avoid having empty pages in a file? What is the cost of 
 the solution you propose?

- When a page becomes completely empty after tuple deletions, 
instead of leaving it in place, swap it with the last page in the file.
- Cost and Trade-offs:
  1. I/O Overhead: Swapping involves reading and writing an entire page to disk. 
  If deletions are frequent, this may introduce noticeable I/O overhead.
  2. Pointer and Metadata Updates: Any external references, indices, or metadata 
  that point to pages may need to be updated to reflect the new page positions. This can add complexity and further overhead.
  3. Complexity in Concurrency: In a multi-threaded or concurrent environment, ensuring consistency during a page swap might require additional locking or coordination.

**Q2**
n this assignment we have fixed size fields. How can we support variable size fields (e.g. VARCHAR)?
 - like the IPv4 packet header, which has field of "Total Length", it can indicate the length of the packet. We can add a prefix to the tuple to indicate the length of the tuple.
This change is to let the tuple “carry” its own length so that variable‐length fields can be stored and later parsed correctly. Thus, To support variable‐length fields, 
we need modify the serialization/deserialization routines to write and read a length prefix.
 - In TupleDesc::serialize: Instead of writing field data at predetermined fixed offsets, first compute the actual length of the tuple.
Then, write a prefix (for example, a 4‑byte integer) with that total length into the output buffer before writing the individual field values.
 - In TupleDesc::deserialize: It needs first reads the length prefix from the input data. That prefix tells you how many bytes form the entire tuple.
Then, when deserializing, for variable‐length fields, read the length and use that value to correctly extract the field data from the byte stream.