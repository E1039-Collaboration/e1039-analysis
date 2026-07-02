# A Program to Run the J/psi Generator

It generates J/psi events by using `e1039-core/generators/JpsiGen`.
The usage is similar to `SimChainDev`.

It is always better to first run test jobs locally and on GRID:
```
./gridsub.sh -n proton
./gridsub.sh -g -n proton -j 1-5 -e 1000
```

You then submit the remaining jobs:
```
./gridsub.sh -g -n proton -j 6-100 -e 1000
```

Here are typical commands to generate events on three target types:
```
./gridsub.sh -g -n proton   -j 1-100 -e 1000
./gridsub.sh -g -n deuteron -j 1-100 -e 1000
./gridsub.sh -g -n nitrogen -j 1-100 -e 1000
```

You can analyze outputs by using `AnaSimDst`.
