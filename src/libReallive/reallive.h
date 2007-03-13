#ifndef LIBREALLIVE_H
#define LIBREALLIVE_H

/**
 * @namespace libReallive
 * @author Haeleth
 * @brief libReallive, a library for manipulating Reallive bytecode
 * 
 * libReallive is a library for manipulating Reallive bytecode. It was
 * orriginally written by Haeleth, and was copy/pasted into
 * RLVM. While I've modified it to suit my needs (for example, adding
 * support for RLv1.3 goto_with constructs, parameter parsing code in
 * expression.cpp, et cetera), 99% of this code was written by
 * Haeleth.
 *
 * In this namespace, you will find classses and function for reading
 * and manipulating Reallive SEEN files. This module is not documented
 * as well, since it is a third party library.
 */

#include "defs.h"     
#include "bytecode.h" 
#include "scenario.h" 
#include "archive.h"

#endif
