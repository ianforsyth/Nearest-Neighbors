//Ian Forsyth
//11.28.2012
//CPSC 445: HW3

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define STORAGE 1000000
#define DEBUG 6
//0: None
//1: Each point, the quad it's in, the points in those quads
//2: Control list after division 
//3: Quads with no children and it's members
//4: Order list after division
//5: Final Output
//6: Time stepper
#define INTERVAL 100000
//Interval for the stepper, check every INTERVAL point
#define SEEK_METHOD 0
//0: Nearest neighbors algorithm
//1: Naive
#define K 8
#define N 300000


const int pnode = 0;
const int x = 1;
const int size = 2;
const int children = 3;
const int start = 4;
const int count = 5;
const int topleft = 6;
const int topright = 7;
const int bottomleft = 8;
const int bottomright = 9;
const int y = 10;

int endIndex = 0, endIndexTwo = 0, endIndexThree = 0;


void MergeSort(double* a, int* neighbors, int left, int right, double x, double y);
void Merge(double* a, int* neighbors, int left, int mid, int right, double x, double y);
void seek_naive(double* a, int n, int k, int* iz);
void seek(double* a, int n, int k, int* iz);
void move(int* order, int index, int moveTo);
void divide(double** control, int* order, double* a, int quad, int n, int k, int parent);
void getQuads(double** control, int* order, double* a, int* quads, int pt, double radius, int quad);

int main()
{
	int i=0,j=0;
	int k = K;
	int n = N;
	int* iz  = (int*) malloc(sizeof(int) * k * n);
	double* a = (double*) malloc(sizeof(double) * 2 * n);
	unsigned int start=0, end=0;
	
	//Create random array of points on the coordinate system
	for(i=0; i<n*2; i++)
			a[i] = rand() / (double)RAND_MAX;
			
	start=time(0);
	
	//Run chosen seek method (based on SEEK_METHOD in header)
	if(SEEK_METHOD == 0)
		seek(a, n, k, iz);
	else if(SEEK_METHOD == 1)
		seek_naive(a, n, k, iz);
	else
	{
		printf("Error: Incorrect SEEK_METHOD chosen\n\n");
		return 0;
	}
	end=time(0);
	
	//**********DEBUG OUPUT**********
	if(DEBUG == 5)
	{
		printf("\nNeighbors:\n");
		for(i=0; i<n; i++)
		{
			printf("Point %d: ", i+1);
			for(j=0; j<k; j++)
			{
				printf("%d ", iz[i*k+j]);
			}
			printf("\n");
		}
		printf("\n");
	}
	
   printf("RUNTIME: %dsecs\n\n", end-start);

	free(iz);
	free(a);
	return 0;
}

