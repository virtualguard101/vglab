/*
LIKE：用于在查询中匹配字符串的模糊查询运算符。
语法：column LIKE pattern

其中：
- column 要匹配的字段
- pattern 匹配模式，可以使用通配符：
  - % 表示任意数量的任意字符
  - _ 表示任意一个字符
*/

-- 从 book 表中筛选出 书名（book_name）包含 “数据库” 的图书，显示 book_id（图书编号）、book_name（书名）、author（作者）字段：
select book_id, book_name, author
from book
where book_name like '%数据库%'
order by book_name;

-- 从 book 表中筛选出 书名（book_name）以 “数据库” 开头 的图书，显示 book_id（图书编号）、book_name（书名）、author（作者）字段：
select book_id, book_name, author
from book
where book_name like '数据库%'
order by book_name;

-- 从 book 表中筛选出 书名（book_name）第二个字为“据” 的图书，显示 book_id（图书编号）、book_name（书名）、author（作者）字段：
select book_id, book_name, author
from book
where book_name like '_据%'
order by book_name;