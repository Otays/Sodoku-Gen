/*==============================================================================
 *	Sodoku Generator
 *
 *	Description: 
 *		This program creates sodoku puzzles!
 *		Written for compatability with g++ (GCC) 4.8.5
 *
 *	Author: Ben Pogrund
 *
 *	Date: 5/24/2017
 *
 *============================================================================*/

#include <stdlib.h>		// rand, srand, malloc
#include <string.h>		// strcpy
#include <fstream>		// printf, time
#include <iostream>		// cin
#include <limits>		// cin control
#include <sys/time.h>	// gettimeofday
#include "colorlogs.h"	// LOG_COLOR() functions
 
/* Macros */
#define subwidth_POW2 sub_width*sub_width
#define subwidth_POW4 sub_width*sub_width*sub_width*sub_width

/* Global variables */
short  sub_width;			// Block width aka region width
short  difficulty_level;	// (1 Easy, 2 Normal)
short  output_total;		// Total number of puzzles to generate
short* main_puzzle;			// 1D Array : size = [sub_width ^4]
short* solved_puzzle;		// 1D Array : size = [sub_width ^4]
bool** available_space;		// 2D Array : size = [sub_width ^2] [sub_width ^4]

/* prototypes */
void  prompt();
bool  invalid_sub_width();
bool  invalid_difficulty();
bool  invalid_number();
void  allocate_puzzle_memory();
void  init_memory();
void  create_puzzle();
void  insert_value(short x, short y, short val);
short get_index(short x, short y);
bool  has_unique_solution();
bool  update_solution();
void  advanced_availability_check();
void  prune_puzzle();
void  print_puzzle(short index);
void  free_memory();


/*=============================================================================
 *	main()
 *===========================================================================*/
int main() 
{
	double runtime;
	timeval start, substart, end;
	srand (time(NULL));
	
	// Set up phase
	prompt();								// Take user input 
	gettimeofday(&start, NULL);				// full runtime timer
	short total_files = output_total;
	do {
		gettimeofday(&substart, NULL);		// single puzzle timer
		allocate_puzzle_memory();
		init_memory();
		
		// Creation phase
		create_puzzle();
		if (difficulty_level > 1) prune_puzzle();
		
		// Output phase
		print_puzzle(output_total - total_files + 1);
		gettimeofday(&end, NULL);			// substart timer

		runtime = end.tv_sec + end.tv_usec / 1000000.0;
		runtime -= substart.tv_sec + substart.tv_usec / 1000000.0;
		printf(" (in %.4f sec)\n", runtime);
	} while (--total_files > 0);
	
	printf("\n");
	runtime = end.tv_sec + end.tv_usec / 1000000.0;
	runtime -= start.tv_sec + start.tv_usec / 1000000.0;
	LOG_GREEN(" Created %d puzzles ", output_total); 
	printf(" (%.4f sec)\n\n", runtime);
	
	// Exit phase
	free_memory();
	return 0;
}

/*=============================================================================
 *	prompt user input
 *===========================================================================*/
