WITH stats AS (
    SELECT
        SUM(?2 - timestamp < ?3) AS today,
        SUM(?2 - timestamp < ?4) AS this_week,
        COUNT(*) AS total
    FROM infractions
    WHERE user_id IS ?1
)
SELECT timestamp, reason FROM (
    SELECT 0 AS i, today     AS timestamp, NULL AS reason FROM stats
    UNION ALL
    SELECT 1 AS i, this_week AS timestamp, NULL AS reason FROM stats
    UNION ALL
    SELECT 2 AS i, total     AS timestamp, NULL AS reason FROM stats
    UNION ALL
    SELECT 3 AS i, timestamp, reason FROM infractions WHERE user_id IS ?1
) ORDER BY i, timestamp DESC LIMIT 13;