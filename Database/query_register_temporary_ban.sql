INSERT INTO tempbans VALUES (?1, ?2) ON CONFLICT DO UPDATE SET expires_at=?2 WHERE user_id IS ?1;