R__LOAD_LIBRARY(libana_embedding)
using namespace std;

void AnaBothData(const string fn_no_emb="not_embedded/ana_tree.root", const string fn_emb="embedded/ana_tree.root")
{
  AnaCleanAndMessyData* ana = new AnaCleanAndMessyData();
  //ana->Verbosity(10);
  ana->RequireFpga1(true); // default = false
  ana->Init(fn_no_emb.c_str(), fn_emb.c_str());
  ana->Analyze();
  ana->End();
  exit(0);
}
