#ifndef DMC_UNRAR_HEADER_INCLUDED
#define DMC_UNRAR_HEADER_INCLUDED

/* --- System properties --- */

/* 32-bit or 64-bit CPU? Set one to 1, or let the autodetect figure it out later. */
#ifndef DMC_UNRAR_32BIT
#define DMC_UNRAR_32BIT 0
#endif
#ifndef DMC_UNRAR_64BIT
#define DMC_UNRAR_64BIT 0
#endif

/* --- Library features --- */

/* Unsigned type to use for file size. Needs to be at least 64-bit wide for
 * dmc_unrar to support large files (>= 2GB). */
#ifndef DMC_UNRAR_SIZE_T
#define DMC_UNRAR_SIZE_T uint64_t
#endif

/* Signed type to use for file size. Needs to be at least 64-bit wide for
 * dmc_unrar to support large files (>= 2GB). */
#ifndef DMC_UNRAR_OFFSET_T
#define DMC_UNRAR_OFFSET_T int64_t
#endif

/* Set DMC_UNRAR_DISABLE_MALLOC to 1 to disable all calls to malloc, realloc
 * and free. Note that if DMC_UNRAR_DISABLE_MALLOC is set to 1, the user must
 * always provide custom user alloc/realloc/free callbacks to the archive API. */
#ifndef DMC_UNRAR_DISABLE_MALLOC
#define DMC_UNRAR_DISABLE_MALLOC 0
#endif

/* The bitstream decoder uses be32toh()/be64toh() on GNU/Linux by default.
 * Set DMC_UNRAR_DISABLE_BE32TOH_BE64TOH to 1 if you don't want that. */
#ifndef DMC_UNRAR_DISABLE_BE32TOH_BE64TOH
#define DMC_UNRAR_DISABLE_BE32TOH_BE64TOH 0
#endif

/* Set DMC_UNRAR_DISABLE_STDIO to 1 to disable functionality that use the
 * stdio file open/read/write functions. */
#ifndef DMC_UNRAR_DISABLE_STDIO
#define DMC_UNRAR_DISABLE_STDIO 0
#endif

/* Set DMC_UNRAR_USE_FSEEKO_FTELLO to 1 to use fseeko/ftello for file seeking.
 * Set DMC_UNRAR_USE_FSEEKO_FTELLO to 0 to use fseek/ftell for file seeking.
 * Leave DMC_UNRAR_USE_FSEEKO_FTELLO unset to use it on 32-bit macOS and
 * glibc builds. */
#if 0
#define DMC_UNRAR_USE_FSEEKO_FTELLO 0
#define DMC_UNRAR_USE_FSEEKO_FTELLO 1
#endif

/* Set DMC_UNRAR_DISABLE_WIN32 to 1 to never use the WIN32 API for file IO.
 * Set DMC_UNRAR_DISABLE_WIN32 to 0 to always use the WIN32 API for file IO.
 * Leave DMC_UNRAR_DISABLE_WIN32 unset to autodetect.
 *
 * On Windows, dmc_unrar_is_rar_path(), dmc_unrar_archive_open_path() and
 * dmc_unrar_extract_file_to_path() will only support plain ASCII paths
 * without the WIN32 API. With the WIN32 API, they support full UTF-8 paths. */
#if 0
#define DMC_UNRAR_DISABLE_WIN32 1
#define DMC_UNRAR_DISABLE_WIN32 0
#endif

/* RAR 2.9/3.6 can optionally compress text using the PPMd algorithm.
 * The PPMd decoder is rather big, uses a lot of memory and needs compiler
 * support for pragma pack. If you don't need to decompress RAR archives
 * with PPMd data, set DMC_UNRAR_DISABLE_PPMD to 1 to disable the whole PPMd
 * decoder. Trying to extract PPMd'd files will then return an error.
 *
 * RAR archives with PPMd data should be relatively rare: WinRAR by default
 * doesn't use this feature and it has to be explicitly enabled. */
#ifndef DMC_UNRAR_DISABLE_PPMD
#define DMC_UNRAR_DISABLE_PPMD 0
#endif

/* RAR 2.9/3.6 can optionally filter the file data through a RARVM program.
 * We don't support generic RARVM bytecode, but we do detect certain
 * commonly used WinRAR stock filters and reimplement them in C. This
 * feature however needs another relatively large memory buffer. If you
 * don't need to decompress RAR archives with filters in them, set
 * DMC_UNRAR_DISABLE_FILTERS to 1 to disable the filters. Trying to
 * extract files with filters will then return an error.
 *
 * RAR archives with filters are relatively common, though: WinRAR by
 * defaults tries a few of them on common files formats, like JPEGs. */
#ifndef DMC_UNRAR_DISABLE_FILTERS
#define DMC_UNRAR_DISABLE_FILTERS 0
#endif

/* Do we have large file (>= 2GB) support? Can be defined to force enabling
 * or disabling of large file support, or kept undefined to let the
 * autodetection figure it out. The autodetection errs on being conservative,
 * so see below for detail.
 *
 * There's two types of large files:
 * - Files within archives that decompress to >= 2GB
 * - Archives that are >= 2GB
 *
 * Archives that are >= 2GB can contain either small files or large files.
 * Files within archives that decompress to >= 2GB can be within small archives
 * (if they compress well) or large archives (if they don't).
 *
 * If we don't have large file support and are asked to extract a large file,
 * the error DMC_UNRAR_FILE_UNSUPPORTED_LARGE will be returned instead. However,
 * we can't properly detect if an archive itself is large without large file
 * support, so this case will have to be caught by this library's user.
 */
