INSERT INTO infractions
VALUES(?1, ?2, 0, ?3)
RETURNING (
    SELECT COUNT(*)
    FROM infractions
    WHERE user_id IS ?1 AND timed_out IS 0 AND ?2 - timestamp < ?4
);