SELECT LAG(timestamp) OVER (ORDER BY timestamp DESC) - timestamp
FROM infractions
WHERE user_id IS ? AND timed_out IS 0
LIMIT 1, 1