/* This tries to be an efficient solution to the NQUEENS problem.
 *
 * It must still trie harder, but for most sizes up to 200 results are returned
 * immediately.
 *
 * Input is read from stdin as per http://spoj.com/problems/NQUEEN/
 *
 * I've deleted eight lines, and changed two others, so it can't resubmitted
 * by anyone without understanding.
 *
 * AND there is still a bug hiding somewhere that I haven't found myself...
 */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXSIZE 510

#ifdef ONLINE_JUDGE
#define DEBUG(...) ({})
#else
#define DEBUG printf
#endif

struct link {
	unsigned char next;
	unsigned char prev;
};

struct cell {
	struct link ns;
	struct link we;
	struct link nwse;
	struct link swne;
};

struct file {
	int size;
	unsigned char next;
	unsigned char prev;
};

static int problemsize;
static int solution[MAXSIZE+2];
static int num_queens_remaining;
static struct cell cell[MAXSIZE+2][MAXSIZE+2];
static struct {
	struct file ns[MAXSIZE+2];
	struct file we[MAXSIZE+2];
	struct file nwse[2*MAXSIZE-1+2];
	struct file swne[2*MAXSIZE-1+2];
} files;
/* number of remaining rows, columns, and diagonals */
static struct {
	int ns;
	int we;
	int nwse;
	int swne;
} count;

#define IGN +0*
#define OPS_NS   +,   IGN, ns
#define OPS_WE   IGN, +,   we
#define OPS_NWSE +,   +,   nwse
#define OPS_SWNE -,   +,   swne

#define FILE_NS files.ns[j]
#define FILE_WE files.we[i]
#define FILE_NWSE files.nwse[i+problemsize+1-j-1]
#define FILE_SWNE files.swne[i+j-1]

