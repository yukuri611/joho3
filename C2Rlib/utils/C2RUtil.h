/*
Copyright 2016 New System Vision Research and Development Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#if !defined(C2R_UTIL_H)
#define C2R_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#define C2R_ERROR(n)                C2R::ErrorMessage(n, __FILE__, __FUNCTION__, __LINE__)
#define C2R_ERROR_VALS(n, v0, v1)   C2R::ErrorMessage(n, (uint64_t)v0, (uint64_t)v1, __FILE__, __FUNCTION__, __LINE__)
#define C2R_ASSERT(n)               do { if (!(n)) { C2R_ERROR(#n); } } while(0)
#define C2R_ASSERT_EXP(n)           ((!(n)) ? C2R_ERROR(#n) : 1)
#define C2R_ASSERT_VALS(n, v0, v1)  ((!(n)) ? C2R_ERROR_VALS(#n, v0, v1) : 1)
#define C2R_WARNING(msg)            C2R::WarningMessage(msg, __FILE__, __FUNCTION__, __LINE__)

#define MEGA_DOUBLE	    1000000.0
#define TOO_BIG_DOUBLE	(MEGA_DOUBLE * MEGA_DOUBLE * 10)

extern bool RVDisableProfilerFlag;
extern bool RVDisableMsg;
extern bool RVDisableTimerMsg;

/// 6/22/20
namespace C2R {
    static int ErrorMessage(const char *n, const char *pathName, const char *funcName, int lineNum) {
        printf("C2R::ERROR!!! %s\n%s, %s:L.%d (aborting...)\n", n, pathName, funcName, lineNum);
        getchar();
        exit(-1);
        return 0;
    }
    static int ErrorMessage(const char *n, uint64_t v0, uint64_t v1, const char *pathName, const char *funcName,
        int lineNum) {
        printf("C2R::ERROR!!! %s\n%s, %s:L.%d (aborting...)\n", n, pathName, funcName, lineNum);
        printf("v0 = %llx, v1 = %llx\n", (unsigned long long)v0, (unsigned long long)v1);
        getchar();
        exit(-1);
        return 0;
    }
    static int WarningMessage(const char *n, const char *pathName, const char *funcName, int lineNum) {
        printf("C2R::WARNING!!! %s\n%s, %s:L.%d\n", n, pathName, funcName, lineNum);
        return 0;
    }
    static void PrintFormatDouble(double val, FILE * fp) {
        char s[200];
        size_t digits, i;
        if (val > TOO_BIG_DOUBLE) { fprintf(fp, "%.3f (too big to format...)\n", val); return; }
        sprintf(s, "%.3f", val);
        digits = strlen(s) - 4;
        for (i = 0; i < digits; i++) {
            fputc(s[i], fp);
            if ((digits - i) % 3 == 1 && i < digits - 1) { fputc(',', fp); }
        }
        for (i = 0; i < 4; i++) { fputc(s[i + digits], fp); }
    }
    static void PrintFormatInt(long val, FILE * fp) {
        char s[20];
        size_t digits, i;
        sprintf(s, "%ld", val);
        digits = strlen(s);
        for (i = 0; i < digits; i++) {
            fputc(s[i], fp);
            if ((digits - i) % 3 == 1 && i < digits - 1) { fputc(',', fp); }
        }
    }
    struct SF { /// StringStreamFormat
        const char *str;
        long long lval;
        size_t len;
        enum Base { BT10, BT16, } base;
        bool left;
        char fill;
        enum Type { T_NONE, T_STR, T_VAL } type;
        std::string prefix, suffix;
        void Reset() {
            str = ""; lval = 0; len = 0; base = BT10; left = 0; fill = ' '; type = T_NONE; prefix = ""; suffix = "";
        }
        SF(const char *fmt, size_t l, const char *s) {
            Format(fmt, l); str = s;
        }
        SF(const char *fmt, size_t l, std::string &s) {
            Format(fmt, l); str = s.c_str();
        }
        SF(const char *fmt, size_t l, long long v) {
            Format(fmt, l); lval = v;
        }
        SF(const char *fmt, const char *s) {
            Format(fmt, 0); str = s;
        }
        SF(const char *fmt, const std::string &s) {
            Format(fmt, 0); str = s.c_str();
        }
        SF(const char *fmt, long long v) {
            Format(fmt, 0); lval = v;
        }
        void Format(const char *fmt, size_t l) {
            Reset();
            auto *p = fmt;
            while (*p != '%') { prefix += *(p++); }
            C2R_ASSERT(*(p++) == '%');
            while (*p) {
                switch (*p) {
                case 's':
                case 'd':
                case 'x':
                    if (type != T_NONE) {
                        printf("ERROR!!! fmt = %s, type = %d, p = %d, suffix = %s, prefix = %s\n",
                            fmt, type, (int)(p - fmt), suffix.c_str(), prefix.c_str());
                    }
                    C2R_ASSERT(type == T_NONE);
                    type = (*p == 's') ? T_STR : T_VAL;
                    if (*p == 'x') { base = BT16; }
                    break;
                case '-': left = 1;     break;
                case '0': fill = '0';   break;
                case '*': len = l;      break;
                default:
                    if (*p >= '0' && *p <= '9') {
                        while (*p >= '0' && *p <= '9') { len = (len * 10) + (*p - '0'); ++p; }
                        continue;
                    }
                    while (*p) { suffix += *(p++); }
                    continue;
                }
                ++p;
            }
        }
        std::stringstream & operator << (std::stringstream &ss) const {
            if (prefix.size()) {
                ss << std::setw(0) << prefix;
            }
            ss << ((base == BT10) ? std::dec : std::hex) << ((left) ? std::left : std::right)
                << std::setfill(fill) << std::setw(len);
            switch (type) {
            case T_STR: ss << str; break;
            case T_VAL: ss << lval; break;
            default: break;
            }
            if (suffix.size()) {
                ss << std::setw(0) << suffix;
            }
            ss << std::setw(0);
            return ss;
        }
    };
    struct Timer {
        const char *name;
        long init_t, t, cycles;
        Timer(const char *n = "") { Reset(n); }
        void Reset(const char *n) {
            name = n;
            init_t = 0;
            t = 0;
            cycles = 0;
        }
        void Set() {
            long t0 = t;
            t = clock();
#if 1
            if (!RVDisableTimerMsg && t0) {
                Print(t0, stdout);
            }
            if (!t0) { init_t = t; }
#else
#if defined(ISA_TEST)
            if (!t0) { init_t = t; }
#elif defined(SUPPRESS_MISC_LOG)
            if (t0) { Print(t0, stdout); }
            else { init_t = t; }
#else
            if (t0) { Print(t0, stdout); }
            else { init_t = t; printf("timer(%s) initialized\n", name); }
#endif
#endif
        }
        void Print(long t0, FILE * fp) {
            double elapsedSeconds = (double)(t - t0) / CLOCKS_PER_SEC;
            fprintf(fp, "timer(%s) : ", name);
            PrintFormatDouble(elapsedSeconds, fp);
            fprintf(fp, " sec(elapsed), ");
            elapsedSeconds = (double)(t - init_t) / CLOCKS_PER_SEC;
            PrintFormatDouble(elapsedSeconds, fp);
            fprintf(fp, " sec(total), ");
            PrintFormatDouble((double)(cycles + 1) / elapsedSeconds, fp);   /// 10/16/22 : elapsed cycles = (cycles + 1)
            fprintf(fp, " cycles/sec, ");
            PrintFormatInt(cycles + 1, fp);
            fprintf(fp, " cycles\n");   /// 10/16/22 : elapsed cycles = (cycles + 1)
        }
    };
};

std::stringstream & operator << (std::stringstream &ss, const C2R::SF &sf);




#endif //#if !defined(C2R_UTIL_H)