#if 0
#define DMC_HAS_LARGE_FILE_SUPPORT 1
#define DMC_HAS_LARGE_FILE_SUPPORT 0
#endif

/* Initial capacity of our internal arrays. Larger values mean less
 * reallocations as new files are discovered in an archive, but wasted
 * memory on archives with few files. The arrays grow exponential, though. */
#ifndef DMC_UNRAR_ARRAY_INITIAL_CAPACITY
#define DMC_UNRAR_ARRAY_INITIAL_CAPACITY 8
#endif

/* Number of bytes used for the bitstream buffer. Larger values means more
 * speed, but also more memory. Must be a multiple of 8. 4096 seems to be
 * a good compromise. */
#ifndef DMC_UNRAR_BS_BUFFER_SIZE
#define DMC_UNRAR_BS_BUFFER_SIZE 4096
#endif

/* Max depth to create a Huffman decoding table for. The Huffman decoder uses
 * a dual tree/table approach: codes shorter than this max depth are decoded
 * directly, by a single load from this table. For longer codes, the remaining
 * bits trace a binary table.
 *
 * Higher max depths mean more memory ((2^depth) * 4 bytes), and considerable
 * longer times to build the table in the first place. 10 is a good compromise. */
#ifndef DMC_UNRAR_HUFF_MAX_TABLE_DEPTH
#define DMC_UNRAR_HUFF_MAX_TABLE_DEPTH 10
#endif

/* --- Basic types --- */

/* We need to set those to get be32toh()/be64toh(). */
#if defined(__linux__) && (DMC_UNRAR_DISABLE_BE32TOH_BE64TOH != 1)
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#endif

/* If you don't have stdint.h and/or stddef.h, you need to typedef the following types and macros. */
#include <stdint.h>
#include <stddef.h>

#if 0
typedef signed char int8_t
typedef signed int int32_t;
typedef signed long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

/* Only used by the PPMd decoder, so could be left out with PPMd disabled. */
typedef uint64_t uintptr_t;
#endif

#ifndef __cplusplus
/* If you don't have stdbool.h, you need to define the following types and macros. */
#include <stdbool.h>

#if 0
typedef int bool;
#define true (1)
#define false (0)
#endif

#endif /* __cplusplus */

#if DMC_UNRAR_DISABLE_STDIO != 1
#include <stdio.h>
#endif

typedef DMC_UNRAR_SIZE_T dmc_unrar_size_t;
typedef DMC_UNRAR_OFFSET_T dmc_unrar_offset_t;

#define DMC_UNRAR_SIZE_MAX ((dmc_unrar_size_t)((dmc_unrar_offset_t)-1))

/* --- System autodetection --- */

/* Autodetecting whether we're on a 64-bit CPU. */
#if (DMC_UNRAR_32BIT != 1) && (DMC_UNRAR_64BIT != 1)
#undef DMC_UNRAR_32BIT
#undef DMC_UNRAR_64BIT

#if defined(_M_X64) || defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__) || defined(__ia64__) || defined(__x86_64__)
#define DMC_UNRAR_32BIT 0
		#define DMC_UNRAR_64BIT 1
#else
#define DMC_UNRAR_32BIT 1
#define DMC_UNRAR_64BIT 0
#endif
#elif (DMC_UNRAR_32BIT == 1) && (DMC_UNRAR_64BIT == 1)
#error Both DMC_UNRAR_32BIT and DMC_UNRAR_64BIT set to 1
#endif

#if (DMC_UNRAR_BS_BUFFER_SIZE <= 0) || ((DMC_UNRAR_BS_BUFFER_SIZE % 8) != 0)
#error DMC_UNRAR_BS_BUFFER_SIZE must be a multiple of 8
#endif

/* Autodetecting whether we're on Win32. */
#ifndef DMC_UNRAR_DISABLE_WIN32
#ifdef _WIN32
#define DMC_UNRAR_DISABLE_WIN32 0
#else
#define DMC_UNRAR_DISABLE_WIN32 1
#endif
#endif

/* Autodetecting whether we should use fseeko/ftello. */
#if DMC_UNRAR_DISABLE_STDIO != 1 && !defined(DMC_UNRAR_USE_FSEEKO_FTELLO)
#if (defined(__APPLE__) || defined(__GLIBC__)) && DMC_UNRAR_32BIT == 1
#define DMC_UNRAR_USE_FSEEKO_FTELLO 1
#else
#define DMC_UNRAR_USE_FSEEKO_FTELLO 0
#endif
#endif

/* Autodetection whether we have large file support. */
#ifndef DMC_HAS_LARGE_FILE_SUPPORT
#if defined(__APPLE__)
/* On macOS, we should always have large file support, but we need to
		 * use fseeko/ftello on 32-bit builds. */

		#if DMC_UNRAR_64BIT == 1
			#define DMC_HAS_LARGE_FILE_SUPPORT 1
		#else
			#if DMC_UNRAR_USE_FSEEKO_FTELLO == 1
				#define DMC_HAS_LARGE_FILE_SUPPORT 1
			#else
				#define DMC_HAS_LARGE_FILE_SUPPORT 0
			#endif
		#endif

#elif defined(_WIN32)
/* On Windows, we should always have large file support with the WinAPI. */

		#if DMC_UNRAR_DISABLE_WIN32 == 0
			#define DMC_HAS_LARGE_FILE_SUPPORT 1
		#else
			#define DMC_HAS_LARGE_FILE_SUPPORT 0
		#endif
