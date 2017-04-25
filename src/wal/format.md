
Format of the encoded log entry:

```
LogEntry  := LogHeader Data
LogHeader := Crc32 Length

Crc32  -> 4 bytes, checksum of Data field
Length -> 4 bytes, length of data
Data   -> bytes,   byte-encoded yaraft.pb.Entry
```

Each segment composes of a series of log entries:

```
Segment := Header LogEntry*
Header  := Committed

Committed -> bool, committed marker of a log segment, if it is true,
all entries in the segment must have been committed.
```