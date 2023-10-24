UPDATE starboard SET num=num-1 WHERE msg_id IS ?1 RETURNING sb_msg_id, num;
DELETE FROM starboard WHERE msg_id IS ?1 RETURNING sb_msg_id, 0;