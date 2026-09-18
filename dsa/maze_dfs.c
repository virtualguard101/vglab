#include <stdio.h>
#include <unistd.h>

#define MAX_ROW 5
#define MAX_COL 5

struct point {
	int row, col;
} stack[512];
int top = 0;

void print_maze(int maze[MAX_ROW][MAX_COL])
{
	int i, j;
	for (i = 0; i < MAX_ROW; i++) {
		for (j = 0; j < MAX_COL; j++) {
			printf("%d ", maze[i][j]);
		}
		putchar('\n');
	}
	printf("*********\n");
}


/* 迭代实现(显式栈) */
void push(struct point p)
{
	stack[top++] = p;
}

struct point pop(void)
{
	return stack[--top];
}

int is_empty(void)
{
	return top == 0;
}

void visit(int row, int col, struct point pre, int maze[MAX_ROW][MAX_COL],
	   struct point predecessor[MAX_ROW][MAX_COL])
{
	struct point visit_point = { row, col };
	maze[row][col] = 2;
	predecessor[row][col] = pre;
	push(visit_point);
}

/**
 * @brief 深度优先搜索迷宫
 * 
 * @return int 1 if path found, 0 if no path found
 */
int dfs_traverse_base_iter(int maze[MAX_ROW][MAX_COL],
			   struct point predecessor[MAX_ROW][MAX_COL])
{
	struct point p = { 0, 0 };

	maze[p.row][p.col] = 2;
	push(p);

	while (!is_empty()) {
		p = pop();
		if (p.row == MAX_ROW - 1	/* goal */
		    && p.col == MAX_COL - 1)
			break;
		if (p.col + 1 < MAX_COL	/* right */
		    && maze[p.row][p.col + 1] == 0)
			visit(p.row, p.col + 1, p, maze, predecessor);
		if (p.row + 1 < MAX_ROW	/* down */
		    && maze[p.row + 1][p.col] == 0)
			visit(p.row + 1, p.col, p, maze, predecessor);
		if (p.col - 1 >= 0	/* left */
		    && maze[p.row][p.col - 1] == 0)
			visit(p.row, p.col - 1, p, maze, predecessor);
		if (p.row - 1 >= 0	/* up */
		    && maze[p.row - 1][p.col] == 0)
			visit(p.row - 1, p.col, p, maze, predecessor);
		print_maze(maze);
		sleep(1);
	}
	if (p.row == MAX_ROW - 1 && p.col == MAX_COL - 1) {
		printf("(%d, %d)\n", p.row, p.col);
		while (predecessor[p.row][p.col].row != -1) {
			p = predecessor[p.row][p.col];
			printf("(%d, %d)\n", p.row, p.col);
		}
	} else {
		return 0;
	}
	return 1;
}


/* 递归实现(系统调用栈) */
/**
 * @brief 深度优先搜索, 递归搜索逻辑
 * 
 * @param row 当前行
 * @param col 当前列
 * @return int 1 if path found, 0 if no path found
 */
int dfs(int row, int col, int maze[MAX_ROW][MAX_COL])
{
	maze[row][col] = 2;	/* 标记为已走过 */
	print_maze(maze);
	sleep(1);

	if (row == MAX_ROW - 1 && col == MAX_COL - 1) {	/* 到达终点 */
		return 1;
	}
	if (col + 1 < MAX_COL && maze[row][col + 1] == 0) {	/* 向右走 */
		if (dfs(row, col + 1, maze)) {	/* 如果向右走能到达终点 */
			return 1;
		}
	}
	if (row + 1 < MAX_ROW && maze[row + 1][col] == 0) {	/* 向下走 */
		if (dfs(row + 1, col, maze)) {	/* 如果向下走能到达终点 */
			return 1;
		}
	}
	if (col - 1 >= 0 && maze[row][col - 1] == 0) {	/* 向左走 */
		if (dfs(row, col - 1, maze)) {	/* 如果向左走能到达终点 */
			return 1;
		}
	}
	if (row - 1 >= 0 && maze[row - 1][col] == 0) {	/* 向上走 */
		if (dfs(row - 1, col, maze)) {	/* 如果向上走能到达终点 */
			return 1;
		}
	}
	return 0;
}

/**
 * @brief 深度优先搜索递归调用入口
 * 
 * @return int dfs() return value, 1 if path found, 0 if no path found
 */
int dfs_traverse_base_recur(int maze[MAX_ROW][MAX_COL])
{
	/* 从 (0, 0) 为起点开始遍历 */
	return dfs(0, 0, maze);
}

int main()
{
	int maze[MAX_ROW][MAX_COL] = {
		0, 1, 0, 0, 0,
		0, 1, 0, 1, 0,
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 1, 0,
	};

	struct point predecessor[MAX_ROW][MAX_COL] = {
		{ { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1} },
		{ { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1} },
		{ { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1} },
		{ { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1} },
		{ { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1} },
	};

	/* 迭代实现 */
	printf("Iterative implementation:\n");
	if (dfs_traverse_base_iter(maze, predecessor)) {
		printf("Path found!\n");
	} else {
		printf("No path!\n");
	}

	int maze2[MAX_ROW][MAX_COL] = {
		0, 1, 0, 0, 0,
		0, 1, 0, 1, 0,
		0, 0, 0, 1, 0,
		0, 1, 0, 1, 0,
		0, 0, 0, 1, 0,
	};

	struct point predecessor2[MAX_ROW][MAX_COL] = {
		{ { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1} },
		{ { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1} },
		{ { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1} },
		{ { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1} },
		{ { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1}, { -1, -1} },
	};


	/* 递归实现 */
	printf("Recursive implementation:\n");
	if (dfs_traverse_base_recur(maze2)) {
		printf("Path found!\n");
	} else {
		printf("No path!\n");
	}
	return 0;
}
