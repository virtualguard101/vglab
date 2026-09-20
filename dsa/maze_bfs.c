#include <stdio.h>
#include <threads.h>
#include <unistd.h>


#define MAX_ROW 5
#define MAX_COL 5

struct point {
	int row, col, predecessor;
} queue[18];
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


void visit(int row, int col, int maze[MAX_ROW][MAX_COL])
{
	/* 访问当前点: 记录当前点的坐标和前驱(前驱在队列中的下标) */
	struct point visit_point = { row, col, head - 1 };
	/* 将当前点标记为已访问 */
	maze[row][col] = 2;
	/* 将当前点加入队列 */
	enqueue(visit_point);
}

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

int main()
{
	int maze[MAX_ROW][MAX_COL] = {
		0, 1, 0, 0, 0,
		0, 1, 0, 1, 0,
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 1, 0,
	};

	bfs_traverse(maze);

	return 0;
}
