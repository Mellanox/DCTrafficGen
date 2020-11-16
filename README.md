# DCTG Data Center Traffic Generator Library
Data Center Network (DCN) research is gaining further momentum as the
Cloud computing model proves scalable and efficient. As the economic
efficiency of Cloud Computers improves with their size, Hyperclouds
are now designed to contain 10,000 to 100,000 hosts. This extreme
scale poses a challenge to DCN researchers, since building an
experimental DCN of reasonable size is prohibitively
expensive. Consequently, the research community greatly depends on the 
availability of formally defined traffic patterns, which describe the
characteristics of Hyperscale DCN traffic. Even though several papers
describing particular-cloud traffic were released over the years, no
formal model was ever published or accepted as standard. 

We contribute, the DCTG package, which provides a solution to this
challenge by introducing a free to use, standard way of publishing
traffic characteristics measured on hyperscale clusters. The package,
is based on OMNeT++ event driven simulation technology that is free
for academic use. DCTG generates traffic according to statistical
profiles of flow duration, total size, packet size, message size, and
destination locality. It also supports the definition of multiple
application types in terms of the different roles of each Traffic
Generator (TG) comprising the application, each with its own
statistical or deterministic traffic injection characteristics. Then,
it allows for describing the placement of multiple occurrences of
these applications on specific network hosts. Each said host may
contain a different number of TGs of each role representing the
application occurrence. Our proposal is inspired by the detailed
traffic  characteristics provided in [1] and the simulator code of

[2] that was generously made available to us. 
We hope our contribution will start a process of open source
development and contributions of common traffic benchmarks, that will
advance DCN research. We call for more groups to contribute
application types, with new traffic statistics as well as code to
improve our current implementation.  

# Generator structure 
To determine the traffic pattern of every TG in the network we use the
following terminology: topology, locality, application type,
application occurrence, role and TG (that is a role occurrence). 
- Topology: The topology of the network is described hierarchically as
a tree but may represent levels of proximity even on non-tree like
topologies. The tree leaves are hosts, connected in racks, aggregated
in clusters and finally to datacenters.  

<img src="/README.pngs/fig1.png" alt="Figure 1 Locality Hierarchy: host, rack, Cluster, Data Center"/>

- Application Type: This object represents a class of an
  application. For example: Web Cache, Map Reduce or Database. Its
  definition consists of a list of TG roles that represent the traffic
  types injected by the processes or threads of this application type.  
- Locality: A role definition includes locality distribution that
  determines the destinations that a TG of this role transmits to:
  intra-host, intra-rack, intra-cluster, intra- and inter-data center. 
- Characteristics: For each locality the role has a set of
  user-provided distribution functions that determine the traffic
  injection characteristics that TGs of this role produce. These
  characteristics are flow size, flow duration, messages size, inter
  arrival of next flow.   
- Application Occurrence: Is an instance of an application type and is
  defined by a list of TGs. 
- TG: A traffic generator which has a unique address, a location
  identifier (its host, rack, cluster, datacenter), and a role defined
  by its application type’s roles.

<img src="/README.pngs/fig2.png" alt="Figure 2 The DCTG Objects their Aggregation (in blue) and Association (in orange and green)."/>

# Limitations
Traffic destinations are only classified by theor locaility, not by
role. For example if you would like to say that 20% of the traffic of
Cacheserver is going to the DB servers and the rest are to the Web
servers you cannot do that. 
  
# Usage
OMNeT++ simulations model should define a sub-class of the
DCTrafficGen class and must provide at least these 2 virtual functions
as they depand on the addressing scheme which is supposed to be
simulation model specific: 
- The function creating a new message: 
  cMessage * createMsg(unsigned int pktSize_B, unsigned int
  msgPackets,string dstAddr); 
- The function which registers the local address of the specific
  traffic generator virtual string getMyAddr();

Optionally one can define the emit_dstAddr method such that the vector
of destinations will be filled. This also requires knowledge of the
coding of addresses which is model dependant. The vector cannot track 
strings so a conversion to int or double is required...

We denote that sub-class as TG, for short. Then the simulation model
should instantiate a set of the TGs. At the given startTime the TG's
start to send messages (returned from the createMsg) without any
intervention. They also stop sending them at stopTime.

NOTE: In order to avoid too fast generation the TG will generate a new
      message only after a "done" event was received for the previous one.

# Installation

## DCTG library:
- You must have an omnetpp >= 4.6 installed and its bin dir in your PATH
- In the root directory run: make makefiles; make; make MODE=release
- You should find the resulting lib in the out/gcc-release or
  out/gcc-debug src directory 

## Demo project
- cd into the directory dctg_example
- run: make makefiles; make; make MODE=release
- cd simulations
- To simulate FBHadoop:
  ../out/gcc-debug/src/dctg_example -u Cmdenv -f omnetpp.ini -n ../src:../../src/ -c Hadoop
- To simulate FBFrontEnd:
  ../out/gcc-debug/src/dctg_example -u Cmdenv -f omnetpp.ini -n ../src:../../src/ -c FrontEnd

Enjoy :)

# References
[1] A. Roy, et al., “Inside the Social Network’s (Datacenter) Network,” 
    in Proceedings of the 2015 SIGCOMM’15, pp. 123–137.

[2] B. Montazeri, et al., “Homa: A Receiver-Driven Low-Latency Transport 
    Protocol Using Network Priorities,” in Proceedings of the 2018
    SIGCOMM’18, pp. 221–235.

# Authors 
{yuvals,eitan}@mellanox.com.
