// clang-format off
#include "table_dt.hpp"
#include "prepare.hpp"
#include "utils.hpp"
// clang-format on

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/time.h>

// Include Hash Table implementation
#include "HashTable.hpp"

#if 0
// FIXME override back to 1G size for test
#undef L_MAX_ROW
#define L_MAX_ROW 6001215l
#undef O_MAX_ROW
#define O_MAX_ROW 1500000l
#endif

// By TPCH design, but should be available in decent DBMS implementation.
#define ORDERKEY_MIN (1l)
#define ORDERKEY_MAX (O_MAX_ROW * 4)

enum q5_debug_level { Q5_ERROR, Q5_WARNING, Q5_INFO, Q5_DEBUG, Q5_ALL };

const q5_debug_level debug_level = Q5_ALL;

#define ORDERKEY_RAGNE (6000000)
#define HORIZ_PART ((ORDERKEY_MAX + ORDERKEY_RAGNE - 1) / ORDERKEY_RAGNE)

// extra space in partition buffers.
#define BUF_L_DEPTH (L_MAX_ROW / HORIZ_PART + VEC_LEN - 1 + 8000)
#define BUF_O_DEPTH (O_MAX_ROW / HORIZ_PART + VEC_LEN - 1 + 2000)

FILE* fo(std::string fn) {
    FILE* f = fopen(fn.c_str(), "rb");
    if (!f) {
        std::cerr << "ERROR: " << fn << " cannot be opened for binary read." << std::endl;
    }
    return f;
}