#elif defined(__GLIBC__)
/* On glibc system (Linux), we should always have large file support on
		 * 64-bit builds. On 32-bit builds, we need _FILE_OFFSET_BITS set to 64
		 * and use fseeko/ftello. */

		#if DMC_UNRAR_64BIT == 1
			#define DMC_HAS_LARGE_FILE_SUPPORT 1
		#else
			#if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS == 64 && DMC_UNRAR_USE_FSEEKO_FTELLO == 1
				#define DMC_HAS_LARGE_FILE_SUPPORT 1
			#else
				#define DMC_HAS_LARGE_FILE_SUPPORT 0
			#endif
		#endif
#else
/* Otherwise, we have no clue. Assume we don't have large file support. */

#define DMC_HAS_LARGE_FILE_SUPPORT 0
#endif
#endif

/* Make sure our dmc_unrar_size_t and dmc_unrar_offset_t types are large enough. */
#if DMC_HAS_LARGE_FILE_SUPPORT == 1
typedef unsigned char dmc_unrar_validate_size_t  [sizeof(dmc_unrar_size_t  )>=8 ? 1 : -1];
	typedef unsigned char dmc_unrar_validate_offset_t[sizeof(dmc_unrar_offset_t)>=8 ? 1 : -1];
#else
typedef unsigned char dmc_unrar_validate_size_t  [sizeof(dmc_unrar_size_t  )>=4 ? 1 : -1];
typedef unsigned char dmc_unrar_validate_offset_t[sizeof(dmc_unrar_offset_t)>=4 ? 1 : -1];
#endif

/* --- Windows-specific headers --- */

#if DMC_UNRAR_DISABLE_WIN32 != 1
#define WIN32_MEAN_AND_LEAN
	#include <windows.h>
	#if DMC_UNRAR_DISABLE_STDIO != 1
		#include <io.h>
	#endif
#endif

/* --- API types and macros --- */

/* Heap allocation functions. */
typedef void *(*dmc_unrar_alloc_func)  (void *opaque, dmc_unrar_size_t items, dmc_unrar_size_t size);
typedef void *(*dmc_unrar_realloc_func)(void *opaque, void *address, dmc_unrar_size_t items, dmc_unrar_size_t size);
typedef void  (*dmc_unrar_free_func)   (void *opaque, void *address);

typedef dmc_unrar_size_t (*dmc_unrar_read_func)(void *opaque, void *buffer, dmc_unrar_size_t n);
typedef int              (*dmc_unrar_seek_func)(void *opaque, dmc_unrar_offset_t offset);



/* --- Public unrar API --- */

