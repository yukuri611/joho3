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

#ifndef _FILTER_NxM_H_
#define _FILTER_NxM_H_


#include "../../C2Rlib/include/typedef.h"


//#define MAX_IMG_WIDTH	1024	//! max image width
#define MAX_IMG_WIDTH	2048	//! max image width

#define NEW_BORDER101
//#if defined(NEW_BORDER101)
#define BORDER_101(x,len) ((x)<0?-(x) :(x)>((len)-1)? 2*((len)-1)-(x): (x))

#if 0
         0 1 2 3 4 5 6 7 8      0 1 2 3 4 5 6 7 8
-------------------------------------------------
i =   0: 0 1 2 3 4 5 6 7 6      0 0 0 0 0 0 0 0-2
i =   1: 0 1 2 3 4 5 6 5 4      0 0 0 0 0 0 0-2-4
i =   2: 0 1 2 3 4 5 4 3 2      0 0 0 0 0 0-2-4-6
i =   3: 0 1 2 3 4 3 2 1 0      0 0 0 0 0-2-4-6-8
i =   4: 8 7 6 5 4 5 6 7 8      8 6 4 2 0 0 0 0 0
i =   5: 6 5 4 3 4 5 6 7 8      6 4 2 0 0 0 0 0 0
i =   6: 4 3 2 3 4 5 6 7 8      4 2 0 0 0 0 0 0 0
i =   7: 2 1 2 3 4 5 6 7 8      2 0 0 0 0 0 0 0 0
i =   8: 0 1 2 3 4 5 6 7 8      0 0 0 0 0 0 0 0 0

srcEven = 0 2 4 6 8
srcOdd = 1 3 5 7
#endif

template<typename T, int ksize>
void border101_core(T* src, T* dst, int i, int len) {
    const int h2 = ksize / 2;
    T srcEven[h2 + 1], srcOdd[h2];
    for (int k = 0; k <= h2; k++) { srcEven[k] = src[k * 2]; }
    for (int k = 0; k < h2; k++)  { srcOdd[k] = src[k * 2 + 1]; }
    for (int k = 0; k < ksize; k++) {
        int flg = 0, kk = 0;
        if (i < h2) {
            kk = (k - ksize) + 2 + i;
            flg = (kk <= 0) ? 0 : (k & 1) ? 1 : 2;
            if (kk > 0) { kk = k / 2 - kk; }
        }
        else if (i < ksize) {
            kk = (ksize - k) - 1 - i;
            flg = (kk <= 0) ? 0 : (k & 1) ? 1 : 2;
            if (kk > 0) { kk = k / 2 + kk; }
        }
        dst[k] = (flg == 0) ? src[k] : (flg == 1) ? srcOdd[kk] : srcEven[kk];
    }
}

template<typename T, int ksize>
void border101(T* src, T* dst, int i, int len) {
    T lsrc[ksize];
    for (int k = 0; k < ksize; k++) { lsrc[k] = src[k]; }   /// this helps reduce HW size if (src.size > ksize)
    border101_core<T,ksize>(lsrc, dst, i, len);
}

template<typename T, int ksize>
void border001(T* src, T* dst, int i, int len) {
	int ii = (i < ksize / 2) ? len : 0;
	ii += (1 - ksize) + i;
	for (int k = 0; k < ksize; k++) {
		int _i = ii + k;
		dst[k] = (_i < 0 || _i >= len) ? 0 : src[k];
	}
}


struct RGBPixel {
    UINT8 r, g, b;
};

#define LB_VLD_DIRECT

#define DELAY_IN_VLD