void seek(double* a, int n, int k, int* iz)
{
	
	int i, j, q, inQuad=0;
	int* order = (int*) malloc(sizeof(int) * n);
	int* neighbors = (int*) malloc(sizeof(int) * n);
	int* quadsToCheck = (int*) malloc(sizeof(int) * n);
	double radius=0, corners[4];
	
	double** control;  
	control = (double**) malloc(STORAGE*sizeof(double*));  
	for (int i = 0; i < STORAGE; i++)  
	   control[i] = (double*) malloc(11*sizeof(double));
	//static double control[STORAGE][11];
			
	//Create the order array, indexed at 1
	for(i=0; i<n; i++)
		order[i] = i + 1;
	
	//Fill in initial data for the root
	control[0][pnode] = 0;
	control[0][x] = 0;
	control[0][y] = 0;
	control[0][size] = 1;
	control[0][children] = 0; 
	control[0][start] = 0;
	control[0][count] = n;
	control[0][topleft] = 0; 
	control[0][bottomleft] = 0;
	control[0][topright] = 0;
	control[0][bottomright] = 0;
	
	if(control[0][count] < k)
	{
		printf("Error: given list is empty\n");
		return;
	}
	else
	{
		//Initial setup, divide the root into the quad tree
		divide(control, order, a, 0, n, k, -77);
		
		//*****DEBUG OUTPUT*****
		if(DEBUG == 6)
			printf("\n - Quad tree created, division() successful...\n");
		
		//*****DEBUG OUTPUT*****
		if(DEBUG == 2)
			for(i = 17; i < 18; i++)
			{	
				printf("INDEX: %d\n", i);
				printf("\tParent Node: %f\n",control[i][pnode]);
				printf("\tX: %f\n", control[i][x]);
				printf("\tY: %f\n", control[i][y]);
				printf("\tSIZE: %f\n", control[i][size]);
				printf("\tCHILDREN?: %f\n", control[i][children]);
				printf("\tStart: %f\n", control[i][start]);
				printf("\tCount: %f\n", control[i][count]);
				printf("\ttleft: %f\n", control[i][topleft]);
				printf("\tbleft: %f\n", control[i][bottomleft]);
				printf("\ttright: %f\n", control[i][topright]);
				printf("\ttbright: %f\n", control[i][bottomright]);
				printf("\tMEMBERS: ");
				for(j=control[i][start]; j<control[i][start]+control[i][count]; j++)
					printf("%d ", order[j]);
				printf("\n\n");
			}
			
		//*****DEBUG OUTPUT*****
		if(DEBUG ==4)
			for(i=0; i<n; i++)
				printf("%d\n",order[i]);
		
		//*****DEBUG OUTPUT*****
		if(DEBUG == 3)
			for(i = 0; i < 100; i++)
			{
				if(control[i][children] == 0 && control[i][pnode] == 0)
					break;
				
				if(control[i][children] == 0)
				{
					printf("%d: ", i);
					for(j=control[i][start]; j<control[i][start]+control[i][count]; j++)
						printf("%d ", order[j]);
					printf("\n");
				}
			}
		
		//Loop through every point, finds its neighbors
		for(i=0; i<n; i++)
		{
			//*****DEBUG OUTPUT*****
			if(DEBUG == 6 && (i+1) % INTERVAL == 0)
				printf(" - At point %d ..\n", i+1);
				
			//Find the smallest quad that contains the point
			for(j=0; j<STORAGE; j++)
			{
				if(inQuad != 0)
					break;
				
				if(a[(order[i]-1)*2] > control[j][x] && a[(order[i]-1)*2] <= control[j][x]+control[j][size]
			  && a[(order[i]-1)*2+1] > control[j][y] && a[(order[i]-1)*2+1] <= control[j][y]+control[j][size])
				{
					if(control[j][children] ==  0)
						inQuad = j; 
					else
						j = control[j][topleft]-1;
				}
			}
			
			//*****DEBUG OUTPUT*****
			if(DEBUG == 6 && (i+1) % INTERVAL == 0)
				printf("\tSmallest quad containg point %d found...\n", i+1);
			
			//Find furthest corner away from the point
			double cornerTwo, cornerThree, corner4;	
			corners[0] = sqrt(pow((control[inQuad][x] - a[(order[i]-1)*2] ), 2) + pow((control[inQuad][y]) - a[(order[i]-1)*2+1], 2));
			corners[1] = sqrt(pow((control[inQuad][x]+control[inQuad][size] - a[(order[i]-1)*2]), 2) + pow((control[inQuad][y] - a[(order[i]-1)*2+1]), 2));
			corners[2] = sqrt(pow((control[inQuad][x] - a[(order[i]-1)*2]), 2) + pow((control[inQuad][y]+control[inQuad][size] - a[(order[i]-1)*2+1]), 2));
			corners[3] = sqrt(pow((control[inQuad][x]+control[inQuad][size] - a[(order[i]-1)*2]), 2) + pow((control[inQuad][y]+control[inQuad][size] - a[(order[i]-1)*2+1]), 2));
			
			//Set radius equal to furthest corner distance
			radius = corners[0];
			for(j=1; j<4; j++)
				if (radius < corners[j])
					radius = corners[j];
					
			//Get the quads that circle contacts (in quadsToCheck)
			getQuads(control, order, a, quadsToCheck, i, radius, 0);
			
			//*****DEBUG OUTPUT*****
			if(DEBUG == 6 && (i+1) % INTERVAL == 0)
				printf("\tAll quads in point %d's radius found...\n", i+1);

			//**********DEBUG OUTPUT**********
			if(DEBUG == 1)
			{
				printf("\tNEIGHBOR QUADS\n");
				for(j=0; j<100; j++)
				{
					if(quadsToCheck[j] == 0)
						break;
					printf("\t%d. %d - ", j+1, quadsToCheck[j]);
					for(q=control[quadsToCheck[j]][start]; q<control[quadsToCheck[j]][start]+ control[quadsToCheck[j]][count]; q++)
						printf("%d ", order[q]);
					printf("\n");
				}
			}
			
			//**********DEBUG OUTPUT**********
			if(DEBUG == 1)
				printf("\tNEIGHBORS: ");

			//Get suspect list
			for(j=0; j<n; j++)
			{
				if(quadsToCheck[j] == 0)
					break;
					
				for(q=control[quadsToCheck[j]][start]; q<control[quadsToCheck[j]][start]+control[quadsToCheck[j]][count]; q++)
				{
					//**********DEBUG OUTPUT**********
					if(DEBUG == 1)
						printf("%d ", order[q]);
					neighbors[endIndexThree] = order[q];
					endIndexThree ++;
				}
			}
			
			//*****DEBUG OUTPUT*****
			if(DEBUG == 6 && (i+1) % INTERVAL == 0)
				printf("\tSuspect list for point %d found...\n", i+1);
			
			//**********DEBUG OUTPUT**********
			if(DEBUG == 1)
				printf("\n\n");
			
			//Sort the neighbors by least distance
			MergeSort(a, neighbors, 0, endIndexThree-1, a[(order[i]-1)*2], a[(order[i]-1)*2+1]);
			
			//*****DEBUG OUTPUT*****
			if(DEBUG == 6 && (i+1) % INTERVAL == 0)
				printf("\tNeighbors sorted...\n", i+1);
			
			//Insert k smallest distances into iz
			//Skip neighbors[0] because that will always be the point itself
			for(j=0; j<k; j++)
					iz[(order[i]-1)*k+j] = neighbors[j+1];
			
			//Reset session variables
			q=0;
			inQuad = 0; 
			endIndexTwo = 0;
			endIndexThree = 0;
			for(j=0; j<n; j++)
			{
				quadsToCheck[j] = 0;
				neighbors[j] = 0;
			}
		}
	}
	 
	//Free malloced storage
	for (int i = 0; i < STORAGE; i++)  
		free(control[i]);
	free(control);
	free(order);
	free(quadsToCheck);
	free(neighbors);
}

