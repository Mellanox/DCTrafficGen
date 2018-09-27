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

#include "DCTG.h"

static bool verbose = false;

//DCTGDist

void DCTGDist::setCallerRandParam(string param) {
  callerRandParam = param;
}

int DCTGDist::setCdfTable(string cdfFilePath) {
  ifstream distFileStream;
  distFileStream.open(cdfFilePath.c_str());
  if (!distFileStream.is_open())
    opp_error("-E- fail to open distribution file %s\n",cdfFilePath.c_str());
  string line;
  // all the lines the CDF table in the format: 
  // value(double or Locality) cdf(double)
  while(getline(distFileStream, line)) {
    if (line[0]=='#')
      continue;
    size_t commaPlace = line.find(",");
    if (commaPlace==string::npos)
      opp_error("-E- wrong distribution file format - invalid line is: %s\n",
                line.c_str());

    string cdf_str = line.substr(commaPlace+1);
    double cdf = stod(cdf_str);
    string value_str = line.substr(0,commaPlace);
    double value = 0;
    bool valueIsLoclity = false;
    try {
      value= stod(value_str);
    } catch(invalid_argument) {
      valueIsLoclity = true;
    }
    Locality locality = INTRA_HOST;
    if (valueIsLoclity) {
      locality = strToLocality(value_str);
      value = locality;
    }
    if (locality==NO_LOCALITY_SPECIFIED)
      opp_error("-E- wrong distribution file format locality %s not "
                "supported - invalid line is: %s\n",
                value_str.c_str(),line.c_str());
    cdfTable.push_back(make_pair(value, cdf));
  }
  distFileStream.close();
  return 0;
}


double  DCTGDist::getRandomValue(cSimpleModule *caller) {
  //    cout << caller->getFullPath()<<endl;
  if(cdfTable.empty()) {
    // randomize in ned file of the caller
    if(callerRandParam.empty())
      opp_error("no distribution defined\n");
    double randValue =caller->par(callerRandParam.c_str());
    return randValue;
  } else {
    //random bin
    double prob = uniform(0.0, 1.0);
    size_t mid, high, low;
    high = cdfTable.size() - 1;
    low = 0;
    while(low < high) {
      mid = (high + low) / 2;
      if (prob <= cdfTable[mid].second) {
        high = mid;
      } else {
        low = mid + 1;
      }
    }
    //random value inside the bin
    double highValue = cdfTable[high].first;
    double lowValue  = (high==0) ? 0 : cdfTable[high-1].first;
    double randValue = uniform(lowValue,highValue);
    return randValue;
  }
}

//DCTGRole

int DCTGRole::getRandFlow(cSimpleModule *caller,  Locality &flowLocality,
                          unsigned int &msgSize, double &duration,
                          unsigned int &flowSize, double &interArrival) {
  flowLocality = (Locality) localityDCTGDist->getRandomValue(caller);
  msgSize = DCTGDists[flowLocality][MSG_SIZE]->getRandomValue(caller);
  duration = DCTGDists[flowLocality][FLOW_DURATION]->getRandomValue(caller);
  flowSize = DCTGDists[flowLocality][FLOW_SIZE]->getRandomValue(caller);
  interArrival = DCTGDists[flowLocality][INTER_ARRIVAL]->getRandomValue(caller);
  return 0;
}

//DCTGAppType

void DCTGAppType::delRoles() {
  for (unsigned i=0;i<roles.size();i++) {
    delete roles[i];
  }
}


inline bool DCTGAppType::roleExists(string roleName) {
  return  roleNameToRoleIndex.find(roleName) != roleNameToRoleIndex.end();
}

inline void DCTGAppType::addRole(DCTGRole* role) {
  roleNameToRoleIndex[role->roleName] = roles.size();
  roles.push_back(role);
}

inline DCTGRole* DCTGAppType::getRole(string roleName) {
  if (!roleExists(roleName))
    opp_error("-E- role %s not exists in appInstlication %s\n",
              roleName.c_str(),appTypeName.c_str());
  return roles[roleNameToRoleIndex[roleName]];
}

