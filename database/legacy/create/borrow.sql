/*
 Navicat Premium Data Transfer

 Source Server         : test
 Source Server Type    : MySQL
 Source Server Version : 80032
 Source Host           : localhost:3306
 Source Schema         :  librarydb

 Target Server Type    : MySQL
 Target Server Version : 80032
 File Encoding         : 65001

 Date: 17/05/2026 10:44:35
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for borrow
-- ----------------------------
DROP TABLE IF EXISTS `borrow`;
CREATE TABLE `borrow`  (
  `borrow_id` int NOT NULL AUTO_INCREMENT COMMENT '借阅编号（主键）',
  `book_id` int NOT NULL COMMENT '图书编号（外键）',
  `reader_id` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '借书证号（外键）',
  `borrow_date` date NOT NULL COMMENT '借出日期',
  `due_date` date NOT NULL COMMENT '应还书日期',
  `actual_return_date` date NULL DEFAULT NULL COMMENT '实际归还日期（NULL表示未还）',
  `fine_amount` decimal(10, 2) NOT NULL DEFAULT 0.00 COMMENT '逾期罚金（元）',
  PRIMARY KEY (`borrow_id`) USING BTREE,
  INDEX `reader_id`(`reader_id`) USING BTREE,
  INDEX `idx_borrow_date`(`borrow_date`) USING BTREE,
  INDEX `idx_overdue`(`due_date`, `actual_return_date`) USING BTREE,
  INDEX `idx_book_reader`(`book_id`, `reader_id`) USING BTREE,
  CONSTRAINT `borrow_ibfk_1` FOREIGN KEY (`book_id`) REFERENCES `book` (`book_id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  CONSTRAINT `borrow_ibfk_2` FOREIGN KEY (`reader_id`) REFERENCES `reader` (`reader_id`) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '借阅信息表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of borrow
-- ----------------------------
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (1, 3, 'R20240016', '2026-01-01', '2026-06-30', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (2, 11, 'R20240017', '2026-01-05', '2026-07-04', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (3, 1, 'R20240001', '2026-01-08', '2026-02-07', '2026-01-20');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (4, 2, 'R20240011', '2026-01-10', '2026-04-10', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (5, 9, 'R20240004', '2026-01-12', '2026-02-11', '2026-01-28');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (6, 50, 'R20240018', '2026-01-15', '2026-07-14', '2026-03-05');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (7, 8, 'R20240002', '2026-01-18', '2026-02-17', '2026-02-01');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (8, 7, 'R20240012', '2026-01-20', '2026-04-20', '2026-03-01');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (9, 14, 'R20240016', '2026-01-22', '2026-07-21', '2026-03-15');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (10, 45, 'R20240003', '2026-01-25', '2026-02-24', '2026-02-10');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (11, 27, 'R20240013', '2026-02-01', '2026-05-02', '2026-04-15');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (12, 18, 'R20240011', '2026-02-05', '2026-05-06', '2026-04-10');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (13, 5, 'R20240001', '2026-02-10', '2026-03-12', '2026-03-01');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (14, 25, 'R20240016', '2026-02-14', '2026-08-13', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (15, 22, 'R20240017', '2026-02-18', '2026-08-17', '2026-03-20');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (16, 20, 'R20240002', '2026-02-20', '2026-03-22', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (17, 39, 'R20240014', '2026-02-22', '2026-05-23', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (18, 85, 'R20240008', '2026-02-25', '2026-03-27', '2026-03-10');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (19, 23, 'R20240012', '2026-02-28', '2026-05-29', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (20, 57, 'R20240019', '2026-03-02', '2026-08-29', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (21, 17, 'R20240005', '2026-03-05', '2026-04-04', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (22, 52, 'R20240003', '2026-03-08', '2026-04-07', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (23, 12, 'R20240001', '2026-03-12', '2026-04-11', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (24, 29, 'R20240011', '2026-03-15', '2026-06-13', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (25, 92, 'R20240009', '2026-03-18', '2026-04-17', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (26, 34, 'R20240017', '2026-03-22', '2026-09-18', NULL);
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (27, 38, 'R20240016', '2026-03-25', '2026-09-21', '2026-04-25');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (28, 41, 'R20240012', '2026-04-01', '2026-06-30', '2026-05-10');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (29, 64, 'R20240006', '2026-04-05', '2026-05-05', '2026-04-20');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (30, 66, 'R20240003', '2026-04-10', '2026-05-10', '2026-04-28');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (31, 49, 'R20240017', '2026-04-15', '2026-10-12', '2026-05-05');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (32, 33, 'R20240002', '2026-04-18', '2026-05-18', '2026-05-02');
INSERT INTO `borrow` (`borrow_id`, `book_id`, `reader_id`, `borrow_date`, `due_date`, `actual_return_date`) VALUES (33, 71, 'R20240007', '2026-04-28', '2026-05-28', NULL);

SET FOREIGN_KEY_CHECKS = 1;
