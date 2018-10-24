
/**
 * \defgroup description...
 *
 * This...
 *
 * @{
 */

/*
 * null-routing.c
 *
 *  Created on: Nov 18, 2017
 *      Author: vinicius
 */
#include <stdio.h>
#include "contiki.h"
#include "iotus-api.h"
#include "iotus-netstack.h"
#include "layer-packet-manager.h"
#include "piggyback.h"
#include "sys/ctimer.h"
#include "random.h"

#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "nullRouting"
#include "safe-printer.h"

// Next dest table using final value{source, final destination}
int routing_table[45][45] =
{
// 0=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
// 1=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  0,  2,  3,  2,  2,  3,  3,  3,  2,  3,  3,  3,  2,  2,  2,  2,  2,  3,  3,  3,  2,  2,  2,  3,  3,  3,  2,  2,  2,  3,  3,  3,  2,  2,  2,  3,  3,  3,  2,  2,  2,  3,  3,  3},
// 2=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  1,  0,  3,  4,  5,  3,  3,  3,  4,  3,  3,  3,  4,  5,  4,  4,  5,  3,  3,  3,  4,  4,  5,  3,  3,  3,  5,  5,  4,  3,  3,  3,  4,  4,  5,  3,  3,  3,  4,  4,  5,  3,  3,  3},
// 3=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  1,  2,  0,  2,  2,  6,  7,  8,  2,  6,  6,  8,  2,  2,  2,  2,  2,  6,  6,  8,  2,  2,  2,  8,  8,  6,  2,  2,  2,  8,  6,  6,  2,  2,  2,  8,  8,  8,  2,  2,  2,  8,  6,  8},
// 4=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  2,  2,  2,  0,  5,  2,  2,  2,  9,  2,  2,  2, 13,  5,  9, 13,  5,  2,  2,  2,  9,  9,  5,  2,  2,  2,  5,  5,  9,  2,  2,  2, 13, 13,  5,  2,  2,  2, 13, 13,  5,  2,  2,  2},
// 5=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  2,  2,  2,  4,  0,  2,  2,  2,  4,  2,  2,  2,  4, 14,  4,  4, 14,  2,  2,  2,  4,  4, 14,  2,  2,  2, 14, 14,  4,  2,  2,  2,  4,  4, 14,  2,  2,  2,  4,  4, 14,  2,  2,  2},
// 6=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  3,  3,  3,  3,  3,  0,  7,  8,  3, 10, 11,  8,  3,  3,  3,  3,  3, 10, 11,  8,  3,  3,  3,  8,  8, 10,  3,  3,  3,  8, 10, 10,  3,  3,  3,  8,  8,  8,  3,  3,  3,  8, 11,  8},
// 7=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  3,  3,  3,  3,  3,  6,  0,  8,  3,  6,  6,  8,  3,  3,  3,  3,  3,  6,  6,  8,  3,  3,  3,  8,  8,  6,  3,  3,  3,  8,  6,  6,  3,  3,  3,  8,  8,  8,  3,  3,  3,  8,  6,  8},
// 8=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  3,  3,  3,  3,  3,  6,  7,  0,  3,  6,  6, 12,  3,  3,  3,  3,  3,  6,  6, 12,  3,  3,  3, 12, 12,  6,  3,  3,  3, 12,  6,  6,  3,  3,  3, 12, 12, 38,  3,  3,  3, 12,  6, 38},
// 9=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  4,  4,  4,  4,  4,  4,  4,  4,  0,  4,  4,  4, 13,  4, 15, 13,  4,  4,  4,  4, 15, 15,  4,  4,  4,  4,  4,  4, 15,  4,  4,  4, 13, 13,  4,  4,  4,  4, 13, 13,  4,  4,  4,  4},
//10=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  6,  6,  6,  6,  6,  6,  6,  6,  6,  0, 11,  6,  6,  6,  6,  6,  6, 18, 11,  6,  6,  6,  6,  6,  6, 18,  6,  6,  6,  6, 18, 18,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6, 11,  6},
//11=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  6,  6,  6,  6,  6,  6,  6,  6,  6, 10,  0,  6,  6,  6,  6,  6,  6, 10, 19,  6,  6,  6,  6,  6,  6, 10,  6,  6,  6,  6, 10, 10,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6, 19,  6},
//12=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  0,  8,  8,  8,  8,  8,  8,  8, 20,  8,  8,  8, 20, 20,  8,  8,  8,  8, 20,  8,  8,  8,  8,  8, 20, 20, 38,  8,  8,  8, 20,  8, 38},
//13=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  4,  4,  4,  4,  4,  4,  4,  4,  9,  4,  4,  4,  0,  4,  9, 16,  4,  4,  4,  4,  9,  9,  4,  4,  4,  4,  4,  4,  9,  4,  4,  4, 16, 16,  4,  4,  4,  4, 16, 16,  4,  4,  4,  4},
//14=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  0,  5,  5, 17,  5,  5,  5,  5,  5, 17,  5,  5,  5, 17, 17,  5,  5,  5,  5,  5,  5, 17,  5,  5,  5,  5,  5, 17,  5,  5,  5},
//15=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  0,  9,  9,  9,  9,  9, 21, 22,  9,  9,  9,  9,  9,  9, 21,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9},
//16=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,  0, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 33, 34, 13, 13, 13, 13, 33, 33, 13, 13, 13, 13},
//17=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,  0, 14, 14, 14, 14, 14, 23, 14, 14, 14, 23, 23, 14, 14, 14, 14, 14, 14, 23, 14, 14, 14, 14, 14, 23, 14, 14, 14},
//18=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,  0, 10, 10, 10, 10, 10, 10, 10, 26, 10, 10, 10, 10, 26, 26, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
//19=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,  0, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 43, 11},
//20=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,  0, 12, 12, 12, 24, 25, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 24, 25, 12, 12, 12, 12, 24, 12, 12},
//21=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  0, 22, 15, 15, 15, 15, 15, 15, 29, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15},
//22=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 21,  0, 15, 15, 15, 15, 15, 15, 21, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15},
//23=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,  0, 17, 17, 17, 27, 28, 17, 17, 17, 17, 17, 17, 27, 17, 17, 17, 17, 17, 27, 17, 17, 17},
//24=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,  0, 25, 20, 20, 20, 20, 30, 20, 20, 20, 20, 20, 30, 25, 20, 20, 20, 20, 30, 20, 20},
//25=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 24,  0, 20, 20, 20, 20, 24, 20, 20, 20, 20, 20, 24, 37, 20, 20, 20, 20, 24, 20, 20},
//26=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,  0, 18, 18, 18, 18, 31, 32, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18},
//27=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,  0, 28, 23, 23, 23, 23, 23, 23, 35, 23, 23, 23, 23, 23, 35, 23, 23, 23},
//28=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 27,  0, 23, 23, 23, 23, 23, 23, 27, 23, 23, 23, 23, 23, 27, 23, 23, 23},
//29=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,  0, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21},
//30=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,  0, 24, 24, 24, 24, 24, 36, 24, 24, 24, 24, 24, 36, 24, 24},
//31=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,  0, 32, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26},
//32=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 31,  0, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26},
//33=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,  0, 34, 16, 16, 16, 16, 39, 39, 16, 16, 16, 16},
//34=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 33,  0, 16, 16, 16, 16, 33, 33, 16, 16, 16, 16},
//35=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,  0, 27, 27, 27, 27, 27, 41, 27, 27, 27},
//36=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,  0, 30, 30, 30, 30, 30, 42, 30, 30},
//37=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,  0, 25, 25, 25, 25, 25, 25, 25},
//38=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8,  8,  8, 12, 12,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8, 12, 12,  0,  8,  8,  8, 12,  8, 44},
//39=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,  0, 40, 33, 33, 33, 33},
//40=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,  0, 39, 39, 39, 39},
//41=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,  0, 35, 35, 35},
//42=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,  0, 36, 36},
//43=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,  0, 19},
//44=> 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  
  {0, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,  0}
};

