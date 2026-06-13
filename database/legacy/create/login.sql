CREATE TABLE login (
    login_id INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '登录编号',
    user_id VARCHAR(20) NOT NULL COMMENT '借书证号（关联 reader 表）',
    phone VARCHAR(15) NOT NULL COMMENT '手机号（短信登录唯一凭证）',
    password_hash CHAR(60) NULL COMMENT '密码哈希（bcrypt，NULL 表示仅短信登录用户）',
    state TINYINT(1) NOT NULL DEFAULT 0 COMMENT '账号状态：0-正常，1-锁定，2-禁用',
    fail_attempts TINYINT(1) NOT NULL DEFAULT 0 COMMENT '连续登录失败次数（上限3次）',
    last_fail_time DATETIME NULL COMMENT '最后一次登录失败时间',
    locked_until DATETIME NULL COMMENT '锁定到期时间（NULL表示未锁定）',
    last_login_time DATETIME NULL COMMENT '最后登录成功时间',
    PRIMARY KEY (login_id),
    UNIQUE INDEX idx_user_id (user_id),
    UNIQUE INDEX idx_phone (phone),
    INDEX idx_locked_until (locked_until)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='登录信息表（支持密码/短信登录及安全锁定）';
