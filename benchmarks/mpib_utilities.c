#include "mpib_utilities.h"
#include <malloc.h>

MPIB_pairs* MPIB_build_pairs(int n)
{
	MPIB_pair* list = NULL;
	MPIB_pair* iter = NULL;
	int i, j;
	for (i = 0; i < n - 1; i++)
	{
		for (j = i + 1; j < n; j++)
		{
			if (!iter)
				(iter = list = (MPIB_pair*)malloc(sizeof(MPIB_pair)))->prev = NULL;
			else
			{
				(iter->next = (MPIB_pair*)malloc(sizeof(MPIB_pair)))->prev = iter;
				iter = iter->next; 
			}
			iter->values[0] = i;
			iter->values[1] = j;
			iter->next = NULL;
		}
	}

	MPIB_pairs* pairs = NULL;
	MPIB_pairs* curr_pairs = NULL;
	while (list)
	{
		if (!curr_pairs)
			(curr_pairs = pairs = (MPIB_pairs*)malloc(sizeof(MPIB_pairs)))->prev = NULL;
		else
		{
			(curr_pairs->next = (MPIB_pairs*)malloc(sizeof(MPIB_pairs)))->prev = curr_pairs;
			curr_pairs = curr_pairs->next;
		}
		curr_pairs->list = NULL;
		curr_pairs->next = NULL;

		MPIB_pair* curr_pair = NULL;
		MPIB_pair* iter1 = list;
		while (iter1)
		{
			int flag = 1;
			MPIB_pair* iter2 = curr_pairs->list;
			while (flag && iter2)
			{
				int i, j;
				for (i = 0; i < 2; i++)
					for (j = 0; j < 2; j++)
						flag &= iter1->values[i] != iter2->values[j];
				iter2 = iter2->next;
			}
			if (flag)
			{
				if (iter1 == list)
					list = list->next;
				if (iter1->prev)
					iter1->prev->next = iter1->next;
				if (iter1->next)
					iter1->next->prev = iter1->prev;
				if (!curr_pair)
				{
					curr_pair = curr_pairs->list = iter1;
					curr_pair->prev = NULL;
				}
				else
				{
					(curr_pair->next = iter1)->prev = curr_pair;
					curr_pair = iter1;
				}
				iter1 = iter1->next;
				curr_pair->next = NULL;
			}
			else
				iter1 = iter1->next;
		}
	}
	return pairs;
}

void MPIB_free_pair(MPIB_pair* pair)
{
	if (pair)
	{
		MPIB_free_pair(pair->next);
		free(pair);
	}
}

void MPIB_free_pairs(MPIB_pairs* pairs)
{
	if (pairs)
	{
		MPIB_free_pair(pairs->list);
		MPIB_free_pairs(pairs->next);
		free(pairs);
	}
}