//Timer for sending neighbor discovery
static struct ctimer sendNDTimer;
static clock_time_t backOffDifference;
iotus_node_t *rootNode;
iotus_node_t *fatherNode;
static uint8_t private_keep_alive[12];
static uint8_t gPkt_created = 0;


static iotus_netstack_return
send(iotus_packet_t *packet)
{
  if(packet_get_parameter(packet, PACKET_PARAMETERS_IS_READY_TO_TRANSMIT)) {
    return active_data_link_protocol->send(packet);
  }

  if(NODES_BROADCAST == packet->finalDestinationNode) {
    packet->nextDestinationNode = NODES_BROADCAST;
  } else {
    //Get the final static destination
    uint8_t *finalDestLastAddress = nodes_get_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT,
                                          packet->finalDestinationNode);
#if EXP_STAR_LIKE == 1
    uint8_t nextHop = 1;
#elif EXP_LINEAR_NODES == 1
    uint8_t nextHop = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0] -1;
#else
    // if(addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0] > 12 ||
    //    finalDestLastAddress[0] > 12) {
    //   SAFE_PRINTF_LOG_ERROR("wrong destination");
    //   return ROUTING_TX_ERR;
    // }
    uint8_t nextHop = routing_table[addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]][finalDestLastAddress[0]];
