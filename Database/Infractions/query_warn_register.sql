INSERT INTO infractions
VALUES(?1, ?2, 0, ?3)
RETURNING (
    SELECT LAG(timestamp) OVER (ORDER BY timestamp DESC) - timestamp
    FROM infractions
    WHERE user_id IS ?1 AND timed_out IS 0
    LIMIT 1, 1
);