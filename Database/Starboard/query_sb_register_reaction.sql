INSERT INTO starboard(msg_id, author_id, num) VALUES(?1, ?2, 1)
ON CONFLICT DO UPDATE SET num=num+1 WHERE msg_id IS ?1
RETURNING sb_msg_id, num;