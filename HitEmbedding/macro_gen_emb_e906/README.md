# Macro to generate embedding data from E906 NIM3 events

This macro is to generate the embedding data from the E906 NIM3 events.
The usage of this macro is not much documented because the embedded data are already available and thus you need not execute it by yourself.

## Location of embedding data of E906 NIM3 events

```
/pnfs/e1039/persistent/users/kenichi/data_emb_e906
```

## Procedure

You usually need not do this by yourself.

The digit files had better be prestaged before use.
```
prestage-input-file.sh
```

The generation process can/should be done on local computer, since one run takes only a few minutes.
It might be better to divide the whole process into multiple blocks like this;
``
./gridsub.sh -n 2000-2500
``

The run-by-run output files are merged;
```
./merge-emb-file.sh
```
