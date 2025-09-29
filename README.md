# ScoutingEvbyEv

The goal is to be able to do event by event comparison on scouting output for the use of the NGT demonstrator. As a first step a NTuplizer for all the scouting objects needs to be created.

# Progress
- [x] PF Candidates $p_T$
- [] etc (to be added)

# How to use
Once I figure everything out, I think one could simply run something like:
```
cmsRun ntuplizer_cfg.py inputFile=file:/path/to/input.root outputFile=scoutingcollection_ntuple.root numEvents=1000
```
where `inputFile`, `outputFile` and `numEvents` are the input parameters one can give it. Alternatively one can edit this in the python config file directly!
