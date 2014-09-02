/* This is trying to be an efficient implementation of the NQUEENS problem,
 * employing the Dancing Links (DLX) technique.
 *
 * It must still try harder, but many sizes below 200 have instant results.
 *
 * While having clear structure, the code is very "unrolled". This is to make
 * it easier to trace the process. I'm still planning to include visualisation,
 * maybe with PostScript...
 *
 * This is a solution to http://spoj.com/problems/NQUEEN/ . I don't want to
 * spoil anything, so the code needs a little modification to work:
 *
 *  - add one little extra case at two locations
 *  - fold the code to get below the 10000 character limit
 *
 * The missing piece won't affect that the code is easy to learn from...
 *
 * I just want to make sure this code isn't submitted without a good
 * understanding of the algorithm...
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WITH_TRACING
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...) ({})
#endif

#define MAXSIZE 510

typedef short idxtype;  /* must index arrays of length 2*MAXSIZE-1+2 */

struct link {
	idxtype next;
	idxtype prev;
};

struct cell {
	struct link we;
	struct link ns;
	struct link nwse;
	struct link swne;
};

struct file {
	int size;
	idxtype next;
	idxtype prev;
};

struct folder {
	int size;
	struct file files[2*MAXSIZE-1+2];
};

static int problemsize;
static int queens_remain;
static int solution[MAXSIZE+2];
static struct cell cell[MAXSIZE+2][MAXSIZE+2];
static struct folder rows;
static struct folder cols;
static struct folder nwse;
static struct folder swne;

static int index_rows(int row, int col)
{
	col = col;  /* unused parameter */
	return row;
}

static int index_cols(int row, int col)
{
	row = row;  /* unused parameter */
	return col;
}

static int index_nwse(int row, int col)
{
	return row + problemsize+1-col - 1;
}

static int index_swne(int row, int col)
{
	return row + col - 1;
}

static void _unlink_file(struct folder *folder, int idx)
{
	int prev = folder->files[idx].prev;
	int next = folder->files[idx].next;

	folder->files[idx-prev].next = next + prev;
	folder->files[idx+next].prev = next + prev;
	folder->size--;
}

static void _relink_file(struct folder *folder, int idx)
{
	int prev = folder->files[idx].prev;
	int next = folder->files[idx].next;

	folder->files[idx-prev].next = prev;
	folder->files[idx+next].prev = next;
	folder->size++;
}

static void unlink_file_rows(int idx)
{
	TRACE("UNLINK_FILE_ROW %d\n", idx);
	_unlink_file(&rows, idx);
}

static void relink_file_rows(int idx)
{
	TRACE("RELINK_FILE_ROW %d\n", idx);
	_relink_file(&rows, idx);
}

static void unlink_file_cols(int idx)
{
	TRACE("UNLINK_FILE_COL %d\n", idx);
	_unlink_file(&cols, idx);
}

static void relink_file_cols(int idx)
{
	TRACE("RELINK_FILE_COL %d\n", idx);
	_relink_file(&cols, idx);
}

static void unlink_file_nwse(int idx)
{
	TRACE("UNLINK_FILE_NWSE %d\n", idx);
	_unlink_file(&nwse, idx);
}

static void relink_file_nwse(int idx)
{
	TRACE("RELINK_FILE_NWSE %d\n", idx);
	_relink_file(&nwse, idx);
}

static void unlink_file_swne(int idx)
{
	TRACE("UNLINK_FILE_SWNE %d\n", idx);
	_unlink_file(&swne, idx);
}

static void relink_file_swne(int idx)
{
	TRACE("RELINK_FILE_SWNE %d\n", idx);
	_relink_file(&swne, idx);
}

static int _decrease_file(struct folder *folder, int idx)
{
	return --folder->files[idx].size == 0;
}

static int _increase_file(struct folder *folder, int idx)
{
	return folder->files[idx].size++ == 0;
}

#define DECREASE_FILE(folder, unlink, typ)         \
({                                                 \
	TRACE("DECREASE_FILE_" typ " %d\n", idx);  \
	if (_decrease_file(&folder, idx))          \
		unlink(idx);                       \
})

#define INCREASE_FILE(folder, relink, typ)         \
({                                                 \
	TRACE("INCREASE_" typ " %d\n", idx);       \
	if (_increase_file(&folder, idx))          \
		relink(idx);                       \
})

static void decrease_rows(int idx)
{
	DECREASE_FILE(rows, unlink_file_rows, "ROW");
}

static void increase_rows(int idx)
{
	INCREASE_FILE(rows, relink_file_rows, "ROW");
}

static void decrease_cols(int idx)
{
	DECREASE_FILE(cols, unlink_file_cols, "COL");
}

