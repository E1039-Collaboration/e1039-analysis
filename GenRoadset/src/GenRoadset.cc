#include <iostream>
#include <iomanip>
#include <vector>
#include <UtilAna/UtilTrigger.h>
#include "RoadInfo.h"
#include "UtilRoad.h"
#include "AnaSignal.h"
#include "GenRoadset.h"
using namespace std;

GenRoadset::GenRoadset(const std::string label)
  : AnaBase       (label)
  , m_map_pos_top (0)
  , m_map_pos_bot (0)
  , m_map_neg_top (0)
  , m_map_neg_bot (0)

{
  m_list_pos_top.SetPNTB(+1, +1);
  m_list_pos_bot.SetPNTB(+1, -1);
  m_list_neg_top.SetPNTB(-1, +1);
  m_list_neg_bot.SetPNTB(-1, -1);
}

GenRoadset::~GenRoadset()
{
  ;
}

void GenRoadset::SetMap(AnaSignal* ana_signal)
{
  m_map_pos_top = ana_signal->GetRoadMapPosTop();
  m_map_pos_bot = ana_signal->GetRoadMapPosBot();
  m_map_neg_top = ana_signal->GetRoadMapNegTop();
  m_map_neg_bot = ana_signal->GetRoadMapNegBot();

  m_list_pos_top.Add(m_map_pos_top);
  m_list_pos_bot.Add(m_map_pos_bot);
  m_list_neg_top.Add(m_map_neg_top);
  m_list_neg_bot.Add(m_map_neg_bot);
}

void GenRoadset::GenerateRoadSetBySignal(const double frac_cut, const std::string rs_id)
{
  if (frac_cut < 0 || frac_cut > 1) return;
  DisableRoadsByWeightFraction(&m_list_pos_top, frac_cut);
  DisableRoadsByWeightFraction(&m_list_pos_bot, frac_cut);
  DisableRoadsByWeightFraction(&m_list_neg_top, frac_cut);
  DisableRoadsByWeightFraction(&m_list_neg_bot, frac_cut);
  UtilRoad::WriteToFile(m_map_pos_top, m_map_pos_bot, m_map_neg_top, m_map_neg_bot, rs_id);
}

void GenRoadset::GenerateRoadSetByFoM(const double frac_cut, const std::string rs_id)
{
  if (frac_cut < 0 || frac_cut > 1) return;
  DisableRoadsByFoM(&m_list_pos_top, frac_cut);
  DisableRoadsByFoM(&m_list_pos_bot, frac_cut);
  DisableRoadsByFoM(&m_list_neg_top, frac_cut);
  DisableRoadsByFoM(&m_list_neg_bot, frac_cut);
  UtilRoad::WriteToFile(m_map_pos_top, m_map_pos_bot, m_map_neg_top, m_map_neg_bot, rs_id);
}

