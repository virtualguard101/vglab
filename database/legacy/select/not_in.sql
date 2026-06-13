/*
 NOT IN：用于判断某个值不在一组列表或子查询结果中的条件运算符。
 语法：value NOT IN (value1, value2, ...)

 其中：
 value 要判断的值
 value1, value2, ... 一组列表或子查询结果
*/


-- 从 reader 表中找出 从未借阅过任何图书 的读者（即该读者的借书证号 不存在 于 borrow 表中）
select reader_id, name
from reader where reader_id not in (
    select distinct reader_id from borrow -- `distinct` 用于去除重复的读者ID
) order by reader_id asc;
