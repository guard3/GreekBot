INSERT INTO welcoming
VALUES(?1, NULL, NULL, ?2)
ON CONFLICT DO UPDATE
SET msg_id=NULL, old_id=msg_id, joined_at=?2
RETURNING ABS(old_id);