//DCTGAppInst
void DCTGAppInst::delGens() {
  for (unsigned i=0;i<gens.size();i++) {
    delete gens[i];
  }
}
string DCTGAppInst::getDestAddress(string sourceAddress, Locality locality) {
  string destAddress;
  vector < DCTGGen* > optionalDestGens;
  DCTGGen* sourceGen = getGen(sourceAddress);
  if (verbose) cout << "-I- appInst: " << appInstName
                    << " gen with address: " << sourceAddress
                    << "and location: (host: " << sourceGen->host
                    << " rack: " << sourceGen->rack
                    << " cluster: " << sourceGen->cluster
                    << " dataCenter: " << sourceGen->dataCenter
                    << ") ask for destination address in locality " ;
  if (locality==INTRA_HOST) {
    if (verbose) cout << "INTRA_HOST" << endl;
    if (topology[sourceGen->dataCenter][sourceGen->cluster][sourceGen->rack][sourceGen->host].size() == 1) {
      opp_error("-E- host %d in appInst %s has only one gen, "
                "the probability to INTRA_HOST locality should be 0",
                sourceGen->host,this->appInstName.c_str());
    }
    for (auto gen : topology[sourceGen->dataCenter][sourceGen->cluster][sourceGen->rack][sourceGen->host]) {
      if (gen->address!="sourceAddress")
        optionalDestGens.push_back(gen);
    }
  } else if (locality==INTRA_RACK) {
    if (verbose) cout << "INTRA_RACK" << endl;
    if (topology[sourceGen->dataCenter][sourceGen->cluster][sourceGen->rack].size() == 1) {
      opp_error("-E- rack %d in appInst %s has only one host, "
                "the probability to INTRA_RACK locality should be 0",
                sourceGen->rack,this->appInstName.c_str());
    }
    for (auto host : topology[sourceGen->dataCenter][sourceGen->cluster][sourceGen->rack]) {
      if (host.first!=sourceGen->host)
        optionalDestGens.insert(optionalDestGens.end(),
                                host.second.begin(),host.second.end() );
    }
  }  else if (locality==INTRA_CLUSTER) {
    if (verbose) cout << "INTRA_CLUSTER" << endl;
    if (topology[sourceGen->dataCenter][sourceGen->cluster].size()==1) {
      opp_error("-E- cluster %d in appInst %s has only one rack, "
                "the probability to INTRA_CLUSTER locality should be 0",
                sourceGen->cluster,this->appInstName.c_str());
    }
    for(auto rack :  topology[sourceGen->dataCenter][sourceGen->cluster]) {
      if (rack.first != sourceGen->rack) {
        for (auto host : rack.second) {
          optionalDestGens.insert(optionalDestGens.end(),
                                  host.second.begin(),host.second.end() );
        }
      }
    }
  } else if (locality==INTRA_DATA_CENTER) {
    if (verbose) cout << "INTRA_DATA_CENTER" << endl;
    if (topology[sourceGen->dataCenter].size()==1) {
      opp_error("-E- data center %d in appInst %s has only one cluster, "
                "the probability to INTRA_DATA_CENTER locality should be 0",
                sourceGen->dataCenter,this->appInstName.c_str());
    }
    for(auto cluster : topology[sourceGen->dataCenter]) {
      if (cluster.first!=sourceGen->cluster) {
        for(auto rack : cluster.second) {
          for(auto host : rack.second) {
            optionalDestGens.insert(optionalDestGens.end(),
                                    host.second.begin(),host.second.end() );
          }
        }
      }
    }
  } else if (locality==INTER_DATA_CENTER) {
    if (verbose) cout << "INTER_DATA_CENTER" << endl;
    if (topology.size()==1) {
      opp_error("-E- there is only one data center in appInst %s, "
                "the probability to INTER_DATA_CENTER locality should be 0",
                this->appInstName.c_str());
    }
    for(auto dataCenter : topology) {
      if (dataCenter.first != sourceGen->dataCenter) {
        for(auto cluster : dataCenter.second) {
          for(auto rack : cluster.second) {
            for(auto host : rack.second)
              optionalDestGens.insert(optionalDestGens.end(),
                                      host.second.begin(),host.second.end() );
          }
        }
      }
    }
  } else {
    opp_error("-E- DCTGAppInst::getDestAddress:  undefined Locality\n");
  }
  int randGen = rand() % optionalDestGens.size();
  destAddress = optionalDestGens[randGen]->address;
  return destAddress;
}


inline bool DCTGAppInst::addressExists(string address) {
  return genAddrToGenIndex.find(address) != genAddrToGenIndex.end();
}

