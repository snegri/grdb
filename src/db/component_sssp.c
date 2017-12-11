#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include "config.h"
#include "cli.h"
#include "graph.h"
#define INT_type 4
#define inf INT_MAX


int get_weight(component_t c, vertexid_t v1, vertexid_t v2, char attr_name[]) {
	struct edge e;
	edge_t e1;
	int offset, weight;

	edge_init(&e);
	edge_set_vertices(&e, v1, v2);

	e1 = component_find_edge_by_ids(c, &e);


	if(e1 == NULL) {
		return inf;
	}

	offset = tuple_get_offset(e1->tuple, attr_name);
	weight = tuple_get_int(e1->tuple->buf + offset);

	return weight;

}

int get_number_of_vertices(component_t c) {
	off_t off;
	ssize_t len, size;
	vertexid_t v;
	struct tuple tuple;
	char *buf;
	int readlen;


	//assert (c != NULL);

	/* Vertices */
	if (c->sv == NULL)
		size = 0;
	else
		size = schema_size(c->sv);

	readlen = sizeof(vertexid_t) + size;
	buf = malloc(readlen);

	int count = 0;
	for (off = 0;; off += readlen) {
		lseek(c->vfd, off, SEEK_SET);
		len = read(c->vfd, buf, readlen);
		if (len <= 0)
			break;

		v = *((vertexid_t *) buf);
		count++;
	}

	free(buf);

	return count; 

}


int get_vertices(component_t c, vertexid_t list[]) {
	off_t off;
	ssize_t len, size;
	vertexid_t v;
	struct tuple tuple;
	char *buf;
	int readlen;


	//assert (c != NULL);

	/* Vertices */
	if (c->sv == NULL)
		size = 0;
	else
		size = schema_size(c->sv);

	readlen = sizeof(vertexid_t) + size;
	buf = malloc(readlen);

	int i = 0;
	int count = 0;
	for (off = 0;; off += readlen) {
		lseek(c->vfd, off, SEEK_SET);
		len = read(c->vfd, buf, readlen);
		if (len <= 0)
			break;

		v = *((vertexid_t *) buf);
		list[i] = v;
		i++;
		count++;
	}

	free(buf);

	return count; 

}

void print_vertices(vertexid_t list[], int number_of_vertices) {
	for (int i = 0; i < number_of_vertices; i++) {

		if (i == 2) {
			printf("%llu",list[i] );
		}
		else {
			printf("%llu,",list[i]);
		}
	}
	printf("\n");
}



int component_sssp(
        component_t c,
        vertexid_t v1,
        vertexid_t v2,
        int *n,
        int *total_weight,
        vertexid_t **path)
{ 		
	c->efd = edge_file_init(gno,cno);
	c->vfd = vertex_file_init(gno, cno);

	attribute_t attr;
	
	for(attr = c->se->attrlist; attr != NULL; attr = attr->next) {
		if (attr->bt == INT_type)
		{
			break;
		}
	}

	int number_of_vertices = get_number_of_vertices(c);
	vertexid_t *vertex_list;
	vertex_list = (vertexid_t *)malloc(sizeof(vertexid_t) * number_of_vertices);

	get_vertices(c, vertex_list);
	//print_vertices(vertex_list, number_of_vertices);


	//Make S list
	vertexid_t *S;
	S = (vertexid_t *)malloc(sizeof(vertexid_t) * (number_of_vertices+1));

	//Make V list
	vertexid_t *V;
	V = (vertexid_t *)malloc(sizeof(vertexid_t) * (number_of_vertices+1));

	vertexid_t *P;
	P = (vertexid_t *)malloc(sizeof(vertexid_t) * (number_of_vertices+1));


	//Make D list
	int *D;
	D = (int *)malloc(sizeof(int) * (number_of_vertices+1));

	for (int i = 0; i <= number_of_vertices; i++) {
		if (i == 0)
		{
			V[i] = 0;
		}
		V[i+1] = vertex_list[i];
		D[i] = INT_MAX;
		//printf("%d\n", D[i]);
	}


	//int weight = 0;
	//weight = get_weight(c, v1, v2, attr->name);
	//printf("Weight is: %d\n", weight);

	S[0] = v1;
	int count = 1;
	int min = 0;
	int indexW = 0;
	vertexid_t reset;
	for (int i = 2; i <= number_of_vertices; i++)
	{

		for (int i = 2; i <= number_of_vertices; ++i)
		{
			S[i] = reset;
			printf("S: %llu\n", S[i]);
		}
		printf("\n");

		min = INT_MAX;
		int isItInS = 0;
		vertexid_t next = V[i];
		D[i] = get_weight(c, v1, next, attr->name);
		//printf("%llu\n", S[1]);

		for (int i = 1; i <= number_of_vertices; i++)
		{

			if (D[i] < min)
			{
				for (int j = 1; j <= number_of_vertices; ++j)
				{
					if (i == S[j])
					{
						isItInS = 1;
					}
				}
				if (isItInS == 0) {	
					min = D[i];
					indexW = i;
				}
			}
		}
		S[count] = V[indexW];
		count++;

		int inS = 0;
		int weightWV = 0;
		int indexV = 0;
		for (int i = 1; i <= number_of_vertices; ++i)
		{
			inS = 0;
			for (int j = 1; j <= number_of_vertices; ++j)
			{
				if (i == S[j])
				{
					inS = 1;
				}
			}
			if (inS == 1)
			{
				continue;
			}
			indexV = i; 
			weightWV = get_weight(c, V[indexW], V[i], attr->name);
			
			if (D[indexW] + weightWV < D[i])
			{
				P[indexV] = V[indexW]; 
				//printf("%llu\n", P[indexV]);
			}
			int minumum = 0;
			if (D[indexW] + weightWV < D[indexV])
			{
				minumum = D[indexW] + weightWV;
				
			}
			else {
				minumum = D[indexV];
			}
			printf("%d\n", minumum);
			D[indexV] = minumum;
		}

		/* Print Statements */

		for (int i = 1; i <= number_of_vertices; ++i)
		{
			printf("Parent List: %llu, ", P[i]);
		}

		printf("\n");
		for (int i = 1; i <= number_of_vertices; ++i)
		{
			printf("Cost list: %d, ", D[i]);
		}
		printf("\n");
	}

	free(D);
	free(V);
	free(S);
	return -1;

}
	

