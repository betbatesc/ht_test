/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TABLE_DT_H
#define TABLE_DT_H

// XXX inline with tpch_read_2.h
#include <stdint.h>
typedef int32_t TPCH_INT;

typedef TPCH_INT MONEY_T;
typedef TPCH_INT DATE_T;
typedef TPCH_INT KEY_T;

#define MONEY_SZ sizeof(TPCH_INT)
#define DATE_SZ sizeof(TPCH_INT)
#define KEY_SZ sizeof(TPCH_INT)

// every cycle, 4 input rows.
#define VEC_LEN 16

// ensure when kernel read in vec, won't over read
// SF = 1
#define L_MAX_ROW 6001215l
#define O_MAX_ROW 1500000l

// SF = 10
//#define L_MAX_ROW 60012150l
//#define O_MAX_ROW 15000000l

// SF = 100
//#define L_MAX_ROW 600037902l
//#define O_MAX_ROW 150000000l

#endif // TABLE_DT_H
