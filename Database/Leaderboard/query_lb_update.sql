INSERT INTO leaderboard VALUES(?1, 1, ABS(RANDOM()%10)+15, ?2)
ON CONFLICT(id) DO UPDATE SET num_msg=num_msg+1, xp=xp+ABS(RANDOM()%10)+15, timestamp=?2
WHERE id IS ?1 AND ?2-timestamp>=60
RETURNING xp;