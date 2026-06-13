/*
 date_sub函数：是 MySQL 中用于从指定日期中减去一个时间间隔的函数。它可以用于计算某个日期之前的日期，常用于查询满足时间条件的记录。
 语法：date_sub(date, interval expr unit)

 其中：
 - date 起始日期（如 '2026-05-17' 或 curdate()）
 - expr 要减去的数值（整数）
 - unit 时间单位，可选：day, month, year, week, hour, minute, second 等
*/


-- 从 book 表中筛选出 入库日期（entry_date）距今已超过3年 的图书，显示以下字段：
-- book_id（图书编号）、book_name（书名）、shelf_location（书架位置）、storage_location（存储位置），entry_date（入库时间）
-- 并按 入库日期从早到晚 排序（最早入库的图书排在前面）。
select book_id, book_name, shelf_location, storage_location, entry_date from book
where entry_date <= date_sub(curdate(), interval 3 year)
order by entry_date asc;
