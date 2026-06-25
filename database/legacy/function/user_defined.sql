-- 根据借书证号返回读者电话号码
delimiter $$
drop function if exists get_reader_phone$$
create function get_reader_phone(p_reader_id varchar(20) character set utf8mb4 collate utf8mb4_0900_ai_ci)
returns varchar(15)
reads sql data
begin
    declare v_phone varchar(15);
    select phone into v_phone from reader where reader_id = p_reader_id;
    return v_phone;
end$$
delimiter ;

select get_reader_phone('R20240011');

-- 根据借书证号返回办卡日期
delimiter $$
drop function if exists get_reader_card_date$$
create function get_reader_card_date(p_reader_id varchar(20) character set utf8mb4 collate utf8mb4_0900_ai_ci)
returns date
reads sql data
begin
    declare v_card_date date;
    select card_date into v_card_date from reader where reader_id = p_reader_id;
    return v_card_date;
end$$
delimiter ;

select get_reader_card_date('R20240011');