inline void DCTGAppInst::addGen(DCTGGen* gen) {
  genAddrToGenIndex[gen->address] = gens.size();
  gens.push_back(gen);
  topology[gen->dataCenter][gen->cluster][gen->rack][gen->host].push_back(gens.back());
  if (verbose) cout << "-I- Topology tree of appInst: "
                    << appInstName <<" add gen with address: "
                    << topology[gen->dataCenter][gen->cluster][gen->rack][gen->host].back()->address  << " to "
                    << " host: " << gen->host
                    << " rack: " << gen->rack
                    << " cluster: " << gen->cluster
                    << " dataCenter: " <<gen->dataCenter
                    << endl;
}

inline DCTGGen* DCTGAppInst::getGen(string address) {
  if (!addressExists(address))
    opp_error("-E- address %s not exists in appInstlication %s\n",
              address.c_str(),appInstName.c_str());
  return gens[genAddrToGenIndex[address]];
}



//DCTGMgr

DCTGMgr::~DCTGMgr () {
  for(unsigned i=0; i<allDCTGDists.size(); i++)
    delete allDCTGDists[i];
  for(unsigned i=0; i<appTypes.size(); i++) {
    appTypes[i]->delRoles();
    delete appTypes[i];
  }
  for(unsigned i=0; i<appInsts.size(); i++) {
    appInsts[i].delGens();
  }
  //Free the global variables that may
  //have been allocated by the xml parser.
  xmlCleanupParser();
}

DCTGMgr* DCTGMgr::singleton = 0;

DCTGMgr* DCTGMgr::get() {
  if (singleton == 0) {
    singleton = new DCTGMgr();
#ifdef DEBUG
    verbose = true;
#endif
  }
  return singleton;
}

int DCTGMgr::getRandFlow(string sourceAddress, cSimpleModule *caller,  
                         string &destAddr,  unsigned int &msgSize,  
                         unsigned int &flowSize, double &flowDuration, 
                         double &interArrival) {
  if (verbose) cout << "-I-  gen of address: " << sourceAddress
                    << " ask for random flow " << endl;
  if (genAddrToAppIndex.find(sourceAddress) == genAddrToAppIndex.end()) {
    opp_error("-E- address %s not exists",sourceAddress.c_str());
  }
  DCTGAppInst appInst = appInsts[genAddrToAppIndex[sourceAddress]];
  DCTGGen* sourceGen = appInst.getGen(sourceAddress);
  DCTGRole* sourceGenRoleType =  sourceGen->roleType;
  Locality flowLocality;
  sourceGenRoleType->getRandFlow(caller,flowLocality,msgSize,flowDuration,
                                 flowSize,interArrival);
  destAddr = appInst.getDestAddress(sourceAddress,flowLocality);
  if (verbose) cout << "-I-  gen of address: "
                    << sourceAddress << " got random flow: " << endl
                    << " destAddr: " << destAddr
                    << " msgSize: "  << msgSize
                    << " flowSize: "  << flowSize
                    << " flowDuration: " << flowDuration
                    << " interArrival: " << interArrival
                    << endl;
  return 0;
}



xmlDoc * DCTGMgr::openXml(string XmlPath,xmlNode* &rootNode) {
  curXmlPath = XmlPath;
  //using libxml2
  xmlDoc * doc = NULL;
  rootNode = NULL;
  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION
    /* parse the file and get the DOM */
    //    std::cout << "yyyyy" << curXmlPath << "vvvvvv" << endl;
    doc = xmlReadFile(curXmlPath.c_str(), NULL,0);
  if (doc == NULL) {
    opp_error("-E-  could not parse xml file %s\n",curXmlPath.c_str());
  }
  /*Get the root element node */
  rootNode = xmlDocGetRootElement(doc);

  string rootName = getXmlNodeName(rootNode);
  if (rootName!="AppInst" && rootName!="AppType")
    opp_error("-E- invalid xml format - "
              "file root name should be AppInst or AppType\n" );
  return doc;
}

