CREATE TABLE book_info (
    auto_id INT PRIMARY KEY AUTO_INCREMENT COMMENT '自增主键',
    id VARCHAR(10) NOT NULL COMMENT '图书业务ID',
    sn VARCHAR(13) COMMENT '序列号',
    isbn VARCHAR(13) COMMENT '国际标准书号',
    name VARCHAR(100) NOT NULL COMMENT '书名',
    price DECIMAL(10,2) COMMENT '价格',
    author VARCHAR(100) COMMENT '作者',
    cate_text VARCHAR(50) COMMENT '分类文本',
    market_price DECIMAL(10,2) COMMENT '市场价',
    has_stock TINYINT COMMENT '是否有库存（0/1）',
    cate_id INT COMMENT '分类ID',        
    cate_name VARCHAR(50) COMMENT '分类名称'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='图书信息表';
