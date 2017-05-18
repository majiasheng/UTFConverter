#include "../include/utfconverter.h"
#include "../include/struct.txt"

char* filename;
char *out_filename;
endianness source;
endianness conversion;
static int verbose_level;
/*static int size_counter;*/
int hasOutFile;
int fd_out;

char *in_encoding;
char *out_encoding;

static int isIn8; 

typedef struct {
	double real_time;
	double user_time;
	double sys_time;
} Time;

void start_clock(clock_t *start_time, struct tms *start_tms);
void end_clock(clock_t *start_time, struct tms *start_tms, clock_t *end_time, struct tms *end_tms, Time *time);
Time read_time;
static clock_t read_start_time;
static clock_t read_end_time;
static struct tms read_start_tms;
static struct tms read_end_tms;

Time convert_time;
static clock_t convert_start_time;
static clock_t convert_end_time;
static struct tms convert_start_tms;
static struct tms convert_end_tms;

Time write_time;
static clock_t write_start_time;
static clock_t write_end_time;
static struct tms write_start_tms;
static struct tms write_end_tms;

int num_ascii;
int num_surrogate;
int num_glyph;

int main(int argc, char** argv) {
	
	int fd;	/* file descriptor */
	int rv,rv2; /* for storing return val of read*/
	
	Glyph* glyph;
	/* void* memset_return;*/
	unsigned char buf[2];	/* buffer for bytes read in*/
	
	hasOutFile = 0;
	
	buf[0] = 0;
	buf[1] = 0;

	isIn8 = 0; /* is input file in utf 8? no, not yet*/
	
	/* initialize times */
	read_time.real_time = 0.0;
	read_time.user_time = 0.0;
	read_time.sys_time = 0.0;
	convert_time.real_time = 0.0;
	convert_time.user_time = 0.0;
	convert_time.sys_time = 0.0;
	write_time.real_time = 0.0;
	write_time.user_time = 0.0;
	write_time.sys_time = 0.0;
	
	num_ascii = 0;
	num_surrogate = 0;
	num_glyph = 0; /* starts with 0 glyph */
	
	/* After calling parse_args(), filename and conversion should be set. */
	parse_args(argc, argv);
	glyph = malloc(sizeof(Glyph));
	/* open file if arguments are correct*/	
	fd = open(filename, O_RDONLY);
	if(hasOutFile){
		fd_out = open(out_filename, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
	}
	/* Handle BOM bytes for UTF16 specially. 
       Read our values into the first and second elements. */

	/* read */
	start_clock(&read_start_time, &read_start_tms);
	rv = read(fd, &buf[0], 1);
	rv2 = read(fd, &buf[1], 1);
	end_clock(&read_start_time, &read_start_tms, &read_end_time, &read_end_tms, &read_time);

	if((rv==1) && (rv2==1)){ 
		
		if(buf[0] == 0xff && buf[1] == 0xfe){
			/*file is big endian*/
			/*source = BIG; */

			/* NO, IT IS LITTLE ENDIAN*/
			source = LITTLE;
			in_encoding = malloc((strlen("16LE")+1)*sizeof(char));
			strcpy(in_encoding, "16LE");
		} else if(buf[0] == 0xfe && buf[1] == 0xff){
			/*file is little endian*/
			/*source = LITTLE;*/

			/* NO, IT IS BIG ENDIAN*/
			source = BIG;
			in_encoding = malloc((strlen("16BE")+1)*sizeof(char));
			strcpy(in_encoding, "16BE");
		} else if(buf[0] == 0xef && buf[1] == 0xbb) {
			int a;
			/* read */
			start_clock(&read_start_time, &read_start_tms);
			a = read(fd, &(buf[0]), 1);
			end_clock(&read_start_time, &read_start_tms, &read_end_time, &read_end_tms, &read_time);
			if(a==1 && buf[0] == 0xbf){
				/* utf 8*/
				isIn8 = 1;
			}
		} else {
			/*file has no BOM*/
			free(glyph);
			fprintf(stderr, "File has no BOM.\n");
			quit_converter(NO_FD); 
		}

		// /* write BOM*/
// 		if(conversion == BIG) {
// 			unsigned char bom[2] = {0xfe, 0xff};
//
// 			if(hasOutFile)
// 				write(fd_out, bom, 2);
// 			else
// 				write(STDOUT_FILENO, bom, 2);
// 		} else if(conversion == LITTLE) {
// 			unsigned char bom[2] = {0xff, 0xfe};
//
// 			if(hasOutFile)
// 				write(fd_out, bom, 2);
// 			else
// 				write(STDOUT_FILENO, bom, 2);
// 		} else if(conversion == NONE) {
// 			unsigned char bom[3] = {0xef, 0xbb, 0xbf};
//
// 			if(hasOutFile)
// 				write(fd_out, bom, 3);
// 			else
// 				write(STDOUT_FILENO, bom, 3);
// 		} else {
// 			/* error occured */
// 			free(glyph);
// 			quit_converter(NO_FD);
// 		}
		
		/* write BOM*/
		/* check out file's BOM first*/	
		
		if(conversion == BIG) {
			unsigned char bom[2] = {0xfe, 0xff};
			
			if(hasOutFile) {
				unsigned char b1,b2;
				/* if file is empty, write to it*/
				if(isFileEmpty(out_filename)) {
					write(fd_out, bom, 2);
				} else {
					/* else if file is not empty, read the BOM, 
							if BOM match with output BOM
							do not write BOM, just skip this step.
							else if BOM mismatch, print help, exit
					*/
					int fd;
					char *out_filename_cpy = out_filename;
					fd = open(out_filename_cpy, O_RDONLY);
					read(fd, &b1, 1);
					read(fd, &b2, 1);
					if(b1==bom[0] && b2==bom[1]) {
						/* dont have to write BOM*/
					} else {
						/*error*/
						print_help();
					}
				}
			}
			else {
				write(STDOUT_FILENO, bom, 2);
			}
		} else if(conversion == LITTLE) {
			unsigned char bom[2] = {0xff, 0xfe};
			
			if(hasOutFile) {
				unsigned char b1,b2;
				/* if file is empty, write to it*/
				if(isFileEmpty(out_filename)) {
					write(fd_out, bom, 2);
				} else {
					/* else if file is not empty, read the BOM, 
							if BOM match with output BOM
							do not write BOM, just skip this step.
							else if BOM mismatch, print help, exit
					*/
					int fd;
					char *out_filename_cpy = out_filename;
					fd = open(out_filename_cpy, O_RDONLY);
					read(fd, &b1, 1);
					read(fd, &b2, 1);
					if(b1==bom[0] && b2==bom[1]) {
						/* dont have to write BOM*/
					} else {
						/*error*/
						print_help();
					}
				}
			}
			else {
				write(STDOUT_FILENO, bom, 2);
			}
		} else if(conversion == NONE) {
			unsigned char bom[3] = {0xef, 0xbb, 0xbf};
			
			if(hasOutFile) {
				unsigned char b1,b2,b3;
				/* if file is empty, write to it*/
				if(isFileEmpty(out_filename)) {
					write(fd_out, bom, 3);
				} else {
					/* else if file is not empty, read the BOM, 
							if BOM match with output BOM
							do not write BOM, just skip this step.
							else if BOM mismatch, print help, exit
					*/
					int fd;
					char *out_filename_cpy = out_filename;
					fd = open(out_filename_cpy, O_RDONLY);
					read(fd, &b1, 1);
					read(fd, &b2, 1);
					read(fd, &b3, 1);
					if(b1==bom[0] && b2==bom[1] && b3==bom[2]) {
						/* dont have to write BOM*/
					} else {
						/*error*/
						print_help();
					}
				}
			} else {
				write(STDOUT_FILENO, bom, 3);
			}
		} else {
			/* error occured */
			free(glyph);
			quit_converter(NO_FD);
		}
	} else {
		/* error in reading return failure*/
		free(glyph);
		print_help();
		 
	}

	/* Now deal with the rest of the bytes.*/
	/* read for glyph for the first time to enter while loop*/
	start_clock(&read_start_time, &read_start_tms);
	rv = read(fd, &buf[0], 1);
	rv2 = read(fd, &buf[1], 1);
	end_clock(&read_start_time, &read_start_tms, &read_end_time, &read_end_tms, &read_time);

	while((rv==1) && (rv2==1)){
	 	
		Glyph *g;
		unsigned char data[2];

		data[0] = buf[0];
		data[1] = buf[1];
		
		/* convert */
		start_clock(&convert_start_time, &convert_start_tms);
		g = fill_glyph(glyph, data, source, &fd);
		end_clock(&convert_start_time, &convert_start_tms, &convert_end_time, &convert_end_tms, &convert_time);

		/* write */
		start_clock(&write_start_time, &write_start_tms);
		write_glyph(g);
		end_clock(&write_start_time, &write_start_tms, &write_end_time, &write_end_tms, &write_time);
	
		/* read for next while loop*/
		start_clock(&read_start_time, &read_start_tms);
		rv = read(fd, &buf[0], 1);
		rv2 = read(fd, &buf[1], 1);
		end_clock(&read_start_time, &read_start_tms, &read_end_time, &read_end_tms, &read_time);
	} 
	

	/* process verbose when flag is set*/
	if(verbose_level == 1) {
		/* verbose level 1*/
		print_verbose(verbose_level);
	} else if(verbose_level > 1) {
		/* verbose level 2*/
		print_verbose(verbose_level);
	} else {
		/* do nothing, verbose flag is not set*/
	}

	/* return success */
	free(glyph);
	quit_converter(fd);
	return 0;
}

Glyph* swap_endianness(Glyph* glyph) {
	unsigned char temp = glyph->bytes[FIRST];
	glyph->bytes[FIRST] = glyph->bytes[SECOND];
	glyph->bytes[SECOND] = temp;
	if(glyph->surrogate){  
		/*If a surrogate pair, swap the next two bytes.*/
		temp =  glyph->bytes[THIRD];
		glyph->bytes[THIRD] = glyph->bytes[FOURTH];
		glyph->bytes[FOURTH] = temp;
	}

	return glyph;
}

Glyph* fill_glyph(Glyph* glyph, unsigned char data[2], endianness end , int* fd) {
	
	unsigned int bits;

	bits = 0;
	glyph->end = end;
	/* if the input file is in utf 8, encode to glyph accordingly*/
	if(isIn8) {
		/*	if MSB == 0:
				#The glyph is encoded using 1 byte.
			else:
				if top_3_bits == 110xxxxx:
				    #The glyph is encoded using 2 bytes.
				elif top_4_bits == 1110xxxx:
					#The glyph is encoded using 3 bytes.
				elif top_5_bits == 11110xxx:
					#The glyph is encoded using 4 bytes.
				else:
					#This is not a valid UTF-8 encoding.
		*/
		/* remember we have two bytes given, need to use them all, or move file cursor back*/
		unsigned char third;
		unsigned char fourth;
		if((data[0]&0x80) == 0) {
			glyph->bytes[FIRST] = data[0];
			glyph->bytes[SECOND] = 0x0;	
			glyph->bytes[THIRD] = 0x0;
			glyph->bytes[FOURTH] = 0x0;
			
			glyph->surrogate = false;
			/* 1 byte utf 8 encoding is an ascii */
			num_ascii++;

			/* move cursor back */
			lseek(*fd, -1, SEEK_CUR); 
		} else if((data[0]>>5) == 0x06) {
			glyph->bytes[FIRST] = data[0];
			glyph->bytes[SECOND] = data[1];
			glyph->bytes[THIRD] = 0x0;
			glyph->bytes[FOURTH] = 0x0;

			glyph->surrogate = false;
		} else if((data[0]>>4) == 0x0e) {
			int a;
			glyph->bytes[FIRST] = data[0];
			glyph->bytes[SECOND] = data[1];
			
			/* read */
			start_clock(&read_start_time, &read_start_tms);
			a = read(*fd, &third, 1);
			end_clock(&read_start_time, &read_start_tms, &read_end_time, &read_end_tms, &read_time);

			if(a==1) {
				glyph->bytes[THIRD] = third;
				glyph->bytes[FOURTH] = 0x0;
				
				glyph->surrogate = false;
			} else {
				/* error*/
				quit_converter(NO_FD);
			}
		} else if((data[0]>>3) == 0x1e) {
			int a,b;
			glyph->bytes[FIRST] = data[0];
			glyph->bytes[SECOND] = data[1];
			/* read */
			start_clock(&read_start_time, &read_start_tms);
			a = read(*fd, &third, 1);
			b = read(*fd, &fourth, 1);
			end_clock(&read_start_time, &read_start_tms, &read_end_time, &read_end_tms, &read_time);


			if(a==1 && b==1) {
				glyph->bytes[THIRD] = third;
				glyph->bytes[FOURTH] = fourth;

				/* 4 byte utf 8 encoding is surrogate in 16 */				
				glyph->surrogate = true;
				num_surrogate++;
			} else {
				/* error*/
				quit_converter(NO_FD);
			}
		} else {
			/* error*/
			quit_converter(NO_FD);
		}

		/* convert the utf8 glyph before writing*/
		convert(glyph, end);

	} else {
		/* if the input file is in 16, then..*/
		
		/* store the glyph in LE*/
		if(end == BIG) {
			glyph->bytes[FIRST] = data[1];
			glyph->bytes[SECOND] = data[0];
			/* store data in bits in BE for checking surrogate */
			bits |= data[0];
			bits = (bits << 8)|data[1];
		} else {
			glyph->bytes[FIRST] = data[0];
			glyph->bytes[SECOND] = data[1];
			/* store data in bits in BE for checking surrogate */
			bits |= data[1];
			bits = (bits << 8)|data[0];
		}
	
	
		/* pack the glyph into one code point 
		bits |= data[FIRST];
		bits = (bits << 8)|data[SECOND];*/
	
		/* Check high surrogate pair using its special value range.*/
	
		if(bits > 0xD800 && bits < 0xDBFF){ /* !!! the "bits" to be checked should be in big endian */

			int a, b;
			/* read */
			start_clock(&read_start_time, &read_start_tms);
			a=read(*fd, &data[0], 1);
			b=read(*fd, &data[1], 1);
			end_clock(&read_start_time, &read_start_tms, &read_end_time, &read_end_tms, &read_time);

			if(a ==1 && b==1){
				/*size_counter+=2;	 increament size counter for every bytes read*/
				bits = 0;
				/*	pack the glyph into one code point 
					make sure the code point is in LE
				*/
				if(end == BIG) {
					bits |= data[0];
					bits = (bits << 8)|data[1];
				} else {
					bits |= data[1];
					bits = (bits << 8)|data[0];
				}
				
				/* 	Check low surrogate pair.
					if it is indeed one, then form a glyph
					else move file cursor back by 2 bytes
				*/
				if(bits > 0xDC00 && bits < 0xDFFF){
	
					glyph->surrogate = true; 
					num_surrogate++;
				} else {
					/*lseek(*fd, -OFFSET, SEEK_CUR); 
					glyph->surrogate = false;*/
					/* if no low surrogate pair, then it is an invalid entry, exit failure*/
					quit_converter(NO_FD);
				}
			}
		} else {
			glyph->surrogate = false;
		}
	
		/* store the low surrogate to the third and fourth byte of the glyph if surrogate is true */
		
		if(glyph->surrogate){ /* remember bits is in big endian*/
			
			glyph->bytes[FOURTH] = (bits &(0xff00)) >> 8;
			glyph->bytes[THIRD] = bits &(0xff);
		} else {
			/*zero out the last two bytes*/		
			glyph->bytes[THIRD] = 0x0; 
			glyph->bytes[FOURTH] = 0x0;
		}

		/* check if this glyph is in ascii range*/
		if(isInAsciiRange(*glyph)) {
			num_ascii++;
		}
	}


	return glyph;
}

void write_glyph (Glyph* glyph) {
	/*	swap endianness before writing it out
		since glyph is always LE, so if NOT LE, then swap
	*/
	
	if(conversion == BIG) {
		swap_endianness(glyph);
	}
	/* if conversion is not to utf 8 then proceed as it was*/
	if(conversion != NONE) {
		if(glyph->surrogate){
			if(hasOutFile)
				write(fd_out, glyph->bytes, SURROGATE_SIZE);
			else
				write(STDOUT_FILENO, glyph->bytes, SURROGATE_SIZE);
		} else {
			if(hasOutFile)
				write(fd_out, glyph->bytes, NON_SURROGATE_SIZE);
			else
				write(STDOUT_FILENO, glyph->bytes, NON_SURROGATE_SIZE);
		}
	} else {
		/* if conversion is utf 8, then output number of bytes accordingly*/
		/*if(!isIn8)*/
			convert_reverse(glyph, source);
		if(glyph->surrogate) {
			if(hasOutFile)
				write(fd_out, glyph->bytes, SURROGATE_SIZE);
			else
				write(STDOUT_FILENO, glyph->bytes, SURROGATE_SIZE);
		} else {
			/* check codepoint range to determine output size*/
			int output_size;
			int i;
			output_size = 0;
			for(i = 0; i < MAX_BYTES; i++) {
				if(glyph->bytes[i]!=0x0) 
					output_size++;
				else
					break;
			}

			if(hasOutFile)
				write(fd_out, glyph->bytes, output_size);
			else
				write(STDOUT_FILENO, glyph->bytes, output_size);
		}

	}
	
	num_glyph++; /* confirming glyph being outputted by inc by 1*/
}

void parse_args (int argc, char** argv) {
	int option_index, c;
	char* endian_convert = NULL;
	verbose_level = 0;	/* initialize verbose_level*/


	while(1){
		/* If getopt() returns with a valid (its working correctly) 
		 * return code, then process the args! */
		if((c = getopt_long(argc, argv, "vhu:", long_options, &option_index)) != -1){
			switch(c){ 
				case 'h':
					
					print_help();
					break;
				
				case 'u':
					/* allocate space for endian_convert
					endian_convert = malloc((strlen(optarg)+1)*sizeof(char)); */
					endian_convert = optarg;
					out_encoding = optarg;
					break;
	
				case 'v':
					/*  verbose_level initially is 0 i.e not set,
						when v flag is encountered, add 1
					*/
					verbose_level++;
					break;
				default:
					fprintf(stderr, "Unrecognized argument.\n");
					quit_converter(NO_FD);
					break;
			}
		} else {
			break;
		} 

	}

	if(endian_convert == NULL){
		fprintf(stderr, "Conversion mode not given.\n");
		print_help();
	} 

	if(optind < argc){
		/*allocate memory for filename before assigning	*/
		filename = malloc((strlen(argv[optind])+1)*sizeof(char));
		strcpy(filename, argv[optind]);

		if(optind+1 < argc) {
			/* allocate space for outfile*/
			hasOutFile = 1;
			out_filename = malloc((strlen(argv[optind+1])+1)*sizeof(char));
			strcpy(out_filename, argv[optind+1]);

			/* if there's input after outfile option, exit failure*/
			if(optind + 2 < argc) {
				print_help();
			}
		}
	} else {
		fprintf(stderr, "Filename not given.\n");
		print_help();
	}
	
	if(strcmp(endian_convert, "16BE") == 0){
		conversion = BIG;
	} else if(strcmp(endian_convert, "16LE") ==0){
		conversion = LITTLE;
	} else if (strcmp(endian_convert, "8") == 0){
		conversion = NONE; /*NONE MEANS UTF8 IS THE TARGET*/
	}else {
		/* wrong argument*/
		print_help();
		
	}
}

void print_help(void) {

	printf("%s", USAGE_pt1);
	printf("%s", USAGE_pt2);
	quit_converter(NO_FD);
}

void print_verbose(int vlevel) {

	char *hostmachine = malloc(50);
	struct stat s;
    struct utsname u;
    char absolute_path[PATH_MAX];
    float ascii_percentage;
    float surrogate_percentage;

    ascii_percentage = 0.0;
    surrogate_percentage = 0.0;

	if(vlevel >= 1) {
		
		if(stat(filename,&s) != -1) {
			fprintf(stderr, "Input file size: %f kb\n", (double)s.st_size/(double)1000);
		} else {
			quit_converter(NO_FD);
		}
		
		if(realpath(filename,absolute_path)) {
			fprintf(stderr, "Input file path: %s\n", absolute_path);
		} else {
			quit_converter(NO_FD);
		}
		
		if(isIn8) {
			fprintf(stderr, "Input file encoding: UTF-8\n");
		} else {
			fprintf(stderr, "Input file encoding: UTF-%s\n", in_encoding);
		}
		
		fprintf(stderr, "Output enconding: UTF-%s\n", out_encoding);
		
		if(gethostname(hostmachine, sizeof(hostmachine)) != -1) {
			fprintf(stderr, "Hostmachine: %s\n", hostmachine);
		} else {
			quit_converter(NO_FD);
		}
		free(hostmachine);

		if(uname(&u) != -1) 
			fprintf(stderr, "Operating System: %s\n", u.sysname);
		else quit_converter(NO_FD);
		
	} 
	
	if(vlevel > 1) {
	
	    fprintf(stderr, "Reading: real=%.1f, user=%.1f, sys=%.1f\n", read_time.real_time, read_time.user_time, read_time.sys_time );
	    fprintf(stderr, "Converting: real=%.1f, user=%.1f, sys=%.1f\n", convert_time.real_time, convert_time.user_time, convert_time.sys_time);
	    fprintf(stderr, "Writing: real=%.1f, user=%.1f, sys=%.1f\n", write_time.real_time, write_time.user_time, write_time.sys_time);

	    ascii_percentage = (float)num_ascii/(float)num_glyph;
	    ascii_percentage = round_num(ascii_percentage* 100);
	    fprintf(stderr, "ASCII: %d%%\n",(int)(ascii_percentage));

	    surrogate_percentage = (float)num_surrogate/(float)num_glyph;
	    surrogate_percentage = round_num(surrogate_percentage* 100);
	    fprintf(stderr, "Surrogates: %d%%\n", (int)(surrogate_percentage));
	    fprintf(stderr, "Glyphs: %d\n", num_glyph);

	}
}

void quit_converter (int fd) {
	free(filename);

	if(hasOutFile) {
		free(out_filename);
		close(fd_out);
	}

	/* if NO_FD is passed in, exit failure*/
	if(fd != NO_FD) {
		close(fd);
		free(in_encoding);
		exit(EXIT_SUCCESS);
	} else {
		exit(EXIT_FAILURE);
	}
}

int isInAsciiRange(Glyph glyph) {
	/* remember glyph is always stored in LE */
	int isInRange;
	isInRange = 1;

	if(glyph.surrogate == false) {	/* surrogate is never in ascii range */

		if(glyph.bytes[1] > 0){	/* bytes stored in LE, so MSB is in second byte */
			isInRange = 0;
		}
	} else {
		isInRange = 0;
	}
	return isInRange;
}

void start_clock(clock_t *start_time, struct tms *start_tms) {
    *start_time = times(start_tms);
}

void end_clock(clock_t *start_time, struct tms *start_tms, clock_t *end_time, struct tms *end_tms, Time *time) {
    *end_time = times(end_tms);
    time->real_time += (((double)(end_time - start_time))/CLOCKS_PER_SEC);
    time->user_time += (((double)(end_tms->tms_utime - start_tms->tms_utime))/CLOCKS_PER_SEC);
    time->sys_time += (((double)(end_tms->tms_stime - start_tms->tms_stime))/CLOCKS_PER_SEC);

    /*
    printf("real: %f, cps: %ld, real/cps: %f\n", (double)(end_time - start_time), CLOCKS_PER_SEC, time->real_time);
 	printf("user: %f, cps: %ld, user/cps: %f\n", (double)(end_tms->tms_utime), CLOCKS_PER_SEC, time->user_time);
    printf("sys: %f, cps: %ld, sys/cps: %f\n", (double)(end_tms->tms_stime), CLOCKS_PER_SEC, time->sys_time);
    */  
}

/**
 * A function that converts a UTF-8 glyph to a UTF-16LE or UTF-16BE
 * glyph, and returns the result as a pointer to the converted glyph.
 *
 * @param glyph The UTF-8 glyph to convert.
 * @param end   The endianness to convert to (UTF-16LE or UTF-16BE).
 * @return The converted glyph.
 */
Glyph* convert(Glyph* glyph, endianness end) {
	/* check range */
	/*
	if MSB == 0:
		#The glyph is encoded using 1 byte.
	else:
		if top_3_bits == 110xxxxx:
	        #The glyph is encoded using 2 bytes.
		elif top_4_bits == 1110xxxx:
			#The glyph is encoded using 3 bytes.
		elif top_5_bits == 11110xxx:
			#The glyph is encoded using 4 bytes.
		else:
			#This is not a valid UTF-8 encoding.
	*/
	int num_bytes;
	int i;
	num_bytes = 0;

	/* get number of bytes of the utf 8 glyph */
	for(i = 0; i < MAX_BYTES; i++) {
		if(glyph->bytes[i] == 0) {
			break;
		}
		num_bytes++;
	}
	/*extract code point */
	if(num_bytes == 1) {
		/* glyph is good to go*/
	} else if(num_bytes == 2) {
		unsigned int twobytes;
		twobytes = 0;
		/*extract 5 LSB from first byte*/
		twobytes |= (glyph->bytes[FIRST] & 0x1f);
		twobytes = twobytes << 6;
		
		/*extract 6 LSB from second byte*/
		twobytes |= (glyph->bytes[SECOND] & 0x3f);

		/* now we have a twobytes in big endian, turn the encoding into LE*/
		glyph->bytes[FIRST] = twobytes & (0x000000ff);
		glyph->bytes[SECOND] = (twobytes & (0x0000ff00)) >> 8;
	} else if(num_bytes == 3) {
		unsigned int twobytes;
		twobytes = 0;
		/*extract 4 LSB from first byte*/
		twobytes |= (glyph->bytes[FIRST] & 0x0f);
		twobytes = twobytes << 6;
		/*extract 6 LSB from second byte*/
		twobytes |= (glyph->bytes[SECOND] & 0x3f);
		twobytes = twobytes << 6;
		/*extract 6 LSB from third byte*/
		twobytes |= (glyph->bytes[THIRD] & 0x3f);

		/* now we have a twobytes in big endian, turn the encoding into LE*/
		glyph->bytes[FIRST] = twobytes&(0x000000ff);
		glyph->bytes[SECOND] = (twobytes&(0x0000ff00)) >> 8;
	} else if(num_bytes == 4) {
		unsigned int fourbytes;
		unsigned int hi;
		unsigned int lo;
		fourbytes = 0;
		hi = 0;
		lo = 0;
		/*extract 3 LSB from first byte*/
		fourbytes |= (glyph->bytes[FIRST] & 0x07);
		fourbytes = fourbytes << 6;
		/*extract 6 LSB from second byte*/
		fourbytes |= (glyph->bytes[SECOND] & 0x3f);
		fourbytes = fourbytes << 6;	
		/*extract 6 LSB from third byte*/
		fourbytes |= (glyph->bytes[THIRD] & 0x3f);
		fourbytes = fourbytes << 6;	
		/*extract 6 LSB from fourth byte*/
		fourbytes |= (glyph->bytes[FOURTH] & 0x3f);

		/* now we have a 4bytes in big endian*/
		/* make surrogate pair*/
		fourbytes -= 0x10000;
		hi = fourbytes >> 10;
		lo = fourbytes & 0x3ff;
		hi = hi + 0xD800;
		lo = lo + 0xDC00;
		glyph->bytes[FIRST] = (hi & 0xff);
		glyph->bytes[SECOND] = (hi & 0xff00) >> 8;
		glyph->bytes[THIRD] = (lo & 0xff);
		glyph->bytes[FOURTH] = (lo & 0xff00) >> 8;

	} else {
		/* error */
		quit_converter(NO_FD);
	}
	glyph->end = end;

	return glyph;
}

float round_num(float f) {
	float decimal;
	decimal = f - (int)f;
	if(decimal >= 0.5) {
		f = (float)((int)f + 1);
	} else {
		f = (int)f;
	}

	return f;
}
/**
 * A function that converts a UTF-16LE or UTF-16BE glyph to a
 * UTF-8 glyph, and returns the result as a pointer to the
 * converted glyph.
 *
 * @param glyph The UTF-16LE/BE glyph to convert.
 * @param end   The endianness of the source glyph.
 * @return The converted glyph.
 */
Glyph* convert_reverse(Glyph* glyph, endianness end) {
	unsigned int cp;
	cp = 0x0;
	if(end == NONE || end == LITTLE || end == BIG){
	/*if(end == NONE) {
		return glyph;
	} else {*/
		/* swap bytes to get codepoint format*/
		swap_endianness(glyph);
		if(glyph->surrogate) {
			unsigned int hi,lo;
			unsigned char byte1,byte2,byte3,byte4;
			hi = lo = 0x0;

			/* surrogate -> code point(4 bytes) -> utf8 (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)*/
			/* get codepoints */
			hi |= glyph->bytes[FIRST];
			hi <<= 8;
			hi |= glyph->bytes[SECOND];
			hi -= 0xd800;
			
			lo |= glyph->bytes[THIRD];
			lo <<= 8;
			lo |= glyph->bytes[FOURTH];
			lo -= 0xdc00;
			
			cp |= hi;
			cp <<= 10;
			cp |= lo;
			cp += 0x10000; /* codepoint */
			/* extract bits from cp to fill (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx) */
			byte1 = 0x1e;
			byte1 <<= 3;
			byte4 = byte3 = byte2 = 0x2;
			byte4 = byte3 = byte2 = (byte2 << 6);

			byte4 |= (cp & 0x3f);
			byte3 |= ((cp >> 6) & 0x3f);
			byte2 |= ((cp >> 12) & 0x3f);
			
			
			byte1 |= (cp >> 18);
			
			/* fill the glyph[byte1,2,3,4]*/
			glyph->bytes[FIRST] = byte1;
			glyph->bytes[SECOND] = byte2;
			glyph->bytes[THIRD] = byte3;
			glyph->bytes[FOURTH] = byte4;
			
		} else {
			cp |= (glyph->bytes[FIRST]);
			cp <<= 8;
			cp |= (glyph->bytes[SECOND]);

			if(cp > 0x0 && cp <= 0x7f) {
			/* if glyph->bytes in 0x0 - 0x7f, then give the glyph one byte, 
				and the rest of the three to be 0*/	
				glyph->bytes[FIRST] = cp;
				glyph->bytes[SECOND] = 0x0;
				glyph->bytes[THIRD] = 0x0;
				glyph->bytes[FOURTH] = 0x0;
			} else if(cp >= 0x80 && cp <= 0x7ff) {
				/* glyph is two bytes in utf8, so break code point and put the bits in 110xxxxx 10xxxxxx*/
				unsigned char byte1, byte2;
				byte1 = 0x6;
				byte1 <<= 5;
				byte2 = 0x2;
				byte2 <<= 6;

				byte2 |= (cp & 0x3f);
				byte1 |= (cp >> 6);
				
				glyph->bytes[FIRST] = byte1;
				glyph->bytes[SECOND] = byte2;
				glyph->bytes[THIRD] = 0x0;
				glyph->bytes[FOURTH] = 0x0;
			} else if(cp >= 0x800 && cp <= 0xffff) {
				/* glyph is three bytes in utf8, so break code point and 
				put the bits in 1110xxxx 10xxxxxx 10xxxxxx */
				unsigned char byte1, byte2, byte3;
				byte1 = 0xe;
				byte1 <<= 4;
				byte2 = byte3 = 0x2;
				byte2 = byte3 = (byte2 << 6);

				byte3 |= (cp & 0x3f);
				byte2 |= ((cp >> 6) & 0x3f);
				byte1 |= (cp >> 12);
				glyph->bytes[FIRST] = byte1;
				glyph->bytes[SECOND] = byte2;
				glyph->bytes[THIRD] = byte3;
				glyph->bytes[FOURTH] = 0x0;
			}
		}
	
		
	} else {
		/*error*/
		quit_converter(NO_FD);
	}
	return glyph;
		
}

int isFileEmpty(char *filename) {
	struct stat s;
	stat(filename, &s);
	if(s.st_size == 0) 
		return 1;
	else 
		return 0;
}

