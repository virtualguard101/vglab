import pymysql
import time


DB_CONFIG = {
    'host': 'localhost',
    'user': 'root',
    'password': '123456',
    'database': 'libraryDB',
    'charset': 'utf8mb4'
}

BOOK_NAMES = ['数据结构（C语言版）', '纳兰容若词传', '汉字的战争', '成长']

def test_table(table_name: str) -> float:
    """对指定表依次查询4本书，返回总耗时（秒）"""
    conn = pymysql.connect(**DB_CONFIG)
    cursor = conn.cursor()
    start = time.perf_counter()
    for name in BOOK_NAMES:
        sql = f"SELECT * FROM {table_name} WHERE name = %s"
        cursor.execute(sql, (name,))
        rows = cursor.fetchall()
        # 可选：打印每个查询返回的行数（便于验证）
        print(f"  {name} -> {len(rows)} 行")
    elapsed = time.perf_counter() - start
    cursor.close()
    conn.close()
    return elapsed


def main():
    print("测试书名列表：")
    for name in BOOK_NAMES:
        print(f"  - {name}")
    print("\n--- 测试 book_info (有索引) ---")
    time_idx = test_table("book_info")
    print(f"总耗时: {time_idx:.6f} 秒")
    print("\n--- 测试 book_info2 (无索引) ---")
    time_no_idx = test_table("book_info2")
    print(f"总耗时: {time_no_idx:.6f} 秒")

    if time_no_idx > 0:
        print(f"\n性能对比：有索引比无索引快 {time_no_idx / time_idx:.2f} 倍")

if __name__ == "__main__":
    main()
