# e1039-analysis/RecoData2024/work_ana

This directory is to analyze a set of reconstructed data files selected by `DIR_DST` at the beginning of `gridsub.sh`.
To analyze officially-reconstructed data, you have to use `gridsub_official.sh` as explained later.


## Test of Analysis Using a Few Runs

The procedure below will analyze the 1st-2nd good runs to draw basic dimuon variables.

```
cd /path/to/your/e1039-analysis/RecoData2024/work_ana
source ../setup.sh
./gridsub.sh -j 1-2
root -b 'AnaEventTree.C("scratch/ana")'
display result/h1_m.png &
```

Here are what happens in each step:
- `gridsub.sh` sets up a subdirectory (per run) to analyze DST files of given runs.
    - The option `-j 1-2` selects the 1st-2nd runs (listed in `list_run_spill.txt`).
    - Analyzing one run is directed by `gridrun.sh` and `Fun4All.C`.
    - A SubsysReco module, `AnaDimuon`, is registered by default.
    - The analysis runs on local computer by default (i.e. when `-g` is not given).
    - Extracted dimuon parameters are stored in `scratch/ana/run_*/out/output.root` as TTree.
- `AnaEventTree.C` reads the TTree files to draw histograms.
    - Histograms are saved under `result/`.


## Analysis Using All Runs

You can analyze other runs by adjusting the `-j` option of `gridsub.sh`.
The `-g` option let you use the Grid computing.
You first submit jobs for the first 10 runs;
```
kinit
./gridsub.sh -g -j 1-10
```

You might execute `jobsub_q_mine` to monitor the job status, which is an alias of `jobsub_q --group spinquest --user=$USER` defined in `setup.sh`.
Once you confirm that the jobs are running fine, you submit more jobs for the remaining runs;
```
./gridsub.sh -g -m 20 -j 11-
```
Here `-m 20` sets the max number of total (idle+running+held) jobs at a time.

The output files are stored under `data/ana/` in case Grid is used.
You can analyze them once (almost) all jobs finish;
```
root -b AnaEventTree.C
display result/h1_m.png &
```

You can find the index of a run, which can be used for `-j` by this command:
```
cut -f 1 list_run_spill.txt | uniq | less -N
```


## Analysis Condition

The analysis condition is programmed in `src/AnaDimuon.(h|cc)`.
Thus you need look into these files to understand the default condition.
You might modify these files as you need.
You need execute `make-this` after modification.


## Dimuon Vertexing

You might redo the dimuon vertexing together with the anlaysis.
It is feasible since the vertexing finishes rather quickly.
To do this, you uncomment the following lines in `Fun4All.C`:
```
//R__LOAD_LIBRARY(libktracker)

  //VertexFit* vtx_fit = new VertexFit();
  //vtx_fit->enable_fit_target_center();
  //se->registerSubsystem(vtx_fit);
```

## Analysis of Officially-Reconstructed Data

To analyze officially-reconstructed data, you have to use `gridsub_official.sh` instead of `gridsub.sh` because the name of data files is not differentiated among runs/spills.