#define UNLINK_FILE(file, count) \
  (&file)[-file.prev].next = (&file)[file.next].prev = file.next + file.prev, \
  (--count < num_queens_remaining ? (possible = 0) : 0), \
  DEBUG("delfile %d %d %s\n", i, j, #file)
#define RELINK_FILE(file, count) \
  (&file)[-file.prev].next = file.prev, (&file)[file.next].prev = file.next, \
  (count++), \
  DEBUG("addfile %d %d %s\n", i, j, #file)

#define DEC(file, count) (--file.size ? 0 : (UNLINK_FILE(file, count))), \
	0/*DEBUG("Decreased %s, size: %d\n", #file, file.size)
	   */
#define INC(file, count) (file.size++ ? 0 : (RELINK_FILE(file, count))), \
	0/*DEBUG("Increased %s, size: %d\n", #file, file.size)
	   */

#define DEC_NS   DEC(FILE_NS, count.ns)
#define DEC_WE   DEC(FILE_WE, count.we)
#define DEC_NWSE DEC(FILE_NWSE, count.nwse)
#define DEC_SWNE DEC(FILE_SWNE, count.swne)
#define INC_NS   INC(FILE_NS, count.ns)
#define INC_WE   INC(FILE_WE, count.we)
#define INC_NWSE INC(FILE_NWSE, count.nwse)
#define INC_SWNE INC(FILE_SWNE, count.swne)

#define CELL cell[i][j]
#define DO_UNLINK_CELL(op1, op2, dir) \
  cell[i op1 -CELL.dir.prev][j op2 -CELL.dir.prev].dir.next = \
  cell[i op1 +CELL.dir.next][j op2 +CELL.dir.next].dir.prev = \
    CELL.dir.prev + CELL.dir.next
#define DO_RELINK_CELL(op1, op2, dir) \
  cell[i op1 -CELL.dir.prev][j op2 -CELL.dir.prev].dir.next = CELL.dir.prev, \
  cell[i op1 +CELL.dir.next][j op2 +CELL.dir.next].dir.prev = CELL.dir.next

#define UNLINK_CELL(ops) DO_UNLINK_CELL(ops), DEBUG("del %d %d %s\n", i, j, #ops)
#define RELINK_CELL(ops) DO_RELINK_CELL(ops), DEBUG("add %d %d %s\n", i, j, #ops)

#define UNLINK_NS   UNLINK_CELL(OPS_NS),   DEC_NS
#define UNLINK_WE   UNLINK_CELL(OPS_WE),   DEC_WE
#define UNLINK_NWSE UNLINK_CELL(OPS_NWSE), DEC_NWSE
#define UNLINK_SWNE UNLINK_CELL(OPS_SWNE), DEC_SWNE
#define RELINK_NS   RELINK_CELL(OPS_NS),   INC_NS
#define RELINK_WE   RELINK_CELL(OPS_WE),   INC_WE
#define RELINK_NWSE RELINK_CELL(OPS_NWSE), INC_NWSE
#define RELINK_SWNE RELINK_CELL(OPS_SWNE), INC_SWNE

#define UNLINK_ALL \
	UNLINK_CELL(OPS_NS), UNLINK_CELL(OPS_WE), \
	UNLINK_CELL(OPS_NWSE), UNLINK_CELL(OPS_SWNE), \
	UNLINK_FILE(FILE_NS, count.ns), UNLINK_FILE(FILE_WE, count.we), \
	UNLINK_FILE(FILE_NWSE, count.nwse), UNLINK_FILE(FILE_SWNE, count.swne)
#define RELINK_ALL \
	RELINK_CELL(OPS_NS), RELINK_CELL(OPS_WE), \
	RELINK_CELL(OPS_NWSE), RELINK_CELL(OPS_SWNE), \
	RELINK_FILE(FILE_NS, count.ns), RELINK_FILE(FILE_WE, count.we), \
	RELINK_FILE(FILE_NWSE, count.nwse), RELINK_FILE(FILE_SWNE, count.swne)

#define UNLINK_BUT_NS   UNLINK_WE, UNLINK_NWSE, UNLINK_SWNE
#define UNLINK_BUT_WE   UNLINK_NS, UNLINK_WE, UNLINK_SWNE
#define UNLINK_BUT_NWSE UNLINK_NS, UNLINK_WE,   UNLINK_SWNE
#define UNLINK_BUT_SWNE UNLINK_NS, UNLINK_WE,   UNLINK_NWSE
#define RELINK_BUT_NS   RELINK_WE, RELINK_NWSE, RELINK_SWNE
#define RELINK_BUT_WE   RELINK_NS, RELINK_WE, RELINK_SWNE
#define RELINK_BUT_NWSE RELINK_NS, RELINK_WE,   RELINK_SWNE
#define RELINK_BUT_SWNE RELINK_NS, RELINK_WE,   RELINK_NWSE

#define DO_STEP(op1, op2, dir) \
  ({ unsigned char __d = CELL.dir.next; i = i op1 __d; j = j op2 __d; })

#define STEP(ops) DO_STEP(ops)

#define STEP_NS   STEP(OPS_NS)
#define STEP_WE   STEP(OPS_WE)
#define STEP_NWSE STEP(OPS_NWSE)
#define STEP_SWNE STEP(OPS_SWNE)

#define FOREACH_NS for (i = 0, j = y; STEP_NS, i <= problemsize;)
#define FOREACH_WE for (i = x, j = 0; STEP_WE, j <= problemsize;)
#define FOREACH_NWSE_UPPER for (i = 0, j = y-x; STEP_NWSE, j <= problemsize;)
#define FOREACH_NWSE_LOWER for (i = x-y, j = 0; STEP_NWSE, i <= problemsize;)
#define FOREACH_SWNE_UPPER for (i = x+y, j = 0; STEP_SWNE, i >= 1;)
#define FOREACH_SWNE_LOWER for (i = problemsize+1, j = y-(problemsize+1-x); STEP_SWNE, j<= problemsize;)


static int try_cell(int x, int y);
static int algorithm(void);


static int place_queen(int x, int y)
{
	int i = x, j = y;
	int possible = 1;

	DEBUG("PLACE %d %d\n", x, y);

	assert(solution[x] == 0);
	solution[x] = y;
	num_queens_remaining -= 1;

	UNLINK_ALL;

	FOREACH_NS UNLINK_BUT_NS;
	FOREACH_WE UNLINK_BUT_WE;
	FOREACH_NWSE_UPPER UNLINK_BUT_NWSE;
	FOREACH_NWSE_LOWER UNLINK_BUT_NWSE;
	FOREACH_SWNE_LOWER UNLINK_BUT_SWNE;
	FOREACH_SWNE_UPPER UNLINK_BUT_SWNE;

	return possible;
}

static void remove_queen(int x, int y)
{
	int i = x, j = y;

	DEBUG("REMOVE %d %d\n", x, y);

	assert(solution[x] == y);
	solution[x] = 0;
	num_queens_remaining += 1;

	FOREACH_NS RELINK_BUT_NS;
	FOREACH_WE RELINK_BUT_WE;
	FOREACH_NWSE_UPPER RELINK_BUT_NWSE;
	FOREACH_NWSE_LOWER RELINK_BUT_NWSE;
	FOREACH_SWNE_LOWER RELINK_BUT_SWNE;
	FOREACH_SWNE_UPPER RELINK_BUT_SWNE;

	i = x, j = y;
	RELINK_ALL;
}

static int try_cell(int x, int y)
{
	int r;

	r = place_queen(x, y);

	if (!r)
		DEBUG("Recognized impossible situation early\n");

	if (r && algorithm())
		/* Shortcut ! */
		return 1;

	remove_queen(x, y);

	return 0;
}

static int algorithm(void)
{
	int row, col, minsize, minrow;

	if (num_queens_remaining == 0)
		return 1;

	if (files.we[0].next > problemsize)
		return 0;

	/* choose row r */

	minrow = -1;
	minsize = problemsize+1;
	for (row = 0; (row += files.we[row].next) <= problemsize;) {
		if (minsize > files.we[row].size) {
			minsize = files.we[row].size;
			minrow = row;
		}
	}
	assert(minrow != -1);
	row = minrow;

	DEBUG("trying %d\n", row);

	/* for each cell in the row... */

	col = 0;
	for (;;) {
		col += cell[row][col].we.next;

		if (col == problemsize+1)
			return 0;

		if (try_cell(row, col))
			return 1;
	}

	assert(0);
}

static int parse(void)
{
	int i, j;

	if (scanf("%d", &problemsize) != 1)
		return 0;
	if (problemsize > MAXSIZE) {
		fprintf(stderr, "Problem size is bigger than MAXSIZE\n");
		exit(1);
	}


	/* Reset */

	for (i = 1; i <= problemsize; i++)
		solution[i] = 0;
	num_queens_remaining = problemsize;

	for (i = 0; i < problemsize+2; i++)
		for (j = 0; j < problemsize+2; j++)
			CELL.ns.prev = CELL.ns.next = \
			CELL.we.prev = CELL.we.next = \
			CELL.nwse.prev = CELL.nwse.next = \
			CELL.swne.prev = CELL.swne.next = 1;

	for (i = 0; i < problemsize+2; i++)
		files.ns[i].prev = files.ns[i].next = \
		files.we[i].prev = files.we[i].next = 1;
	for (i = 0; i < 2*problemsize-1+2; i++)
		files.nwse[i].prev = files.nwse[i].next = \
		files.swne[i].prev = files.swne[i].next = 1;
	for (i = 1; i <= problemsize; i++)
		files.ns[i].size = \
		files.we[i].size = problemsize;
	for (i = 1; i <= problemsize; i++)
		files.nwse[i].size = files.nwse[2*problemsize-1+1-i].size = \
		files.swne[i].size = files.swne[2*problemsize-1+1-i].size = i;

	count.ns = count.we = problemsize;
	count.nwse = count.swne = 2*problemsize-1;

	/* Place initial queens as stdin tells us */

	for (i = 1; i <= problemsize; i++) {
		if (scanf("%d", &j) != 1)
			assert(0);
		if (j)
			place_queen(i, j);
	}

	return 1;
}

static void solve(void)
{
	algorithm();
}

static void print_solution(void)
{
	int i;

	for (i = 1; i <= problemsize; i++) {
		printf("%d", solution[i]);
		printf(i < problemsize ? " " : "\n");
	}
}

int main(void)
{
	while (parse()) {
		DEBUG("\n");
		/*
#define TESTIT printf("(%d %d) (%d %d) (%d %d)\n", files.we[0].prev, files.we[0].next, files.we[1].prev, files.we[1].next, files.we[2].prev, files.we[2].next)
		TESTIT;
		int i = 1, j = 1; UNLINK_FILE(FILE_WE);
		TESTIT;
		i = 2, j = 3; UNLINK_FILE(FILE_WE);
		TESTIT;
		return 0;
		*/

		solve();
		print_solution();
	}

	return 0;
}
