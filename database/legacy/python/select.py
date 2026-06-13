import pymysql


user_input = input("请输入读者姓名: ")
sql = f"SELECT * FROM reader WHERE name = '{user_input}'"

conn = pymysql.connect(
    host='localhost',
    user='root',
    password='123456',
    database='libraryDB',
    charset='utf8mb4'
)

try:
    with conn.cursor() as cursor:
        # cursor.execute(sql) # 直接执行拼接后的 SQL
        sql = "SELECT * FROM reader WHERE name = %s" # 使用参数化查询，防止SQL注入
        cursor.execute(sql, (user_input,))

        results = cursor.fetchall()
        if results:
            for row in results:
                print(f"借书证号: {row[0]}, 姓名: {row[1]}, 电话: {row[3]}")
        else:
            print("未找到该读者")
finally:
    conn.close()
