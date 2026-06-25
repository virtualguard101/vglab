DELIMITER $$
DROP PROCEDURE IF EXISTS get_reader_by_name$$

CREATE PROCEDURE get_reader_by_name(IN p_name VARCHAR(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci)
BEGIN
    SELECT
        reader_id,
        name,
        CONCAT(LEFT(phone, 3), '****', RIGHT(phone, 4)) AS phone,
        CONCAT(LEFT(id_card, 6), '****', RIGHT(id_card, 2)) AS id_card
    FROM reader
    WHERE name LIKE CONCAT('%', p_name, '%')
    LIMIT 20;
END$$
DELIMITER ;

CALL get_reader_by_name('张');