CREATE TABLE `publisher` (
  `publisher_id` int NOT NULL AUTO_INCREMENT COMMENT '出版社编号（主键）',
  `name` varchar(50) NOT NULL COMMENT '出版社名称',
  `address` varchar(200) DEFAULT NULL COMMENT '地址（可选）',
  `phone` varchar(20) DEFAULT NULL COMMENT '联系电话（可选）',
  PRIMARY KEY (`publisher_id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=32 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='出版社信息表'
