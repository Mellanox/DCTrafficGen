//////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2018 Mellanox Technologies LTD. All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//     Redistribution and use in source and binary forms, with or
//     without modification, are permitted provided that the following
//     conditions are met:
//
//      - Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      - Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials
//        provided with the distribution.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _DC_TRAFFIC_GEN_H_
#define _DC_TRAFFIC_GEN_H_

#include <omnetpp.h>
#include "DCTG.h"
#include <string>

/**
 * DCTrafficGen generates packets according to instance role in a job
 */
class DCTrafficGen : public cSimpleModule
{
  // parameters:
  unsigned int linkBW_Bps; // output link bandwidth in B/s
  double startTime_s;      // the time to start generating packets
  double stopTime_s;       // the time to stop
  double statCollPeriod_s; // statistics collection period
  double startColl_s;      // start of outBwMBps collection
  double endColl_s;        // end of outBwMBps collection
  unsigned int numPending; // pending msgs - 1 means sched Q will drain
  string dstAddr;

  //traffic configurations
  string address;
  string appXmlConfig;
  string rolesXmlConfig;
  DCTGMgr * trafficMgr;
  unsigned int msgSize_B;
  unsigned int flowSize_B;
  double flowDuration_s;
  double interArival_s;
  int numMsgsInFlow;
  double avgTimeBetweenMsgs;
  simtime_t lastNegDelay;
  double flowRate;
  double dlyPerByte_s;
  simtime_t lastMsgTime;
  simtime_t minNextFlowTime;

  // state
  cMessage *p_flowMsg;
  cMessage *p_statMsg;
  cGate *p_outOGate;

  // statistics
  simsignal_t egressBWSignal;
  simsignal_t numMsgsSignal;
  simsignal_t flowDurationSignal;
  simsignal_t interArivalSignal;
  simsignal_t msgSizeSignal;
  simsignal_t flowRateSignal;

  unsigned int bytesSentStat;
  cStdDev outBwMBps; // collected during [statColl_s, endColl_s]

 protected:
  // must be provided by sub-class since convering address to destination is undefined
  virtual cMessage * createMsg(unsigned int pktSize_B,
                               unsigned int msgPackets,string dstAddr) = 0;
  // must be provided by sub-class since addressing is usage specific
  virtual string getMyAddr() = 0;

  virtual void emit_dstAddr(string dstAddr) {};
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  virtual void finish();
};

#endif
