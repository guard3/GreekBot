SELECT rank, xp, num_msg
FROM (SELECT ROW_NUMBER() OVER (ORDER BY xp DESC, num_msg DESC) AS rank, * FROM leaderboard)
WHERE id = ?1;