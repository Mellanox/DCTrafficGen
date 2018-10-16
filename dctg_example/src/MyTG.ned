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

import DCTrafficGen.DCTrafficGen;

package dctg_example;

simple MyTG extends DCTrafficGen
{
    parameters:
    // for Hadoop
    double interArrival    @unit(s) = default(exponential(10us));
    double flowSize        @unit(B) = default(64KiB);
    double flowDuration_IntraRack @unit(s) = default(5.2us); // 64KiB over 100Gbps
    double flowDuration_all       @unit(s) = default(20.8us); // 64KiB over 100Gbps/4
   
    // for FrontEnd
    double web_interArrival    @unit(s) = default(exponential(10us));
    double cache_interArrival  @unit(s) = default(exponential(10us));

    @class(MyTG);
}

