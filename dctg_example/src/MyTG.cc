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

#include <stdlib.h>
#include <sstream>
#include <omnetpp.h>
#include "MyTG.h"
#include "mymsg_m.h"

Define_Module(MyTG);

// endcode/decode the unsigned id as string
static unsigned int strToAddr(string addr) {
  unsigned int i = strtoul(addr.c_str(), NULL, 0);
  return i;
}

static string addrToStr(unsigned int i) {
  ostringstream s;
  s << i ;
  return s.str();
}

cMessage *MyTG::createMsg(unsigned int pktSize_B,
                          unsigned int msgPackets, string dstAddr)
{
  MyMsg *msg = new MyMsg();
  msg->setSrc(getIndex()); // src id could be cached for better perf
  msg->setDst(strToAddr(dstAddr));
  msg->setPktBytes(pktSize_B);
  msg->setNumPkts(msgPackets);
  return msg;
}

string MyTG::getMyAddr()
{
  return addrToStr(getIndex());
}

