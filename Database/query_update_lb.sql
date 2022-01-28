INSERT INTO leaderboard
VALUES(?1, 1, ABS(RANDOM()%10)+15, ?2)
ON CONFLICT(id) DO UPDATE
SET
    num_msg   = num_msg+1,
    xp        = CASE WHEN ?2-timestamp < 60 THEN xp ELSE xp+ABS(RANDOM()%10)+15 END,
    timestamp = CASE WHEN ?2-timestamp < 60 THEN timestamp ELSE ?2 END
WHERE id is ?1
RETURNING xp, timestamp is ?2 as updated;