INSERT INTO nicknames(user_id) VALUES(?1)
ON CONFLICT DO UPDATE SET msg_id=NULL WHERE user_id IS ?1;