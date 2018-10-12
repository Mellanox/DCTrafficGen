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

#ifndef _DCTG_H_
#define _DCTG_H_

#include <string>
#include <list>
#include <fstream>
#include <omnetpp.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
using namespace std;

#if OMNETPP_VERSION > 0x0500
using namespace omnetpp;
#define opp_error cRuntimeError
#define UNIFORM getSimulation()->getModule(0)->uniform 
#else
#define UNIFORM uniform 
#endif

enum Locality {
  INTRA_HOST = 0,
  INTRA_RACK,
  INTRA_CLUSTER,
  INTRA_DATA_CENTER,
  INTER_DATA_CENTER,
  //all next 3 must be in this order and last
  ALL,
  NONE,
  NO_LOCALITY_SPECIFIED
};

enum DistType {
  MSG_SIZE,
  INTER_ARRIVAL,
  FLOW_DURATION,
  FLOW_SIZE,
  //next 2 must be in this order and last
  LOCALITY,
  NO_DIST_TYPE_SPECIFIED
};

class DCTGDist {
 public:
  string callerRandParam; //used if there is no cdfTable
  double average;
  vector <pair<double,double>> cdfTable;
  DCTGDist() {};
  int setCdfTable(string cdfFilePath);
  void setCallerRandParam(string param);
  double  getRandomValue(cSimpleModule *caller);
};

class DCTGRole {
 public:
  string roleName;
  DCTGDist * localityDCTGDist;
  // Locality x DistType matrix - the entry is DCTGDist
  map <Locality , map<DistType,DCTGDist*> > DCTGDists; 

  DCTGRole(string name) {roleName=name; localityDCTGDist=NULL;};
  DCTGRole() {localityDCTGDist=NULL;};
  int getRandFlow(cSimpleModule *caller,  Locality &flowLocality, 
                  unsigned int &msgSize, double &duration, 
                  unsigned int &flowSize, double &interArrival);
};

class DCTGGen{
 public:
  string address; //in caller simulation
  unsigned int host;
  unsigned int rack;
  unsigned int cluster;
  unsigned int dataCenter;
  DCTGRole* roleType;
  DCTGGen() {rack=0;cluster=0;dataCenter=0;roleType=NULL;};
};

class DCTGAppType
{
 public:
  string appTypeName;
  vector < DCTGRole* > roles;

  map < string , int > roleNameToRoleIndex;

  DCTGAppType(string name) {appTypeName=name;};
  DCTGAppType(){};
  void delRoles();
  inline bool roleExists(string roleName);
  inline DCTGRole* getRole(string roleName);
  inline void addRole(DCTGRole* role);
};

// map between location (DC,cluster,rack,host) -> gen list on this host
typedef map < unsigned , map < unsigned , map < unsigned ,  map <unsigned , vector <  DCTGGen* > > > > > Topology;

class DCTGAppInst
{
 public:
  string appInstName;
  DCTGAppType* appType;
  Topology  topology;
  vector < DCTGGen* > gens;
  map < string , int > genAddrToGenIndex;

  DCTGAppInst(string name) {appInstName=name;appType=NULL;};
  DCTGAppInst(){appType=NULL;};
  void delGens();
  string getDestAddress(string sourceAddress, Locality locality);

  inline bool addressExists(string address);
  inline DCTGGen* getGen(string address);
  inline void addGen(DCTGGen* gen);
};

/**
 * DCTGMgr: the manager of all known roles and jobs
 */
// singleton
class DCTGMgr
{
 private:
  //singleton staff
  static DCTGMgr * singleton;
  DCTGMgr() {};
  DCTGMgr(DCTGMgr const&) {};
  void operator=(DCTGMgr const&) {};

  //main data base
  vector<DCTGAppInst> appInsts;
  vector<DCTGAppType*> appTypes;
  map < string , int > appTypeToIndex;
  map < string , int > genAddrToAppIndex;
  map < string , int > appInstNameToAppIndex;

  //vectors of all DCTGDist* for releasing of memory
  vector<DCTGDist *> allDCTGDists;

  //xml
  string curXmlPath;
  xmlDoc*  openXml(string xmlPath,xmlNode* &rootNode);
  int  createXmlAppInst(xmlNode * rootNode);
  DCTGAppType*  createXmlAppType(xmlNode * rootNode);

  // example to xml line: <NodeName propName=NodeProp>NodeContent</NodeName>
  inline string getXmlNodeName(xmlNode * node);
  inline string getXmlNodeProp(xmlNode * node,const char * propName);
  inline string getXmlNodeContent(xmlNode * node);

  DCTGDist * createDCTGDist(xmlNode * node);
  int  strToInteger(string intStr,string appInstName);

 public:

  static DCTGMgr * get(); //for the singletone
  ~DCTGMgr();

  int getRandFlow(string sourceAddress, cSimpleModule *caller,  
                  string &destAddr,  unsigned int &msgSize,
                  unsigned int &flowSize, double &flowDuration, 
                  double &interArrival);
  int reg(string address,string appInstXml);
};

// string converting
DistType strToDistType(string distTypeStr);
Locality strToLocality(string localityStr);

#endif