#endif

    if(nextHop == 0) {
      //This is for ourself. Cancel...
      return ROUTING_TX_ERR;
    }

    SAFE_PRINTF_LOG_INFO("Final %u next %u ID:%u\n",finalDestLastAddress[0],nextHop,packet->pktID);

//     uint8_t rootValue = 0;
// #if EXP_STAR_LIKE == 0
//     rootValue = routing_table[addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]][1];
// #else
//     rootValue = 1;
// #endif
  
    uint8_t address[2] = {nextHop,0};
    fatherNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address);

    packet->nextDestinationNode = fatherNode;

    uint8_t bitSequence[1];
    bitSequence[0] = finalDestLastAddress[0];
    packet_push_bit_header(8, bitSequence, packet);
  }

  return active_data_link_protocol->send(packet);
}


static void
send_cb(iotus_packet_t *packet, iotus_netstack_return returnAns)
{
  SAFE_PRINTF_LOG_INFO("Frame %p processed %u", packet, returnAns);
  // if(returnAns == MAC_TX_OK) {
    packet_destroy(packet);
  // }
}

static iotus_netstack_return
input_packet(iotus_packet_t *packet)
{
  // SAFE_PRINTF_CLEAN("Got packet: ");
  // int i;
  // for (i = 0; i < packet_get_payload_size(packet); ++i)
  // {
  //   SAFE_PRINTF_CLEAN("%02x ", packet_get_payload_data(packet)[i]);
  // }
  // SAFE_PRINTF_CLEAN("\n");
  uint8_t finalDestAddr = packet_unwrap_pushed_byte(packet);

  if(finalDestAddr == addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0]) {
    //This is for us...
    active_transport_protocol->receive(packet);
    return RX_PROCESSED;
  } else {
    iotus_packet_t *packetForward = NULL;


    //search for the next node...
    uint8_t ourAddr = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];
#if EXP_LINEAR_NODES == 1
    uint8_t nextHop = ourAddr-1;
#else
    uint8_t nextHop = routing_table[ourAddr][finalDestAddr];
#endif

    if(nextHop != 0) {
        uint8_t address2[2] = {1,0};
        rootNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address2);
        if(rootNode != NULL) {
          packetForward = iotus_initiate_packet(
                              packet_get_payload_size(packet),
                              packet_get_payload_data(packet),
                              packet->params | PACKET_PARAMETERS_WAIT_FOR_ACK,
                              IOTUS_PRIORITY_ROUTING,
                              ROUTING_PACKETS_TIMEOUT,
                              rootNode,
                              send_cb);

          if(NULL == packetForward) {
            SAFE_PRINTF_LOG_INFO("Packet failed");
            return RX_ERR_DROPPED;
          }

          iotus_netstack_return status = send(packetForward);
          SAFE_PRINTF_LOG_INFO("Packet %u forwarded %u stats %u\n", packet->pktID, packetForward->pktID, status);
          // // if (!(MAC_TX_OK == status ||
          // //     MAC_TX_DEFERRED == status)) {
          // if (MAC_TX_DEFERRED != status) {
          //   send_cb(packetForward, status);
          //   // printf("Packet fwd del %u\n", packetForward->pktID);
          //   // packet_destroy(packetForward);
          // }
        }
    }
    return RX_PROCESSED;
  }

}



