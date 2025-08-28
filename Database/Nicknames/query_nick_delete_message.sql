WITH before AS MATERIALIZED(
    SELECT user_id, msg_id FROM nicknames WHERE user_id IS ?
)
UPDATE nicknames
SET msg_id=NULL
WHERE user_id IN (SELECT user_id FROM before)
RETURNING (SELECT msg_id FROM before);