int DCTGMgr::createXmlAppInst(xmlNode * rootNode) {
  string rootNodeName =getXmlNodeName(rootNode);
  if (rootNodeName!="AppInst")
    opp_error("-E- %s invalid xml format - root of appInst instance xml "
              "should be AppInst and not %s\n",
              curXmlPath.c_str(),rootNodeName.c_str());
  string appInstName = getXmlNodeProp(rootNode, "Name");
  if (verbose) cout<< "appInstName "  << appInstName << endl;
  if (appInstNameToAppIndex.find(appInstName)!=appInstNameToAppIndex.end()) {
    if (verbose) cout<< "appInst "  << appInstName
                     << " already created"<< endl;
    return 1;
  }
  DCTGAppInst curApp(appInstName);
  for (xmlNode *appInstParamNode = rootNode->children; appInstParamNode; 
       appInstParamNode = appInstParamNode->next) {
    if (appInstParamNode->type != XML_ELEMENT_NODE)
      continue;
    string appInstParamName = getXmlNodeName(appInstParamNode);
    if (appInstParamName=="AppType") {
      string appInstInstXml = curXmlPath;
      xmlNode * appTypeXmlRoot;
      xmlDoc* doc = openXml(getXmlNodeContent(appInstParamNode),appTypeXmlRoot);
      if (verbose) cout <<"start parsing " << curXmlPath << endl;
      curApp.appType = createXmlAppType(appTypeXmlRoot);
      xmlFreeDoc(doc);
      if (verbose) cout <<"finish parsing " << curXmlPath << endl;
      curXmlPath = appInstInstXml;
    } else if (appInstParamName=="Gen") {
      if (curApp.appType==NULL) {
        opp_error("-E- %s invalid xml format - AppType should define before "
                  "the Gens\n",curXmlPath.c_str());
      }
      DCTGGen* curGen = new DCTGGen;
      for (xmlNode *genParamNode= appInstParamNode->children; genParamNode; 
           genParamNode = genParamNode->next) {
        if (genParamNode->type != XML_ELEMENT_NODE)
          continue;
        // Example:
        //                <Role>hadoop</Role>
        //                <Addr>addr0</Addr>
        //                <Host>3</Host>
        //                <Rack>23</Rack>
        //                <Cluster>1</Cluster>
        //                <DC>0</DC>
        string genPropName = getXmlNodeName(genParamNode);
        string genProp = getXmlNodeContent(genParamNode);
        if (genPropName=="Role") {
          curGen->roleType = curApp.appType->getRole(genProp);
        } else if (genPropName=="Addr") {
          curGen->address =genProp;
          if (genAddrToAppIndex.find(curGen->address) != genAddrToAppIndex.end()) {
            opp_error("-E- %s - address %s allocated twice\n",
                      curXmlPath.c_str(),curGen->address.c_str());
          }
          genAddrToAppIndex[curGen->address] = appInsts.size();
        } else if (genPropName=="Host") {
          curGen->host = strToInteger(genProp,appInstName);
        } else if (genPropName=="Rack") {
          curGen->rack = strToInteger(genProp,appInstName);
        } else if (genPropName=="Cluster") {
          curGen->cluster = strToInteger(genProp,appInstName);
        } else if (genPropName=="DC")  {
          curGen->dataCenter = strToInteger(genProp,appInstName);
        } else {
          opp_error("-E- %s invalid xml format - gen property %s not "
                    "supported we support only Role,Addr,Rack,Cluster and "
                    "DC (App %s)\n",
                    curXmlPath.c_str(),genPropName.c_str(),appInstName.c_str());
        }
      }
      curApp.addGen(curGen);
    } else {
      opp_error("-E- %s invalid xml format - appInstInst child should be"
                " AppType or Gen not %s (App %s)/n",
                curXmlPath.c_str(),appInstParamName.c_str(),
                appInstName.c_str());
    }
  }
  appInstNameToAppIndex[curApp.appInstName] = appInsts.size();
  appInsts.push_back(curApp);
  if (verbose) cout << "appInst: " <<curApp.appInstName
                    << " created! index in appInsts vector is "
                    << appInstNameToAppIndex[curApp.appInstName] << endl;
  return 0;
}

