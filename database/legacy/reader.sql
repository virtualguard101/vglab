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

 Date: 17/05/2026 10:44:50
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for reader
-- ----------------------------
DROP TABLE IF EXISTS `reader`;
CREATE TABLE `reader`  (
  `reader_id` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '借书证号（主键）',
  `name` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '姓名',
  `gender` char(1) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '性别：M-男，F-女',
  `phone` varchar(15) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL COMMENT '电话号码',
  `id_card` varchar(18) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '身份证号（唯一）',
  `card_level` varchar(10) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT '普通' COMMENT '借书证等级（如普通、高级、VIP）',
  `card_date` date NOT NULL COMMENT '办卡日期',
  `card_status` tinyint(1) NOT NULL DEFAULT 0 COMMENT '借书卡状态：0-正常，1-禁用',
  `max_borrow_limit` int NOT NULL DEFAULT 5 COMMENT '最大可借总数（根据等级设定）',
  PRIMARY KEY (`reader_id`) USING BTREE,
  UNIQUE INDEX `id_card`(`id_card`) USING BTREE,
  UNIQUE INDEX `idx_id_card`(`id_card`) USING BTREE,
  INDEX `idx_phone`(`phone`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '读者信息表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of reader
-- ----------------------------
INSERT INTO `reader` VALUES ('R20240001', '张三', 'M', '13812345678', '11010119900307663X', '普通', '2024-01-10', 0, 5);
INSERT INTO `reader` VALUES ('R20240002', '李四', 'M', '13987654321', '110101199105124567', '普通', '2024-02-15', 0, 5);
INSERT INTO `reader` VALUES ('R20240003', '王芳', 'F', '15112345678', '110101199212019876', '普通', '2024-03-20', 0, 5);
INSERT INTO `reader` VALUES ('R20240004', '赵磊', 'M', '15223456789', '110101199303042345', '普通', '2024-04-05', 0, 5);
INSERT INTO `reader` VALUES ('R20240005', '孙丽', 'F', '15334567890', '110101199410156789', '普通', '2024-05-18', 0, 5);
INSERT INTO `reader` VALUES ('R20240006', '周强', 'M', '15445678901', '110101199511224567', '普通', '2024-06-22', 0, 5);
INSERT INTO `reader` VALUES ('R20240007', '吴娜', 'F', '15556789012', '110101199612034567', '普通', '2024-07-30', 0, 5);
INSERT INTO `reader` VALUES ('R20240008', '郑伟', 'M', '15667890123', '110101199701185678', '普通', '2024-08-12', 0, 5);
INSERT INTO `reader` VALUES ('R20240009', '陈晨', 'F', '15778901234', '110101199802246789', '普通', '2024-09-09', 0, 5);
INSERT INTO `reader` VALUES ('R20240010', '林华', 'M', '15889012345', '110101199903017890', '普通', '2024-10-05', 0, 5);
INSERT INTO `reader` VALUES ('R20240011', '徐静', 'F', '15990123456', '110101200004129012', '高级', '2024-01-20', 0, 8);
INSERT INTO `reader` VALUES ('R20240012', '黄涛', 'M', '15001234567', '110101200105231234', '高级', '2024-02-28', 0, 8);
INSERT INTO `reader` VALUES ('R20240013', '刘洋', 'F', '18112345678', '110101200207156789', '高级', '2024-03-15', 0, 8);
INSERT INTO `reader` VALUES ('R20240014', '杨军', 'M', '18223456789', '110101200310188901', '高级', '2024-04-18', 0, 8);
INSERT INTO `reader` VALUES ('R20240015', '朱琳', 'F', '18334567890', '110101200411229012', '高级', '2024-05-22', 0, 8);
INSERT INTO `reader` VALUES ('R20240016', '沈峰', 'M', '18445678901', '110101200512311234', 'VIP', '2024-01-05', 0, 12);
INSERT INTO `reader` VALUES ('R20240017', '韩梅', 'F', '18556789012', '110101200601142345', 'VIP', '2024-02-10', 0, 12);
INSERT INTO `reader` VALUES ('R20240018', '崔健', 'M', '18667890123', '110101200702254567', 'VIP', '2024-03-25', 0, 12);
INSERT INTO `reader` VALUES ('R20240019', '苏琪', 'F', '18778901234', '110101200803366789', 'VIP', '2024-04-30', 0, 12);
INSERT INTO `reader` VALUES ('R20240020', '卢勇', 'M', '18889012345', '110101200904479012', 'VIP', '2024-05-10', 0, 12);

SET FOREIGN_KEY_CHECKS = 1;
