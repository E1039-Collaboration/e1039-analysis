# e1039-analysis/TrackMixingNMSU

This package is to carry out the NMSU mixing method, which creates a sample of random dimuon backgrounds from FPGA1-triggered events.
It was first created from `/seaquest/users/harshaka/Combinatoric_background_e1039/` on 2025-08-15.

Mixing method explained in (https://arxiv.org/abs/2302.04152) was implemented by Abinash Pun for e906 experiment.
A modified version that can extract data from e1039 DST files and perform mixing with new SQVertexing alogorithm is included here.

## Key modifications 

Two major changes were made to make the scripts compatible with the e1039 DST files.
- E906 had SRawEvent and SRecEvent objects in the DST file. But SQEvent and SRecEvent for e1039. Created the SRawEvent objects using SQEvent and SQHitVector objects available in e1039 DST file.
- E906 used old `VertexFit` algorithm to perform the vertexing of the tracks. But e1039 data is reconstructed using new `SQVeretexing` algorithm. A custom version (outside the Fun4All framework) was created to introduce the new vertxign to this project.

## Usage

Soruce codes related to mixing in `/src` are,
- mixing : `AnaSortMixVertex.cc`
- modified SQVertexing : `SQVertexing_v2.cc`

All other files in `/src` except the ones related to base and tree data structure modules, performs various analysis tasks as explained here (https://github.com/abinashpun/seaquest-projects/blob/main/e906-root-ana/src/AnaSortMixVertex.cc).
You may modify the soruce codes to implement your own analysis.
Do `make-this` whenever a file in `/src` is modified.
Do `cmake-this` and `make-thi` whenever a file is added or removed from `/src`.

To run the mixing in GPVM,
- `source setup.sh`
- cd to `work`
- list all runs and spills in `list_run_spill.txt`.
- execute `./exec-step1.sh`
All outputs will be saved in a newly made `scratch` directory.

## Contact

Harsha Kaluarachchi <harsha92@nmsu.edu>
Kenichi Nakano <knakano0524@gmail.com>