/// 5/29/20
template<int FH, int FHX>
struct LBState {
    UINT16  pos_x, pos_y;
    UINT8   lb_pos_y;
    UINT8   lbxbar[FHX];
    BIT     lbvld[FH];
    /// case 1 : FHX == FH : simple lineBuffer of height FH (separated read/write lines : 1 line redundant)
    /// case 2 : FHX == FH - 1 : shared read/write lines (interleaved or read1st)
    void Init() {
        for (int i = 0; i < FHX; i++) { lbxbar[i] = (i + 1 - (FH - FHX)) % FHX; }
        for (int i = 0; i < FH; i++) { lbvld[i] = (i == 0); }
        pos_x = 0; pos_y = 0; lb_pos_y = 0;
    }
    void GetValidLines(BIT vldout[]) { for (int i = 0; i < FH; i++) { vldout[i] = lbvld[i]; } }
    void Update(int imW, int imH) {
        int edge_x = (pos_x == imW - 1);
        int edge_y = (pos_y == imH - 1);
        int lb_edge_y = (lb_pos_y == FHX - 1);
        /* 5x5 case (FHX == FH) : simple (separated read/write lines)
        LB0(din): do[4] |       : do[3] |       : do[2] |       : do[1] |       : do[0]
        LB1     : do[0] |(din)  : do[4] |       : do[3] |       : do[2] |       : do[1]
        LB2     : do[1] |       : do[0] |(din)  : do[4] |       : do[3] |       : do[2]
        LB3     : do[2] |       : do[1] |       : do[0] |(din)  : do[4] |       : do[3]
        LB4     : do[3] |       : do[2] |       : do[1] |       : do[0] |(din)  : do[4]
        xbar0   : 1     |       : 2     |       : 3     |       : 4     |       : 0
        xbar1   : 2     |       : 3     |       : 4     |       : 0     |       : 1
        xbar2   : 3     |       : 4     |       : 0     |       : 1     |       : 2
        xbar3   : 4     |       : 0     |       : 1     |       : 2     |       : 3
        xbar4   : 0     |       : 1     |       : 2     |       : 3     |       : 4
        */

        /* 5x5 case (FHX == FH - 1 : interleaved/read-first) (N-1 line-buffers)
        din     : do[4] |       : do[4] |       : do[4] |       : do[4] |       : do[4]

        LB0(din): do[0] |       : do[3] |       : do[2] |       : do[1] |(din)  : do[0]
        LB1     : do[1] |(din)  : do[0] |       : do[3] |       : do[2] |       : do[1]
        LB2     : do[2] |       : do[1] |(din)  : do[0] |       : do[3] |       : do[2]
        LB3     : do[3] |       : do[2] |       : do[1] |(din)  : do[0] |       : do[3]
        xbar0   : 0     |       : 1     |       : 2     |       : 3     |       : 0
        xbar1   : 1     |       : 2     |       : 3     |       : 0     |       : 1
        xbar2   : 2     |       : 3     |       : 0     |       : 1     |       : 2
        xbar3   : 3     |       : 0     |       : 1     |       : 2     |       : 3
        */
        if (edge_x) {	///	newline starts after this pixel --> update xbar array
            UINT8 tmpxb = lbxbar[0];
            int i;
            for (i = 0; i < FHX - 1; i++) { lbxbar[i] = lbxbar[i + 1]; }
            lbxbar[i] = tmpxb;
            for (i = FH - 1; i > 0; i--) { lbvld[i] = lbvld[i - 1]; }
        }
        pos_x    = (edge_x) ? 0 : pos_x + 1;
        pos_y    = (edge_x) ? (edge_y)    ? 0 : pos_y + 1    : pos_y;
        lb_pos_y = (edge_x) ? (lb_edge_y) ? 0 : lb_pos_y + 1 : lb_pos_y;
    }
};

template <typename T, int FH, int FHX> struct LineBufBase {
    LBState<FH, FHX> stt _TYPE(state);
    virtual void Init() { stt.Init(); }
    virtual void UpdateData(T din, T dout[]) = 0;
    void Update(T din, T dout[], int imW, int imH, BIT vldout[], void(*border)(T*, T*, int, int)) {
        T lb0[FH];
        UpdateData(din, lb0);
        border(lb0, dout, stt.pos_y, imH);
        stt.GetValidLines(vldout);
        stt.Update(imW, imH);
    }
};

template <typename T, int FH, int MAX_WIDTH> struct LineBufSimple : LineBufBase<T, FH, FH> {
    T lbuf[FH][MAX_WIDTH] _TYPE(memory);
    virtual void UpdateData(T din, T dout[]) {
        T tmp[FH];
        for (int i = 0; i < FH; i++) {
            if (this->stt.lb_pos_y == i) {
                lbuf[i][this->stt.pos_x] = din;		        ///	write pixel to line-buffer
                tmp[i] = din;
            }
            else { tmp[i] = lbuf[i][this->stt.pos_x]; }     ///	read pixel from line-buffer
        }
        for (int i = 0; i < FH; i++) { dout[i] = tmp[this->stt.lbxbar[i]]; }    ///	rearrange horizontal pixels according to lbc->lb_pos_x
    }
};