void getQuads(double** control, int* order, double* a, int* quads, int pt, double radius, int quad)
{
	
	int i, j, cornerCheck=0, axisCheck=0;
	double corners[4], axis[4][2];

	//Calculate distance to the four corners
  corners[0] = sqrt(pow((control[quad][x] - a[(order[pt]-1)*2]), 2) + pow((control[quad][y] - a[(order[pt]-1)*2+1]), 2));
	corners[1] = sqrt(pow((control[quad][x]+control[quad][size] - a[(order[pt]-1)*2]), 2) + pow((control[quad][y] - a[(order[pt]-1)*2+1]), 2));
	corners[2] = sqrt(pow((control[quad][x] - a[(order[pt]-1)*2]), 2) + pow((control[quad][y]+control[quad][size] - a[(order[pt]-1)*2+1]), 2));
	corners[3] = sqrt(pow((control[quad][x]+control[quad][size] - a[(order[pt]-1)*2]), 2) + pow((control[quad][y]+control[quad][size] - a[(order[pt]-1)*2+1]), 2));
	
	//Calculate the axis points of the circle
	axis[0][0] = a[(order[pt]-1)*2];
	axis[0][1] = a[(order[pt]-1)*2+1] + radius;
	axis[1][0] = a[(order[pt]-1)*2] + radius;
	axis[1][1] = a[(order[pt]-1)*2+1];
	axis[2][0] = a[(order[pt]-1)*2];
	axis[2][1] = a[(order[pt]-1)*2+1] - radius;
	axis[3][0] = a[(order[pt]-1)*2] - radius;
	axis[3][1] = a[(order[pt]-1)*2+1];
	
	//Test the corners
	for(i=0; i<4; i++)
		if(corners[i] <= radius)
			cornerCheck ++;
	
	//Test the axes
	for(i=0; i<4; i++)
		if(axis[i][0] >= control[quad][x] && axis[i][0] <= control[quad][x]+control[quad][size]
			&& axis[i][1] >= control[quad][y] && axis[i][1] <= control[quad][y]+control[quad][size])
			axisCheck++;
	
	//If all four corners are contained, no need to drill down
	if(cornerCheck == 4)
	{
		quads[endIndexTwo] = quad;
		endIndexTwo++;
	}
	//If at least one corner is in circle or one axis falls in the quad - add it
	else if(cornerCheck > 0 || axisCheck > 0)
	{
		if(control[quad][children] == 0)
		{
			quads[endIndexTwo] = quad;
			//printf("QUAD: %d\n", quads[endIndexTwo]);
			endIndexTwo++;
		}
		//Recurse 
		else
		{
			getQuads(control, order, a, quads, pt, radius, (int)control[quad][topleft]);
			getQuads(control, order, a, quads, pt, radius, (int)control[quad][bottomleft]);
			getQuads(control, order, a, quads, pt, radius, (int)control[quad][topright]);
			getQuads(control, order, a, quads, pt, radius, (int)control[quad][bottomright]);
		}
	}
}



