CREATE TABLE book_info2 LIKE book_info;
INSERT INTO book_info2 SELECT * FROM book_info;

CREATE INDEX idx_name ON book_info(name);

CREATE INDEX idx_author ON book_info(author);