DCTGAppType* DCTGMgr::createXmlAppType(xmlNode * rootNode) {
  string appTypeName = getXmlNodeProp(rootNode,"Name");
  //allocate new appType
  if (appTypeToIndex.find(appTypeName)!=appTypeToIndex.end()) {
    return appTypes[appTypeToIndex[appTypeName]];
  }
  DCTGAppType* curAppType = new DCTGAppType(appTypeName);
  // loop on roles
  for (xmlNode *roleNode= rootNode->children; roleNode; 
       roleNode = roleNode->next) {
    if (roleNode->type != XML_ELEMENT_NODE)
      continue;
    string roleNodeName = getXmlNodeName(roleNode);
    if(roleNodeName!="Role") {
      opp_error("-E- %s invalid xml format - son of AppRoles should be "
                "Role and it is %s (App: %s)",
                curXmlPath.c_str(),roleNodeName.c_str(),appTypeName.c_str());
    }
    //allocate new role in curAppType
    string roleName = getXmlNodeProp(roleNode,"Name");
    if (curAppType->roleExists(roleName)) {
      opp_error("-E- %s Role %s is already allocated in appType %s\n",
                curXmlPath.c_str(),roleName.c_str(),
                curAppType->appTypeName.c_str());
    }
    DCTGRole* curRole = new DCTGRole(roleName);
    //loop on distribution types
    for (xmlNode *distTypeNode= roleNode->children; distTypeNode; 
         distTypeNode = distTypeNode->next) {
      if (distTypeNode->type != XML_ELEMENT_NODE)
        continue;
      string distTypeNodeName = getXmlNodeName(distTypeNode);
      if (distTypeNodeName=="LocalityDist") {
        curRole->localityDCTGDist = createDCTGDist(distTypeNode);
      } else if (distTypeNodeName=="DistType") {
        string distTypeName = getXmlNodeProp(distTypeNode,"Name");
        DistType distType = strToDistType(distTypeName);
        if (distType==NO_DIST_TYPE_SPECIFIED) {
          opp_error("-E- %s invalid xml format - DistType %s not supported\n",
                    curXmlPath.c_str(),distTypeName.c_str() );
        }
        //loop on localities
        for (xmlNode *localityNode = distTypeNode->children; localityNode; 
             localityNode = localityNode->next) {
          if (localityNode->type != XML_ELEMENT_NODE)
            continue;
          string localityNodeName = getXmlNodeName(localityNode);
          if (localityNodeName!="Locality") {
            opp_error("-E- %s invalid xml format - son of DistType "
                      "should be Locality (App: %s Role %s DistType %s)\n",
                      curXmlPath.c_str(), appTypeName.c_str(),
                      roleName.c_str(),distTypeName.c_str());
          }
          string localityName=  getXmlNodeProp(localityNode,"Name");
          Locality locality = strToLocality(localityName);
          if(locality==NO_LOCALITY_SPECIFIED) {
            opp_error("-E- %s invalid xml format - Locality %s not supported\n",
                      curXmlPath.c_str(),localityName.c_str());
          }
          DCTGDist * p_DCTGDist = createDCTGDist(localityNode);
          if (locality==ALL) {
            for (int loc=INTRA_HOST;loc<ALL;loc++) {
              curRole->DCTGDists[(Locality)loc][distType] = p_DCTGDist;
            }
          } else {
            curRole->DCTGDists[locality][distType] = p_DCTGDist;
          }
        }
      } else {
        opp_error("-E- %s invalid xml format - son of Role should be "
                  "DistType or LocalityDist not %s (App: %s Role: %s)\n",
                  curXmlPath.c_str(),distTypeNodeName.c_str(),
                  appTypeName.c_str(),roleName.c_str());
      }
    }
    if(curRole->localityDCTGDist==NULL) {
      opp_error("-E- %s invalid xml format - no locality distribution "
                "(App: %s Role: %s)\n",
                curXmlPath.c_str(),appTypeName.c_str(),roleName.c_str());
    }
    if(curRole->DCTGDists.empty()) {
      opp_error("-E- %s invalid xml format - no distType distribution  "
                "(App: %s Role: %s)\n",
                curXmlPath.c_str(),appTypeName.c_str(),roleName.c_str());
    }
    appTypeToIndex[appTypeName] = appInsts.size();
    curAppType->addRole(curRole);
  }
  if(curAppType->roles.empty()) {
    opp_error("-E- %s invalid xml format - no role allocated (AppType: %s)\n",
              curXmlPath.c_str(),curAppType->appTypeName.c_str());
  }
  appTypeToIndex[appTypeName] = appTypes.size();
  appTypes.push_back(curAppType);
  
  if (verbose)  {
    for (unsigned int i=0;i<curAppType->roles.size();i++) {
      cout<<"role " << i << " is " <<curAppType->roles[i]->roleName << endl;
    }
  }
  return appTypes.back();
}

