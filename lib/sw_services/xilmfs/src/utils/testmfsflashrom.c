/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
#include <stdio.h>
#include "xilmfs.h"

// char *fs = (char *)0xffe00000; /* base address of SRAM */
char *fs = (char *)0x1f800000; /* base address of Flash */
/* NUMBLOCKS must be the same number that is used in mfsgen when creating the
   MFS image that is pre-loaded into RAM */
/* for big file system */
/* #define NUMBLOCKS 200 */
/* for small file system */
#define NUMBLOCKS 10
int main(int argc, char *argv[]) {

  int numbytes;

  numbytes = NUMBLOCKS *sizeof(struct mfs_file_block);

  mfs_init_fs(numbytes, fs, MFSINIT_ROM_IMAGE);

  mfs_ls_r(-1);
  /* for big file system */
  /* mfs_cat("xilmfs.h"); */

  return 0;
}



