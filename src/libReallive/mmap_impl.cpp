/* Copyright (c) 2006 Peter Jolly
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* /////////////////////////////////////////////////////////////////////////////
 * File:    mmap.c
 *
 * Purpose: mmap(),  for the Win32 platform.
 *
 * Created: 18th December 2003
 * Updated: 21st September 2005
 *
 * Home:    http://synesis.com.au/software/
 *
 * Copyright (c) 2003-2005, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer. 
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the names of Matthew Wilson and Synesis Software nor the names of
 *   any contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////////// */

// Modified 2006-5-23 by Haeleth for use in libreallive.

#ifdef WIN32
#include <windows.h>
#include <mman.h>
#include <errno.h>

/* /////////////////////////////////////////////////////////////////////////////
 * API functions
 */

static int win32_flags_from_mmap_(int prot, int flags, DWORD *cfmFlags, DWORD *mvofFlags) {
    *cfmFlags   =   0;
    *mvofFlags  =   0;

    if (PROT_NONE == prot) {
        *cfmFlags   =   PAGE_NOACCESS;
        *mvofFlags  =   0;
    } else {
        if (prot & PROT_WRITE) {
            if ((flags & MAP_PRIVATE)) {
                *mvofFlags |= FILE_MAP_COPY;
            } else {
                *mvofFlags |= FILE_MAP_WRITE;
            }
        } else {
            *mvofFlags |= FILE_MAP_READ;
        }

        if (*mvofFlags & FILE_MAP_COPY) {
            *cfmFlags = PAGE_WRITECOPY;
        } else if (*mvofFlags & FILE_MAP_WRITE) {
            *cfmFlags = PAGE_READWRITE;
        } else {
            *cfmFlags = PAGE_READONLY;
        }
    }

    if (flags & MAP_ANONYMOUS) {
#if 0
        *cfmFlags |= SEC_RESERVE;
#endif /* 0 */
    }

    return 0;
}

/** \brief Maps a file into memory, and returns a pointer to it
 *
 * \param addr Suggested address for the mapping
 * \param len The number of bytes in the mapped region. The combination of
 * len and offset must not exceed the length of the mapped file
 * \param prot Either PROT_NONE, or a combination of the other PROT_* flags
 * \param flags One of MAP_PRIVATE, MAP_ANONYMOUS or MAP_FIXED.
 * \param fd The file descriptor of the file to map, or -1 to allocate an anonymous map
 * \param offset The offset within the file to start the mapped region
 *
 * \retval pointer to mapped region if successful
 * \retval MAP_FAILED if failed
 *
 * \note The current implementation supports the following three modes:
 * 1. Read-only file; 2. Read-write file; 3. Anonymous Read-write block of
 * system paging file.
 */

void *mmap(void *addr, size_t len, int prot, int flags, HANDLE fh, off_t offset) {
    /* Sanity checks first */
    int     errno_      =   0;

    if ( NULL == addr && 
        0 != (flags & MAP_FIXED)) {
        errno_ = ENOMEM;
    } else if (MAP_ANONYMOUS == (flags & MAP_ANONYMOUS) &&
            INVALID_HANDLE_VALUE != fh) {
        errno_ = EINVAL;
    } else if (MAP_ANONYMOUS == (flags & MAP_ANONYMOUS) &&
            (   0 == len ||
                0 != offset)) {
        errno_ = EINVAL;
    } else {
        if (MAP_ANONYMOUS != (flags & MAP_ANONYMOUS)) {
            DWORD   fileSize    =   GetFileSize(fh, NULL);

            if ( 0xFFFFFFFF == fileSize &&
                ERROR_SUCCESS != GetLastError()) {
                errno_ = EBADF;
            }
        }
    }

    if (0 != errno_) {
        errno = errno_;
        return MAP_FAILED;
    } else {
        DWORD   cfmFlags;
        DWORD   mvofFlags;

        errno_  =   win32_flags_from_mmap_(prot, flags, &cfmFlags, &mvofFlags);

        if (0 != errno_) {
            return MAP_FAILED;
        } else {
            HANDLE  hMap    =   CreateFileMapping(fh, NULL, cfmFlags, 0, len, NULL);

            if (NULL == hMap) {
                DWORD   dwErr   =   GetLastError();

                if (dwErr == ERROR_ACCESS_DENIED) {
                    errno = EACCES;
                } else if (dwErr == ERROR_INVALID_PARAMETER) {
                    errno = EINVAL;
                } else if (dwErr == ERROR_FILE_INVALID) {
                    errno = EBADF;
                }

                return MAP_FAILED;
            } else {
                void    *pvMap  =   MapViewOfFileEx(hMap, mvofFlags, 0, offset, len, addr);

                CloseHandle(hMap);

                return pvMap;
            }
        }
    }
}

/** \brief Deletes a mapped region
 *
 * The munmap() system call deletes the mappings for the specified address
 * range, and causes further references to addresses within the range to
 * generate invalid memory references.
 *
 * \param addr The base address of the mapped region to unmap
 * \param len The length of the mapped region. Ignore in the Win32 implementation
 *
 * \retval 0 if successful
 * \retval -1 if failed
 */

int munmap(void *addr, size_t len) {
    ((void)len);

    return UnmapViewOfFile(addr) ? 0 : (errno = EINVAL, -1);
}

/** \brief Writes any dirty pages within the given range to disk
 *
 * \param addr The base address of the mapped region
 * \param len The length of the mapped region to flush to disk. Will be rounded up
 * to next page boundary.
 * \param flags Ignored
 */

int msync(void *addr, size_t len, int flags) {
    ((void)flags);

    return FlushViewOfFile(addr, len) ? 0 : (errno = EINVAL, -1);
}

#endif