template <typename T, int FH, int MAX_WIDTH> struct LineBufReadFirst : LineBufBase<T, FH, FH - 1> {
    T lbuf[FH - 1][MAX_WIDTH] _TYPE(memory);
    virtual void UpdateData(T din, T dout[]) { /// it's ok to write "invalid" data, because valid data will be eventually written
        T tmp[FH - 1];
        for (int i = 0; i < FH - 1; i++) { tmp[i] = lbuf[i][this->stt.pos_x]; }  /// read line-buffer
        lbuf[this->stt.lb_pos_y][this->stt.pos_x] = din; /// write line-buffer (write-after-read on this line-buffer)
        dout[FH - 1] = din;
        for (int i = 0; i < FH - 1; i++) { dout[i] = tmp[this->stt.lbxbar[i]]; }	///	rearrange horizontal pixels according to lbc->lb_pos_x
    }
};

template <typename T, int FH, int MAX_WIDTH> struct LineBufInterleaved : LineBufBase<T, FH, FH - 1> {
    T		    lbuf[FH - 1][2][(MAX_WIDTH + 1) / 2] _TYPE(memory); /// divide into odd/even banks
    T           w_din _TYPE(state);
    ST_UINT8    w_lb_pos;
    ST_UINT16   w_addr;
    virtual void Init(void) { this->stt.Init(); w_din = 0; w_addr = 0; w_lb_pos = 0; }
    virtual void UpdateData(T din, T dout[]) { /// it's ok to write "invalid" data, because valid data will be eventually written
        T tmp[FH - 1];
        int bank = this->stt.pos_x & 1;   /// even (bank-0) or odd (bank-1)
        int addr = this->stt.pos_x >> 1;
        if (bank) {
            for (int i = 0; i < FH - 1; i++) { tmp[i] = lbuf[i][1][addr]; }  /// read line-buffer @ bank1
            lbuf[w_lb_pos][0][w_addr] = w_din; /// write line-buffer @ bank0 (previous data)
        }
        else {  /// (vld == 0) happens only on (pos_x == 0) : so (w_vld == 1) is always true for bank == 1
            for (int i = 0; i < FH - 1; i++) { tmp[i] = lbuf[i][0][addr]; }  /// read line-buffer @ bank0
            lbuf[w_lb_pos][1][w_addr] = w_din; /// write line-buffer @ bank1 (previous data)
        }
        dout[FH - 1] = din;
        for (int i = 0; i < FH - 1; i++) { dout[i] = tmp[this->stt.lbxbar[i]]; }	///	rearrange horizontal pixels according to lbc->lb_pos_x
        w_din = din;
        w_lb_pos = this->stt.lb_pos_y;
        w_addr = addr;
    }
};

#define USE_PRV_SHIFT_REG_STATE

template <typename T, int FH, int FW> struct ShiftRegWindow
{
    T	w[FH*FW] _TYPE(state);
    ST_BIT vld[FH*FW];
    void Init() { for (int i = 0; i < FH*FW; ++i) { w[i] = 0; vld[i] = 0; } }
    void UpdateData(T din[], BIT vldin[], BIT com_vld, T wout[], BIT vldout[]) {
        int i, j;
#if defined(USE_PRV_SHIFT_REG_STATE)
        for (i = 0; i < FH; i++) {
            for (j = 0; j < FW - 1; j++) {
                wout[i * FW + j] = w[i * FW + j + 1];
                vldout[i * FW + j] = vld[i * FW + j + 1];
            }
            wout[i * FW + j] = din[i];
            vldout[i * FW + j] = vldin[i] & com_vld;
        }
        for (i = 0; i < FH * FW; i++) {
            w[i] = wout[i];
            vld[i] = vldout[i];
        }
#else
        for (i = 0; i < FH; i++) {
            for (j = 0; j < FW - 1; j++) {
                w[i * FW + j] = w[i * FW + j + 1];
                vld[i * FW + j] = vld[i * FW + j + 1];
            }
            w[i * FW + j] = din[i];
            vld[i * FW + j] = vldin[i] & com_vld;
        }
        for (i = 0; i < FH * FW; i++) {
            wout[i] = w[i];
            vldout[i] = vld[i];
        }
#endif
    }
    void Update(T din[], BIT vldin[], BIT com_vld, T wout[], BIT vldout[],
        int px, int w, void(*border)(T*, T*, int, int)) {
        T sr0[FH*FW];
        UpdateData(din, vldin, com_vld, sr0, vldout);
        for (int i = 0; i < FH; i++) {
            border(&sr0[i*FW], &wout[i*FW], px, w);
        }
    }
};

#endif /*_FILTER_NxM_H_*/

