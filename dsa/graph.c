#define MAX_VERTEX_NUM 500

/**
 * @brief 无向图的邻接矩阵表示
 */
struct mgraph {
	char vexs[MAX_VERTEX_NUM];
	int edge[MAX_VERTEX_NUM][MAX_VERTEX_NUM];
	int vexnum, arcnum;
};

/**
 * @brief 求邻接矩阵中顶点 v 的第一个邻接点
 * 
 * @param G 邻接矩阵
 * @param v 顶点
 * @return int 第一个邻接点
 */
int first_neighbor_for_mgraph(struct mgraph *G, int v) {
	for (int j = 0; j < G->vexnum; ++j) {
		if (G->edge[v][j] != 0) {
			return j;
		}
	}
	return -1;
}

/**
 * @brief 求邻接矩阵中顶点 v 的下一个邻接点
 * 
 * @param G 邻接矩阵
 * @param v 当前顶点
 * @param w 当前顶点的邻接点
 * @return int 下一个邻接点
 */
int next_neighbor_for_mgraph(struct mgraph *G, int v, int w) {
	for (int j = w + 1; j < G->vexnum; ++j) {
		if (G->edge[v][j] != 0) {
			return j;
		}
	}
	return -1;
}


/**
 * @brief 邻接表弧结点
 * 
 */
struct arc_node {
	int adjvex;
	struct arc_node *next;
	// double weight; /* 权值域 */
};

/**
 * @brief 邻接表顶点结点
 */
struct vex_node {
	char *data;
	struct arc_node *first;
};

/**
 * @brief 邻接表
 */
struct adj_list_graph {
	struct vex_node vexs[MAX_VERTEX_NUM];
	int vexnum, arcnum;
};

/**
 * @brief 求邻接表中顶点 v 的第一个邻接点
 * 
 * @param G 邻接表
 * @param v 顶点
 * @return int 第一个邻接点
 */
int first_neighbor_for_algraph(struct adj_list_graph *G, int v) {
	if (!G->vexs[v].first) {
		return -1;
	}
	return G->vexs[v].first->adjvex;
}

/**
 * @brief 求邻接表中顶点 v 相对于邻接点 w 的下一个邻接点
 * 
 * 
 * @param G 邻接表
 * @param v 当前顶点
 * @param w 当前顶点的邻接点
 * @return int 下一个邻接点
 */
int next_neighbor_for_algraph(struct adj_list_graph *G, int v, int w) {
	struct arc_node *p = G->vexs[v].first;
	while (p && p->adjvex != w) {
		p = p->next;
	}
	if (p && p->next) {
		return p->next->adjvex;
	}
	return -1;
}