void divide(double** control, int* order, double* a, int quad, int n, int k, int parent)
{
	int i,j;
	int tleft, bleft, tright, bright;
	int* inQuad = (int*) malloc(sizeof(int) * n);;
	
	if(control[quad][count] > k)
	{		
		//Set 'has children' to true and the parent node
		control[quad][children] = 1;
		control[quad][pnode] = parent;
		
		//------------------------------------------------------------------------
		//------------------------------------------------------------------------
		//------------------------------------------------------------------------
		
		//Set some preliminary tleft data
		tleft = endIndex + 1;
		control[quad][topleft] = tleft;
		control[tleft][pnode] = quad;
		control[tleft][x] = control[quad][x];
		control[tleft][y] = control[quad][y] + control[quad][size]/2;
		control[tleft][size] = control[quad][size]/2;
		control[tleft][children] = 0; 
		control[tleft][topleft] = 0; 
		control[tleft][topright] = 0; 
		control[tleft][bottomleft] = 0;
		control[tleft][bottomright] = 0;
		
		//Puts all indexes (of points in order[]) included in the top left boundary into inQuad
		j=0;
		for(i=control[quad][start]; i<control[quad][start]+control[quad][count]; i++)
		{					
			if(a[(order[i]-1)*2] > control[tleft][x] && a[(order[i]-1)*2] <= control[tleft][x]+control[tleft][size]
				&& a[(order[i]-1)*2+1] > control[tleft][y] && a[(order[i]-1)*2+1] <= control[tleft][y]+control[tleft][size])
			{
				inQuad[j] = i;
				j++;
			}
		}
		//Set the start and size of the tleft cluster of points
		control[tleft][start] = control[quad][start];
		control[tleft][count] = j;
		
		//Re-order order[] with the included points bunched together
		j=0;
		for(i=control[tleft][start]; i<control[tleft][start]+control[tleft][count]; i++)
		{
			move(order, inQuad[j], i);
			j++;
		}
		
		//------------------------------------------------------------------------
		//------------------------------------------------------------------------
		//------------------------------------------------------------------------
		
		//Set some preliminary bleft data
		bleft = endIndex + 2;
		control[quad][bottomleft] = bleft;
		control[bleft][pnode] = quad;
		control[bleft][x] = control[quad][x];
		control[bleft][y] = control[quad][y];
		control[bleft][size] = control[quad][size]/2;
		control[bleft][children] = 0; 
		control[bleft][topleft] = 0; 
		control[bleft][topright] = 0; 
		control[bleft][bottomleft] = 0;
		control[bleft][bottomright] = 0;
		
		//Puts all indexes (of points in order[]) included in the bottom left boundary into inQuad
		j=0;
		control[bleft][start] = control[quad][start]+control[tleft][count];
		
		for(i=control[bleft][start]; i<control[quad][start]+control[quad][count]; i++)
		{
			if(a[(order[i]-1)*2] > control[bleft][x] && a[(order[i]-1)*2] <= control[bleft][x]+control[bleft][size]
				&& a[(order[i]-1)*2+1] > control[bleft][y] && a[(order[i]-1)*2+1] <= control[bleft][y]+control[bleft][size])
			{
				inQuad[j] = i;
				j++;
			}
		}
		
		control[bleft][count] = j;
		
		//Re-order order[] with the included points bunched together
		j=0;
		for(i=control[bleft][start]; i<control[bleft][start]+control[bleft][count]; i++)
		{
			move(order, inQuad[j], i);
			j++;
		}
		//------------------------------------------------------------------------
		//------------------------------------------------------------------------
		//------------------------------------------------------------------------
		
		//Set some preliminary tright data
		tright = endIndex + 3;
		control[quad][topright] = tright;
		control[tright][pnode] = quad;
		control[tright][x] = control[quad][x] + control[quad][size]/2;
		control[tright][y] = control[quad][y] + control[quad][size]/2;
		control[tright][size] = control[quad][size]/2;
		control[tright][children] = 0; 
		control[tright][topleft] = 0; 
		control[tright][topright] = 0; 
		control[tright][bottomleft] = 0;
		control[tright][bottomright] = 0;
		
		//Puts all indexes (of points in order[]) included in the top right boundary into inQuad
		j=0;
		control[tright][start] = control[quad][start]+control[tleft][count]+control[bleft][count];
	
		for(i=control[tright][start]; i<control[quad][start]+control[quad][count]; i++)
		{
			if(a[(order[i]-1)*2] > control[tright][x] && a[(order[i]-1)*2] <= control[tright][x]+control[tright][size]
				&& a[(order[i]-1)*2+1] > control[tright][y] && a[(order[i]-1)*2+1] <= control[tright][y]+control[tright][size])
			{	
				inQuad[j] = i;
				j++;
			}
		}
		
		control[tright][count] = j;
		
		
		//Re-order order[] with the included points bunched together
		j=0;
		for(i=control[tright][start]; i<control[tright][start] + control[tright][count]; i++)
		{
			move(order, inQuad[j], i);
			j++;
		}
		
		//------------------------------------------------------------------------
		//------------------------------------------------------------------------
		//------------------------------------------------------------------------
		
		//Set the start and size of the bright cluser of points
		bright = endIndex + 4;
		control[quad][bottomright] = bright;
		control[bright][pnode] = quad;
		control[bright][x] = control[quad][x] + control[quad][size]/2;
		control[bright][y] = control[quad][y];
		control[bright][size] = control[quad][size]/2;
		control[bright][children] = 0; 
		control[bright][topleft] = 0; 
		control[bright][topright] = 0; 
		control[bright][bottomleft] = 0;
		control[bright][bottomright] = 0;
		
		//Puts all indexes (of points in order[]) included in the top right boundary into inQuad
		j=0;
		control[bright][start] = control[quad][start]+control[tleft][count]+control[bleft][count]+control[tright][count];
	
		for(i=control[bright][start]; i<control[quad][start]+control[quad][count]; i++)
		{
			if(a[(order[i]-1)*2] > control[bright][x] && a[(order[i]-1)*2] <= control[bright][x]+control[bright][size]
				&& a[(order[i]-1)*2+1] > control[bright][y] && a[(order[i]-1)*2+1] <= control[bright][y]+control[bright][size])
			{	
				inQuad[j] = i;
				j++;
			}
		}
		
		control[bright][count] = j;
		
		//Re-order order[] with the included points bunched together
		j=0;
		for(i=control[bright][start]; i<control[bright][start] + control[bright][count]; i++)
		{
			//printf("Insert %d at %d\n", inQuad[j], i);
			move(order, inQuad[j], i);
			j++;
		}
		
		//------------------------------------------------------------------------
		//------------------------------------------------------------------------
		//------------------------------------------------------------------------
		
		//Check there's enough room in the array
		
		endIndex += 4;
		if(endIndex+5 > STORAGE)
		{
			printf("Your array is not big enough to store all the quads. Increase STORAGE.\n");
			exit(0);
		}
			
		//Recurse
		divide(control, order, a, (int)control[quad][topleft], n, k, (int)quad);
		divide(control, order, a, (int)control[quad][bottomleft], n, k, (int)quad);
		divide(control, order, a, (int)control[quad][topright], n, k, (int)quad);
		divide(control, order, a, (int)control[quad][bottomright], n, k, (int)quad);
	}
	free(inQuad);
}

