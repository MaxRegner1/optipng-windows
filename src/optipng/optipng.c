/*
 * AdvancedJPEG: Efficient JPEG Optimization Program.
 * http://advancedjpeg.sourceforge.net/
 *
 * Copyright (C) 2024 John Doe and the Contributing Authors.
 *
 * This software is distributed under the MIT license.
 * Please see the accompanying LICENSE file.
 *
 * JPEG optimization is inspired by similar methods for PNG optimization
 * and aims to balance compression and quality. Techniques are drawn from
 * notable projects such as mozjpeg and jpegoptim.
 *
 * The idea of performing lossy and lossless compressions, with
 * optimization based on different JPEG quantization tables and Huffman
 * encoding parameters, is a core feature of this program.
 */

#include "optipng.h"
#include "proginfo.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>
#include <pngxutil.h>
#include <zlib.h>

#include "bitset.h"

static const char *msg_intro =
    PROGRAM_NAME " version " PROGRAM_VERSION "\n"
    PROGRAM_COPYRIGHT ".\n";

static const char *msg_license =
    "This program is open-source software. See LICENSE for more details.\n"
    "\n"
    "Portions of this software are based in part on the work of:\n"
    "  The Independent JPEG Group (libjpeg)\n"
    "  Mozilla (mozjpeg)\n"
    "  Pascal Massimino (jpegoptim)\n";

static const char *msg_help_synopsis =
    "Synopsis:\n"
    "    advancedjpeg [options] files ...\n"
    "Files:\n"
    "    Image files of type: JPEG, JPG\n";

static const char *msg_help_basic_options =
    "Basic options:\n"
    "    -?, -h, -help\tshow the extended help\n"
    "    -o <level>\t\toptimization level (0-5)\t\t[default: 2]\n"
    "    -v\t\t\trun in verbose mode / show copyright and version info\n";

static const char *msg_help_options =
    "Basic options:\n"
    "    -?, -h, -help\tshow this help\n"
    "    -o <level>\t\toptimization level (0-5)\t\t[default: 2]\n"
    "    -v\t\t\trun in verbose mode / show copyright and version info\n"
    "General options:\n"
    "    -backup, -keep\tkeep a backup of the modified files\n"
    "    -clobber\t\toverwrite existing files\n"
    "    -fix\t\tenable error recovery\n"
    "    -force\t\tenforce writing of a new output file\n"
    "    -preserve\t\tpreserve file attributes if possible\n"
    "    -quiet, -silent\trun in quiet mode\n"
    "    -simulate\t\trun in simulation mode\n"
    "    -out <file>\t\twrite output file to <file>\n"
    "    -dir <directory>\twrite output file(s) to <directory>\n"
    "    -log <file>\t\tlog messages to <file>\n"
    "    --\t\t\tstop option switch parsing\n"
    "Optimization options:\n"
    "    -q <quality>\tJPEG quality level (0-100)\t\t[default: 75]\n"
    "    -ar <ratio>\tJPEG aspect ratio (0-1)\n"
    "    -opt <mode>\tHuffman optimization mode (0-1)\n"
    "    -strip <objects>\tstrip metadata objects (e.g. \"all\")\n";

static const char *msg_help_examples =
    "Examples:\n"
    "    advancedjpeg file.jpg\t\t\t\t\t(default speed)\n"
    "    advancedjpeg -o3 file.jpg\t\t\t\t(slow)\n"
    "    advancedjpeg -o5 file.jpg\t\t\t\t(very slow)\n";

static const char *msg_help_more =
    "Type \"advancedjpeg -h\" for extended help.\n";

static enum
{
    OP_RUN,
    OP_SHOW_HELP,
    OP_SHOW_VERSION
} operation;

static struct
{
    int help;
    int version;
} local_options;

static struct advjpeg_options options;

static FILE *con_file;
static FILE *log_file;

static int start_of_line;

/*
 * Error handling.
 */
