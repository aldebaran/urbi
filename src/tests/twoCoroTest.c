#include "Coro.h"
#include <stdio.h>
#include <stdlib.h>

Coro *firstCoro, *secondCoro;

void secondTask(void *context)
{
	int num = 0;

	printf("secondTask created with value %d\n", *(int *)context);

	while (1)
	{
		printf("secondTask: %d %d\n", (int)Coro_bytesLeftOnStack(secondCoro), num++);
		Coro_switchTo_(secondCoro, firstCoro);
	}
}

void firstTask(void *context)
{
	int value = 2;
	int num = 0;

	printf("firstTask created with value %d\n", *(int *)context);
	secondCoro = Coro_new();
	Coro_startCoro_(firstCoro, secondCoro, (void *)&value, secondTask);

	while (1)
	{
		printf("firstTask:  %d %d\n", (int)Coro_bytesLeftOnStack(firstCoro), num++);
		Coro_switchTo_(firstCoro, secondCoro);
		if (num == 5)
		  exit (0);
	}
}

int main()
{
	Coro *mainCoro = Coro_new();
	int value = 1;

	Coro_initializeMainCoro(mainCoro);

	firstCoro = Coro_new();
	Coro_startCoro_(mainCoro, firstCoro, (void *)&value, firstTask);
}
