import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing
params = VarParsing('analysis')


params.register('inputFile',
                'file:XYZ.root',
                VarParsing.multiplicity.singleton,
                VarParsing.varType.string,
                "Input file")

params.register('outputFile',
                'XYZ.root', 
                VarParsing.multiplicity.singleton,
                VarParsing.varType.string,
                "Output file")

params.register('numEvents',
                -1,
                VarParsing.multiplicity.singleton,
                VarParsing.varType.int,
                "Number of events to process")

params.parseArguments()

process = cms.Process('Ntuplizer')
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100 
process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(params.numEvents))
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(params.inputFile)
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string(params.outputFile)
)

process.scoutingCollectionNtuplizer = cms.EDAnalyzer('ScoutingCollectionNtuplizer',
    pfcands = cms.InputTag("hltScoutingPFPacker")
)

process.p = cms.Path(process.scoutingCollectionNtuplizer)