static void increase_cols(int idx)
{
	INCREASE_FILE(cols, relink_file_cols, "COL");
}

static void decrease_nwse(int idx)
{
	DECREASE_FILE(nwse, unlink_file_nwse, "NWSE");
}

static void increase_nwse(int idx)
{
	INCREASE_FILE(nwse, relink_file_nwse, "NWSE");
}

static void decrease_swne(int idx)
{
	DECREASE_FILE(swne, unlink_file_swne, "SWNE");
}

static void increase_swne(int idx)
{
	INCREASE_FILE(swne, relink_file_swne, "SWNE");
}

static void decrease_but_rows(int row, int col)
{
	TRACE("DECREASE_BUT_ROWS %d %d\n", row, col);
	decrease_cols(index_cols(row, col));
	decrease_nwse(index_nwse(row, col));
	decrease_swne(index_swne(row, col));
}

static void increase_but_rows(int row, int col)
{
	TRACE("INCREASE_BUT_ROWS %d %d\n", row, col);
	increase_cols(index_cols(row, col));
	increase_nwse(index_nwse(row, col));
	increase_swne(index_swne(row, col));
}

static void decrease_but_cols(int row, int col)
{
	TRACE("DECREASE_BUT_COLS %d %d\n", row, col);
	decrease_rows(index_rows(row, col));
	decrease_nwse(index_nwse(row, col));
	decrease_swne(index_swne(row, col));
}

static void increase_but_cols(int row, int col)
{
	TRACE("INCREASE_BUT_COLS %d %d\n", row, col);
	increase_rows(index_rows(row, col));
	increase_nwse(index_nwse(row, col));
	increase_swne(index_swne(row, col));
}

static void decrease_but_nwse(int row, int col)
{
	TRACE("DECREASE_BUT_NWSE %d %d\n", row, col);
	decrease_rows(index_rows(row, col));
	decrease_cols(index_cols(row, col));
	decrease_swne(index_swne(row, col));
}

static void increase_but_nwse(int row, int col)
{
	TRACE("INCREASE_BUT_NWSE %d %d\n", row, col);
	increase_rows(index_rows(row, col));
	increase_cols(index_cols(row, col));
	increase_swne(index_swne(row, col));
}

static void decrease_but_swne(int row, int col)
{
	TRACE("DECREASE_BUT_SWNE %d %d\n", row, col);
	decrease_rows(index_rows(row, col));
	decrease_cols(index_cols(row, col));
	decrease_nwse(index_nwse(row, col));
}

static void increase_but_swne(int row, int col)
{
	TRACE("INCREASE_BUT_SWNE %d %d\n", row, col);
	increase_rows(index_rows(row, col));
	increase_cols(index_cols(row, col));
	increase_nwse(index_nwse(row, col));
}

static void unlink_we(int row, int col)
{
	TRACE("UNLINK_WE %d %d\n", row, col);

	int prev = cell[row][col].we.prev;
	int next = cell[row][col].we.next;

	cell[row][col-prev].we.next = prev + next;
	cell[row][col+next].we.prev = prev + next;
}

static void relink_we(int row, int col)
{
	TRACE("RELINK_WE %d %d\n", row, col);

	int prev = cell[row][col].we.prev;
	int next = cell[row][col].we.next;

	cell[row][col-prev].we.next = prev;
	cell[row][col+next].we.prev = next;
}

static void unlink_ns(int row, int col)
{
	TRACE("UNLINK_NS %d %d\n", row, col);

	int prev = cell[row][col].ns.prev;
	int next = cell[row][col].ns.next;

	cell[row-prev][col].ns.next = prev + next;
	cell[row+next][col].ns.prev = prev + next;
}

static void relink_ns(int row, int col)
{
	TRACE("RELINK_NS %d %d\n", row, col);

	int prev = cell[row][col].ns.prev;
	int next = cell[row][col].ns.next;

	cell[row-prev][col].ns.next = prev;
	cell[row+next][col].ns.prev = next;
}

static void unlink_nwse(int row, int col)
{
	TRACE("UNLINK_NWSE %d %d\n", row, col);

	int prev = cell[row][col].nwse.prev;
	int next = cell[row][col].nwse.next;

	cell[row-prev][col-prev].nwse.next = prev + next;
	cell[row+next][col+next].nwse.prev = prev + next;
}

static void relink_nwse(int row, int col)
{
	TRACE("RELINK_NWsE %d %d\n", row, col);

	int prev = cell[row][col].nwse.prev;
	int next = cell[row][col].nwse.next;

	cell[row-prev][col-prev].nwse.next = prev;
	cell[row+next][col+next].nwse.prev = next;
}

