import csv
import pymysql

with open("data/book_info.csv", encoding="utf-8") as f:
    rows = list(csv.DictReader(f))

conn = pymysql.connect(host="localhost", user="root", password="123456", database="libraryDB")
with conn.cursor() as cur:
    cur.executemany(
        """INSERT INTO book_info
           (id, sn, isbn, name, price, author, cate_text, market_price, has_stock, cate_id, cate_name)
           VALUES (%(id)s, %(sn)s, %(isbn)s, %(name)s, %(price)s, %(author)s,
                   %(cate_text)s, %(market_price)s, %(has_stock)s, %(cate_id)s, %(cate_name)s)""",
        rows,
    )
conn.commit()
