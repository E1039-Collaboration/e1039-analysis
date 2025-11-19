# RUS Reconstructor

The **RUSReconstructor** package is an example simulated reconstruction module
that shows how to use the DST/RUS I/O managers and how to save dimuon variables
using `DimuAnaRUS`. It reads hit-level and/or truth-level information from a
RUS ROOT tree, could performs reconstruction and dimuon selection, and writes DST/RUS
output files.


You can start by moving to your working directory and cloning the repository:

```bash
cd /path/to/your_working_directory
git clone https://github.com/E1039-Collaboration/e1039-analysis.git

If you are a member of the E1039 GitHub organization and want write access, use the SSH URL instead:
git clone git@github.com:E1039-Collaboration/e1039-analysis.git
```

1. Go to the project directory
```cd RUSReconstructor```
2. Set up the environment
```source setup.sh```
3. Configure and build
```
cmake-this
make-this
```
## Test Process
```
cd work_reco
root -b -q Fun4Reco.C
```

# RUS Input Tree Variables

| Variable Name | Type      | Description                                      |
|---------------|-----------|--------------------------------------------------|
| `eventID`     | `int`     | Event ID                                        |
| `runID`       | `int`     | Run ID                                          |
| `spillID`     | `int`     | Spill ID                                        |
| `rfID`        | `int`     | RF ID                                           |
| `turnID`      | `int`     | Turn ID                                         |
| `fpgaTrigger` | `int[5]`  | FPGA trigger                                    |
| `nimTrigger`  | `int[5]`  | NIM trigger                                     |
| `rfIntensity` | `int[33]` | RF intensity                                    |

## Hit-Level Variables`

| Variable Name   | Type                  | Description                                  |
|-----------------|-----------------------|----------------------------------------------|
| `hitID`         | `std::vector<int>`    | Hit IDs for all hits                         |
| `hitTrackID`    | `std::vector<int>`    | Track IDs associated with each hit           |
| `detectorID`    | `std::vector<int>`    | Detector IDs for all hits                    |
| `elementID`     | `std::vector<int>`    | Element IDs associated with each hit         |
| `driftDistance` | `std::vector<double>` | Drift distances for each hit                 |
| `tdcTime`       | `std::vector<double>` | TDC timing values for each hit               |

---

## Truth-Track-Level Variables


| Variable Name | Type               | Description                  |
|---------------|--------------------|------------------------------|
| `gCharge`     | `std::vector<int>` | True particle charges        |
| `gTrackID`    | `std::vector<int>` | True track IDs               |

### Position (Vertex, Station 1, Station 3)

| Variable Name | Type                  | Description                                     |
|---------------|-----------------------|-------------------------------------------------|
| `gvx`         | `std::vector<double>` | True vertex \(x\)-coordinate                    |
| `gvy`         | `std::vector<double>` | True vertex \(y\)-coordinate                    |
| `gvz`         | `std::vector<double>` | True vertex \(z\)-coordinate                    |
| `gx_st1`      | `std::vector<double>` | True \(x\)-position at Station 1                |
| `gy_st1`      | `std::vector<double>` | True \(y\)-position at Station 1                |
| `gz_st1`      | `std::vector<double>` | True \(z\)-position at Station 1                |
| `gx_st3`      | `std::vector<double>` | True \(x\)-position at Station 3                |
| `gy_st3`      | `std::vector<double>` | True \(y\)-position at Station 3                |
| `gz_st3`      | `std::vector<double>` | True \(z\)-position at Station 3                |

### Momentum (Vertex, Station 1, Station 3)

| Variable Name | Type                  | Description                                           |
|---------------|-----------------------|-------------------------------------------------------|
| `gpx`         | `std::vector<double>` | True momentum \(p_x\) at vertex                       |
| `gpy`         | `std::vector<double>` | True momentum \(p_y\) at vertex                       |
| `gpz`         | `std::vector<double>` | True momentum \(p_z\) at vertex                       |
| `gpx_st1`     | `std::vector<double>` | True momentum \(p_x\) at Station 1                    |
| `gpy_st1`     | `std::vector<double>` | True momentum \(p_y\) at Station 1                    |
| `gpz_st1`     | `std::vector<double>` | True momentum \(p_z\) at Station 1                    |
| `gpx_st3`     | `std::vector<double>` | True momentum \(p_x\) at Station 3                    |
| `gpy_st3`     | `std::vector<double>` | True momentum \(p_y\) at Station 3                    |
| `gpz_st3`     | `std::vector<double>` | True momentum \(p_z\) at Station 3                    |