static void unlink_swne(int row, int col)
{
	TRACE("UNLINK_SWNE %d %d\n", row, col);

	int prev = cell[row][col].swne.prev;
	int next = cell[row][col].swne.next;

	cell[row+prev][col-prev].swne.next = prev + next;
	cell[row-next][col+next].swne.prev = prev + next;
}

static void relink_swne(int row, int col)
{
	TRACE("RELINK_SWNE %d %d\n", row, col);

	int prev = cell[row][col].swne.prev;
	int next = cell[row][col].swne.next;

	cell[row+prev][col-prev].swne.next = prev;
	cell[row-next][col+next].swne.prev = next;
}

static void unlink_but_ns(int row, int col)
{
	TRACE("UNLINK_BUT_NS %d %d\n", row, col);
	unlink_we(row, col);
	unlink_nwse(row, col);
	unlink_swne(row, col);
}

static void relink_but_ns(int row, int col)
{
	TRACE("RELINK_BUT_NS %d %d\n", row, col);
	relink_we(row, col);
	relink_nwse(row, col);
	relink_swne(row, col);
}

static void unlink_but_we(int row, int col)
{
	TRACE("UNLINK_BUT_WE %d %d\n", row, col);
	unlink_ns(row, col);
	unlink_nwse(row, col);
	unlink_swne(row, col);
}

static void relink_but_we(int row, int col)
{
	TRACE("RELINK_BUT_WE %d %d\n", row, col);
	relink_ns(row, col);
	relink_nwse(row, col);
	relink_swne(row, col);
}

static void unlink_but_nwse(int row, int col)
{
	TRACE("UNLINK_BUT_NWSE %d %d\n", row, col);
	unlink_ns(row, col);
	unlink_we(row, col);
	unlink_swne(row, col);
}

static void relink_but_nwse(int row, int col)
{
	TRACE("RELINK_BUT_NWSE %d %d\n", row, col);
	relink_ns(row, col);
	relink_we(row, col);
	relink_swne(row, col);
}

static void unlink_but_swne(int row, int col)
{
	TRACE("UNLINK_BUT_SWNE %d %d\n", row, col);
	unlink_ns(row, col);
	unlink_we(row, col);
	unlink_nwse(row, col);
}

static void relink_but_swne(int row, int col)
{
	TRACE("RELINK_BUT_SWNE %d %d\n", row, col);
	relink_ns(row, col);
	relink_we(row, col);
	relink_nwse(row, col);
}

static int choose_row(void)
{
	int row = 0;
	int minsize = problemsize + 1;
	int minrow = -1;

	while (row += rows.files[row].next, row <= problemsize) {
		if (minsize > rows.files[row].size) {
			minsize = rows.files[row].size;
			minrow = row;
		}
	}
	assert(row > 0);

	TRACE("CHOOSEROW %d\n", minrow);

	return minrow;
}

static void protect(int row, int col)
{
	TRACE("PROTECT %d %d\n", row, col);

	unlink_we(row, col);
	unlink_ns(row, col);
	unlink_nwse(row, col);
	unlink_swne(row, col);

	unlink_file_rows(index_rows(row, col));
	unlink_file_cols(index_cols(row, col));
	unlink_file_nwse(index_nwse(row, col));
	unlink_file_swne(index_swne(row, col));
}

static void unprotect(int row, int col)
{
	TRACE("UNPROTECT %d %d\n", row, col);

	relink_file_rows(index_rows(row, col));
	relink_file_cols(index_cols(row, col));
	relink_file_nwse(index_nwse(row, col));
	relink_file_swne(index_swne(row, col));

	relink_we(row, col);
	relink_ns(row, col);
	relink_nwse(row, col);
	relink_swne(row, col);
}

#define FOREACH_WE for (i = row, j = 0; j += cell[i][j].we.next, j <= problemsize;)

#define FOREACH_NS for (i = 0, j = col; i += cell[i][j].ns.next, i <= problemsize;)

#define FOREACH_NWSE for (i = 0, j = col-row; d = cell[i][j].nwse.next, i += d, j += d, j <= problemsize;)

#define FOREACH_SWNE for (i = row+col, j = 0; d = cell[i][j].swne.next, i -= d, j += d, i > 0;)

