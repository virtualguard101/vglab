/*
  ①情况1：图书尚未归还（actual_return_date IS NULL）且当前日期已超过应还日期（due_date < CURDATE()），则逾期天数 = DATEDIFF(CURDATE(), due_date)
  ②情况2：图书已归还（actual_return_date IS NOT NULL）且实际归还日期晚于应还日期（actual_return_date > due_date），则逾期天数 = DATEDIFF(actual_return_date, due_date)
  ③其他情况（未逾期或未超期未还），罚金为 0
*/

-- 若 borrow 表尚无 fine_amount 列，先添加（已有该列可跳过此句）
ALTER TABLE borrow
ADD COLUMN fine_amount DECIMAL(10, 2) NOT NULL DEFAULT 0.00 COMMENT '逾期罚金（元）';

DELIMITER $$
DROP PROCEDURE IF EXISTS update_borrow_fine$$
CREATE PROCEDURE update_borrow_fine()
BEGIN
    -- 情况1：未归还且已逾期
    UPDATE borrow
    SET fine_amount = DATEDIFF(CURDATE(), due_date)* 0.2
    WHERE actual_return_date IS NULL 
      AND due_date < CURDATE();

    -- 情况2：已归还且逾期
    UPDATE borrow
    SET fine_amount = DATEDIFF(actual_return_date, due_date)*  0.2
    WHERE actual_return_date IS NOT NULL 
      AND actual_return_date > due_date;

    -- 情况3：其他情况（未逾期或未超期未还）罚金应设为0
    UPDATE borrow
    SET fine_amount = 0.00
    WHERE (actual_return_date IS NOT NULL AND actual_return_date <= due_date)
       OR (actual_return_date IS NULL AND due_date >= CURDATE());
END$$
DELIMITER ;
