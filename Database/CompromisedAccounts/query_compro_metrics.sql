SELECT COUNT(DISTINCT channel_id), MAX(id >> 22) - MIN(id >> 22)
FROM messages
WHERE author_id IS ? AND content IS ? AND (? >> 22) - (id >> 22) < 60000;