//Moves a value in an array (index) to a new location and
//shifts the elements left or right.
void move(int* order, int index, int moveTo)
{
	int i, holder;
	
	if(index < moveTo)
	{
		holder = order[index];
		for(i=index; i<moveTo; i++)
			order[i] = order[i+1];
		order[moveTo] = holder;
	}
	else
	{
		holder = order[index];
		for(i=index; i>moveTo; i--)
			order[i] = order[i-1];
		order[moveTo] = holder;
	}
}

void seek_naive(double* a, int n, int k, int* iz)
{
	int i, j, q;
	
	for(i = 0; i < n; i++)
	{
		int* neighbors = (int*) malloc(sizeof(int) * n);
		
		for(j = 0; j < n; j++)
			neighbors[j] = j+1;
		
		MergeSort(a, neighbors, 0, n-1, a[i*2], a[i*2+1]);
	
		for(j=0; j<k; j++)
			iz[i*k+j] = neighbors[j+1];
			
		free(neighbors);
	}		
}

//Special Mergesort that mimics the distance sort in neighbor array.
void MergeSort(double* a, int* neighbors, int left, int right, double x, double y)
{
	int mid = (left+right)/2;
  if(left<right)
  {
  	MergeSort(a, neighbors, left, mid, x, y);
    MergeSort(a, neighbors, mid+1, right, x, y);
    Merge(a, neighbors, left, mid, right, x, y);
  }
}

void Merge(double *a, int* neighbors, int left, int mid, int right, double x, double y)
{
	int i;
	int pos = 0;
	int lpos = left;
	int rpos = mid + 1;
	double tempDistance[right-left+1];
	int tempNeighbor[right-left+1];
	double lDistance=0, rDistance=0;
	
	while(lpos <= mid && rpos <= right)
	{
		lDistance = pow((x-a[(neighbors[lpos]-1)*2]), 2) + pow((y-a[(neighbors[lpos]-1)*2+1]), 2);
		rDistance = pow((x-a[(neighbors[rpos]-1)*2]), 2) + pow((y-a[(neighbors[rpos]-1)*2+1]), 2);
		
		if(lDistance < rDistance)
			tempNeighbor[pos++] = neighbors[lpos++];
		else
			tempNeighbor[pos++] = neighbors[rpos++];
	}
	
	while(lpos <= mid)
	{
		tempNeighbor[pos++] = neighbors[lpos++];
	}

	while(rpos <= right)
	{
		tempNeighbor[pos++] = neighbors[rpos++];
	}

	for(i = 0; i < pos; i++)
	{
		neighbors[i+left] = tempNeighbor[i];
	}
	return;
}