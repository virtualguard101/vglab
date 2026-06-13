-- select database
use libraryDB;

-- show all columns
select * from book;

-- show specific columns
select book_id, book_name, author from book;

-- order by descending, default is ascending
select book_id, book_name, author from book order by book_id desc;

-- `where` clause
select book_id, book_name, author from book where state = 0;


-- limit
select book_id, book_name, author from book limit 10;