int main(int argc, const char* argv[]) {
    std::cout << "\n*** Test Hash Table implementation with TPC-H data (SF-1) ***\n" << std::endl;
    
    // cmd arg parser.
    ArgParser parser(argc, argv);

    std::string work_dir;
    if (!parser.getCmdOption("-work", work_dir)) {
        std::cout << "ERROR: work dir is not set!\n";
        return 1;
    }

    int sf = 1;
    std::string sf_s;
    if (parser.getCmdOption("-sf", sf_s)) {
        try {
            sf = std::stoi(sf_s);
        } catch (...) {
            sf = 1;
        }
    }

    // call data generator
    std::string in_dir = prepare(work_dir, sf);

    KEY_T* col_l_orderkey[HORIZ_PART];
    MONEY_T* col_l_extendedprice[HORIZ_PART];
    MONEY_T* col_l_discount[HORIZ_PART];
    int l_nrow_part[HORIZ_PART];

    KEY_T* col_o_orderkey[HORIZ_PART];
    DATE_T* col_o_orderdate[HORIZ_PART];
    int o_nrow_part[HORIZ_PART];

    FILE* f_l_orderkey = fo(in_dir + "/l_orderkey.dat");
    FILE* f_l_extendedprice = fo(in_dir + "/l_extendedprice.dat");
    FILE* f_l_discount = fo(in_dir + "/l_discount.dat");
    FILE* f_o_orderkey = fo(in_dir + "/o_orderkey.dat");
    FILE* f_o_orderdate = fo(in_dir + "/o_orderdate.dat");

    KEY_T t_l_orderkey;
    MONEY_T t_l_extendedprice;
    MONEY_T t_l_discount;
    KEY_T t_o_orderkey;
    DATE_T t_o_orderdate;

    int l_nrow = 0;
    int o_nrow = 0;
    bool no_more = false;
    bool fit_in_one = false;
    bool overflow = false;

    if (debug_level >= Q5_DEBUG) printf("DEBUG: horizontal part: %d\n", HORIZ_PART);

    for (int i = 0; i < HORIZ_PART; ++i) {
        KEY_T okey_max = ORDERKEY_RAGNE * (i + 1) + 1;
        if (debug_level >= Q5_DEBUG) printf("DEBUG: part %d, orderkey max = %d\n", i, okey_max);

        // alloc o
        col_o_orderkey[i] = aligned_alloc<KEY_T>(BUF_O_DEPTH);
        col_o_orderdate[i] = aligned_alloc<DATE_T>(BUF_O_DEPTH);
        // read o
        int j = 0;
        if (i > 0 && !no_more) {
            col_o_orderkey[i][j] = t_o_orderkey;
            col_o_orderdate[i][j] = t_o_orderdate;
            j++;
            o_nrow++;
        }
        // read new data
        while (o_nrow < O_MAX_ROW) {
            int rn = fread(&t_o_orderkey, sizeof(KEY_T), 1, f_o_orderkey);
            if (rn != 1) {
                no_more = true;
                break;
            }
            rn = fread(&t_o_orderdate, sizeof(DATE_T), 1, f_o_orderdate);
            if (rn != 1) {
                no_more = true;
                break;
            }
            if (t_o_orderkey < okey_max) {
                if (j < BUF_O_DEPTH) {
                    col_o_orderkey[i][j] = t_o_orderkey;
                    col_o_orderdate[i][j] = t_o_orderdate;
                } else {
                    overflow = true;
                }
                j++;
                o_nrow++;
            } else {
                break;
            }
        }
        o_nrow_part[i] = j;
        if (i == 0 && no_more) {
            fit_in_one = true;
        }
        if (debug_level >= Q5_DEBUG)
            printf("DEBUG: BUF_O_DEPTH=%ld, part %d: %d (%ld slots unused)\n", BUF_O_DEPTH, i, j, BUF_O_DEPTH - j);

        // alloc l
        col_l_orderkey[i] = aligned_alloc<KEY_T>(BUF_L_DEPTH);
        col_l_extendedprice[i] = aligned_alloc<MONEY_T>(BUF_L_DEPTH);
        col_l_discount[i] = aligned_alloc<MONEY_T>(BUF_L_DEPTH);
        // read l
        // data failed to be written to last part.
        j = 0;
        if (i > 0 && !no_more) {
            col_l_orderkey[i][j] = t_l_orderkey;
            col_l_extendedprice[i][j] = t_l_extendedprice;
            col_l_discount[i][j] = t_l_discount;
            j++;
            l_nrow++;
        }
        // read new data.
        while (l_nrow < L_MAX_ROW) {
            int rn = fread(&t_l_orderkey, sizeof(KEY_T), 1, f_l_orderkey);
            if (rn != 1) {
                no_more = true;
                break;
            }
            rn = fread(&t_l_extendedprice, sizeof(MONEY_T), 1, f_l_extendedprice);
            if (rn != 1) {
                no_more = true;
                break;
            }
            rn = fread(&t_l_discount, sizeof(MONEY_T), 1, f_l_discount);
            if (rn != 1) {
                no_more = true;
                break;
            }
            // test whether the data belong to the part
            if (t_l_orderkey < okey_max) {
                if (j < BUF_L_DEPTH) {
                    col_l_orderkey[i][j] = t_l_orderkey;
                    col_l_extendedprice[i][j] = t_l_extendedprice;
                    col_l_discount[i][j] = t_l_discount;
                } else {
                    overflow = true;
                }
                j++;
                l_nrow++;
            } else {
                break;
            }
        };
        l_nrow_part[i] = j;
        if (debug_level >= Q5_DEBUG)
            printf("DEBUG: BUF_L_DEPTH=%ld, part %d: %d (%ld slots unused)\n", BUF_L_DEPTH, i, j, BUF_L_DEPTH - j);
    
    }

    fclose(f_l_orderkey);
    fclose(f_l_extendedprice);
    fclose(f_l_discount);
    fclose(f_o_orderkey);
    fclose(f_o_orderdate);

    std::cout << "\nLineitem " << l_nrow << " rows\n"
              << "\nOrders " << o_nrow << " rows\n";

    std::cout << "\nLineitem buffer has been loaded.\n";
    std::cout << "Order buffer has been loaded.\n";

    if (overflow) {
        printf("ERROR: some buffer has overflow!\n");
        return 1;
    }

    /**
     * Canonical Hash Join
     * 
     * Relation R (Order) is used to build the Hash Table.
     * Relation S (Lineitem) is used to probe the Hash Table.
     * For each tuple in S, we probe for a matching key in the corresponding bucket of the hash table.
    */

    // Create Hash Table object
    HashTable myHashTable(o_nrow);

    // Insert R tuples into Hash Table
    for (int i = 0; i < HORIZ_PART; ++i) {
        for (int j = 0; j < o_nrow_part[i]; ++j) {
            myHashTable.insert(col_o_orderkey[i][j], col_o_orderdate[i][j]);
        }
    }

    std::cout << "\nHash Table size:" << myHashTable.getSize() << std::endl;

    std::cout << "Hash Table count:" << myHashTable.getCount() << std::endl;

    int matching_keys = 0; 

    // Probe S tuples against the Hash Table
    for (int i = 0; i < HORIZ_PART; ++i) {
        for (int j = 0; j < l_nrow_part[i]; ++j) {
            int value = myHashTable.search(col_l_orderkey[i][j]);
            if (value != -1) {
                ++matching_keys;
            }
        }
    }

    std::cout << "\nMatching keys: " << matching_keys << "\n\n" << std::endl;

    // Calculate histogram
    //myHashTable.calculateHistogram();

    // Calculate prefix sum
    //myHashTable.calculatePrefixSum();    

    // Get histogram and prefix sum
    //std::vector<int>& histogram = myHashTable.getHistogram();

    //std::vector<int>& prefixSum = myHashTable.getPrefixSum();

    /*
    // Print histogram
    for (int i = 0; i < histogram.size(); ++i) {
        std::cout << "histogram[" << i << "] = " << histogram[i] << std::endl;
    }

    // Print prefix sum
    for (int i = 0; i < prefixSum.size(); ++i) {
        std::cout << "prefixSum[" << i << "] = " << prefixSum[i] << std::endl;
    }
    */

    //std::cout << "Histogram size: " << histogram.size() << std::endl;

    //std::cout << "Prefix sum size: " << prefixSum.size() << std::endl;

    return 0;
}