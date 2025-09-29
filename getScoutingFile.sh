#!/bin/bash -ex

hltGetConfiguration /users/musich/tests/dev/CMSSW_15_0_0/NGT_DEMONSTRATOR/TestData/online/HLT/V1 \
            --globaltag 150X_dataRun3_Prompt_v1 \
            --data \
            --unprescale \
            --output all \
            --max-events -1\
            --eras Run3_2024 --l1-emulator uGT --l1 L1Menu_Collisions2025_v1_0_0_xml \
            --input /store/data/Run2025E/EphemeralHLTPhysics0/RAW/v1/000/396/058/00000/0ceac4e4-e378-48ce-9aee-24a60aec59d2.root \
            > hltData.py

cmsRun hltData.py


