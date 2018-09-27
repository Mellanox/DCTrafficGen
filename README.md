# DCTG Data Center Traffic Generator Library
Data Center Network (DCN) research is gaining further momentum as the Cloud computing model proves scalable and efficient. As the economic efficiency of Cloud Computers improves with their size, Hyperclouds are now designed to contain 10,000 to 100,000 hosts. This extreme scale poses a challenge to DCN researchers, since building an experimental DCN of reasonable size is prohibitively expensive. Consequently, the research community greatly depends on the availability of formally defined traffic patterns, which describe the characteristics of Hyperscale DCN traffic. Even though several papers describing particular-cloud traffic were released over the years, no formal model was ever published or accepted as standard.

We contribute, the DCTG package, which provides a solution to this challenge by introducing a free to use, standard way of publishing traffic characteristics measured on hyperscale clusters. The package, is based on OMNeT++ event driven simulation technology that is free for academic use. DCTG generates traffic according to statistical profiles of flow duration, total size, packet size, message size, and destination locality. It also supports the definition of multiple application types in terms of the different roles of each Traffic Generator (TG) comprising the application, each with its own statistical or deterministic traffic injection characteristics. Then, it allows for describing the placement of multiple occurrences of these applications on specific network hosts. Each said host may contain a different number of TGs of each role representing the application occurrence. Our proposal is inspired by the detailed traffic  characteristics provided in [1] and the simulator code of  [2] that was generously made available to us.
We hope our contribution will start a process of open source development and contributions of common traffic benchmarks, that will advance DCN research. We call for more groups to contribute application types, with new traffic statistics as well as code to improve our current implementation. 

# Generator structure 
To determine the traffic pattern of every TG in the network we use the following terminology: topology, locality, application type, application occurrence, role and TG (that is a role occurrence).
-	Topology: The topology of the network is described hierarchically as a tree but may represent levels of proximity even on non-tree like topologies. The tree leaves are hosts, connected in racks, aggregated in clusters and finally to datacenters. 

Figure 1 Locality Hierarchy:  host, rack, Cluster, Data Center

-	Application Type: This object represents a class of an application. For example: Web Cache, Map Reduce or Database. Its definition consists of a list of TG roles that represent the traffic types injected by the processes or threads of this application type. 
  - Locality: A role definition includes locality distribution that determines the destinations that a TG of this role transmits to: intra-host, intra-rack, intra-cluster, intra- and inter-data center.
  - Characteristics: For each locality the role has a set of user-provided distribution functions that determine the traffic injection characteristics that TGs of this role produce. These characteristics are flow size, flow duration, messages size, inter arrival of next flow.  
-	Application Occurrence: Is an instance of an application type and is defined by a list of TGs.
-	TG: A traffic generator which has a unique address, a location identifier (its host, rack, cluster, datacenter), and a role defined by its application type’s roles.

Figure 2 The DCTG Objects their Aggregation (in blue) and Association (in orange and green).

# Usage
OMNeT++ simulations model should instantiate the DCTG Manager singleton and a set of TG objects (or a subclass of the one provided by the DCTG library). The TGs should invoke the manager getNextFlow() method and obtain back the full definition of a new flow which follows the exact characteristics of the role that TG carries within the application occurrence it belongs to. 

# References
[1] A. Roy, et al., “Inside the Social Network’s (Datacenter) Network,” in Proceedings of the 2015 SIGCOMM’15, pp. 123–137.

[2] B. Montazeri, et al., “Homa: A Receiver-Driven Low-Latency Transport Protocol Using Network Priorities,” 
      in Proceedings of the 2018 SIGCOMM’18, pp. 221–235.

# Authors 
{yuvals,eitan}@mellanox.com.
