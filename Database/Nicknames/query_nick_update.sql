INSERT INTO nicknames(user_id, nick) VALUES(?1, ?2)
ON CONFLICT DO UPDATE SET nick=?2
WHERE user_id IS ?1;