SELECT * FROM (
    SELECT author_id, COUNT(msg_id), SUM(num), MAX(num), ROW_NUMBER() OVER (
        ORDER BY SUM(num) DESC, MAX(num) DESC, COUNT(msg_id), author_id
    ) FROM starboard
    WHERE num >= ?
    GROUP BY author_id
) WHERE author_id IS ?;