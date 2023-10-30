SELECT author_id, COUNT(msg_id) as num_msg, SUM(num) as num_reactions, MAX(num) as max_per_msg
FROM starboard
WHERE num >= ?1
GROUP BY author_id
ORDER BY num_reactions DESC, max_per_msg DESC, num_msg, author_id
LIMIT 10;