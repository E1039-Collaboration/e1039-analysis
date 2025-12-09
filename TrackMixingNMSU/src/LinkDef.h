#ifdef __CINT__
#pragma link off all class;
#pragma link off all function;
#pragma link off all global;

// Analysis classes
//   Defined in Ana*.(h|cc)
#pragma link C++ class AnaSortMixVertex-!;

// Data classes
//   Defined in TreeData.(h|cc) & Ana*.(h|cc)
#pragma link C++ class EventData+;
#pragma link C++ class OccData+;
#pragma link C++ class TrackData+;
#pragma link C++ class DimuonData+;
#pragma link C++ class TrackList+;
#pragma link C++ class DimuonList+;
//#pragma link C++ class SRecDimuonData+;

//
// Utility classes
//
#pragma link C++ namespace RunList-!;
#pragma link C++ namespace UtilBeam-!;
#pragma link C++ namespace UtilHist-!;
#pragma link C++ namespace UtilTdc-!;

#endif
