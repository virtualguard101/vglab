import pymysql

conn = pymysql.connect(
    host='localhost',
    user='root',
    password='123456',
    database='libraryDB'

)

cursor = conn.cursor()
reader_id = 'R20240011'
cursor.execute(f"SELECT get_reader_card_date('{reader_id}')")
result = cursor.fetchone()[0]
print(result)   # 输出日期，如 2023-05-20
cursor.close()
conn.close()