/*---------------------------------------------------------------------------*/
static void
send_keep_alive(void *ptr)
{
  if(gPkt_created >= MAX_GENERATED_KA) {
      return;
  }
  gPkt_created++;

  static uint8_t selfAddrValue;
  selfAddrValue = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];

  if(selfAddrValue != 1) {
    clock_time_t backoff = CLOCK_SECOND*KEEP_ALIVE_INTERVAL - backOffDifference;//ms

#if USE_NEW_FEATURES == 1
    backOffDifference = 0;
#else
    backOffDifference = (CLOCK_SECOND*((random_rand()%BACKOFF_TIME)))/1000;
#endif
    backoff += backOffDifference;
    ctimer_set(&sendNDTimer, backoff, send_keep_alive, NULL);

    printf("Net sending to 1\n");
#if USE_NEW_FEATURES == 1
    uint8_t address3[2] = {1,0};
    rootNode = nodes_update_by_address(IOTUS_ADDRESSES_TYPE_ADDR_SHORT, address3);
    SAFE_PRINTF_LOG_INFO("Creating piggy routing\n");
    piggyback_create_piece(12, private_keep_alive, IOTUS_PRIORITY_ROUTING, rootNode, ROUTING_PACKETS_TIMEOUT);
#else
    if(rootNode != NULL) {
        // iotus_initiate_msg(
        //         12,
        //         private_keep_alive,
        //         PACKET_PARAMETERS_WAIT_FOR_ACK,
        //         IOTUS_PRIORITY_ROUTING,
        //         5000,
        //         rootNode,
        //         send_cb);

        iotus_packet_t *packet = iotus_initiate_packet(
                                  12,
                                  private_keep_alive,
                                  PACKET_PARAMETERS_WAIT_FOR_ACK,
                                  IOTUS_PRIORITY_ROUTING,
                                  5000,
                                  rootNode,
                                  send_cb);

        if(NULL == packet) {
          SAFE_PRINTF_LOG_INFO("Packet failed");
          return;
        }

        SAFE_PRINTF_LOG_INFO("Packet KA %u \n", packet->pktID);
        iotus_netstack_return status = send(packet);
        // // if (!(MAC_TX_OK == status ||
        // //     MAC_TX_DEFERRED == status)) {
        // if (MAC_TX_DEFERRED != status) {
        //   send_cb(packet, status);
        //   // printf("Packet KA del %u\n", packet->pktID);
        //   // packet_destroy(packet);
        // }
      }
#endif
  }
}

/*---------------------------------------------------------------------------*/
static void
start(void)
{
  SAFE_PRINTF_LOG_INFO("Starting null routing\n");

  iotus_subscribe_for_chore(IOTUS_PRIORITY_ROUTING, IOTUS_CHORE_NEIGHBOR_DISCOVERY);

  uint8_t selfAddrValue;
  selfAddrValue = addresses_self_get_pointer(IOTUS_ADDRESSES_TYPE_ADDR_SHORT)[0];
  sprintf((char *)private_keep_alive, "### %02u %02u###", selfAddrValue,
                                                        selfAddrValue);
}

/*---------------------------------------------------------------------------*/
static void
post_start(void)
{
#if KEEP_ALIVE_SERVICE == 1
  #if BROADCAST_EXAMPLE == 0
    if(IOTUS_PRIORITY_ROUTING == iotus_get_layer_assigned_for(IOTUS_CHORE_NEIGHBOR_DISCOVERY)) {
    #if USE_NEW_FEATURES == 1
      backOffDifference = 0;
      clock_time_t backoff = CLOCK_SECOND/8;//ms
      ctimer_set(&sendNDTimer, backoff, send_keep_alive, NULL);
    #else /*USE_NEW_FEATURES*/
      backOffDifference = (CLOCK_SECOND*((random_rand()%BACKOFF_TIME)))/1000;
      clock_time_t backoff = CLOCK_SECOND*KEEP_ALIVE_INTERVAL + backOffDifference;//ms
      ctimer_set(&sendNDTimer, backoff, send_keep_alive, NULL);
    #endif /*USE_NEW_FEATURES*/
    }
  #endif /*BROADCAST_EXAMPLE*/
#endif /*KEEP_ALIVE_SERVICE*/
}

static void
close(void)
{
  
}

struct iotus_network_protocol_struct null_network_protocol = {
  start,
  post_start,
  close,
  send,
  send_cb,
  input_packet
};
/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
