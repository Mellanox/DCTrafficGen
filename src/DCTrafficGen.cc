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

#include "DCTrafficGen.h"

void DCTrafficGen::initialize()
{
  //params
  startTime_s = par("startTime"); // the time to start generating packets
  stopTime_s = par("stopTime");  // the time to stop
  appXmlConfig = par("appXmlConfig").stringValue();
  linkBW_Bps = par("linkBW_Bps");
  dlyPerByte_s = 1.0/(linkBW_Bps);
  statCollPeriod_s = par("statCollPeriod");
  startColl_s = par("startColl");
  endColl_s = par("endColl");

  //start msgs
  p_flowMsg = new cMessage("DCTrafficGen");
  scheduleAt(simTime()+startTime_s, p_flowMsg);
  p_statMsg = new cMessage("stat"); // self message to collect statistics
  p_outOGate = gate("out$o");

  //statistics
  egressBWSignal = registerSignal("genBWVecMBps");
  numMsgsSignal = registerSignal("numMsgs");
  flowDurationSignal = registerSignal("flowDuration");
  interArivalSignal = registerSignal("interArival");
  msgSizeSignal = registerSignal ("msgSize");
  flowRateSignal = registerSignal("flowRate");

  trafficMgr = DCTGMgr::get();
  address = getMyAddr();
  trafficMgr->reg(address,appXmlConfig);
  lastMsgTime = 0;
  lastNegDelay = 0;
  numMsgsInFlow =0;
  flowRate =1;
  dstAddr = "";
  minNextFlowTime = 0;

  if (statCollPeriod_s > 0) {
    bytesSentStat = 0;
    scheduleAt(simTime() +
               statCollPeriod_s*ceil(startTime_s/statCollPeriod_s), p_statMsg);
  }
  outBwMBps.setName("gen-BW-MBps");
}

void DCTrafficGen::handleMessage(cMessage *msg)
{
  simtime_t t = simTime();
  if (msg == p_statMsg) {
    if ((t>= startTime_s) && (t <= stopTime_s)) {
      double bw = 1e-6*bytesSentStat/statCollPeriod_s;
      emit(egressBWSignal, 1.0*bw);
      if ((t >= startColl_s) && (t < endColl_s)) 
        outBwMBps.collect(bw);
      bytesSentStat = 0;
      scheduleAt(simTime() + statCollPeriod_s, p_statMsg);
    }
    return;
  }
  
  // must be flow msg or done msg here
  unsigned int numToGen = 1;
  if (msg == p_flowMsg) {
    //start new flow
    emit(numMsgsSignal,numMsgsInFlow);
    emit(flowDurationSignal,flowDuration_s);
    emit(interArivalSignal,interArival_s);
    emit(msgSizeSignal,msgSize_B);
    emit(flowRateSignal,flowRate);
    emit_dstAddr(dstAddr);
    trafficMgr->getRandFlow(address,check_and_cast<cSimpleModule*>(this),
									 dstAddr,msgSize_B,flowSize_B,flowDuration_s,
									 interArival_s);
    if (numMsgsInFlow!=0) {
      cout << "-E- " << getFullPath() << " new flow about to start and " 
			  << numMsgsInFlow << " msg remain in last flow." << endl;
      error("-E- new flow about to start and %d msg remain in last flow.",
				numMsgsInFlow);
    }
    numMsgsInFlow = round(1.0*flowSize_B/msgSize_B + 0.5);
    flowSize_B        = max(numMsgsInFlow*msgSize_B,msgSize_B);
    flowDuration_s    = max(flowDuration_s,dlyPerByte_s * flowSize_B);
    interArival_s     = max(interArival_s,flowDuration_s);
    minNextFlowTime   = simTime() + interArival_s;
    flowRate =(1.0*flowSize_B/flowDuration_s)/(linkBW_Bps);
    numToGen = numMsgsInFlow>1 ? 2 : 1;
    double sendMsgTime = dlyPerByte_s * msgSize_B;
    avgTimeBetweenMsgs = numMsgsInFlow==1 ? 0 : 
		1.0*(flowDuration_s - numMsgsInFlow * sendMsgTime)/(numMsgsInFlow-1);
    EV << "-I- " << getFullPath() << " start new flow!!! "
      "flow size Bytes: " << flowSize_B << " flow duration sec: "
		 << flowDuration_s << " mags size Bytes: " << msgSize_B
		 << " msgs in flow: "  << numMsgsInFlow
       << " avgTimeBetweenMsgs: "<< avgTimeBetweenMsgs
       << " flowRate " <<flowRate
       << " minNextFlowTime: "<< minNextFlowTime << endl;
    emit(numMsgsSignal,numMsgsInFlow);
    emit(flowDurationSignal,flowDuration_s);
    emit(interArivalSignal,interArival_s);
    emit(msgSizeSignal,msgSize_B);
    emit(flowRateSignal,flowRate);
    emit_dstAddr(dstAddr);
  } else {
    delete msg;
  }

  if (t <= stopTime_s) {
    if (numMsgsInFlow) {
      // generate msgs - we generate enough msgs to meet the target
		// pending number
      for (unsigned i = 0; i < numToGen; i++) {
        // randomize message size in bytes
        unsigned pktSize_B = min(msgSize_B,(unsigned)par("pktSize"));
        unsigned msgPackets = msgSize_B/pktSize_B;
        numMsgsInFlow --;

        cMessage * p_msg =createMsg(pktSize_B,msgPackets,dstAddr);
        double timeBetweenMsgs = exponential(avgTimeBetweenMsgs);
        simtime_t delay = timeBetweenMsgs - (simTime() - lastMsgTime) +
			 lastNegDelay;
        if ((delay<0)  || (numToGen==2 && i==0)) {
          lastNegDelay = delay;
          delay = 0;
        } else {
          lastNegDelay =0;
        }
        sendDelayed(p_msg, delay ,p_outOGate);
        EV << "-I- " << getFullPath() << " sent MSG to sched delayd in "
			  << delay << "s" " bytes: " << pktSize_B << " pkts: " << msgPackets
           << " num of msgs remain in flow: " << numMsgsInFlow << endl;
        lastMsgTime = simTime() + delay;
        bytesSentStat += pktSize_B*msgPackets;
      }
    } else {
      simtime_t nextFlowTime = max(minNextFlowTime,simTime());
      if (!p_flowMsg->isScheduled())
        scheduleAt(nextFlowTime,p_flowMsg);
    }
  }
}

void DCTrafficGen::finish()
{
  if (p_statMsg)
    cancelAndDelete(p_statMsg);
  if (p_flowMsg)
    cancelAndDelete(p_flowMsg);
  outBwMBps.record();
  recordScalar("gen-span-to-mean-ratio",
					(outBwMBps.getMax() - outBwMBps.getMin())/outBwMBps.getMean());
  recordScalar("gen-stddev-to-mean-ratio", 
					outBwMBps.getStddev()/outBwMBps.getMean());
}

