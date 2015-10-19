#ifndef RPCRecHitInfo_H
#define RPCRecHitInfo_H

struct RPCRecHitInfo
{
  int region, ring, station, sector, layer, subsector, roll;
  double lx, lex;
  double gx, gy, gz;
  
  int clusterSize, bx;
};

struct RPCRefRecHitInfo : public RPCRecHitInfo
{
  double mupt, mueta, muphi;
  double rlx, rly, rlex;
  double dx;
};

#endif

