Network Simulation
---
SPRING2021_ECEN602_DI_TIAN
---

### Install and Usage
First make sure ns2, tcl and xgraph are installed
```
apt-get install ns2
apt-get install tcl
apt-get install xgraph
```
If all of the required components are ready, run
```
ns ns2.tcl <TCP_FLAVOR> <CASE_NO>
```
TCP_FLAVOR enum: VEGAS, SACK <br/>
CASE_NO enum: 1 - 10 (ten RTT ratios are explored in this experiments)

---
#### Design goals
This script aims to simulate the throughput of TCP under different latencies and TCP flavors.

--- 
#### Code summary
The code consists of following parts:
- Parsing command line arguments                (line 1 - line 28)
- Simulation setup                              (line 30 - line 49)
    - initialize ns simulator instance
    - nam setup
    - create result files
- Network topology                              (line 51 - line 113)
    - create nodes
    - create links
    - setup TCP connection and FTP
    - setup scheduler
- Throughput recording                          (line 115 - line 130)
- Plotting result in Nam                        (line 132 - line 143)
- Finish procedure                              (line 145 - line 160)

---
### Contribution
Di Tian.

