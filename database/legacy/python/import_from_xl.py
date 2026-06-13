import openpyxl
import pymysql

wb = openpyxl.load_workbook("data/publisher.xlsx")
rows = list(wb.active.iter_rows(min_row=2, values_only=True))

conn = pymysql.connect(host="localhost", user="root", password="123456", database="libraryDB")
with conn.cursor() as cur:
    cur.executemany(
        "INSERT INTO publisher (name, address, phone) VALUES (%s, %s, %s)",
        [(r[1], r[2], r[3]) for r in rows],
    )
conn.commit()
