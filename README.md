# log-analyzer



## Architecture Overview
The system is divided into three layers:
1. Parsing & Validation
2. In-Memory Storage
3. Analytics & Querying

This separation improves readability, testability, and performance reasoning.

## Data Structures Used
- unordered_map<string, vector<LogEntry>>
  - Groups logs by service name
  - O(1) average lookup
- vector<LogEntry>
  - Cache-friendly
  - Sorted once after ingestion
- unordered_set<string>
  - Deduplicates request_id in O(1) average time

## Time Complexity
- Parsing: O(N)
- Deduplication: O(1) average per insert
- Sorting per service: O(K log K)
- Analytics: O(K)
- Time-window query: O(log K + M)

## Trade-offs & Limitations
- Stores all logs in memory
- Deduplication set grows with log volume
- Sorting happens after full ingestion

## Improvements With More Time
- Streaming analytics
- Memory pooling
- Sharded storage for very large services
- Persistent on-disk index