#ifdef __cplusplus
extern "C" {
#endif

#define DMC_UNRAR_VERSION "1.7.0"
#define DMC_UNRAR_VERSION_MAJOR 1
#define DMC_UNRAR_VERSION_MINOR 7
#define DMC_UNRAR_VERSION_PATCH 0
#define DMC_UNRAR_VERSION_FULL ( (DMC_UNRAR_VERSION_MAJOR * 100000) + (DMC_UNRAR_VERSION_MINOR * 10000) + (DMC_UNRAR_VERSION_PATCH) )

/** The return code of a dmc_unrar operation. See dmc_unrar_strerror(). */
typedef enum {
    DMC_UNRAR_OK = 0,

    DMC_UNRAR_NO_ALLOC,
    DMC_UNRAR_ALLOC_FAIL,

    DMC_UNRAR_OPEN_FAIL,
    DMC_UNRAR_READ_FAIL,
    DMC_UNRAR_WRITE_FAIL,
    DMC_UNRAR_SEEK_FAIL,

    DMC_UNRAR_INVALID_DATA,

    DMC_UNRAR_ARCHIVE_EMPTY,

    DMC_UNRAR_ARCHIVE_IS_NULL,
    DMC_UNRAR_ARCHIVE_NOT_CLEARED,
    DMC_UNRAR_ARCHIVE_MISSING_FIELDS,

    DMC_UNRAR_ARCHIVE_NOT_RAR,
    DMC_UNRAR_ARCHIVE_UNSUPPORTED_ANCIENT,

    DMC_UNRAR_ARCHIVE_UNSUPPORTED_VOLUMES,
    DMC_UNRAR_ARCHIVE_UNSUPPORTED_ENCRYPTED,

    DMC_UNRAR_FILE_IS_INVALID,
    DMC_UNRAR_FILE_IS_DIRECTORY,

    DMC_UNRAR_FILE_SOLID_BROKEN,
    DMC_UNRAR_FILE_CRC32_FAIL,

    DMC_UNRAR_FILE_UNSUPPORTED_VERSION,
    DMC_UNRAR_FILE_UNSUPPORTED_METHOD,
    DMC_UNRAR_FILE_UNSUPPORTED_ENCRYPTED,
    DMC_UNRAR_FILE_UNSUPPORTED_SPLIT,
    DMC_UNRAR_FILE_UNSUPPORTED_LINK,
    DMC_UNRAR_FILE_UNSUPPORTED_LARGE,

    DMC_UNRAR_HUFF_RESERVED_SYMBOL,
    DMC_UNRAR_HUFF_PREFIX_PRESENT,
    DMC_UNRAR_HUFF_INVALID_CODE,

    DMC_UNRAR_PPMD_INVALID_MAXORDER,

    DMC_UNRAR_FILTERS_UNKNOWN,
    DMC_UNRAR_FILTERS_INVALID_FILTER_INDEX,
    DMC_UNRAR_FILTERS_REUSE_LENGTH_NEW_FILTER,
    DMC_UNRAR_FILTERS_INVALID_LENGTH,
    DMC_UNRAR_FILTERS_INVALID_FILE_POSITION,
    DMC_UNRAR_FILTERS_XOR_SUM_NO_MATCH,

    DMC_UNRAR_15_INVALID_FLAG_INDEX,
    DMC_UNRAR_15_INVALID_LONG_MATCH_OFFSET_INDEX,

    DMC_UNRAR_20_INVALID_LENGTH_TABLE_DATA,

    DMC_UNRAR_30_DISABLED_FEATURE_PPMD,
    DMC_UNRAR_30_DISABLED_FEATURE_FILTERS,

    DMC_UNRAR_30_INVALID_LENGTH_TABLE_DATA,

    DMC_UNRAR_50_DISABLED_FEATURE_FILTERS,

    DMC_UNRAR_50_INVALID_LENGTH_TABLE_DATA,
    DMC_UNRAR_50_BLOCK_CHECKSUM_NO_MATCH

} dmc_unrar_return;

/** The operating system a file was packed into a RAR. */
typedef enum {
    DMC_UNRAR_HOSTOS_DOS   = 0, /**< DOS, MS-DOS. */
    DMC_UNRAR_HOSTOS_OS2   = 1, /**< OS/2. */
    DMC_UNRAR_HOSTOS_WIN32 = 2, /**< Windows. */
    DMC_UNRAR_HOSTOS_UNIX  = 3, /**< Unix. */
    DMC_UNRAR_HOSTOS_MACOS = 4, /**< Mac OS. */
    DMC_UNRAR_HOSTOS_BEOS  = 5  /**< BeOS. */
} dmc_unrar_host_os;

/** DOS/Windows file attributes. */
typedef enum {
    DMC_UNRAR_ATTRIB_DOS_READONLY    = 0x00001,
    DMC_UNRAR_ATTRIB_DOS_HIDDEN      = 0x00002,
    DMC_UNRAR_ATTRIB_DOS_SYSTEM      = 0x00004,
    DMC_UNRAR_ATTRIB_DOS_VOLUMELABEL = 0x00008,
    DMC_UNRAR_ATTRIB_DOS_DIRECTORY   = 0x00010,
    DMC_UNRAR_ATTRIB_DOS_ARCHIVE     = 0x00020,
    DMC_UNRAR_ATTRIB_DOS_DEVICE      = 0x00040,
    DMC_UNRAR_ATTRIB_DOS_NORMAL      = 0x00080,
    DMC_UNRAR_ATTRIB_DOS_TEMPORARY   = 0x00100,
    DMC_UNRAR_ATTRIB_DOS_SPARSE      = 0x00200,
    DMC_UNRAR_ATTRIB_DOS_SYMLINK     = 0x00400,
    DMC_UNRAR_ATTRIB_DOS_COMPRESSED  = 0x00800,
    DMC_UNRAR_ATTRIB_DOS_OFFLINE     = 0x01000,
    DMC_UNRAR_ATTRIB_DOS_NOTINDEXED  = 0x02000,
    DMC_UNRAR_ATTRIB_DOS_ENCRYPTED   = 0x04000,
    DMC_UNRAR_ATTRIB_DOS_INTEGRITY   = 0x08000,
    DMC_UNRAR_ATTRIB_DOS_VIRTUAL     = 0x10000,
    DMC_UNRAR_ATTRIB_DOS_NOSCRUB     = 0x20000
} dmc_unrar_windows_attribute;

/** Unix file attributes. */
typedef enum {
    /* Mask to check for the types of a file. */
    DMC_UNRAR_ATTRIB_UNIX_FILETYPE_MASK       = 0170000,
    /* Mask to check for the permissions of a file. */
    DMC_UNRAR_ATTRIB_UNIX_PERMISSIONS_MASK    = 0007777,

    /* .--- File types. Mutually exclusive. */
    DMC_UNRAR_ATTRIB_UNIX_IS_SYMBOLIC_LINK    = 0120000,
    DMC_UNRAR_ATTRIB_UNIX_IS_SOCKET           = 0140000,

    DMC_UNRAR_ATTRIB_UNIX_IS_REGULAR_FILE     = 0100000,

    DMC_UNRAR_ATTRIB_UNIX_IS_BLOCK_DEVICE     = 0060000,
    DMC_UNRAR_ATTRIB_UNIX_IS_DIRECTORY        = 0040000,
    DMC_UNRAR_ATTRIB_UNIX_IS_CHARACTER_DEVICE = 0020000,
    DMC_UNRAR_ATTRIB_UNIX_IS_FIFO             = 0010000,
    /* '--- */

    /* .--- File permissions. OR-able. */
    DMC_UNRAR_ATTRIB_UNIX_SET_USER_ID         = 0004000,
    DMC_UNRAR_ATTRIB_UNIX_SET_GROUP_ID        = 0002000,
    DMC_UNRAR_ATTRIB_UNIX_STICKY              = 0001000,

    DMC_UNRAR_ATTRIB_UNIX_USER_READ           = 0000400,
    DMC_UNRAR_ATTRIB_UNIX_USER_WRITE          = 0000200,
    DMC_UNRAR_ATTRIB_UNIX_USER_EXECUTE        = 0000100,
    DMC_UNRAR_ATTRIB_UNIX_GROUP_READ          = 0000040,
    DMC_UNRAR_ATTRIB_UNIX_GROUP_WRITE         = 0000020,
    DMC_UNRAR_ATTRIB_UNIX_GROUP_EXECUTE       = 0000010,
    DMC_UNRAR_ATTRIB_UNIX_OTHER_READ          = 0000004,
    DMC_UNRAR_ATTRIB_UNIX_OTHER_WRITE         = 0000002,
    DMC_UNRAR_ATTRIB_UNIX_OTHER_EXECUTE       = 0000001
    /* '--- */
} dmc_unrar_unix_attribute;

struct dmc_unrar_internal_state_tag;
typedef struct dmc_unrar_internal_state_tag dmc_unrar_internal_state;

/** A file entry within a RAR archive. */
typedef struct dmc_unrar_file_tag {
    uint64_t compressed_size;   /**< Size of the compressed file data, in bytes. */
    uint64_t uncompressed_size; /**< Size of the uncompressed file data, in bytes. */

    /** The operating system on which the file was packed into the RAR. */
    dmc_unrar_host_os host_os;

    bool has_crc; /**< Does this file entry have a checksum? */

    uint32_t crc;       /**< Checksum (CRC-32, 0xEDB88320 polynomial). */
    uint64_t unix_time; /**< File modification timestamp, POSIX epoch format. */

    /** File attributes, operating-system-specific.
     *
     *  The meaning depends on the host_os value:
     *  - DMC_UNRAR_HOSTOS_DOS:   see dmc_unrar_windows_attribute
     *  - DMC_UNRAR_HOSTOS_OS2:   ???
     *  - DMC_UNRAR_HOSTOS_WIN32: see dmc_unrar_windows_attribute
     *  - DMC_UNRAR_HOSTOS_UNIX:  see dmc_unrar_unix_attribute
     *  - DMC_UNRAR_HOSTOS_MACOS: ???
     *  - DMC_UNRAR_HOSTOS_BEOS:  ???
     */
    uint64_t attrs;

} dmc_unrar_file;

typedef struct dmc_unrar_alloc_tag {
    dmc_unrar_alloc_func func_alloc;     /**< Memory allocation function, or NULL to use malloc(). */
    dmc_unrar_realloc_func func_realloc; /**< Memory allocation function, or NULL to use realloc(). */
    dmc_unrar_free_func func_free;       /**< Memory deallocation function, or NULL to use free(). */
    void *opaque;                        /**< Private data passed to func_alloc, func_realloc and func_free. */

} dmc_unrar_alloc;

/** Wrappers around stdio SEEK_* constants */
typedef enum {
#if DMC_UNRAR_DISABLE_STDIO == 1
    DMC_UNRAR_SEEK_SET = 0,
	DMC_UNRAR_SEEK_CUR = 1,
	DMC_UNRAR_SEEK_END = 2
#else
    DMC_UNRAR_SEEK_SET = SEEK_SET,
    DMC_UNRAR_SEEK_CUR = SEEK_CUR,
    DMC_UNRAR_SEEK_END = SEEK_END
#endif
} dmc_unrar_seek_origin;

typedef struct dmc_unrar_io_handler_tag {
    void *(*open)(const char *path);
    void (*close)(void *opaque);
    dmc_unrar_size_t (*read)(void *opaque, void *buffer, dmc_unrar_size_t n);
    bool (*seek)(void *opaque, dmc_unrar_offset_t offset, int origin);
    dmc_unrar_offset_t (*tell)(void *opaque);
} dmc_unrar_io_handler;

typedef struct dmc_unrar_io_tag {
    dmc_unrar_io_handler *funcs; /**< RAR file management functions. Must not be NULL. */
    void *opaque;                /**< Private data passed to funcs' pointers. */

    dmc_unrar_size_t size;       /**< Size of the IO stream. */

} dmc_unrar_io;

/** A RAR archive. */
typedef struct dmc_unrar_archive_tag {
    dmc_unrar_alloc alloc;
    dmc_unrar_io io;

    /** Private internal state. */
    dmc_unrar_internal_state *internal_state;

} dmc_unrar_archive;

/** Return a human-readable description of a return code. */
const char *dmc_unrar_strerror(dmc_unrar_return code);

/** Initialize an IO structure. */
bool dmc_unrar_io_init(dmc_unrar_io *io, dmc_unrar_io_handler *handler, void *opaque);

#if DMC_UNRAR_DISABLE_STDIO != 1
/** Initialize an IO structure from a FILE*. */
bool dmc_unrar_io_init_from_file(dmc_unrar_io *io, FILE *file, bool *allocated_new_opaque);
#endif

/** Close an IO structure. */
void dmc_unrar_io_close(dmc_unrar_io *io);

/** Read from an IO structure. */
dmc_unrar_size_t dmc_unrar_io_read(dmc_unrar_io *io, void *buffer, dmc_unrar_size_t n);

/** Seek in an IO structure. */
bool dmc_unrar_io_seek(dmc_unrar_io *io, dmc_unrar_offset_t offset, int origin);

/** Get the position in an IO structure. */
dmc_unrar_offset_t dmc_unrar_io_tell(dmc_unrar_io *io);

/** Detect whether an IO structure contains a RAR archive. */
bool dmc_unrar_is_rar(dmc_unrar_io *io);

/** Detect whether the memory region contains a RAR archive. */
bool dmc_unrar_is_rar_mem(const void *mem, dmc_unrar_size_t size);

#if DMC_UNRAR_DISABLE_STDIO != 1
/* Detect whether this FILE contains a RAR archive. */
bool dmc_unrar_is_rar_file(FILE *file);
#endif /* DMC_UNRAR_DISABLE_STDIO */

/** Detect whether the file at this path contains a RAR archive.
 *
 *  Please note that on Windows, full UTF-8 paths only work when using the WIN32 API
 *  (see DMC_UNRAR_DISABLE_WIN32 above). Without the WIN32 API, only plain ASCII paths
 *  are supported on Windows.
 *
 *  @param  path The path of the file to dmc_unrar_io_default_handler and read out of.
 *               This must be UTF-8.
 *  @return true on success
 */
bool dmc_unrar_is_rar_path(const char *path);

/** Initialize/clear this archive struct.
 *
 *  @param  archive A valid pointer to an archive structure to initialize.
 *  @return DMC_UNRAR_OK on success. Any other value is an error condition.
 */
dmc_unrar_return dmc_unrar_archive_init(dmc_unrar_archive *archive);

/** Open this RAR archive, reading its block and file headers.
 *  The io field must be initialized.
 *  The func_alloc, func_realloc, func_free and opaque_mem fields may be set.
 *  All other fields must have been cleared.
 *
 *  @param  archive Pointer to the archive structure to use. Needs to be a valid
 *                  pointer, with the fields properly initialized and set.
 *  @return DMC_UNRAR_OK if the archive was successfully opened. Any other value
 *          describes an error condition.
 */
dmc_unrar_return dmc_unrar_archive_open(dmc_unrar_archive *archive);

/** Open this RAR archive from a memory block, reading its block and file headers.
 *  The func_alloc, func_realloc, func_free and opaque_mem fields may be set.
 *  All other fields must have been cleared.
 *
 *  @param  archive Pointer to the archive structure to use. Needs to be a valid
 *                  pointer, with the fields properly initialized and set.
 *  @param  mem Pointer to a block of memory to read the RAR file out of.
 *  @param  size Size of the RAR memory region.
 *  @return DMC_UNRAR_OK if the archive was successfully opened. Any other value
 *          describes an error condition.
 */
dmc_unrar_return dmc_unrar_archive_open_mem(dmc_unrar_archive *archive,
                                            const void *mem, dmc_unrar_size_t size);

#if DMC_UNRAR_DISABLE_STDIO != 1
/** Open this RAR archive from a stdio FILE, reading its block and file headers.
 *  The func_alloc, func_realloc, func_free and opaque_mem fields may be set.
 *  All other fields must have been cleared.
 *
 *  The stdio FILE will be taken over and will be closed when the archive is
 *  closed with dmc_unrar_archive_close().
 *
 *  @param  archive Pointer to the archive structure to use. Needs to be a valid
 *                  pointer, with the fields properly initialized and set.
 *  @param  file The stdio FILE structure to read out of.
 *  @return DMC_UNRAR_OK if the archive was successfully opened. Any other value
 *          describes an error condition.
 */
dmc_unrar_return dmc_unrar_archive_open_file(dmc_unrar_archive *archive, FILE *file);
#endif /* DMC_UNRAR_DISABLE_STDIO */

/** Open this RAR archive from a path, opening the file with dmc_unrar_io_default_handler,
 *  and reading its block and file headers. The func_alloc, func_realloc, func_free and
 *  opaque_mem fields may be set. All other fields must have been cleared.
 *
 *  Please note that on Windows, full UTF-8 paths only work when using the WIN32 API
 *  (see DMC_UNRAR_DISABLE_WIN32 above). Without the WIN32 API, only plain ASCII paths
 *  are supported on Windows.
 *
 *  @param  archive Pointer to the archive structure to use. Needs to be a valid
 *                  pointer, with the fields properly initialized and set.
 *  @param  path The path of the file to dmc_unrar_io_default_handler and read out of.
 *               This must be UTF-8.
 *  @return DMC_UNRAR_OK if the archive was successfully opened. Any other value
 *          describes an error condition.
 */
dmc_unrar_return dmc_unrar_archive_open_path(dmc_unrar_archive *archive, const char *path);

/** Close this RAR archive again.
 *
 *  All allocated memory will be freed. */
void dmc_unrar_archive_close(dmc_unrar_archive *archive);

/** Get the global archive comment of a RAR archive.
 *
 *  Note: we don't necessarily know the encoding of this data, nor is
 *  the data always \0-terminated or even a human-readable string!
 *
 *  - RAR 5.0 always stores UTF-8 data.
 *  - RAR 2.9/3.6 stores either ASCII or UTF-16LE data.
 *    We don't know which is which.
 *  - RAR 2.0/2.6 stores *anything*.
 *  - RAR 1.5 doesn't support archive comments.
 *
 *  Use dmc_unrar_unicode_detect_encoding() to roughly detect the
 *  encoding of a comment.
 *
 *  Use dmc_unrar_unicode_convert_utf16le_to_utf8() to convert a
 *  UTF-16LE comment into UTF-8.
 *
 *  Returns the number of bytes written to comment. If comment is NULL, this function
 *  returns the number of bytes needed to fully store the comment.
 */
dmc_unrar_size_t dmc_unrar_get_archive_comment(dmc_unrar_archive *archive, void *comment,
                                               dmc_unrar_size_t comment_size);

/** Return the number of file entries in this RAR archive. */
dmc_unrar_size_t dmc_unrar_get_file_count(dmc_unrar_archive *archive);

/** Return the detailed information about a file entry, or NULL on error.
 *  Does not need to be free'd. */
const dmc_unrar_file *dmc_unrar_get_file_stat(dmc_unrar_archive *archive, dmc_unrar_size_t index);

/** Get the filename of a RAR file entry, UTF-8 encoded and \0-terminated.
 *
 *  Note: the filename is *not* checked to make sure it contains fully
 *  valid UTF-8 data. Use dmc_unrar_unicode_is_valid_utf8() and/or
 *  dmc_unrar_unicode_make_valid_utf8() for that.
 *
 *  Returns the number of bytes written to filename. If filename is NULL, this function
 *  returns the number of bytes needed to fully store the filename.
 */
dmc_unrar_size_t dmc_unrar_get_filename(dmc_unrar_archive *archive, dmc_unrar_size_t index,
                                        char *filename, dmc_unrar_size_t filename_size);

/** Is this file entry a directory? */
bool dmc_unrar_file_is_directory(dmc_unrar_archive *archive, dmc_unrar_size_t index);

/** Does this file entry have a comment attached? */
bool dmc_unrar_file_has_comment(dmc_unrar_archive *archive, dmc_unrar_size_t index);

/** Check if we support extracted this file entry.
 *
 *  If we do support extracting this file entry, DMC_UNRAR_OK is returned.
 *  Otherwise, the return code gives an idea why we don't have support. */
dmc_unrar_return dmc_unrar_file_is_supported(dmc_unrar_archive *archive, dmc_unrar_size_t index);

/** Get the comment of a file entry.
 *
 *  Note: we don't necessarily know the encoding of this data, nor is
 *  the data always \0-terminated or even a human-readable string!
 *
 *  Only RAR 2.0/2.6 supports file comments.
 *
 *  Use dmc_unrar_unicode_detect_encoding() to roughly detect the
 *  encoding of a comment.
 *
 *  Use dmc_unrar_unicode_convert_utf16le_to_utf8() to convert a
 *  UTF-16LE comment into UTF-8.
 *
 *  Returns the number of bytes written to comment. If comment is NULL, this function
 *  returns the number of bytes needed to fully store the comment.
 */
dmc_unrar_size_t dmc_unrar_get_file_comment(dmc_unrar_archive *archive, dmc_unrar_size_t index,
                                            void *comment, dmc_unrar_size_t comment_size);

/** Extract a file entry into a pre-allocated memory buffer.
 *
 *  @param  archive The archive to extract from.
 *  @param  index The index of the file entry to extract.
 *  @param  buffer The pre-allocated memory buffer to extract into.
 *  @param  buffer_size The size of the pre-allocated memory buffer.
 *  @param  uncompressed_size If != NULL, the number of bytes written
 *          to the buffer will be stored here.
 *  @param  validate_crc If true, validate the uncompressed data against
 *          the CRC-32 stored within the archive. If the validation fails,
 *          this counts as an error (DMC_UNRAR_FILE_CRC32_FAIL).
 *  @return An error condition, or DMC_UNRAR_OK if extraction succeeded.
 */
dmc_unrar_return dmc_unrar_extract_file_to_mem(dmc_unrar_archive *archive, dmc_unrar_size_t index,
                                               void *buffer, dmc_unrar_size_t buffer_size, dmc_unrar_size_t *uncompressed_size, bool validate_crc);

/** Extract a file entry into a dynamically allocated heap buffer.
 *
 *  @param  archive The archive to extract from.
 *  @param  index The index of the file entry to extract.
 *  @param  buffer The heap-allocated memory buffer will be stored here.
 *  @param  uncompressed_size The size of the heap-allocated memory buffer
 *          will be stored here. Must not be NULL.
 *  @param  validate_crc If true, validate the uncompressed data against
 *          the CRC-32 stored within the archive. If the validation fails,
 *          this counts as an error (DMC_UNRAR_FILE_CRC32_FAIL).
 *  @return An error condition, or DMC_UNRAR_OK if extraction succeeded.
 */
dmc_unrar_return dmc_unrar_extract_file_to_heap(dmc_unrar_archive *archive, dmc_unrar_size_t index,
                                                void **buffer, dmc_unrar_size_t *uncompressed_size, bool validate_crc);

/** The callback function for dmc_unrar_extract_file_with_callback().
 *
 *  Note that even with small buffer slices, decompressing a buffer
 *  full might take an unexpected long time, if the requested file
 *  is part of a solid block and/or uses the PPMd decoder.
 *
 *  @param  opaque Opaque memory pointer for personal use.
 *  @param  buffer Pointer to the buffer where the current part of the
 *          extracted file resides. Can be changed, to use a different
 *          buffer for further extraction. Can be set to NULL to let
 *          dmc_unrar_extract_file_with_callback() allocate its own
 *          internal buffer.
 *  @param  buffer_size Size of the buffer. Can be modified, to use
 *          a different buffer size for further extraction.
 *  @param  uncompressed_size Number of bytes of extracted file waiting
 *          in the buffer.
 *  @param  err In combination with returning false, the callback can
 *          set this parameter to something other than DMC_UNRAR_OK to
 *          signal an error. dmc_unrar_extract_file_with_callback() will
 *          return with that error condition.
 *  @return true if extraction should continue, false otherwise.
 */
typedef bool (*dmc_unrar_extract_callback_func)(void *opaque, void **buffer,
                                                dmc_unrar_size_t *buffer_size, dmc_unrar_size_t uncompressed_size, dmc_unrar_return *err);

/** Extract a file entry using a callback function.
 *
 *  Extract into the buffer of buffer_size, calling callback every time the
 *  buffer has been filled (or all the input has been processed).
 *
 *  @param  archive The archive to extract from.
 *  @param  index The index of the file entry to extract.
 *  @param  buffer The pre-allocated memory buffer to extract into. Can be
 *          NULL to mean that a buffer of buffer_size should be allocated.
 *  @param  buffer_size The size of the output buffer.
 *  @param  uncompressed_size If != NULL, the total number of bytes written
 *          to the buffer will be stored here.
 *  @param  validate_crc If true, validate the uncompressed data against
 *          the CRC-32 stored within the archive. If the validation fails,
 *          this counts as an error (DMC_UNRAR_FILE_CRC32_FAIL).
 *  @param  opaque Opaque memory pointer to pass to the callback.
 *  @param  callback The callback to call.
 *  @return An error condition, or DMC_UNRAR_OK if extraction succeeded.
 */
dmc_unrar_return dmc_unrar_extract_file_with_callback(dmc_unrar_archive *archive,
                                                      dmc_unrar_size_t index, void *buffer, dmc_unrar_size_t buffer_size,
                                                      dmc_unrar_size_t *uncompressed_size, bool validate_crc, void *opaque,
                                                      dmc_unrar_extract_callback_func callback);

#if DMC_UNRAR_DISABLE_STDIO != 1
/** Extract a file entry into a file.
 *
 *  @param  archive The archive to extract from.
 *  @param  index The index of the file entry to extract.
 *  @param  file The file to write into.
 *  @param  uncompressed_size If not NULL, the number of bytes written
 *          to the file will be stored here.
 *  @param  validate_crc If true, validate the uncompressed data against
 *          the CRC-32 stored within the archive. If the validation fails,
 *          this counts as an error (DMC_UNRAR_FILE_CRC32_FAIL).
 *  @return An error condition, or DMC_UNRAR_OK if extraction succeeded.
 */
dmc_unrar_return dmc_unrar_extract_file_to_file(dmc_unrar_archive *archive, dmc_unrar_size_t index,
                                                FILE *file, dmc_unrar_size_t *uncompressed_size, bool validate_crc);

/** Open a file and extract a RAR file entry into it.
 *
 *  Please note that on Windows, full UTF-8 paths only work when using the WIN32 API
 *  (see DMC_UNRAR_DISABLE_WIN32 above). Without the WIN32 API, only plain ASCII paths
 *  are supported on Windows.
 *
 *  @param  archive The archive to extract from.
 *  @param  index The index of the file entry to extract.
 *  @param  path The file to open and write into. This must be UTF-8.
 *  @param  uncompressed_size If not NULL, the number of bytes written
 *          to the file will be stored here.
 *  @param  validate_crc If true, validate the uncompressed data against
 *          the CRC-32 stored within the archive. If the validation fails,
 *          this counts as an error (DMC_UNRAR_FILE_CRC32_FAIL).
 *  @return An error condition, or DMC_UNRAR_OK if extraction succeeded.
 */
dmc_unrar_return dmc_unrar_extract_file_to_path(dmc_unrar_archive *archive, dmc_unrar_size_t index,
                                                const char *path, dmc_unrar_size_t *uncompressed_size, bool validate_crc);
#endif /* DMC_UNRAR_DISABLE_STDIO */

/** Return true if the given \0-terminated string contains valid UTF-8 data. */
bool dmc_unrar_unicode_is_valid_utf8(const char *str);

/** Cut off the given \0-terminated string at the first invalid UTF-8 sequence.
 *
 *  @param str The string to check and potentially modify.
 *  @return True if the string was modified, false otherwise.
 */
bool dmc_unrar_unicode_make_valid_utf8(char *str);

typedef enum {
    DMC_UNRAR_UNICODE_ENCODING_UTF8,
    DMC_UNRAR_UNICODE_ENCODING_UTF16LE,

    DMC_UNRAR_UNICODE_ENCODING_UNKNOWN

} dmc_unrar_unicode_encoding;

/** Try to detect the encoding of a memory region containing human-readable text.
 *
 *  This is of course far from 100% reliable. The detection is rather simplistic
 *  and just meant to roughly detect the encoding of archive comments.
 *
 *  This function does not check for \0-termination.
 */
dmc_unrar_unicode_encoding dmc_unrar_unicode_detect_encoding(const void *data,
                                                             dmc_unrar_size_t data_size);

/** Convert UTF-16LE data into UTF-8.
 *
 *  Conversion will stop at the first invalid UTF-16 sequence. The result will
 *  always be fully valid, \0-terminated UTF-8 string, but possibly cut off.
 *
 *  A leading UTF-16LE BOM will be removed.
 *
 *  @param utf16le_size Size of utf16le_data in bytes.
 *  @param utf8_size Size of utf8_data in bytes.
 *
 *  Returns the number of bytes written to utf8_data. If utf8_data is NULL, this
 *  function returns the number of bytes needed to fully store the UTF-8 string.
 */
dmc_unrar_size_t dmc_unrar_unicode_convert_utf16le_to_utf8(const void *utf16le_data,
                                                           dmc_unrar_size_t utf16le_size, char *utf8_data, dmc_unrar_size_t utf8_size);

/** Calculate a CRC-32 (0xEDB88320 polynomial) checksum from this memory region. */
uint32_t dmc_unrar_crc32_calculate_from_mem(const void *mem, dmc_unrar_size_t size);

/** Append the CRC-32 (0xEDB88320 polynomial) checksum calculate from this memory region
  * to the CRC-32 of a previous memory region. The result is the CRC-32 of the two
  * memory regions pasted together.
  *
  * I.e. these two functions will result in the same value:
  *
  * uint32_t crc32_1(const uint8_t *mem, dmc_unrar_size_t size) {
  *   assert(size >= 10);
  *   return dmc_unrar_crc32_calculate_from_mem(mem, size);
  * }
  *
  * uint32_t crc32_2(const uint8_t *mem, dmc_unrar_size_t size) {
  *   assert(size >= 10);
  *   uint32_t crc = dmc_unrar_crc32_calculate_from_mem(mem, 10);
  *   dmc_unrar_crc32_continue_from_mem(crc, mem + 10, size - 10);
  *   return crc;
  * }
  */
uint32_t dmc_unrar_crc32_continue_from_mem(uint32_t hash, const void *mem, dmc_unrar_size_t size);
#ifdef __cplusplus
}
#endif

#endif /* DMC_UNRAR_HEADER_INCLUDED */

/* --- End of header, implementation follows --- */