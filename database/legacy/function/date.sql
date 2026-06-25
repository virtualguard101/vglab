/*
  datediff(date1, date2)：计算两个日期之间的天数差。
  这里用于查询尚未归还的图书的借阅天数与期限
*/
select reader_id, borrow_date, due_date,
    datediff(due_date, borrow_date) as borrow_days
from borrow where actual_return_date is null;
