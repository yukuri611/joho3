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


#if !defined(AXI4L_H)
#define AXI4L_H

//#define USE_VOLATILE    /// or may volatile is needed....
/// it seems that volatile is not needed... 
////#if defined(USE_VOLATILE)
////#define VT  volatile void
////#else
////#define VT void
////#endif

/// 6/9/17 : redo design...
#if !defined(__LLVM_C2RTL__)
//#define USE_VIRTUAL_BUS /// !!! /// 11/26/17 : USE_VIRTUAL_BUS removed for now....
#endif
#define ENABLE_AXI_AID
#define ENABLE_AXI_BURST
#define AXI_BURST_SIZE_256
#define SET_AXI_ID_AT_BUS	///	10/5/17
#define USE_EXT_MEM	///	10/5/17

#define ASSIGN_AXI_OUTPUT_SIGNALS_AFTER_FSM

struct AXI4L
{
	enum RespEnum { RSP_OK = 0, RSP_EXOK = 1, RSP_SLV_ERR = 2, RSP_DEC_ERR = 3, };
	struct CH { // channel
		struct ADDR { /// addr-read, addr-write
			struct MA {
                UINT32 addr; BIT valid; UINT8 len; UINT4 id;
                void set(UINT32 a, BIT v, UINT8 l, UINT4 i) { addr = a; valid = v; len = l; id = i; }
                void set(UINT4 midx, MA &m) { set(m.addr, m.valid, m.len, midx); }
                void reset() { set(0, 0, 0, 0); }
			} m;
			struct SL {
				BIT ready;
                void set(BIT r) { ready = r; }
                void set(SL &s) { set(s.ready); }
                void reset() { set(0); }
			} s;
		} raddr, waddr;
		struct RDAT {  /// read-data
			struct MA {
				BIT ready;
                void set(BIT r) { ready = r; }
                void set(MA &m) { set(m.ready); }
                void reset() { set(0); }
			} m;
			struct SL {
				UINT32 data; UINT2 resp; BIT valid, last;
                void set(UINT32 d, UINT2 r, BIT v, BIT l) { data = d; valid = v; resp = r; last = l; }
                void set(SL &s) { set(s.data, s.resp, s.valid, s.last); }
                void reset() { set(0, 0, 0, 0); }
			} s;
		} rdat;
		struct WDAT {  /// write-data
			struct MA {
				UINT32 data; UINT4 strobe; BIT valid, last;
                void set(UINT32 d, UINT4 s, BIT v, BIT l) { data = d; strobe = s; valid = v; last = l; }
                void set(MA &m) { set(m.data, m.strobe, m.valid, m.last); }
                void reset() { set(0, 0, 0, 0); }
			} m;
			struct SL {
				BIT ready;
                void set(BIT r) { ready = r; }
                void set(SL &s) { set(s.ready); }
                void reset() { set(0); }
			} s;
		} wdat;
		struct WRES {  /// write-resp
			struct MA {
				BIT ready;
                void set(BIT r) { ready = r; }
                void set(MA &m) { set(m.ready); }
                void reset() { set(0); }
			} m;
			struct SL {
				UINT2 resp; BIT valid;
                void set(UINT2 r, BIT v) { resp = r; valid = v; }
                void set(SL &s) { set(s.resp, s.valid); }
                void reset() { set(0, 0); }
			} s;
		} wres;
        UINT32 intr _TYPE(ungrouped_direct_signal);
        void setMARead (UINT4 midx, ADDR::MA &ra, RDAT::MA &rd)               { raddr.m.set(midx, ra); rdat.m.set(rd); }
        void setMAWrite(UINT4 midx, ADDR::MA &wa, WDAT::MA &wd, WRES::MA &wr) { waddr.m.set(midx, wa); wdat.m.set(wd); wres.m.set(wr); }
        void setMARead (UINT4 midx, CH &c) { raddr.m.set(midx, c.raddr.m); rdat.m.set(c.rdat.m); }
        void setMAWrite(UINT4 midx, CH &c) { waddr.m.set(midx, c.waddr.m); wdat.m.set(c.wdat.m); wres.m.set(c.wres.m); }
        void resetMA() { resetMARead(); resetMAWrite(); }
        void resetMARead()  { raddr.m.reset(); rdat.m.reset(); }
        void resetMAWrite() { waddr.m.reset(); wdat.m.reset(); wres.m.reset(); }
        void resetSL() { resetSLRead(); resetSLWrite(); }
        void setSLRead (ADDR::SL &ra, RDAT::SL &rd) { raddr.s.set(ra); rdat.s.set(rd); }
        void setSLWrite(ADDR::SL &wa, WDAT::SL &wd, WRES::SL &wr) { waddr.s.set(wa); wdat.s.set(wd); wres.s.set(wr); }
        void setSLRead (CH &c) { raddr.s.set(c.raddr.s); rdat.s.set(c.rdat.s); }
        void setSLWrite(CH &c) { waddr.s.set(c.waddr.s); wdat.s.set(c.wdat.s); wres.s.set(c.wres.s); }
        void resetSLRead()  { raddr.s.reset(); rdat.s.reset(); }
        void resetSLWrite() { waddr.s.reset(); wdat.s.reset(); wres.s.reset(); }
		static void connectRCh(UINT4 midx, CH &mc, CH &sc) { sc.setMARead(midx, mc);  mc.setSLRead(sc); }
		static void connectWCh(UINT4 midx, CH &mc, CH &sc) { sc.setMAWrite(midx, mc); mc.setSLWrite(sc); }
        int isActive() {
            return raddr.m.valid || waddr.m.valid || rdat.s.valid || wdat.m.valid || wres.s.valid;
        }
        void printInfo(int cycle, const char *name, int id) {
            printf("[%6d] %s%d : ", cycle, name, id);
            if (raddr.m.valid) {	///	UINT32 addr; BIT valid; UINT4 id;
                printf("RA(v:%x,a:%08x,id:%x r.s:%x) ", raddr.m.valid, raddr.m.addr, raddr.m.id, raddr.s.ready);
            }
            if (waddr.m.valid) {
                printf("WA(v:%x,a:%08x,id:%x r.s:%x) ", waddr.m.valid, waddr.m.addr, waddr.m.id, waddr.s.ready);
            }
            if (rdat.s.valid) {
                printf("RD(v:%x,d:%08x,r:%x r.m:%x) ", rdat.s.valid, rdat.s.data, rdat.s.resp, rdat.m.ready);
            }
            if (wres.s.valid) {
                printf("WR(v:%x,r:%x r.m:%x) ", wres.s.valid, wres.s.resp, wres.m.ready);
            }
            printf("\n");
        }
		void printIfActive(int cycle, const char *name, int id) { if (isActive()) { printInfo(cycle, name, id); } }
        void reset() { resetMA(); resetSL(); }
	} _TYPE(direct_signal);    /// struct CH
	struct PORT {   /// for AXI-FSM IOs : split each MA/SL channels and merge as MA/SL PORTs
		struct MA { /// for AXI-Master-FSM outPorts
			CH::ADDR::MA raddr, waddr; CH::RDAT::MA rdat; CH::WDAT::MA wdat; CH::WRES::MA wres;
            void setRead(UINT32 ra_addr, BIT ra_valid, BIT rd_ready, UINT4 len, UINT4 id)
			{ raddr.set(ra_addr, ra_valid, len, id); rdat.set(rd_ready); }
            void setWrite(UINT32 wa_addr, BIT wa_valid, UINT4 wa_len, UINT4 wa_id,
                UINT32 wd_data, UINT4 wd_strobe, BIT wd_valid, BIT wd_last,
				BIT wr_ready) {
				waddr.set(wa_addr, wa_valid, wa_len, wa_id); wdat.set(wd_data, wd_strobe, wd_valid, wd_last);
				wres.set(wr_ready);
			}
            void reset() { raddr.reset(); waddr.reset(); rdat.reset(); wdat.reset(); wres.reset(); }
		};
		struct SL { /// for AXI-Slave-FSM outPorts, AXI-Master-FSM inPorts
			CH::ADDR::SL raddr, waddr; CH::RDAT::SL rdat; CH::WDAT::SL wdat; CH::WRES::SL wres;
            void resetRead()  { raddr.reset(); rdat.reset(); }
            void resetWrite() { waddr.reset(); wdat.reset(); wres.reset(); }
            void reset() { resetRead(); resetWrite(); }
		};
	};
	struct MasterFSM;
	struct SlaveFSM;
	struct MasterFSM {
		PORT::MA out _TYPE(state); /// for driving MA-outputs
		PORT::SL in _TYPE(state);  /// for latching MA-inputs
		enum RWState { RW_Init, RW_RAddr, RW_WAddr, RW_WBurstSetup};
		enum AXIOp { AXI_Read, AXI_Write, AXI_Idle, };
		ST_UINT3    rw_state;
        ST_UINT8	burst_count;
        UINT8		burst_length;
        bool fsm(UINT2 op, CH *axi, UINT32 addr, UINT32 dout, UINT32 *din, BIT *data_latched,
            BIT *dout_ready) {
            BIT stalled = 0;
            switch (rw_state) {
            case RW_Init:
                if (op == AXI_Read) {
                    out.setRead(addr, 1, 1, burst_length, 0);    /// ra_addr, ra_valid, rd_ready, id = 0 (at master)
                    burst_count = burst_length;
                    in.resetRead();
                    stalled = 1;
                    rw_state = RW_RAddr;
                }
                else if (op == AXI_Write) {  /// isWrite
                    /// 11/14/17 : problem modeling burst write transfer on C++-dataflow model (not cycle accurate)
                    /// burst flow control using wdat.ready(from slave) requires precise cycle accuracy
                    /// this is not possible in C++-dataflow model which contains AXI-master/slave/bus-control
                    /// in sequentially ordered function calls... some signals propagate faster than others in the dataflow model
                    /// In order to make the dataflow model work with burst write transfer, following is assumed:
                    /// 1. Master asserts waddr and waits for waddr.ready AND wdat.ready from slave
                    /// 2. Master then sends all burst words in sequence.
                    /// 3. Slave cannot interrupt the burst write transfer 
                    /// 11/14/17
                    if (burst_length) { /// only set waddr
                        out.waddr.set(addr, 1, burst_length, 0);
                        rw_state = RW_WBurstSetup;
                    }
                    else {  /// wa_addr, wa_valid, wa_len, wa_id = 0, wd_data, wd_strobe, wd_valid, wd_last, wr_ready (at master)
                        out.setWrite(addr, 1, burst_length, 0, dout, 0xf, 1, (burst_length == 0), 1);
                        rw_state = RW_WAddr;
                    }
                    burst_count = burst_length;
                    in.resetWrite();
                    stalled = 1;
                }
                break;
            case RW_RAddr: {
                int n_rflag = (in.rdat.valid << 1) | in.raddr.ready;
                if (axi->raddr.s.ready) { n_rflag |= 1; in.raddr.set(axi->raddr.s); out.raddr.reset(); }
                if (axi->rdat.s.valid) {
                    if (burst_count == 0) {
                        n_rflag |= 2; in.rdat.set(axi->rdat.s);   out.rdat.reset();
                    }
                    else {
                        burst_count--;
                    }
                    *din = axi->rdat.s.data;
                    *data_latched = 1;
                    if (axi->rdat.s.resp != RSP_OK) {
                        printf("[AXI::Master] bad rdat.resp from slave!!!\n");
                    }
                }
                if (n_rflag == 3) { rw_state = RW_Init; }
                else { stalled = 1; }
                break;
            }
            case RW_WBurstSetup: { /// wait (waddr.ready && wdat.ready)
                int wready = in.waddr.ready;
                if (axi->waddr.s.ready) { wready = 1; in.waddr.set(1); out.waddr.reset(); }
                if (wready && axi->wdat.s.ready) {
                    out.wdat.set(dout, 0xf, 1, 0); /// last = 0;
                    out.wres.set(1); /// wres.ready = 1
                    rw_state = RW_WAddr;
                    *dout_ready = 1;    /// 11/15/17
                }
                break;
            }
            case RW_WAddr: {
                int n_wflag = (in.wres.valid << 2) | (in.wdat.ready << 1) | in.waddr.ready;
                int bad_resp = (in.wres.resp != RSP_OK);
                if (axi->waddr.s.ready) { n_wflag |= 1; in.waddr.set(1); out.waddr.reset(); }
                if (axi->wdat.s.ready && !(n_wflag & 2)) {   /// 11/15/17
                    if (burst_count == 0) {
                        n_wflag |= 2;
                        in.wdat.set(1);
                        out.wdat.reset();
                    }
                    else {
                        out.wdat.set(dout, 0xf, 1, burst_count == 1);
                        burst_count--;
                    }
                    *dout_ready = 1;
                }
                if (axi->wres.s.valid) {
                    n_wflag |= 4; in.wres.set(axi->wres.s);   out.wres.reset();
                    if (axi->wres.s.resp != RSP_OK) {
#if !defined(SUPRESS_LOCK_ERROR_MSG)
                        printf("[AXI::Master] bad wres.resp from slave!!! (lock/unlock error?)\n");
#endif
                        bad_resp = 1;
                    }
                }
                if (n_wflag == 7) {   /// 6/23/17 : if (bad response) { try again; }
                    rw_state = RW_Init;
                    if (bad_resp) { stalled = 1; }  /// try again
                }
                else { stalled = 1; }
                break;
            }
            }
            axi->setMARead(0, out.raddr, out.rdat);
            axi->setMAWrite(0, out.waddr, out.wdat, out.wres);
            return !stalled;
        }
        void reset() { out.reset(); in.reset(); rw_state = RW_Init; burst_count = 0; burst_length = 0; }
	};
#define ENABLE_MUTEX_MSG
#if 1
#define MUTEX_MSG_COND	(cycle < 2000)
#else
#define MUTEX_MSG_COND	(cycle > 60000 && cycle < 67000)
#endif
////#define MUTEX_MSG_COND2	(cycle > 62300 && cycle < 63503)
////#define MUTEX_MSG_COND3	(1)
	/// 6/22/17 : change to counting semaphore!!!
	struct Mutex {
		ST_UINT4    lockCount;
		ST_UINT4    ownerID;
		ST_UINT32   lockErrorCount;
		BIT getLock(UINT4 ID, const char *name, UINT32 cycle) {
			if (lockCount && ownerID != ID) {
				if ((lockErrorCount & 0x7ff) == 0)
					printf("[%8d] [getLock:%s:%d] ERROR (already locked by owner(%d))!! (total lockErrors = %d)\n",
						cycle, name, ID, ownerID, lockErrorCount);
				lockErrorCount++;
				return 0;
			}   /// already locked!!!
#if defined(ENABLE_MUTEX_MSG)
			if (MUTEX_MSG_COND)//cycle < 2000)
				printf("[%8d] [getLock:%s] locked by owner(%d) : lockCount(%d)!!\n", cycle, name, ID, lockCount + 1);
#endif
			ownerID = ID; lockCount++; return 1;
		}
		BIT releaseLock(UINT4 ID, const char *name, UINT32 cycle) {
			if (!lockCount || ID != ownerID) {
				if (!lockCount) { printf("[%8d] [releaseLock:%s:%d] ERROR (not locked)!!\n", cycle, name, ID); }
				else { printf("[%8d] [releaseLock:%s:%d] ERROR (ownerID(%d) != ID(%d))!!\n", cycle, name, ID, ownerID, ID); }
				return 0; /// not locked or not owner
			}
#if defined(ENABLE_MUTEX_MSG)
			if (MUTEX_MSG_COND)//cycle < 2000)
				printf("[%8d] [releaseLock:%s] released by owner(%d) : lockCount(%d)!!\n", cycle, name, ID, lockCount - 1);
#endif
			lockCount--; return 1;
		}
	};
#define TOTAL_INTERRUPTS    5   /// # of interrupts per CPU
#define INTERRUPT_MASK      ((1 << TOTAL_INTERRUPTS) - 1)
	struct SlaveFSM {
#define INIT_VIRTUAL_BUS_SLAVE(dev, axi)    /// nothing...
		PORT::SL out _TYPE(state);
		enum ReadState  { R_Init, R_Addr,     R_End, };
		enum WriteState { W_Init, W_AddrData, W_End, };
		/// regs (states)
		ST_UINT3 r_state, w_state;
		ST_UINT32 raddr, waddr;
		ST_BIT raddr_end, waddr_end, wres_end; /// indicate that raddr/waddr are released by master
		ST_BIT writePending;   /// need to indicate difference of write-operation and mutex
        ST_BIT writeBurstMode;  /// 11/14/17 : added
		ST_UINT32 intrFlag;
		Mutex mutex;
		/// wires (no-states)
		UINT32 nxt_intrFlag;
		int readFlag, writeFlag;
		UINT32 writeData;
		ST_UINT32 cycle;
		BIT mutexError;
		UINT4		midx_r, midx_w;	///	master ID
		ST_UINT4	midx_r_reg, midx_w_reg;	///	master ID
		ST_UINT4	burst_count_r, burst_count_w;
		ST_BIT		burst_end_r;
		/// methods
        void fsmRead(CH *axi) {
            if (r_state == R_Init) {
                midx_r = axi->raddr.m.id;
                midx_r_reg = midx_r;
            }
            else {
                midx_r = midx_r_reg;
            }
            switch (r_state) {
            case R_Init:
                if (axi->raddr.m.valid && !readBlocked()) {
                    raddr = axi->raddr.m.addr;
                    burst_count_r = axi->raddr.m.len;
                    burst_end_r = 0;
                    out.raddr.set(1);
                    raddr_end = 0;
                    r_state = R_Addr;
                    devReadSetup(axi->raddr.m.addr);
                }
                break;
            case R_Addr: {
                out.raddr.reset();
                if (!axi->raddr.m.valid) { raddr_end = 1; }
                UINT32 d = out.rdat.data;
                if ((out.rdat.valid && burst_end_r) || devRead(&d, raddr)) {
                    if ((!out.rdat.valid || !burst_end_r) && cycle < 100) {
                        printf("[%6d] axislv : raddr(%08x), d(%08x), bc(%d)\n", cycle, raddr, d, burst_count_r);
                    }
                    out.rdat.set(d, RSP_OK, 1, (burst_count_r == 0));
                    if (burst_count_r == 0) {
                        if (axi->rdat.m.ready) { r_state = R_End; }
                        burst_end_r = 1;
                    }
                    else {
                        burst_count_r--;
                    }
                }
                break;
            }
            case R_End: /// make sure raddr.valid is deasserted BEFORE going back to R_Init
                out.rdat.reset();
                if (raddr_end || !axi->raddr.m.valid) { r_state = R_Init; nxt_intrFlag |= 1; }
                break;
            }
            axi->setSLRead(out.raddr, out.rdat);
        }
        void fsmWrite(CH *axi) {
            if (w_state == R_Init) {
                midx_w = axi->waddr.m.id;
                midx_w_reg = midx_w;
            }
            else {
                midx_w = midx_w_reg;
            }
            BIT devWriteFlag = 0;
            switch (w_state) {
            case W_Init: {
                if (axi->waddr.m.valid && !writeBlocked()) {
                    waddr = axi->waddr.m.addr;
                    burst_count_w = axi->waddr.m.len;
                    out.waddr.set(1);
                    waddr_end = 0;
                    w_state = W_AddrData;
                    devWriteSetup();
                    /// 11/14/17 : changed protocol.... --> assert wdat.ready when waddr.valid in burst mode!!!
                    if (axi->waddr.m.len) { out.wdat.set(1); writeBurstMode = 1; }
                }
                break;
            case W_AddrData:
                out.waddr.reset();
                if (!axi->waddr.m.valid) { waddr_end = 1; }
                /// 11/14/17 : changed protocol.... --> assert wdat.ready when waddr.valid in burst mode!!!
                devWriteFlag = (axi->wdat.m.valid && devWrite(axi->wdat.m.data, waddr));
                if (writeBurstMode || devWriteFlag) { out.wdat.set(1); }
                if (devWriteFlag) {
                    wres_end = 0;
                    if (burst_count_w == 0) {
                        w_state = W_End;
                        out.wres.set((mutexError) ? RSP_SLV_ERR : RSP_OK, 1);   /// 6/23/17
                    }
                    else {
                        burst_count_w--;
                    }
                    writePending = (writeFlag == 1);
                }
                break;
            }
            case W_End:
                out.wdat.reset();
                {	/// fixed version
                    BIT nxt_wres_end = wres_end;
                    if (axi->wres.m.ready) { wres_end = 1; nxt_wres_end = 1; }
                    if ((nxt_wres_end || axi->wres.m.ready) && (waddr_end || !axi->waddr.m.valid)) {
                        out.wres.reset();
                        w_state = W_Init;
                        if (writePending) { nxt_intrFlag |= 2; }
                    }
                }
                break;
            }
            axi->setSLWrite(out.waddr, out.wdat, out.wres);
        }
		void fsm(CH *axi) {
            int mutexID = mutex.ownerID;
			nxt_intrFlag = 0;
			preFsmUser();
			fsmRead(axi);
			fsmWrite(axi);
			fsmUser();
            /// use virtual function to do "intrFlag = nxt_intrFlag;" because llvm will optimize out intrFlag if you don't
            setInterrupt(mutexID); /// instead of: intrFlag = nxt_intrFlag;
            axi->intr = intrFlag;
            cycle++;
		}
		virtual BIT devRead(UINT32 *data, UINT32 addr) = 0;
		virtual BIT devWrite(UINT32 data, UINT32 addr) = 0;
        virtual void devReadSetup(UINT32 addr) {}
        virtual void devWriteSetup() {}
        virtual BIT readBlocked()  { return 0; }    /// by default, readFSM and writeFSM are independent
        virtual BIT writeBlocked() { return 0; }    /// by default, readFSM and writeFSM are independent
        virtual void preFsmUser() { readFlag = 0; writeFlag = 0; writeData = 0; mutexError = 0; };
		virtual void fsmUser() {};
        virtual void setInterrupt(int mutexID) { intrFlag = (nxt_intrFlag) << (TOTAL_INTERRUPTS * mutexID); }
	};
    static void printBus(CH *m_ch, CH *s_ch, int MC, int SC, int cycle) {
        int active = 0;
        for (int i = 0; i < MC; i++) { active += m_ch[i].isActive(); }
        for (int i = 0; i < SC; i++) { active += s_ch[i].isActive(); }
        if (!active) { return; }
        for (int i = 0; i < MC; i++) { m_ch[i].printIfActive(cycle, "M", i); }
        for (int i = 0; i < SC; i++) { s_ch[i].printIfActive(cycle, "S", i); }
    }
	template <int MC, int SC> struct BUS {//: VirtualBUS {
		CH m_ch[MC], s_ch[SC];
		void resetAllChannelSinks() {
			for (unsigned i = 0; i < SC; ++i) { s_ch[i].resetMA(); }
			for (unsigned i = 0; i < MC; ++i) { m_ch[i].resetSL(); }
		}
		void connectInterrupts() { // hard-coded for now... : no intr for spi
			m_ch[0].intr = (s_ch[DI_UART].intr);// | (s_ch[DI_SYSCTL].intr << 2);
		}
		void printInfo(int cycle) { printBus(m_ch, s_ch, MC, SC, cycle); }
	};
	struct MasterStatus {   /// ok, another problem... C2R crashes if MasterStatus is declared under BUS
							/// put _T(state) inside MasterStatus/SlaveStatus
		ST_UINT8   slaveID; ////    0xffff : invalid device
		ST_BIT     active;
		BIT        granted;     /// wire
		BIT        requested;   /// wire
		UINT8      reqSlaveID;  /// wire
		void set(UINT8 sID, BIT a) { slaveID = sID; active = a; }
        void set(MasterStatus &ms) { set(ms.slaveID, ms.active); }
        void reset() { slaveID = 0; active = 0; }
        void resetFlags() { granted = 0; requested = 0; reqSlaveID = 0; }
	};
	struct SlaveStatus {
		ST_UINT8    masterID;
		ST_BIT      active;
	};
	template <int MC, int SC> struct CTRL {
		/// first try... assume MA_COUNT == 1; (no arbitration)
		/// put _T(state) inside MasterStatus/SlaveStatus
		MasterStatus MRStat[MC], MWStat[MC];
		SlaveStatus  SRStat[SC], SWStat[SC];
		ST_UINT4     roundRobinRVal, roundRobinWVal;
		void checkMRReq(BUS<MC, SC> *bus, int midx) {
			CH &mc = bus->m_ch[midx];
			MasterStatus &rs = MRStat[midx];
			rs.resetFlags();
			if (!rs.active && mc.raddr.m.valid) {
				UINT32 sidx = decodeAddr(mc.raddr.m.addr);
				if (!SRStat[sidx].active && !bus->s_ch[sidx].rdat.s.valid) {
					rs.requested = 1;
					rs.reqSlaveID = sidx;
				}
			}
		}
		void checkMWReq(BUS<MC, SC> *bus, int midx) {
			CH &mc = bus->m_ch[midx];
			MasterStatus &ws = MWStat[midx];
			ws.resetFlags();
			if (!ws.active && mc.waddr.m.valid) {
				UINT32 sidx = decodeAddr(mc.waddr.m.addr);
				if (!SWStat[sidx].active && !bus->s_ch[sidx].wres.s.valid) {
					ws.requested = 1;
					ws.reqSlaveID = sidx;
				}
			}
		}
		BIT arbitrate(UINT4 sidx, MasterStatus *ms, UINT4 rrVal) {
			BIT flag = 0;
			for (int i = 0; i < MC; ++i) {  /// 1st round
				if (i >= rrVal && !flag && ms[i].requested && ms[i].reqSlaveID == sidx) {
					ms[i].granted = 1; flag = 1;
				}
			}
			for (int i = 0; i < MC; ++i) {  /// 2nd round
				if (!flag && ms[i].requested && ms[i].reqSlaveID == sidx) {
					ms[i].granted = 1; flag = 1;
				}
			}
			return flag;
		}
		void arbitrateMR() {
			int granted = 0;
			for (int i = 0; i < SC; ++i) { granted |= arbitrate(i, MRStat, roundRobinRVal); }
			if (granted) { roundRobinRVal = (roundRobinRVal < MC - 1) ? roundRobinRVal + 1 : 0; }
		}
		void arbitrateMW() {
			int granted = 0;
			for (int i = 0; i < SC; ++i) { granted |= arbitrate(i, MWStat, roundRobinWVal); }
			if (granted) { roundRobinWVal = (roundRobinWVal < MC - 1) ? roundRobinWVal + 1 : 0; }
		}
//#define DBG_MW
#if defined(DBG_MW)
		ST_UINT32 dbg_cycle;
#endif
		_C2R_FUNC(1)
		void connectChannel(BUS<MC, SC> *bus) {
			bus->resetAllChannelSinks();
			bus->connectInterrupts();
#if defined(DBG_MW)
			if (dbg_cycle < 2000) {
				if (bus->m_ch[0].intr) { printf("[%8d] m[0].intr = %08x\n", dbg_cycle, bus->m_ch[0].intr); }
				if (bus->m_ch[1].intr) { printf("[%8d] m[1].intr = %08x\n", dbg_cycle, bus->m_ch[1].intr); }
			}
#endif
			int i, j;
			for (i = 0; i < MC; ++i) { checkMRReq(bus, i); checkMWReq(bus, i); }
			arbitrateMR(); arbitrateMW();
			for (i = 0; i < MC; ++i) {  /// i : const (due to loop-unrolling), j : variable
				if (updateMRStat(bus, i, &j)) { CH::connectRCh(i, bus->m_ch[i], bus->s_ch[j]); }
			}
			for (i = 0; i < MC; ++i) {
				if (updateMWStat(bus, i, &j)) { CH::connectWCh(i, bus->m_ch[i], bus->s_ch[j]); }
			}
#if defined(DBG_MW)
			++dbg_cycle;
#endif
		}
		BIT updateMRStat(BUS<MC, SC> *bus, int midx, int *sidx) {
			CH &mc = bus->m_ch[midx];
			MasterStatus &rs = MRStat[midx];
			*sidx = rs.slaveID;
			if (rs.granted) {
				*sidx = rs.reqSlaveID;
				rs.set(*sidx, 1);
				SRStat[*sidx].active = 1;
				SRStat[*sidx].masterID = midx;
				return 1;
			}
			if (rs.active && mc.rdat.m.ready && bus->s_ch[*sidx].rdat.s.last) {
				rs.reset();  SRStat[*sidx].active = 0; return 1;
			}
			else { return rs.active; }
		}
		BIT updateMWStat(BUS<MC, SC> *bus, int midx, int *sidx) {
			CH &mc = bus->m_ch[midx];
			MasterStatus &ws = MWStat[midx];
			*sidx = ws.slaveID;
			if (ws.granted) {
				*sidx = ws.reqSlaveID;
				ws.set(*sidx, 1);
				SWStat[*sidx].active = 1;
				SWStat[*sidx].masterID = midx;
#if defined(DBG_MW)
				if (dbg_cycle < 2000)
					printf("[%8d] W-granted : M[%d]->S[%d]\n", dbg_cycle, midx, *sidx);
#endif
				return 1;
			}
			if (ws.active && mc.wres.m.ready && bus->s_ch[*sidx].wres.s.valid) {
				ws.reset(); SWStat[*sidx].active = 0;
#if defined(DBG_MW)
				if (dbg_cycle < 2000)
					printf("[%8d] W-end     : M[%d]->S[%d]\n", dbg_cycle, midx, *sidx);
#endif
				return 1;
			}
			else { return ws.active; }
		}
	};
};

struct AXI_ST { /// axi-stream
    struct CH { /// 11/21/17 : axi-stream channel
        struct MA {
            UINT32 data; BIT valid, last;
            void set(UINT32 d, BIT v, BIT l) { data = d; valid = v; last = l; }
            void set(MA &m) { set(m.data, m.valid, m.last); }
            void reset() { set(0, 0, 0); }
        } m;
        struct SL {
            BIT ready;
            void set(BIT r) { ready = r; }
            void set(SL &s) { set(s.ready); }
            void reset() { set(0); }
        } s;
        void reset() { m.reset(); s.reset(); }
    } _TYPE(direct_signal);
};

#endif  ///#if !defined(AXI4L_H)