int DCTGMgr::reg(string address,string appInstXml) {
  if (verbose) cout << "register address " << address << " appInstXml:  "
                    << appInstXml <<endl;
  xmlNode * appInstXmlRoot;
  if (verbose) cout <<"start parsing " << appInstXml << endl;
  xmlDoc* doc = openXml(appInstXml,appInstXmlRoot);
  if (verbose) cout <<"finish parsing " << appInstXml << endl;
  createXmlAppInst(appInstXmlRoot);
  xmlFreeDoc(doc);
  if (genAddrToAppIndex.find(address) == genAddrToAppIndex.end()) {
    opp_error("-E- gen registration fail - address %s not found\n",
              address.c_str());
  }
  return 0;
}


DCTGDist * DCTGMgr::createDCTGDist(xmlNode * node ) {
  bool fromFile = false;
  bool fromPar  = false;
  string path_or_parName;
  // get locality feedType and Dist (could be of from file or parameter
  // randomize in caller ned file)
  for (xmlNode *randGenPropNode = node->children; randGenPropNode; 
       randGenPropNode = randGenPropNode->next) {
    if (randGenPropNode->type != XML_ELEMENT_NODE)
      continue;
    string localityPropName = getXmlNodeName(randGenPropNode);
    string contetnt = getXmlNodeContent(randGenPropNode);
    if (localityPropName=="FeedType") {
      fromFile = contetnt=="file";
      fromPar = contetnt=="param";
    } else if (localityPropName=="Dist") {
      path_or_parName=contetnt;
    } else {
      opp_error("-E- %s  invalid xml format - Locality son  should be "
                "FeedType or Dist not %s\n",
                curXmlPath.c_str() ,localityPropName.c_str());
    }
  }
  if (!(fromFile ^ fromPar)) {
    opp_error("-E- %s invalid xml format - FeedType should be file xor param\n",
              curXmlPath.c_str());
  }
  if (path_or_parName.empty()) {
    opp_error("-E- %s invalid xml format - no Dist define "
              "(App: %s Role: %s DistType: %s Locality: %s)\n", 
              curXmlPath.c_str());
  }
  //create new DCTGDist.
  DCTGDist * p_DCTGDist = new DCTGDist;
  allDCTGDists.push_back(p_DCTGDist); //for delete manner
  if (fromFile)
    p_DCTGDist->setCdfTable(path_or_parName);
  else
    p_DCTGDist->setCallerRandParam(path_or_parName);
  return p_DCTGDist;
}

inline string DCTGMgr::getXmlNodeName(xmlNode * node) {
  return string((const char*) node->name);
}

inline string DCTGMgr::getXmlNodeProp(xmlNode * node,const char * propName) {
  return string((const char *) xmlGetProp(node, (xmlChar*) propName));
}

inline string DCTGMgr::getXmlNodeContent(xmlNode * node) {
  xmlBufferPtr buf = xmlBufferCreate();
  xmlNodeBufGetContent(buf,node);
  string content((const char*) xmlBufferContent(buf));
  xmlBufferFree(buf);
  return content;
}

int  DCTGMgr::strToInteger(string intStr,string appInstName) {
  int i = 0;
  try {
    i = stoi(intStr);
  } catch(invalid_argument) {
    opp_error("-E- %s App %s gen num is not a number it is %s\n",
              curXmlPath.c_str(),appInstName.c_str(),intStr.c_str());
  } catch(out_of_range) {
    opp_error("-E- %s App %s gen num %s out of integer range\n",
              curXmlPath.c_str(),appInstName.c_str(),intStr.c_str());
  }
  return i;
}

DistType  strToDistType(string distTypeStr) {
  if (distTypeStr=="MSG_SIZE")      return MSG_SIZE ;
  if (distTypeStr=="INTER_ARRIVAL") return  INTER_ARRIVAL;
  if (distTypeStr=="FLOW_DURATION") return FLOW_DURATION;
  if (distTypeStr=="FLOW_SIZE")     return FLOW_SIZE;
  if (distTypeStr=="LOCALITY")      return LOCALITY;
  return NO_DIST_TYPE_SPECIFIED;
}

Locality  strToLocality(string localityStr) {
  if (localityStr=="INTRA_HOST")        return INTRA_HOST ;
  if (localityStr=="INTRA_RACK")        return INTRA_RACK;
  if (localityStr=="INTRA_CLUSTER")     return INTRA_CLUSTER;
  if (localityStr=="INTRA_DATA_CENTER") return INTRA_DATA_CENTER;
  if (localityStr=="INTER_DATA_CENTER") return INTER_DATA_CENTER;
  if (localityStr=="ALL")               return ALL;
  if (localityStr=="NONE")              return NONE;
  return NO_LOCALITY_SPECIFIED;
}
