#ifndef RPCRecHitInfo_H
#define RPCRecHitInfo_H

struct RPCRecHitInfo
{
  int region, ring, station, sector, layer, subsector, roll;
  double lx, lex;
  double gx, gy, gz;
  
  int clusterSize, bx;
};

#endif