void GenRoadset::GenerateSymmetricRoadSetByFoM(const double frac_cut, const std::string rs_id)
{
  if (frac_cut < 0 || frac_cut > 1) return;

  RoadList* list_roads[4] = { &m_list_pos_top, &m_list_pos_bot, &m_list_neg_top, &m_list_neg_bot };
  RoadMap*   map_roads[4] = {   m_map_pos_top,   m_map_pos_bot,   m_map_neg_top,   m_map_neg_bot };
  int        num_roads[4] = { 0, 0, 0, 0 };
  double w_tot = 0;
  for (int ir = 0; ir < 4; ir++) {
    RoadList* roads = list_roads[ir];
    roads->SortByFoM();
    int n_ent = num_roads[ir] = roads->Size();
    for (int ii = 0; ii < n_ent; ii++) w_tot += roads->Get(ii)->GetFoM();
    
    m_ofs << "RoadList: " << roads->GetPosNegStr() << " " << roads->GetTopBotStr() << "\n"
          << "  First Five\n";
    UtilRoad::PrintList(roads, +1, +5, m_ofs);
    m_ofs << "  Last Five\n";
    UtilRoad::PrintList(roads, -5, -1, m_ofs);
  }

  double w_dis = 0;
  while (true) {
    int ir_min = -1;
    double fom_min = 0;
    for (int ir = 0; ir < 4; ir++) {
      RoadList* roads = list_roads[ir];
      RoadInfo* info = roads->Get(num_roads[ir] - 1);
      double fom = info->GetFoM();
      if (ir_min < 0 || fom < fom_min) {
        ir_min  = ir;
        fom_min = fom;
      }
    }
    RoadList* roads = list_roads[ir_min];
    RoadInfo* info = roads->Get(--num_roads[ir_min]);
    int road_id = info->GetRoadID();
    //info->SetOnOff(false);
    //w_dis += info->GetFoM();

    //int h1, h2, h3, h4, tb;
    //UtilTrigger::Road2Hodo(road_id, h1, h2, h3, h4, tb);
    int road_id4[4]; //{ m_list_pos_top, m_list_pos_bot, m_list_neg_top, m_list_neg_bot }
    road_id4[ir_min] = road_id;
    if (road_id == 0) { // pos_top
      road_id4[1] = FlipRoadTopBottom(road_id);
      road_id4[2] = FlipRoadLeftRight(road_id4[0]);
      road_id4[3] = FlipRoadTopBottom(road_id4[2]);
    } else if (road_id == 1) { // pos_bot
      road_id4[0] = FlipRoadTopBottom(road_id);
      road_id4[2] = FlipRoadLeftRight(road_id4[0]);
      road_id4[3] = FlipRoadTopBottom(road_id4[2]);
    } else if (road_id == 2) { // neg_top
      road_id4[0] = FlipRoadLeftRight(road_id);
      road_id4[1] = FlipRoadTopBottom(road_id4[0]);
      road_id4[3] = FlipRoadTopBottom(road_id4[2]);
    } else { // == 3, neg_bot
      road_id4[2] = FlipRoadTopBottom(road_id);
      road_id4[0] = FlipRoadLeftRight(road_id4[2]);
      road_id4[1] = FlipRoadLeftRight(road_id);
    }

    for (int ir = 0; ir < 4; ir++) {
      //RoadList* roads = list_roads[ir];
      RoadMap*  map_rd = map_roads[ir];
      RoadInfo* info = map_rd->Find(road_id4[ir]);
      if (info && info->GetOnOff()) {
        info->SetOnOff(false);
        w_dis += info->GetFoM();
      }
    }
    
    if (w_dis / w_tot >= frac_cut) break;
    //if (num_roads[ir_min] == 0) {
    //  cout << "No road remains for ir = " << ir_min << ".  Abort." << endl;
    //  exit(1);
    //}
  }
  
  UtilRoad::WriteToFile(m_map_pos_top, m_map_pos_bot, m_map_neg_top, m_map_neg_bot, rs_id);
}

void GenRoadset::DisableRoadsByWeightFraction(RoadList* road_list, const double frac)
{
  road_list->SortBySignal();

  m_ofs << "RoadList: " << road_list->GetPosNegStr() << " " << road_list->GetTopBotStr() << "\n"
        << "  First Five\n";
  UtilRoad::PrintList(road_list, +1, +5, m_ofs);
  m_ofs << "  Last Five\n";
  UtilRoad::PrintList(road_list, -5, -1, m_ofs);

  int n_ent = road_list->Size();
  double w_tot = 0;
  for (int ii = 0; ii < n_ent; ii++) w_tot += road_list->Get(ii)->GetWeight();
  double w_dis = 0;
  int i_ent = n_ent - 1;
  while (i_ent >= 0) {
    RoadInfo* info = road_list->Get(i_ent);
    info->SetOnOff(false);
    w_dis += info->GetWeight();
    if (w_dis / w_tot >= frac) break;
    i_ent--;
  }
  int n_dis = n_ent - i_ent;
  m_ofs << "  Total   : weight = " << w_tot << ", N = " << n_ent << "\n"
        << "  Disabled: weight = " << w_dis << ", N = " << n_dis << "\n"
        << "                     " << w_dis/w_tot << ", " << 1.0*n_dis/n_ent << endl;
}

