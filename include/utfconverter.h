#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <sys/utsname.h>
#include <limits.h>
#include <sys/times.h>
#include <time.h>


/*#define MAX_BYTES 2*/
#define MAX_BYTES 4
/*
#define SURROGATE_SIZE 2
#define NON_SURROGATE_SIZE 1
*/
#define SURROGATE_SIZE 4
#define NON_SURROGATE_SIZE 2
#define NO_FD -1
/* #define OFFSET 4*/
#define OFFSET 2
/*
#define FIRST  10000000
#define SECOND 20000000
#define THIRD  30000000
#define FOURTH 40000000
*/
#define FIRST  0
#define SECOND 1
#define THIRD  2
#define FOURTH 3

#ifdef __STDC__
	#define P(x) x
#else
	/*#define P(x) ()*/
	#define P(x) 
#endif

/** The enum for endianness. */
typedef enum {LITTLE, BIG, NONE} endianness;

/** The struct for a codepoint glyph. */
typedef struct Glyph{
	unsigned char bytes[MAX_BYTES];
	endianness end;
	bool surrogate;
} Glyph;

/** The given filename. */
extern char* filename;

/** The usage statement. */
const char *USAGE_pt1 = "Commnad line utility for converting files to and from UTF-8, UTF-16LE and UTF-16BE.\n\n"
						"Usage:\n"
						"  ./utf [-h|--help] -u OUT_ENC | --UTF=OUT_ENC IN_FILE [OUT_FILE]\n\n";

const char *USAGE_pt2 = "  Option arguments:\n"
						"    -h, --help      Displays this usage.\n"
						"    -v, -vv         Toggles the verbosity of the program to level 1 or 2.\n\n"
						"  Mandatory argument:\n"
						"    -u OUT_ENC, --UTF=OUT_ENC   Sets the output encoding.\n"
						"                                  Valid values for OUT_ENC: 8, 16LE, 16BE\n\n"
						"  Positional Arguments:\n"
						"    IN_FILE         The file to convert.\n"
						"    [OUT_FILE]	     Output file name. If not present, defaults to stdout.\n";

/** Which endianness to convert to. */
extern endianness conversion;

/** Which endianness the source file is in. */
extern endianness source;

/**
 * A function that swaps the endianness of the bytes of an encoding from
 * LE to BE and vice versa.
 *
 * @param glyph The pointer to the glyph struct to swap.
 * @return Returns a pointer to the glyph that has been swapped.
 */
/*Glyph* swap_endianness P((Glyph*));*/
Glyph* swap_endianness(Glyph*);

/**
 * Fills in a glyph with the given data in data[2], with the given endianness 
 * by end.
 *
 * @param glyph 	The pointer to the glyph struct to fill in with bytes.
 * @param data[2]	The array of data to fill the glyph struct with.
 * @param end	   	The endianness enum of the glyph.
 * @param fd 		The int pointer to the file descriptor of the input 
 * 			file.
 * @return Returns a pointer to the filled-in glyph.
 */
/*Glyph* fill_glyph P((Glyph*, unsigned int, endianness, int*));*/
Glyph* fill_glyph(Glyph*, unsigned char[2], endianness, int*);

/**
 * Writes the given glyph's contents to stdout.
 *
 * @param glyph The pointer to the glyph struct to write to stdout.
 */
/*void write_glyph P((Glyph*));*/
void write_glyph (Glyph*);

/**
 * Calls getopt() and parses arguments.
 *
 * @param argc The number of arguments.
 * @param argv The arguments as an array of string.
 */
/*void parse_args P((int, char**));*/
void parse_args (int, char**);

/**
 * Prints the usage statement.
 */
/*void print_help P((void));*/
void print_help (void);

/**
 * Closes file descriptors and frees list and possibly does other
 * bookkeeping before exiting.
 *
 * @param The fd int of the file the program has opened. Can be given
 * the macro value NO_FD (-1) to signify that we have no open file
 * to close.
 */
/*void quit_converter P((int));*/
void quit_converter (int);

/**
 * Prints info according to verbosity level
 * @param vlevel Verbosity level [either 1 or 2+]
 */
 void print_verbose(int vlevel);

 int isInAsciiRange(Glyph glyph);

 /**
 * A function that converts a UTF-8 glyph to a UTF-16LE or UTF-16BE
 * glyph, and returns the result as a pointer to the converted glyph.
 *
 * @param glyph The UTF-8 glyph to convert.
 * @param end   The endianness to convert to (UTF-16LE or UTF-16BE).
 * @return The converted glyph.
 */
Glyph* convert(Glyph* glyph, endianness end);

float round_num(float f);

/**
 * A function that converts a UTF-16LE or UTF-16BE glyph to a
 * UTF-8 glyph, and returns the result as a pointer to the
 * converted glyph.
 *
 * @param glyph The UTF-16LE/BE glyph to convert.
 * @param end   The endianness of the source glyph.
 * @return The converted glyph.
 */
Glyph* convert_reverse(Glyph* glyph, endianness end);

int isFileEmpty(char *filename);
