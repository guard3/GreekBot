INSERT INTO nicknames(user_id, msg_id) VALUES(?1, ?2)
ON CONFLICT DO UPDATE SET msg_id=?2
WHERE user_id IS ?1;