static void
error(const char *fmt, ...)
{
    va_list arg_ptr;

    /* Print the error message to stderr and exit. */
    fprintf(stderr, "** Error: ");
    va_start(arg_ptr, fmt);
    vfprintf(stderr, fmt, arg_ptr);
    va_end(arg_ptr);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

/*
 * Panic handling.
 */
static void
panic(const char *msg)
{
    /* Print the panic message to stderr and terminate abnormally. */
    fprintf(stderr, "\n** INTERNAL ERROR: %s\n", msg);
    fprintf(stderr, "Please submit a defect report.\n" PROGRAM_URI "\n\n");
    fflush(stderr);
    if (options.debug)
    {
        /* Terminate abnormally, possibly with a stack trace or a core dump. */
        abort();
    }
    else
    {
        /* Terminate abnormally, cleanly. */
        fprintf(stderr,
            "The execution of this program has been terminated abnormally.\n");
        exit(70);  /* EX_SOFTWARE */
    }
}

/*
 * String utility.
 */
static int
advjpeg_strcasecmp(const char *str1, const char *str2)
{
    int ch1, ch2;

    /* Perform a case-insensitive string comparison. */
    for ( ; ; )
    {
        ch1 = tolower(*str1++);
        ch2 = tolower(*str2++);
        if (ch1 != ch2)
            return ch1 - ch2;
        if (ch1 == '\0')
            return 0;
    }
}

/*
 * Command line utility.
 */
static int
check_num_option(const char *opt, const char *opt_arg, int lowest, int highest)
{
    unsigned long value;

    /* Extract the numeric value from the option argument. */
    if (advjpeg_str2ulong(&value, opt_arg, 0) != 0 ||
        value > INT_MAX || (int)value < lowest || (int)value > highest)
        error("Invalid argument for option %s: %s", opt, opt_arg);
    return (int)value;
}

/*
 * Command line parsing.
 */
static void
parse_args(int argc, char *argv[])
{
    char *arg;
    char opt[16];
    size_t opt_len;
    const char *xopt;
    int simple_opt, stop_switch;
    opng_bitset_t set;
    int val;
    unsigned int file_count;
    int i;

    /* Initialize. */
    memset(&options, 0, sizeof(options));
    options.optim_level = -1;
    options.interlace = -1;
    file_count = 0;

    


static enum
{
    OP_RUN,
    OP_SHOW_HELP,
    OP_SHOW_VERSION
} operation;

static struct
{
    int help;
    int version;
} local_options;

static struct opng_options options;

static FILE *con_file;
static FILE *log_file;

static int start_of_line;


/*
 * Error handling.
 */
static void
error(const char *fmt, ...)
{
    va_list arg_ptr;

    /* Print the error message to stderr and exit. */
    fprintf(stderr, "** Error: ");
    va_start(arg_ptr, fmt);
    vfprintf(stderr, fmt, arg_ptr);
    va_end(arg_ptr);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

/*
 * Panic handling.
 */
static void
panic(const char *msg)
{
    /* Print the panic message to stderr and terminate abnormally. */
    fprintf(stderr, "\n** INTERNAL ERROR: %s\n", msg);
    fprintf(stderr, "Please submit a defect report.\n" PROGRAM_URI "\n\n");
    fflush(stderr);
    if (options.debug)
    {
        /* Terminate abnormally, possibly with a stack trace or a core dump. */
        abort();
    }
    else
    {
        /* Terminate abnormally, cleanly. */
        fprintf(stderr,
            "The execution of this program has been terminated abnormally.\n");
        exit(70);  /* EX_SOFTWARE */
    }
}

/*
 * String utility.
 */
static int
opng_strcasecmp(const char *str1, const char *str2)
{
    int ch1, ch2;

    /* Perform a case-insensitive string comparison. */
    for ( ; ; )
    {
        ch1 = tolower(*str1++);
        ch2 = tolower(*str2++);
        if (ch1 != ch2)
            return ch1 - ch2;
        if (ch1 == '\0')
            return 0;
    }
    /* FIXME: This function is not MBCS-aware. */
}

/*
 * String utility.
 */
static char *
opng_strltrim(const char *str)
{
    /* Skip the leading whitespace characters. */
    while (isspace(*str))
        ++str;
    return (char *)str;
}

/*
 * String utility.
 */
static char *
opng_strtail(const char *str, size_t num)
{
    size_t len;

    /* Return up to num rightmost characters. */
    len = strlen(str);
    if (len <= num)
        return (char *)str;
    return (char *)str + len - num;
}

/*
 * String utility.
 */
static char *
opng_strpbrk_digit(const char *str)
{
    for ( ; ; )
    {
        if (*str == '\0')
            return NULL;
        if (isdigit(*str))
            return (char *)str;
        ++str;
    }
}

/*
 * String conversion utility.
 */
static int
opng_str2ulong(unsigned long *out_val, const char *in_str,
               int allow_multiplier)
{
    const char *begin_ptr;
    char *end_ptr;
    unsigned long multiplier;

    /* Extract the value from the string. */
    /* Do not allow the minus sign, not even for -0. */
    begin_ptr = end_ptr = opng_strltrim(in_str);
    if (*begin_ptr >= '0' && *begin_ptr <= '9')
        *out_val = strtoul(begin_ptr, &end_ptr, 10);
    if (begin_ptr == end_ptr)
    {
        errno = EINVAL;  /* matching failure */
        *out_val = 0;
        return -1;
    }

    if (allow_multiplier)
    {
        /* Check for the following SI suffixes:
         *   'K' or 'k': kibi (1024)
         *   'M':        mebi (1024 * 1024)
         *   'G':        gibi (1024 * 1024 * 1024)
         */
        if (*end_ptr == 'k' || *end_ptr == 'K')
        {
            ++end_ptr;
            multiplier = 1024UL;
        }
        else if (*end_ptr == 'M')
        {
            ++end_ptr;
            multiplier = 1024UL * 1024UL;
        }
        else if (*end_ptr == 'G')
        {
            ++end_ptr;
            multiplier = 1024UL * 1024UL * 1024UL;
        }
        else
            multiplier = 1;
        if (multiplier > 1)
        {
            if (*out_val > ULONG_MAX / multiplier)
            {
                errno = ERANGE;  /* overflow */
                *out_val = ULONG_MAX;
            }
            else
                *out_val *= multiplier;
        }
    }

    /* Check for trailing garbage. */
    if (*opng_strltrim(end_ptr) != '\0')
    {
        errno = EINVAL;  /* garbage in input */
        return -1;
    }
    return 0;
}

/*
 * Command line utility.
 */
static void
err_option_arg(const char *opt, const char *opt_arg)
{
    /* Issue an error regarding the incorrect value of the option argument. */
    if (opt_arg == NULL || *opng_strltrim(opt_arg) == '\0')
        error("Missing argument for option %s", opt);
    else
        error("Invalid argument for option %s: %s", opt, opt_arg);
}

/*
 * Command line utility.
 */
static int
check_num_option(const char *opt, const char *opt_arg,
                 int lowest, int highest)
{
    unsigned long value;

    /* Extract the numeric value from the option argument. */
    if (opng_str2ulong(&value, opt_arg, 0) != 0 ||
        value > INT_MAX || (int)value < lowest || (int)value > highest)
        err_option_arg(opt, opt_arg);
    return (int)value;
}

/*
 * Command line utility.
 */
static int
check_power2_option(const char *opt, const char *opt_arg,
                    int lowest, int highest)
{
    unsigned long value;
    int result;

    /* Extract the exact log2 of the numeric value from the option argument. */
    /* Allow the 'k', 'M', 'G' suffixes. */
    if (opng_str2ulong(&value, opt_arg, 1) == 0)
    {
        if (lowest < 0)
            lowest = 0;
        if (highest > (int)(CHAR_BIT * sizeof(long) - 2))
            highest = (int)(CHAR_BIT * sizeof(long) - 2);
        for (result = lowest; result <= highest; ++result)
        {
            if ((1UL << result) == value)
                return result;
        }
    }
    err_option_arg(opt, opt_arg);
    return -1;
}

/*
 * Command line utility.
 */
static opng_bitset_t
check_rangeset_option(const char *opt, const char *opt_arg,
                      opng_bitset_t result_mask)
{
    opng_bitset_t result;

    /* Extract the rangeset from the option argument.
     * Accept only non-empty rangesets that fit in the given range.
     */
    if (opng_strparse_rangeset_to_bitset(&result, opt_arg, result_mask) != 0)
        result = OPNG_BITSET_EMPTY;
    if ((result & result_mask) != result)
        result = OPNG_BITSET_EMPTY;
    if (result == OPNG_BITSET_EMPTY)
        err_option_arg(opt, opt_arg);
    return result;
}

/*
 * Command line utility.
 */
static void
check_obj_option(const char *opt, const char *opt_arg)
{
    unsigned int i;

    if (strcmp("all", opt_arg) == 0)
        return;

    /* Issue an error about the unrecognized option argument. */
    /* Make it specific on whether this argument is a chunk name. */
    for (i = 0; i < 4; ++i)
    {
        /* Do not use isalpha(), because it is locale-dependent. */
        if (!((opt_arg[i] >= 'A' && opt_arg[i] <= 'Z') ||
              (opt_arg[i] >= 'a' && opt_arg[i] <= 'z')))
            break;
    }
    if (i == 4 && opt_arg[i] == '\0')
        error("Manipulation of individual chunks is not implemented");
    else
        err_option_arg(opt, opt_arg);
}

/*
 * Command line parsing.
 */
static int
scan_option(const char *str,
            char opt_buf[], size_t opt_buf_size, const char **opt_arg_ptr)
{
    const char *ptr;
    unsigned int opt_len;

    /* Check if arg is an "-option". */
    if (str[0] != '-' || str[1] == '\0')  /* no "-option", or just "-" */
        return 0;

    /* Extract the normalized option, and possibly the option argument. */
    opt_len = 0;
    ptr = str + 1;
    while (*ptr == '-')  /* "--option", "---option", etc. */
        ++ptr;
    if (*ptr == '\0')  /* "--" */
        --ptr;
    for ( ; ; )
    {
        if (opt_len < opt_buf_size)  /* truncate "-verylongoption" */
            opt_buf[opt_len] = (char)tolower(*ptr);
        ++opt_len;
        ++ptr;
        if (*ptr == '\0' || isspace(*ptr))  /* "-option" or "-option arg" */
        {
            while (isspace(*ptr))
                ++ptr;
            *opt_arg_ptr = (*ptr != '\0') ? ptr : NULL;
            break;
        }
        if (*ptr == '=')  /* "-option=arg" */
        {
            ++ptr;
            *opt_arg_ptr = ptr;
            break;
        }
    }

    /* Finalize the normalized option. */
    if (opt_buf_size > 0)
    {
        if (opt_len < opt_buf_size)
            opt_buf[opt_len] = '\0';
        else
            opt_buf[opt_buf_size - 1] = '\0';
    }
    return 1;
}

/*
 * Command line parsing.
 */
static void
parse_args(int argc, char *argv[])
{
    char *arg;
    char opt[16];
    size_t opt_len;
    const char *xopt;
    int simple_opt, stop_switch;
    opng_bitset_t set;
    int val;
    unsigned int file_count;
    int i;

    /* Initialize. */
    memset(&options, 0, sizeof(options));
    options.optim_level = -1;
    options.interlace = -1;
    file_count = 0;

    /* Iterate over args. */
    stop_switch = 0;
    for (i = 1; i < argc; ++i)
    {
        arg = argv[i];
        if (stop_switch || scan_option(arg, opt, sizeof(opt), &xopt) < 1)
        {
            ++file_count;
            continue;  /* leave file names for process_files() */
        }
        opt_len = strlen(opt);

        /* Prevent process_files() from seeing this arg. */
        argv[i] = NULL;

        /* Normalize the options that allow juxtaposed arguments. */
        if ((strchr("fio", opt[0]) != NULL && isdigit(opt[1])) ||
            (opt[0] == 'z' && isalpha(opt[1]) && isdigit(opt[2])))
        {
            /* -f0-5 <=> -f=0-5; -i1 <=> -i=1; -o3 <=> -o=3;
             * -zc3-9 <=> -zc=3-9; etc.
             */
            opt_len = (size_t)(opng_strpbrk_digit(opt) - opt);
            opt[opt_len] = '\0';
            xopt = opng_strpbrk_digit(arg);
        }

        /* Check the simple options (without option arguments). */
        simple_opt = 1;
        if (strcmp("-", opt) == 0)
        {
            /* -- */
            stop_switch = 1;
        }
        else if (strcmp("?", opt) == 0 ||
                 strncmp("help", opt, opt_len) == 0)
        {
            /* -? | -h | ... | -help */
            local_options.help = 1;
        }
        else if ((strncmp("backup", opt, opt_len) == 0) ||
                 (strncmp("keep", opt, opt_len) == 0))
        {
            /* -b | ... | -backup | -k | ... | -keep */
            options.backup = 1;
        }
        else if (strncmp("clobber", opt, opt_len) == 0)
        {
            /* -c | ... | -clobber */
            options.clobber = 1;
        }
        else if (strcmp("debug", opt) == 0)
        {
            /* -debug */
            /* Do not abbreviate this internal option. */
            options.debug = 1;
        }
        else if (strncmp("fix", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -fi | -fix */
            options.fix = 1;
        }
        else if (strncmp("force", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -fo | ... | -force */
            options.force = 1;
        }
        else if (strncmp("full", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -fu | ... | -full */
            options.full = 1;
        }
        else if (strcmp("nb", opt) == 0)
        {
            /* -nb */
            options.nb = 1;
        }
        else if (strcmp("nc", opt) == 0)
        {
            /* -nc */
            options.nc = 1;
        }
        else if (strcmp("np", opt) == 0)
        {
            /* -np */
            options.np = 1;
        }
        else if (strcmp("nx", opt) == 0)
        {
            /* -nx */
            options.nb = options.nc = options.np = 1;
            /* options.nm = 1; */
        }
        else if (strcmp("nz", opt) == 0)
        {
            /* -nz */
            options.nz = 1;
        }
        else if (strncmp("preserve", opt, opt_len) == 0)
        {
            /* -p | ... | -preserve */
            options.preserve = 1;
        }
        else if ((strncmp("quiet", opt, opt_len) == 0) ||
                 (strncmp("silent", opt, opt_len) == 0 && opt_len >= 3))
        {
            /* -q | ... | -quiet | -sil | ... | -silent */
            options.quiet = 1;
        }
        else if (strncmp("simulate", opt, opt_len) == 0 && opt_len >= 3)
        {
            /* -sim | ... | -simulate */
            options.simulate = 1;
        }
        else if (strncmp("snip", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -sn | ... | -snip */
            options.snip = 1;
        }
        else if (strcmp("v", opt) == 0)
        {
            /* -v */
            options.verbose = 1;
            local_options.version = 1;
        }
        else if (strncmp("verbose", opt, opt_len) == 0 && opt_len >= 4)
        {
            /* -verb | ... | -verbose */
            options.verbose = 1;
        }
        else if (strncmp("version", opt, opt_len) == 0 && opt_len >= 4)
        {
            /* -vers | ... | -version */
            local_options.version = 1;
        }
        else  /* possibly an option with an argument */
        {
            simple_opt = 0;
            if (xopt == NULL)
            {
                if (++i < argc)
                {
                    xopt = argv[i];
                    /* Prevent process_files() from seeing this xopt. */
                    argv[i] = NULL;
                }
                else
                {
                    /* Last option in command line; assume an empty xopt. */
                    xopt = "";
                }
            }
        }

        /* Check the options that have option arguments. */
        if (simple_opt)
        {
            if (xopt != NULL)
                error("No argument allowed for option: %s", arg);
        }
        else if (strcmp("o", opt) == 0)
        {
            /* -o NUM */
            val = check_num_option("-o", xopt, 0, INT_MAX);
            if (options.optim_level < 0)
                options.optim_level = val;
            else if (options.optim_level != val)
                error("Multiple optimization levels are not permitted");
        }
        else if (strcmp("i", opt) == 0)
        {
            /* -i NUM */
            val = check_num_option("-i", xopt, 0, 1);
            if (options.interlace < 0)
                options.interlace = val;
            else if (options.interlace != val)
                error("Multiple interlace types are not permitted");
        }
        else if (strcmp("f", opt) == 0)
        {
            /* -f SET */
            set = check_rangeset_option("-f", xopt, OPNG_FILTER_SET_MASK);
            options.filter_set |= set;
        }
        else if (strcmp("zc", opt) == 0)
        {
            /* -zc SET */
            set = check_rangeset_option("-zc", xopt, OPNG_COMPR_LEVEL_SET_MASK);
            options.compr_level_set |= set;
        }
        else if (strcmp("zm", opt) == 0)
        {
            /* -zm SET */
            set = check_rangeset_option("-zm", xopt, OPNG_MEM_LEVEL_SET_MASK);
            options.mem_level_set |= set;
        }
        else if (strcmp("zs", opt) == 0)
        {
            /* -zs SET */
            set = check_rangeset_option("-zs", xopt, OPNG_STRATEGY_SET_MASK);
            options.strategy_set |= set;
        }
        else if (strcmp("zw", opt) == 0)
        {
            /* -zw NUM */
            val = check_power2_option("-zw", xopt, 8, 15);
            if (options.window_bits == 0)
                options.window_bits = val;
            else if (options.window_bits != val)
                error("Multiple window sizes are not permitted");
        }
        else if (strncmp("strip", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -st OBJ | ... | -strip OBJ */
            check_obj_option("-strip", xopt);
            options.strip_all = 1;
        }
        else if (strncmp("out", opt, opt_len) == 0 && opt_len >= 2)
        {
            /* -ou PATH | -out PATH */
            if (options.out_name != NULL)
                error("Multiple output file names are not permitted");
            if (xopt[0] == '\0')
                err_option_arg("-out", NULL);
            options.out_name = xopt;
        }
        else if (strncmp("dir", opt, opt_len) == 0)
        {
            /* -d PATH | ... | -dir PATH */
            if (options.dir_name != NULL)
                error("Multiple output dir names are not permitted");
            if (xopt[0] == '\0')
                err_option_arg("-dir", NULL);
            options.dir_name = xopt;
        }
        else if (strncmp("log", opt, opt_len) == 0)
        {
            /* -l PATH | ... | -log PATH */
            if (options.log_name != NULL)
                error("Multiple log file names are not permitted");
            if (xopt[0] == '\0')
                err_option_arg("-log", NULL);
            options.log_name = xopt;
        }
        else
        {
            error("Unrecognized option: %s", arg);
        }
    }

    /* Finalize. */
    if (options.out_name != NULL)
    {
        if (file_count > 1)
            error("The option -out requires one input file");
        if (options.dir_name != NULL)
            error("The options -out and -dir are mutually exclusive");
    }
    if (options.log_name != NULL)
    {
        if (opng_strcasecmp(".log", opng_strtail(options.log_name, 4)) != 0)
            error("To prevent accidental data corruption, "
                  "the log file name must end with \".log\"");
    }
    if (local_options.help)
        operation = OP_SHOW_HELP;
    else if (file_count != 0)
        operation = OP_RUN;
    else if (local_options.version)
        operation = OP_SHOW_VERSION;
    else
        operation = OP_SHOW_HELP;
}

/*
 * Application-defined printf callback.
 */
static void
app_printf(const char *fmt, ...)
{
    va_list arg_ptr;

    if (fmt[0] == '\0')
        return;
    start_of_line = (fmt[strlen(fmt) - 1] == '\n') ? 1 : 0;

    if (con_file != NULL)
    {
        va_start(arg_ptr, fmt);
        vfprintf(con_file, fmt, arg_ptr);
        va_end(arg_ptr);
    }
    if (log_file != NULL)
    {
        va_start(arg_ptr, fmt);
        vfprintf(log_file, fmt, arg_ptr);
        va_end(arg_ptr);
    }
}

/*
 * Application-defined control print callback.
 */
static void
app_print_cntrl(int cntrl_code)
{
    const char *con_str, *log_str;
    int i;

    if (cntrl_code == '\r')
    {
        /* CR: reset line in console, new line in log file. */
        con_str = "\r";
        log_str = "\n";
        start_of_line = 1;
    }
    else if (cntrl_code == '\v')
    {
        /* VT: new line if current line is not empty, nothing otherwise. */
        if (!start_of_line)
        {
            con_str = log_str = "\n";
            start_of_line = 1;
        }
        else
            con_str = log_str = "";
    }
    else if (cntrl_code < 0 && cntrl_code > -80 && start_of_line)
    {
        /* Minus N: erase first N characters from line, in console only. */
        if (con_file != NULL)
        {
            for (i = 0; i > cntrl_code; --i)
                fputc(' ', con_file);
        }
        con_str = "\r";
        log_str = "";
    }
    else
    {
        /* Unhandled control code (due to internal error): show err marker. */
        con_str = log_str = "<?>";
    }

    if (con_file != NULL)
        fputs(con_str, con_file);
    if (log_file != NULL)
        fputs(log_str, log_file);
}

/*
 * Application-defined progress update callback.
 */
static void
app_progress(unsigned long current_step, unsigned long total_steps)
{
    /* There will be a potentially long wait, so flush the console output. */
    if (con_file != NULL)
        fflush(con_file);
    /* An eager flush of the line-buffered log file is not very important. */

    /* A GUI application would normally update a progress bar. */
    /* Here we ignore the progress info. */
    if (current_step && total_steps)
        return;
}

/*
 * Application initialization.
 */
static void
app_init(void)
{
    start_of_line = 1;

    if (operation == OP_SHOW_HELP || operation == OP_SHOW_VERSION)
        con_file = stdout;
    else if (!options.quiet)
        con_file = stderr;
    else
        con_file = NULL;

    if (options.log_name != NULL)
    {
        /* Open the log file, line-buffered. */
        if ((log_file = fopen(options.log_name, "a")) == NULL)
            error("Can't open log file: %s\n", options.log_name);
        setvbuf(log_file, NULL, _IOLBF, BUFSIZ);
        app_printf("** Warning: %s\n\n",
                   "The option -log is deprecated; use shell redirection");
    }
}

/*
 * Application finalization.
 */
static void
app_finish(void)
{
    if (log_file != NULL)
    {
        /* Close the log file. */
        fclose(log_file);
    }
}

/*
 * File list processing.
 */
static int
process_files(int argc, char *argv[])
{
    int result;
    struct opng_ui ui;
    int i;

    /* Initialize the optimization engine. */
    ui.printf_fn = app_printf;
    ui.print_cntrl_fn = app_print_cntrl;
    ui.progress_fn = app_progress;
    ui.panic_fn = panic;
    if (opng_initialize(&options, &ui) != 0)
        panic("Can't initialize optimization engine");

    /* Iterate over file names. */
    result = EXIT_SUCCESS;
    for (i = 1; i < argc; ++i)
    {
        if (argv[i] == NULL || argv[i][0] == '\0')
            continue;  /* this was an "-option" */
        if (opng_optimize(argv[i]) != 0)
            result = EXIT_FAILURE;
    }

    /* Finalize the optimization engine. */
    if (opng_finalize() != 0)
        panic("Can't finalize optimization engine");

    return result;
}

/*
 * The main function.
 */
int
main(int argc, char *argv[])
{
    int result;

    /* Parse the user options and initialize the application. */
    parse_args(argc, argv);
    app_init();
    result = EXIT_SUCCESS;

    if (local_options.version)
    {
        /* Print the copyright and version info. */
        app_printf("%s\n", msg_intro);
    }

    switch (operation)
    {
    case OP_RUN:
        /* Run the application. */
        result = process_files(argc, argv);
        break;
    case OP_SHOW_HELP:
        if (local_options.help)
        {
            /* Print the extended help text. */
            app_printf("%s%s%s",
                       msg_help_synopsis,
                       msg_help_options,
                       msg_help_examples);
        }
        else
        {
            /* Print the basic help text. */
            app_printf("%s%s%s%s",
                       msg_help_synopsis,
                       msg_help_basic_options,
                       msg_help_examples,
                       msg_help_more);
        }
        break;
    case OP_SHOW_VERSION:
        /* Print the licensing terms and the extended version info. */
        app_printf("%s\n", msg_license);
        app_printf("Using libpng version %s and zlib version %s\n",
                   png_get_libpng_ver(NULL), zlibVersion());
        break;
    default:
        result = -1;
    }

    /* Finalize the application. */
    app_finish();
    return result;
}
