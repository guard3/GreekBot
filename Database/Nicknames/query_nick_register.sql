WITH old AS (
    SELECT msg_id FROM nicknames WHERE user_id IS ?1
)
INSERT INTO nicknames(user_id) VALUES(?1)
ON CONFLICT DO UPDATE SET msg_id=NULL WHERE user_id IS ?1
RETURNING (SELECT msg_id FROM old);