void GenRoadset::DisableRoadsByFoM(RoadList* road_list, const double frac)
{
  road_list->SortByFoM();

  m_ofs << "RoadList: " << road_list->GetPosNegStr() << " " << road_list->GetTopBotStr() << "\n"
        << "  First Five\n";
  UtilRoad::PrintList(road_list, +1, +5, m_ofs);
  m_ofs << "  Last Five\n";
  UtilRoad::PrintList(road_list, -5, -1, m_ofs);

  int n_ent = road_list->Size();
  double w_tot = 0;
  for (int ii = 0; ii < n_ent; ii++) w_tot += road_list->Get(ii)->GetFoM();
  double w_dis = 0;
  int i_ent = n_ent - 1;
  while (i_ent >= 0) {
    RoadInfo* info = road_list->Get(i_ent);
    info->SetOnOff(false);
    w_dis += info->GetFoM();
    if (w_dis / w_tot >= frac) break;
    i_ent--;
  }
  int n_dis = n_ent - i_ent;
  m_ofs << "  Total   : weight = " << w_tot << ", N = " << n_ent << "\n"
        << "  Disabled: weight = " << w_dis << ", N = " << n_dis << "\n"
        << "                     " << w_dis/w_tot << ", " << 1.0*n_dis/n_ent << endl;
}

int GenRoadset::FlipRoadLeftRight(const int road)
{
  int h1, h2, h3, h4, tb;
  UtilTrigger::Road2Hodo(road, h1, h2, h3, h4, tb);
  h1 = 24 - h1; // 1 <-> 23, 2 <-> 22,,,
  h2 = 17 - h2; // 1 <-> 16, 2 <-> 15,,,
  h3 = 17 - h3;
  h4 = 17 - h4;
  return UtilTrigger::Hodo2Road(h1, h2, h3, h4, tb);
}

int GenRoadset::FlipRoadTopBottom(const int road)
{
  int h1, h2, h3, h4, tb;
  UtilTrigger::Road2Hodo(road, h1, h2, h3, h4, tb);
  return UtilTrigger::Hodo2Road(h1, h2, h3, h4, -tb);
}

void GenRoadset::SymmetrizeRoads()
{
  map<int, int> road_count; // [road ID as pos_top] = count
  for (int ii = 0; ii < m_list_pos_top.Size(); ii++) {
    RoadInfo* info = m_list_pos_top.Get(ii);
    int road_id = info->GetRoadID();
    road_count[road_id]++;
  }
  for (int ii = 0; ii < m_list_pos_bot.Size(); ii++) {
    RoadInfo* info = m_list_pos_bot.Get(ii);
    int road_id = FlipRoadTopBottom(info->GetRoadID());
    road_count[road_id]++;
  }
  for (int ii = 0; ii < m_list_neg_top.Size(); ii++) {
    RoadInfo* info = m_list_neg_top.Get(ii);
    int road_id = FlipRoadLeftRight(info->GetRoadID());
    road_count[road_id]++;
  }
  for (int ii = 0; ii < m_list_neg_bot.Size(); ii++) {
    RoadInfo* info = m_list_neg_bot.Get(ii);
    int road_id = FlipRoadTopBottom( FlipRoadLeftRight(info->GetRoadID()) );
    road_count[road_id]++;
  }

  /// Keep only the roads that are found in the four groups (i.e. count = 4).
  for (int ii = 0; ii < m_list_pos_top.Size(); ii++) {
    RoadInfo* info = m_list_pos_top.Get(ii);
    int road_id = info->GetRoadID();
    if (road_count[road_id] != 4) info->SetOnOff(false);
  }
  for (int ii = 0; ii < m_list_pos_bot.Size(); ii++) {
    RoadInfo* info = m_list_pos_bot.Get(ii);
    int road_id = FlipRoadTopBottom(info->GetRoadID());
    if (road_count[road_id] != 4) info->SetOnOff(false);
  }
  for (int ii = 0; ii < m_list_neg_top.Size(); ii++) {
    RoadInfo* info = m_list_neg_top.Get(ii);
    int road_id = FlipRoadLeftRight(info->GetRoadID());
    if (road_count[road_id] != 4) info->SetOnOff(false);
  }
  for (int ii = 0; ii < m_list_neg_bot.Size(); ii++) {
    RoadInfo* info = m_list_neg_bot.Get(ii);
    int road_id = FlipRoadTopBottom( FlipRoadLeftRight(info->GetRoadID()) );
    if (road_count[road_id] != 4) info->SetOnOff(false);
  }
}
