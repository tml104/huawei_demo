#pragma once

#include <iostream>
#include <queue>
#include <vector>

const int NIL =  -1;
const int INF = 1e+10;
enum Color{white, gray};

template <typename T>
class CoreSolver {
public:
	CoreSolver()
	{
		vertex_num = 0;
	}
	CoreSolver(int vertex_num, int s, int t)
	{
		this->vertex_num = vertex_num;
		this->s = s;
		this->t = t;
		precessor.resize (vertex_num);
		state.resize (vertex_num);
		capacity.resize (vertex_num);
		flow.resize (vertex_num);
		residual.resize (vertex_num);
		std::cout<<"Before core sovler for"<<std::endl;
		for (int i=0; i<vertex_num; i++) {
			capacity[i].resize (vertex_num);
			flow[i].resize (vertex_num);
			residual[i].resize (vertex_num);
		}
		std::cout << vertex_num << std::endl;
		for (int i=0; i<vertex_num; i++)
			for (int j=0; j<vertex_num; j++)
				capacity[i][j] = 0;
	}
	void insert_edge(int u, int v, T weight)
	{
		if (u<0 || u>=vertex_num || v<0 || v>=vertex_num)
			return;
		capacity[u][v] = weight;
	}
	T ford_fulkerson()
	{
		T max_flow = (T)0.0;
		initial();
		while (find_argumenting_path()) {
			T minIncrease = get_min_increase();
			max_flow += minIncrease;
			update(minIncrease);
		}
		return max_flow;
	}
	std::vector<std::pair<int, int> > mincut (){
		std::vector<std::pair<int, int> > result;
		std::vector<bool> visited;
		visited.resize (vertex_num, false);

		std::queue<int> bfs;
		bfs.push (s);
		visited[s] = true;

		do 
		{
			int cur_node = bfs.front ();
			bfs.pop ();
			for (int i = 0; i != vertex_num; ++i){
				if (i == cur_node) continue;
				if (residual[cur_node][i] && !visited[i]){
					bfs.push (i); visited[i] = true;
				}
			}

		} while (!bfs.empty ());

		for (int i = 0; i != vertex_num - 1; ++i){
			for (int j = i + 1; j != vertex_num; ++j){
				if (capacity[i][j] && (visited[i] ^ visited[j])){
					result.push_back (std::make_pair (i, j));
				}
			}
		}

		return result;
	}
private:
	void update(T min)
	{
		int cur = t;
		int pre = precessor[t];
		do {
			flow[pre][cur] += min;
			flow[cur][pre] = -flow[pre][cur];
			residual[pre][cur] = capacity[pre][cur] - flow[pre][cur];
			residual[cur][pre] = capacity[cur][pre] - flow[cur][pre];
			cur = pre;
		} while((pre = precessor[pre]) != NIL);
	}

	bool find_argumenting_path()
	{
		for (int i = 0; i < vertex_num; i++)
			state[i] = white;
		std::queue<int> que;
		que.push(s);
		state[s] = gray;
		while (!que.empty()) {
			int head = que.front();
			que.pop();
			for (int i=0; i < vertex_num; i++) {
				if (residual[head][i] > (T)0.0 && state[i] == white) {
					precessor[i] = head;
					if (i == t)
						return true;
					que.push(i);
					state[i] = gray;
				}
			}
		}
		return false;
	}
	T get_min_increase()
	{
		int pre = precessor[t];
		T min = residual[pre][t];
		while (pre != s) {
			int cur = pre;
			pre = precessor[cur];
			if (min > residual[pre][cur])
				min = residual[pre][cur];
		}
		return min;
	}
	void initial(){
		for (int i=0; i<vertex_num; i++) {
			for (int j=0; j<vertex_num; j++) {
				flow[i][j] = 0;
				residual[i][j] = capacity[i][j] - flow[i][j];
			}
		}
		precessor[s] = NIL;
	}

	int vertex_num;
	int s; // Ô´
	int t; // ¶Ë
	std::vector<int> precessor;
	std::vector<Color> state;
	std::vector<std::vector<T> > capacity;
	std::vector<std::vector<T> > flow;
	std::vector<std::vector<T> > residual;
};

const int inf = 0x3fffffff;


template<typename type>
struct Isap{
	struct Edge {
		int to, next;
		type cap, flow;
		Edge(){}
		Edge(int to, int next, type cap, type flow):to(to), next(next), cap(cap), flow(flow){}
	};
	std::vector<Edge> edge;
	int tot;
	std::vector<int> head, gap, dep, pre, cur;

	void init(int N, int M){
		tot = 0;
		head = std::vector<int>(N+50, -1);
		pre = std::vector<int>(N+50, 0);
		gap = std::vector<int>(N+50, 0);
		dep = std::vector<int>(N+50, 0);
		cur = std::vector<int>(N+50, -1);
	}
	void add_edge(int u, int v, type w, type rw = 0){
		edge.push_back(  Edge(v, head[u], w, 0) );
		//edge[tot] = Edge(v, head[u], w, 0);
		head[u] = tot++;
		edge.push_back(  Edge(u, head[v], rw, 0) );
		//edge[tot] = Edge(u, head[v], rw, 0);
		head[v] = tot++;
	}
	type sap(int s, int t, int N){
		int u = s;
		pre[u] = -1, gap[0] = N;
		type ans = 0;
		while(dep[s] < N){
			if(u == t){
				type Min = INF;
				for(int i = pre[u]; i != -1; i = pre[edge[i^1].to])
					Min = std::min(Min, edge[i].cap-edge[i].flow);
				for(int i = pre[u]; i != -1; i = pre[edge[i^1].to]){
					edge[i].flow += Min;
					edge[i^1].flow -= Min;
				}
				u = s;
				ans += Min;
				continue;
			}
			bool flag = false;
			int v;
			for(int i = cur[u]; i != -1; i = edge[i].next){
				v = edge[i].to;
				if(edge[i].cap-edge[i].flow&&dep[v]+1 == dep[u]){
					flag = true;
					cur[u] = pre[v] = i;
					break;
				}
			}
			if(flag){
				u = v;
				continue;
			}
			int Min = N;
			for(int i = head[u]; i != -1; i = edge[i].next)
				if(edge[i].cap-edge[i].flow&&dep[edge[i].to] < Min){
					Min = dep[edge[i].to];
					cur[u] = i;
				}
				if(--gap[dep[u]] == 0) break;
				dep[u] = Min+1;
				gap[dep[u]]++;
				if(u != s) u = edge[pre[u]^1].to;
		}
		return ans;
	}
	std::vector< std::pair<int, int> > min_cut(int s, int t, int N){
		type Maxflow =  sap(s, t, N);
		std::queue<int> Q;
		std::vector<bool> vis;
		vis.resize (N, false);

		Q.push(s);
		vis[s] = true;
		while(!Q.empty()){
			int now = Q.front(); Q.pop();
			for(int i = head[now]; i != -1; i = edge[i].next){
				if(!vis[ edge[i].to ] && edge[i].cap > edge[i].flow){
					int to = edge[i].to;
					Q.push(to); vis[to] = true;
				}
			}
		}
		std::vector< std::pair<int, int> > ans;
		for(int i = 0; i <= N; i++) {
			for(int j = head[i]; j != -1; j = edge[j].next){
				int from = i, to = edge[j].to;
				if(edge[j].cap && (vis[from]^vis[to]))
					ans.push_back(std::make_pair(from,to));
			}
		}
		return ans;
	}
};