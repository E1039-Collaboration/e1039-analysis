#ifndef _UTIL_TRACK_X__H_
#define _UTIL_TRACK_X__H_
#include <vector>
class SRecTrack;

namespace UtilTrackX {
  extern int verbosity;
  std::vector<int> FindMatchedRoads(SRecTrack* trk, const double margin=0);
}

#endif /* _UTIL_TRACK_X__H_ */
