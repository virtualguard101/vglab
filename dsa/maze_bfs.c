#include <stdio.h>
#include <threads.h>
#include <unistd.h>


#define MAX_ROW 5
#define MAX_COL 5

#define NORMAL_QUEUE_SIZE 18
#define CIRCULAR_QUEUE_SIZE 5

/* 普通队列 */
struct point {
	int row, col, predecessor;
} queue[NORMAL_QUEUE_SIZE];
int head = 0, tail = 0;

void enqueue(struct point p)
{
	queue[tail++] = p;
}

struct point dequeue()
{
	return queue[head++];
}

int is_empty()
{
	return head == tail;
}

void visit(int row, int col, int maze[MAX_ROW][MAX_COL])
{
	/* 访问当前点: 记录当前点的坐标和前驱(前驱在队列中的下标) */
	struct point visit_point = { row, col, head - 1 };
	/* 将当前点标记为已访问 */
	maze[row][col] = 2;
	/* 将当前点加入队列 */
	enqueue(visit_point);
}


/* 循环队列 */
struct point circular_queue[CIRCULAR_QUEUE_SIZE];
int circular_head = 0, circular_tail = 0;

int is_circular_queue_empty()
{
	return circular_head == circular_tail;
}

void circular_enqueue(struct point p)
{
	if ((circular_tail + 1) % CIRCULAR_QUEUE_SIZE == circular_head) {
		printf("The circular queue has been filled!\n");
	}

	circular_queue[circular_tail] = p;
	circular_tail = (circular_tail + 1) % CIRCULAR_QUEUE_SIZE;
}

struct point circular_dequeue()
{
	if (is_circular_queue_empty()) {
		printf("The circular queue has been emptied!\n");
		return (struct point) { -1, -1, -1 };
	}

	struct point p = circular_queue[circular_head];
	circular_head = (circular_head + 1) % CIRCULAR_QUEUE_SIZE;
	return p;
}

void circular_visit(int row, int col, int maze[MAX_ROW][MAX_COL])
{
	struct point visit_point = { row, col, circular_head - 1 };
	maze[row][col] = 2;
	circular_enqueue(visit_point);
}


/* 辅助函数 */
void print_maze(int maze[MAX_ROW][MAX_COL])
{
	int i, j;
	for (i = 0; i < MAX_ROW; i++) {
		for (j = 0; j < MAX_COL; j++)
			printf("%d ", maze[i][j]);
		putchar('\n');
	}
	printf("*********\n");
}


/**
 * @brief BFS 遍历迷宫, 并打印搜索路径
 * 
 * @param maze 迷宫
 */
void bfs_traverse(int maze[MAX_ROW][MAX_COL])
{
	/* 遍历初始化: 起点入队, 标记为已访问 */
	struct point p = { 0, 0, -1 };
	maze[p.row][p.col] = 2;
	enqueue(p);

	while (!is_empty()) {
		p = dequeue();
		if (p.row == MAX_ROW - 1	/* goal */
		    && p.col == MAX_COL - 1)
			break;
		if (p.col + 1 < MAX_COL	/* right */
		    && maze[p.row][p.col + 1] == 0)
			visit(p.row, p.col + 1, maze);
		if (p.row + 1 < MAX_ROW	/* down */
		    && maze[p.row + 1][p.col] == 0)
			visit(p.row + 1, p.col, maze);
		if (p.col - 1 >= 0	/* left */
		    && maze[p.row][p.col - 1] == 0)
			visit(p.row, p.col - 1, maze);
		if (p.row - 1 >= 0	/* up */
		    && maze[p.row - 1][p.col] == 0)
			visit(p.row - 1, p.col, maze);
		print_maze(maze);
		sleep(1);
	}
	if (p.row == MAX_ROW - 1 && p.col == MAX_COL - 1) {
		printf("(%d, %d)\n", p.row, p.col);
		while (p.predecessor != -1) {
			p = queue[p.predecessor];
			printf("(%d, %d)\n", p.row, p.col);
		}
	} else
		printf("No path!\n");
}

/**
 * @brief BFS 基于循环队列的实现, 只返回是否找到路径
 * 
 * @param maze 迷宫
 * @return int 是否找到路径
 * @return int 
 */
int bfs_traverse_based_circular(int maze[MAX_ROW][MAX_COL])
{
	/* 遍历初始化: 起点入队, 标记为已访问 */
	struct point p = { 0, 0, -1 };
	maze[p.row][p.col] = 2;
	circular_enqueue(p);

	while (!is_circular_queue_empty()) {
		p = circular_dequeue();
		if (p.row == MAX_ROW - 1	/* goal */
		    && p.col == MAX_COL - 1)
			break;
		if (p.col + 1 < MAX_COL	/* right */
		    && maze[p.row][p.col + 1] == 0)
			circular_visit(p.row, p.col + 1, maze);
		if (p.row + 1 < MAX_ROW	/* down */
		    && maze[p.row + 1][p.col] == 0)
			circular_visit(p.row + 1, p.col, maze);
		if (p.col - 1 >= 0	/* left */
		    && maze[p.row][p.col - 1] == 0)
			circular_visit(p.row, p.col - 1, maze);
		if (p.row - 1 >= 0	/* up */
		    && maze[p.row - 1][p.col] == 0)
			circular_visit(p.row - 1, p.col, maze);
	}
	if (p.row == MAX_ROW - 1 && p.col == MAX_COL - 1) {
		printf("Found the path!\n");
		return 1;
	} else {
		printf("No path!\n");
		return 0;
	}
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

	int result = bfs_traverse_based_circular(maze);
	if (result) {
		int copy_for_display[MAX_ROW][MAX_COL] = {
			0, 1, 0, 0, 0,
			0, 1, 0, 1, 0,
			0, 0, 0, 0, 0,
			0, 1, 1, 1, 0,
			0, 0, 0, 1, 0,
		};
		bfs_traverse(copy_for_display);
	}

	return 0;
}