static void place_queen(int row, int col)
{
	int i, j, d;

	TRACE("PLACE_QUEEN %d %d\n", row, col);

	queens_remain -= 1;
	assert(solution[row] == 0);
	solution[row] = col;

	protect(row, col);

	TRACE("UNLINK_CELLS %d %d\n", row, col);

	TRACE("UNLINK_ALONG_WE %d %d\n", row, col);
	FOREACH_WE
		unlink_but_we(i, j), decrease_but_rows(i, j);
	TRACE("UNLINK_ALONG_NS %d %d\n", row, col);
	FOREACH_NS
		unlink_but_ns(i, j), decrease_but_cols(i, j);
	TRACE("UNLINK_ALONG_NWSE %d %d\n", row, col);
	FOREACH_NWSE
		unlink_but_nwse(i, j), decrease_but_nwse(i, j);
	TRACE("UNLINK_ALONG_SWNE %d %d\n", row, col);
	FOREACH_SWNE
		unlink_but_swne(i, j), decrease_but_swne(i, j);
}

static void remove_queen(int row, int col)
{
	int i, j, d;

	TRACE("REMOVE_QUEEN %d %d\n", row, col);

	TRACE("RELINK_CELLS %d %d\n", row, col);

	TRACE("RELINK_ALONG_SWNE %d %d\n", row, col);
	FOREACH_SWNE
		relink_but_swne(i, j), increase_but_swne(i, j);
	TRACE("RELINK_ALONG_NWSE %d %d\n", row, col);
	FOREACH_NWSE
		relink_but_nwse(i, j), increase_but_nwse(i, j);
	TRACE("RELINK_ALONG_NS %d %d\n", row, col);
	FOREACH_NS
		relink_but_ns(i, j), increase_but_cols(i, j);
	TRACE("RELINK_ALONG_WE %d %d\n", row, col);
	FOREACH_WE
		relink_but_we(i, j), increase_but_rows(i, j);

	unprotect(row, col);

	assert(solution[row] == col);
	solution[row] = 0;
	queens_remain += 1;
}

static int recurse(void)
{
	int row, col;

	if (queens_remain == 0)
		return 1;

	if (rows.size < queens_remain)
		return 0;
	if (cols.size < queens_remain)
		return 0;
	if (nwse.size < queens_remain)
		return 0;
	if (swne.size < queens_remain)
		return 0;

	row = choose_row();

	for (col = 0; col += cell[row][col].we.next, col <= problemsize;) {
		place_queen(row, col);

		if (recurse())
			return 1;

		remove_queen(row, col);
	}

	return 0;
}

static int parse(void)
{
	int i;
	int j;

	if (scanf("%d", &problemsize) != 1)
		return 0;

	if (problemsize > MAXSIZE) {
		fprintf(stderr, "The given problemsize (%d) is greater than "
			"the compile time constant MAXSIZE (%d)\n",
			problemsize, MAXSIZE);
		exit(1);
	}

	queens_remain = problemsize;

	for (i = 1; i <= problemsize; i++)
		solution[i] = 0;

	for (i = 0; i < problemsize+2; i++) {
		for (j = 0; j < problemsize+2; j++) {
			cell[i][j].ns.next = 1;
			cell[i][j].ns.prev = 1;
			cell[i][j].we.next = 1;
			cell[i][j].we.prev = 1;
			cell[i][j].nwse.next = 1;
			cell[i][j].nwse.prev = 1;
			cell[i][j].swne.next = 1;
			cell[i][j].swne.prev = 1;
		}
	}

	rows.size = problemsize;
	for (i = 0; i < problemsize+2; i++) {
		rows.files[i].size = problemsize;
		rows.files[i].next = 1;
		rows.files[i].prev = 1;
	}

	cols.size = problemsize;
	for (i = 0; i < problemsize+2; i++) {
		cols.files[i].size = problemsize;
		cols.files[i].next = 1;
		cols.files[i].prev = 1;
	}

	nwse.size = 2*problemsize-1;
	for (i = 0; i < 2*problemsize-1+2; i++) {
		nwse.files[i].size = (i <= problemsize) ? i : 2*problemsize-i;
		nwse.files[i].next = 1;
		nwse.files[i].prev = 1;
	}

	swne.size = 2*problemsize-1;
	for (i = 0; i < 2*problemsize-1+2; i++) {
		swne.files[i].size = (i <= problemsize) ? i : 2*problemsize-i;
		swne.files[i].next = 1;
		swne.files[i].prev = 1;
	}

	for (i = 1; i <= problemsize; i++) {
		if (scanf("%d", &j) != 1) {
			fprintf(stderr, "Failed to read all pre-set queens. "
				"Problemsize is %d, but could read only %d.\n",
				problemsize, i-1);
			exit(1);
		}
		if (j != 0)
			place_queen(i, j);
	}

	return 1;
}

static void solve(void)
{
	recurse();
}

static void print(void)
{
	int i;
	for (i = 1; i <= problemsize; i++)
		printf("%d%c", solution[i], i < problemsize ? ' ' : '\n');
}

int main(void)
{
	while (parse()) {
		solve();
		print();
	}

	return 0;
}