void prompt()
{
	LOG_WHITE("\n");
	LOG_WHITE("--------------------------------------------\n");
	LOG_CRIM (" Welcome "); 
	LOG_CYAN ("                     version 1.0.0 \n");
	LOG_WHITE("--------------------------------------------\n");
	
	printf("\n");
	printf("This program creates sodoku puzzles.  Please\n");
	printf("select a block size.\n");
	printf("\n");
	
	do {
		LOG_WHITE("Block size? (2,3,4) \n"); 
		LOG_CRIM (" > "); 
		
		if (!(std::cin >> sub_width)) { 
			std::cin.clear(); 
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		
		printf("\n");
	} while (invalid_sub_width());
	
	do {
		LOG_WHITE("Select difficulty (1: Easy) \n"); 
		LOG_WHITE("                  (2: Normal) \n"); 
		LOG_CRIM (" > "); 
		
		if (!(std::cin >> difficulty_level)) { 
			std::cin.clear(); 
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		
		printf("\n");
	} while (invalid_difficulty());
	
	do {
		LOG_WHITE("How many puzzles?\n"); 
		LOG_CRIM (" > "); 
		
		if (!(std::cin >> output_total)) { 
			std::cin.clear(); 
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		
		printf("\n");
	} while (invalid_number());
	
	LOG_GREEN(" [Generating Sodoku]\n");
}

/*=============================================================================
 *	input validation check
 *===========================================================================*/
bool invalid_sub_width()
{
	switch (sub_width)
	{
		case 2:
		case 3:
		case 4:
			return false;
		default:
			LOG_CRIM ("Invalid\n"); 
			return true;
	}
}

/*=============================================================================
 *	input validation check (part 2)
 *===========================================================================*/
bool invalid_difficulty()
{
	switch (difficulty_level)
	{
		case 1:
		case 2:
			return false;
		default:
			LOG_CRIM ("Invalid\n"); 
			return true;
	}
}

/*=============================================================================
 *	input validation check (part 3)
 *===========================================================================*/
bool invalid_number()
{
	if (output_total > 0) return false;
	LOG_CRIM ("Invalid\n"); 
	return true;
}

/*=============================================================================
 *	allocate puzzle memory
 *===========================================================================*/
void allocate_puzzle_memory()
{
	available_space = new bool* [subwidth_POW2];
	main_puzzle = new short[subwidth_POW4];
	solved_puzzle = new short[subwidth_POW4];
	
	for (short i = 0; i < subwidth_POW2; i++) {
		available_space[i] = new bool[subwidth_POW4];
	}
}

/*=============================================================================
 *	initialize memory
 *===========================================================================*/
void init_memory()
{
	for (short i = 0; i < subwidth_POW2; i++)
		for (short k = 0; k < subwidth_POW4; k++)
			available_space[i][k] = 0;
	
	for (short i = 0; i < subwidth_POW4; i++)	{
		main_puzzle[i] = 0;
		solved_puzzle[i] = 0;
	}
}

/*=============================================================================
 *	create puzzle
 *===========================================================================*/
void create_puzzle()
{
	short rand_col;
	short rand_row;
	short rand_val;
	
	// some tests show this is a good max out number
	short maxout = subwidth_POW4*sub_width;
	
	// possible unbounded runtime? ruh roh
	do {
		// Randomization
		rand_row = rand() % (subwidth_POW2) + 1;
		rand_col = rand() % (subwidth_POW2) + 1;
		
		// Value possibilities
		short pool[subwidth_POW2];
		short cardinality = 0;
		short index = get_index(rand_col, rand_row);
		for (short i = 0; i < subwidth_POW2; i++) {
			if (available_space[i][index] == 0) {
				pool[cardinality] = i+1;
				cardinality++;
			}
		}
		
		// Value insertion
		if (cardinality > 0) {
			rand_val = pool[rand() % cardinality];
			main_puzzle[get_index(rand_col, rand_row)] = rand_val;
			insert_value(rand_col, rand_row, rand_val);
		}
		
		// Solution updating
		bool updating;
		do {
			updating = update_solution();
		} while (updating);
		
		// Backtrack control
		maxout--;
		if (maxout == 0) {
			init_memory();	// Reset
			maxout = subwidth_POW4*sub_width;
		}
	} while (!has_unique_solution());
}

/*=============================================================================
 *	insert value
 *===========================================================================*/
void insert_value(short x, short y, short val)
{
	solved_puzzle[get_index(x,y)] = val;
	
	val--; // Now using val as index
	
	for (short i = 1; i <= subwidth_POW2; i++) {
		available_space[val][get_index(x,i)] = 1;	// Claim row
		available_space[val][get_index(i,y)] = 1;	// Claim col
	}
	
	for (short k = 0; k < subwidth_POW2; k++)
		available_space[k][get_index(x,y)] = 1;		// Claim spot
	
	// Truncate
	x--; y--;
	x /= sub_width;
	y /= sub_width;
	x *= sub_width;
	y *= sub_width;
	
	for (short i = 1; i <= sub_width; i++)
		for (short j = 1; j <= sub_width; j++)
			available_space[val][get_index(i+x,j+y)] = 1;	// Claim sub square
	
	advanced_availability_check();
	
	/* Check for puzzle validation here? */
}

/*=============================================================================
 *	get index
 *===========================================================================*/
short get_index(short x, short y)
{
	x--; y--;
	return y*subwidth_POW2 + x;
}

/*=============================================================================
 *	unique solution test
 *===========================================================================*/
bool has_unique_solution()
{
	for (short i = 0; i < subwidth_POW4; i++)	{
		if (solved_puzzle[i] == 0) return false;
	}
	return true;
}

/*=============================================================================
 *	prune puzzle
 *===========================================================================*/
void prune_puzzle() 
{
	short removed_index = 0;
	short removed_value = 0;
	
	// for each non zero in main_puzzle
	for (short i = 0; i < subwidth_POW4; i++) {
		if (main_puzzle[i] != 0) {
			// save element info
			removed_index = i;
			removed_value = main_puzzle[i];
			
			// remove the element
			main_puzzle[i] = 0;
			
			// reset claim space & known puzzle
			for (short j = 0; j < subwidth_POW2; j++)
				for (short k = 0; k < subwidth_POW4; k++)
					available_space[j][k] = 0;
			
			for (short j = 0; j < subwidth_POW4; j++)	{
				solved_puzzle[j] = 0;
			}
			
			// calculate claim space
			for (short j = 0; j < subwidth_POW4; j++) {
				if (main_puzzle[j] != 0) {
					int x = 1+j % (subwidth_POW2);
					int y = 1+j / (subwidth_POW2);
					insert_value(x, y, main_puzzle[j]);
				}
			}
			
			// calculate known
			bool updating;
			do {
				updating = update_solution();
			} while (updating);
			
			// check for zero elements in known
			// if zero element exists, place the element back
			short k = 0;
			while (k < subwidth_POW4) {
				if (solved_puzzle[k] == 0) {
					main_puzzle[removed_index] = removed_value;
					solved_puzzle[removed_index] = removed_value;
					break;
				}
				k++;
			}
		}
	}
	
	// calculate solution
	for (short j = 0; j < subwidth_POW4; j++) {
		if (main_puzzle[j] != 0) {
			int x = 1+j % (subwidth_POW2);
			int y = 1+j / (subwidth_POW2);
			insert_value(x, y, main_puzzle[j]);
		}
	}
	bool updating;
	do {
		updating = update_solution();
	} while (updating);
	
}

/*=============================================================================
 *	update solution
 *===========================================================================*/
bool update_solution()
{
	bool updating = false;	// True if more updates needed
	
	// Value loop
	for (short k = 0; k < subwidth_POW2; k++)	{
		// Diagonal crawl
		for (short j = 1; j <= subwidth_POW2; j++)	{
			// Scan rows and columns
			short xopenings = subwidth_POW2;
			short yopenings = subwidth_POW2;
			short xindex = 0;
			short yindex = 0;
			
			for (short i = 1; i <= subwidth_POW2; i++) {
				if (available_space[k][get_index(i,j)] == 1) {
					xopenings--;	// Count row openings
				} else {
					xindex = i;		// Save index for later
				}
				if (available_space[k][get_index(j,i)] == 1) {
					yopenings--;	// Count column openings
				} else {
					yindex = i;		// Save index for later
				}
			}
			
			if (xopenings == 1) {
				insert_value(xindex, j, k+1);
				updating = true;
			} else if (yopenings == 1) {
				insert_value(j, yindex, k+1);
				updating = true;
			}
		}
		
		// Scan blocks
		for (short i = 0; i < subwidth_POW2; i += sub_width) {
			for (short j = 0; j < subwidth_POW2; j += sub_width) {	
				short subopenings = subwidth_POW2;
				short xindex;
				short yindex;
				
				for (short y = 1; y <= sub_width; y++) {
					for (short x = 1; x <= sub_width; x++) {
						if (available_space[k][get_index(x+i, y+j)] == 1) {
							subopenings--;		// Count block openings
						} else {
							xindex = x+i;		// Save indices
							yindex = y+j;
						}
					}
				}
				
				if (subopenings == 1) { 
					insert_value(xindex, yindex, k+1);
					updating = true;
				}
			}
		}
	}
	
	return updating;
}

/*=============================================================================
 *	advanced availability check
 *	
 *	description: This detects if a value must be somewhere in a column/row
 *				 and then updates the rest of that column/row as unavailable.
 *===========================================================================*/
void advanced_availability_check()
{
	// Iterate values
	for (short k = 0; k < subwidth_POW2; k++)	{
		// Iterate blocks
		for (short j = 0; j < subwidth_POW2; j += sub_width) {
			for (short i = 0; i < subwidth_POW2; i += sub_width) {	
				short subopenings = subwidth_POW2;
				short open_columns = sub_width;
				short col_index = 0;
				
				// Iterate cells
				for (short x = 1; x <= sub_width; x++) {
					int empty_column_cells = sub_width;
					for (short y = 1; y <= sub_width; y++) {
						if (available_space[k][get_index(x+i, y+j)] != 0) {
							subopenings--;				// Count block openings
							empty_column_cells--;		// Count open columns
						} 
					}
					if (empty_column_cells == 0) {
						open_columns--;
					} else {
						col_index = x+i;
					}
				}
				
				// Update availability
				if (open_columns == 1 && subopenings > 1) { 
					for (short y = 1; y <= subwidth_POW2; y++) {
						if (y > j+sub_width || y <= j) {
							available_space[k][get_index(col_index, y)] = 1;
						}
					}
				}
				
				subopenings = subwidth_POW2;
				short open_rows = sub_width;
				short row_index = 0;
				
				// Iterate cells
				for (short y = 1; y <= sub_width; y++) {
					int empty_row_cells = sub_width;
					for (short x = 1; x <= sub_width; x++) {
						if (available_space[k][get_index(x+i, y+j)] != 0) {
							subopenings--;			// Count block openings
							empty_row_cells--;		// Count open columns
						} 
					}
					if (empty_row_cells == 0) {
						open_rows--;
					} else {
						row_index = y+j;
					}
				}
				
				// Update availability
				if (open_rows == 1 && subopenings > 1) { 
					for (short x = 1; x <= subwidth_POW2; x++) {
						if (x > i+sub_width || x <= i) {
							available_space[k][get_index(x, row_index)] = 1;
						}
					}
				}
			}
		}
	}
}

/*=============================================================================
 *	print puzzle
 *===========================================================================*/
void print_puzzle(short index)
{
	short ascii_offset = (sub_width != 4) ? 48 : 64;
	short size = 100; 	// sloppy max size
	char* line1 = (char*) malloc(size);
	char* line2 = (char*) malloc(size);
	char* line3 = (char*) malloc(size);
	char* line4 = (char*) malloc(size);
	char* line5 = (char*) malloc(size);
	char* filename = (char*) malloc(size);
	
	std::ofstream outfile;
	switch (difficulty_level) {
		case 1: sprintf (filename, "Easy Sodoku %i (3x3).txt", index);	 break;
		case 2: sprintf (filename, "Normal Sodoku %i (3x3).txt", index); break;
	}
	outfile.open (filename);
	
	switch (sub_width)
	{
		case 4:
			strcpy(line1, "......................................................................................................\n");
			strcpy(line2, "::     :     :     :     ::     :     :     :     ::     :     :     :     ::     :     :     :     ::\n");
			strcpy(line3, "::  _  :  _  :  _  :  _  ::  _  :  _  :  _  :  _  ::  _  :  _  :  _  :  _  ::  _  :  _  :  _  :  _  ::\n");
			strcpy(line4, "::.....:.....:.....:.....::.....:.....:.....:.....::.....:.....:.....:.....::.....:.....:.....:.....::\n");
			strcpy(line5, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
			break;
		case 3:
			strcpy(line1, "...........................................................\n");
			strcpy(line2, "::     :     :     ::     :     :     ::     :     :     ::\n");
			strcpy(line3, "::  x  :  y  :  z  ::  a  :  b  :  c  ::  1  :  2  :  3  ::\n");
			strcpy(line4, "::.....:.....:.....::.....:.....:.....::.....:.....:.....::\n");
			strcpy(line5, ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
			break;
		case 2:	
			strcpy(line1, "............................\n");
			strcpy(line2, "::     :     ::     :     ::\n");
			strcpy(line3, "::  x  :  y  ::  z  :  a  ::\n");
			strcpy(line4, "::.....:.....::.....:.....::\n");
			strcpy(line5, "::::::::::::::::::::::::::::\n");
			break;
	}
	
	outfile << "\nSodoku Puzzle:\n\n";
	for (short i = 0; i < subwidth_POW2; i++)
	{
		if (i % sub_width == 0)
			outfile << line5;
		else
			outfile << line4;
		
		for (short k = 0; k < subwidth_POW2; k++)
		{
			int ascii_index = 6*k+4 + k/sub_width;
			int main_index = i*subwidth_POW2 + k;
			if (main_puzzle[main_index] != 0) {
				line3[ascii_index] = main_puzzle[main_index]+ascii_offset;
			} else {
				line3[ascii_index] = ' ';
			}
		}
		outfile << line2;
		outfile << line3;
		outfile << line2;
	}
	outfile << line5;
	outfile << "\n";
	
	LOG_GREEN(" > ");	LOG_WHITE(filename);	printf("\n");
	
	outfile.close();
	switch (difficulty_level) {
		case 1: sprintf (filename, "Easy SOLVED %i (3x3).txt", index);	 break;
		case 2: sprintf (filename, "Normal SOLVED %i (3x3).txt", index); break;
	}
	outfile.open (filename);
	
	outfile << "\nSolution:\n\n";
	for (short i = 0; i < subwidth_POW2; i++)
	{
		if (i % sub_width == 0)
			outfile << line5;
		else
			outfile << line4;
		
		for (short k = 0; k < subwidth_POW2; k++)
		{
			int ascii_index = 6*k+4 + k/sub_width;
			int main_index = i*subwidth_POW2 + k;
			if (solved_puzzle[main_index] != 0) {
				line3[ascii_index] = solved_puzzle[main_index]+ascii_offset;
			} else {
				line3[ascii_index] = ' ';
			}
		}
		outfile << line2;
		outfile << line3;
		outfile << line2;
	}
	outfile << line5;
	outfile << "\n";
	
	LOG_GREEN(" > ");	LOG_WHITE(filename);
	
	outfile.close();
	free(filename);
	free(line1); free(line2); free(line3); free(line4); free(line5);
}

/*=============================================================================
 *	free memory
 *===========================================================================*/
void free_memory()
{
	for (short i = 0; i < subwidth_POW2; i++)
		delete [] available_space[i];
	
	delete [] available_space;
	delete [] main_puzzle;
	delete [] solved_puzzle;
}

























