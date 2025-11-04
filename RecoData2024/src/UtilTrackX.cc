#include <cmath>
#include <geom_svc/GeomSvc.h>
#include <ktracker/SRecEvent.h>
#include <UtilAna/UtilTrigger.h>
#include "UtilTrackX.h"
using namespace std;

int UtilTrackX::verbosity = 0;

/// Find all roads matching to the given track.
/**
 * The closest paddle to the given track at each hodoscope X plane (H1T/B, H2T/B, H3T/B and H4T/B) is selected based on the expected track x-position.
 * The two adjacent paddles are also selected if they are within "width/2 + margin".
 * All selected paddles are used to list up matched roads.
 * @code
 *   SRecTrack* trk = ...
 *   const double margin = 1.0; // in cm
 *   vector<int> list_road_id = UtilTrack::FindMatchedRoad(trk, margin);
 * @endcode
 *
 * The original top/bottom position of each track is decided in `TriggerRoad::TriggerRoad(Tracklet& tracklet)`.
 * The element ID of a paddle is sometimes nElements+1, as returned by GeomSvc::getExpElementID().
 */
std::vector<int> UtilTrackX::FindMatchedRoads(SRecTrack* trk, const double margin)
{
  GeomSvc* geom = GeomSvc::instance();
  double y_st1 = trk->get_pos_st1().Y();
  double y_st3 = trk->get_pos_st3().Y();
  int top_bot = y_st3>0 ? +1 : -1;
  if (verbosity > 0) cout << "UtilTrackX::FindMatchedRoads(): charge=" << trk->getCharge() << " y_st1=" << y_st1 << " y_st3=" << y_st3 << endl;
  
  vector<int> list_ele_id[5];
  for (int st = 1; st <= 4; st++) {
    string det_name = (string)"H" + (char)('0'+st) + (top_bot>0 ? 'T' : 'B');
    int det_id = geom->getDetectorID(det_name);
    Plane* det = geom->getPlanePtr(det_id);
    double x_det = det->xc + det->deltaX;
    //double y_det = det->yc + det->deltaY;
    double z_det = det->zc + det->deltaZ;
    int    n_ele = det->nElements;
    double space = det->spacing;
    double width = det->cellWidth;
    if (verbosity > 2) cout << "  st" << st << ":";
    if (verbosity > 3) cout << " x_det=" << x_det << " n_ele=" << n_ele << " space=" << space << " width=" << width;

    double x_trk, y_trk;
    trk->getExpPositionFast(z_det, x_trk, y_trk);
    int ele_cent = (int)((n_ele+1)/2.0 + (x_trk-x_det)/space + 0.5);
    if (verbosity > 2) cout << " x_trk=" << x_trk << " y_trk=" << y_trk;

    /// Check three elements, assuming the margin is much smaller than the half width.
    for (int i_ele = ele_cent - 1; i_ele <= ele_cent + 1; i_ele++) {
      if (i_ele <= 0 || i_ele > n_ele) continue;
      double x_ele = x_det + space * (i_ele - (n_ele+1)/2.0);
      if (verbosity > 2) cout << " [ele=" << i_ele << " x=" << x_ele;
      if (verbosity > 3) cout << " edge=" << (width/2-fabs(x_trk-x_ele));
      if (verbosity > 2) cout << "]";
      if (fabs(x_trk - x_ele) < width/2 + margin) list_ele_id[st].push_back(i_ele);
    }
    if (verbosity > 2) cout << endl;
  
    //int i_ele = (int)((n_ele+1)/2.0 + (x_trk-x_det)/space + 0.5);
    //double x_ele = x_det + space * (i_ele - (n_ele+1)/2.0);
    //if (verbosity > 2) cout << "  st" << st << ":";
    //if (verbosity > 3) cout << " x_det=" << x_det << " n_ele=" << n_ele << " space=" << space << " width=" << width;
    //if (verbosity > 2) cout << " x_trk=" << x_trk << " y_trk=" << y_trk << " i_ele=" << i_ele << " x_ele=" << x_ele;
    //if (verbosity > 3) cout << " edge=" << (width/2-fabs(x_trk-x_ele));
    //if (verbosity > 2) cout << endl;
    //
    //if (fabs(x_trk - (x_ele-space)) < width/2 + margin) {
    //  if (0 < i_ele-1 && i_ele-1 <= n_ele) list_ele_id[st].push_back(i_ele-1);
    //}
    //if (0 < i_ele && i_ele <= n_ele) list_ele_id[st].push_back(i_ele);
    //if (fabs(x_trk - (x_ele+space)) < width/2 + margin) {
    //  if (0 < i_ele+1 && i_ele+1 <= n_ele) list_ele_id[st].push_back(i_ele+1);
    //}
  }
  vector<int> list_road_id;
  for (auto it1 = list_ele_id[1].begin(); it1 != list_ele_id[1].end(); it1++) {
  for (auto it2 = list_ele_id[2].begin(); it2 != list_ele_id[2].end(); it2++) {
  for (auto it3 = list_ele_id[3].begin(); it3 != list_ele_id[3].end(); it3++) {
  for (auto it4 = list_ele_id[4].begin(); it4 != list_ele_id[4].end(); it4++) {
    if (verbosity > 0) cout << "  road " << *it1 << " " << *it2 << " " << *it3 << " " << *it4;
    if (verbosity > 1) cout << " " << UtilTrigger::Hodo2Road(*it1, *it2, *it3, *it4, top_bot);
    if (verbosity > 0) cout << endl;
    list_road_id.push_back(UtilTrigger::Hodo2Road(*it1, *it2, *it3, *it4, top_bot));
  }}}}
  return list_road_id;
}
