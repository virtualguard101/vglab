import pymysql


try:
    connection = pymysql.connect(
        host='localhost',
        user='root',
        password='你的密码', 
        database='UniversityDB',
        charset='utf8mb4'
    )
    print("数据库连接成功！")
except pymysql.Error as e:
    print(f"数据库连接失败：{e}")
finally:
    if 'connection' in locals():
        connection.close()

