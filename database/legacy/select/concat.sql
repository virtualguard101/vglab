/*
 CONCAT：用于连接两个或多个字符串的函数。可用于脱敏处理，如手机号、身份证号等。
 语法：CONCAT(string1, string2, ...)
*/

-- 从 reader 表中读取 读者姓名 和 电话号码：
-- 1、姓名：只显示第一个字（姓氏），剩余部分用 代替（例如“张三”显示为“张*”，“欧阳雪”显示为“欧**”）。
-- 2、电话号码：只显示前 3 位和后 4 位，中间部分用 *代替（假设电话号码为 11 位，例如“13812345678”显示为“138****5678”）。
select
    concat(left(name, 1), repeat('*', char_length(name) - 1)) AS masked_name,
    concat(left(phone, 3), '****', right(phone, 4)) AS masked_phone
from reader;
 