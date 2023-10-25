INSERT INTO starboard VALUES(?1, ?2, NULL, 1)
ON CONFLICT DO UPDATE SET num=num+1 WHERE msg_id IS ?1
RETURNING sb_msg_id, num;