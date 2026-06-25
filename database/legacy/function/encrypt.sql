-- 更改列宽度为64位，用于存储SHA-2哈希值。
alter table login modify password_hash CHAR(64) NULL comment '密码哈希';
/*
  sha2(str, length)：计算字符串的 SHA-2 哈希值。
  这里用于插入登录信息，密码使用 SHA-2 哈希值存储。
*/
insert into login (user_id, phone, password_hash, state, last_login_time)
values ('R20240001', '13812345678', sha2('SecPasswd123', 256), 0, now());

-- 查询登录信息，密码使用SHA-2哈希值进行比较。
select * from login
where user_id = 'R20240001' and password_hash = sha2('SecPasswd123', 256);
