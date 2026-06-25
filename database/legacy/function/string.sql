/*
  concat(string1, string2, ...)
  用于连接两个或多个字符串的函数。
*/
select concat(name, '(', reader_id, ')') as reader_info,
    length(id_card) as id_len,
    upper(left(name, 1)) as first_char
from reader limit 5;

/*
  LEFT(str, length)：从字符串的左侧开始截取指定长度的子字符串。
  RIGHT(str, length)：从字符串的右侧开始截取指定长度的子字符串。
*/
select
    reader_id,
    name,
    card_level,
    left(id_card, 6) as prefix_id_card,
    right(phone, 4) as suffix_phone
from reader limit 5;
