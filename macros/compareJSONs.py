#!/usr/bin/env python3

import functools
import json
import os
import matplotlib.pyplot as plt
plt.style.use('seaborn-whitegrid')

inputFilePath1 = "/home/veelken/gnnTauReco/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/"
jsonFileName1 = "Training_2022Mar07.json" # made using DNN in dnn_2022Mar07_Laurits.pb
label1 = "Training"

inputFilePath2 = "/home/veelken/gnnTauReco/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/"
jsonFileName2 = "TallinnTauProducer_forLaurits_v8.json"
label2 = "EDProducer"

ignore_missing_keys = True

diff_threshold = 1.e-4

# load dictionaries from JSON files
jsonFile1 = open(os.path.join(inputFilePath1, jsonFileName1))
dict1 = json.load(jsonFile1)
jsonFile1.close()

jsonFile2 = open(os.path.join(inputFilePath2, jsonFileName2))
dict2 = json.load(jsonFile2)
jsonFile2.close()

def split_keys(keys1, keys2):
    keys_common = []
    keys_only1 = []
    keys_only2 = []
    all_keys = set()
    for key in keys1:
        if not key in all_keys:
            all_keys.add(key)
    for key in keys2:
        if not key in all_keys:
            all_keys.add(key)
    for key in all_keys:
        if key in [ "inputs", "full_input", "output" ]:
            continue
        if key in keys1 and key in keys2:
            keys_common.append(key)
        elif key in keys1:
            keys_only1.append(key)
        elif key in keys2:
            keys_only2.append(key)
    return ( keys_common, keys_only1, keys_only2 )

def separate_event_and_jet(event_and_jet):
    pos = event_and_jet.index("|")
    event = event_and_jet[0:pos]
    jet = event_and_jet[pos + 1:]
    return ( event, jet )

def event_and_jet_order(event_and_jet1, event_and_jet2):
    ( event1, jet1 ) = separate_event_and_jet(event_and_jet1)
    event_items1 = event1.split(":")
    jet_items1 = jet1.split("|")
    jet_pT1 = float(jet_items1[0])
    ( event2, jet2 ) = separate_event_and_jet(event_and_jet2)
    event_items2 = event2.split(":")
    jet_items2 = jet2.split("|")
    jet_pT2 = float(jet_items2[1])
    for idx in range(3):
        if int(event_items1[idx]) < int(event_items2[idx]):
            return True
        if int(event_items1[idx]) > int(event_items2[idx]):
            return False
    return jet_pT1 > jet_pT2

( events_and_jets_common, events_and_jets_only1, events_and_jets_only2 ) = split_keys(dict1.keys(), dict2.keys())
all_events_and_jets = []
all_events_and_jets.extend(events_and_jets_common)
all_events_and_jets.extend(events_and_jets_only1)
all_events_and_jets.extend(events_and_jets_only2)
all_events_and_jets.sort(key = functools.cmp_to_key(event_and_jet_order))

#print("events_and_jets_common = %s" % events_and_jets_common)
#print("events_and_jets_only1 = %s" % events_and_jets_only1)
#print("events_and_jets_only2 = %s" % events_and_jets_only2)

# compare dictionaries and print differences to stdout
if not ignore_missing_keys:
    for event_and_jet in events_and_jets_only1:
        ( event, jet ) = separate_event_and_jet(event_and_jet)
        print("Differences in event %s, jet = '%s':" % (event, jet))
        print(" combination of event & jet only in %s." % label1)
    for event_and_jet in events_and_jets_only2:
        ( event, jet ) = separate_event_and_jet(event_and_jet)
        print("Differences in event %s, jet = '%s':" % (event, jet))
        print(" combination of event & jet only in %s." % label2)
for event_and_jet in events_and_jets_common:
    ( event, jet ) = separate_event_and_jet(event_and_jet)
    isFirst = True
    #----------------------------------------------------------------------------
    # CV: compare "full" DNN input vectors (only needed for low-level debugging)
    full_input1 = dict1[event_and_jet]["full_input"]
    full_input2 = dict2[event_and_jet]["full_input"]
    if len(full_input1) != len(full_input2):
        raise ValueError("Mismatch in length of input vectors (1 = %i, 2 = %i) !!" % (len(full_input1), len(full_input2)))
    num_full_input = len(full_input1)
    for idx_full_input in range(num_full_input):
        input1 = full_input1[idx_full_input]
        input2 = full_input2[idx_full_input]
        diff = abs(input1 - input2)
        mean = 0.5*(abs(input1) + abs(input2))
        if diff > diff_threshold:
            if isFirst:        
                print("Differences in event %s, jet = '%s':" % (event, jet))
                isFirst = False
            print(" full_input #%i: 1 = %1.6f, 2 = %1.6f" % (idx_full_input, input1, input2))
    #----------------------------------------------------------------------------
    pfCands_that_differ = []
    ( pfCands_common, pfCands_only1, pfCands_only2 ) = split_keys(dict1[event_and_jet], dict2[event_and_jet])
    for pfCand in pfCands_common:
        value1 = dict1[event_and_jet][pfCand]["output"]
        value2 = dict2[event_and_jet][pfCand]["output"]
        diff = abs(value1 - value2)
        mean = 0.5*(abs(value1) + abs(value2))
        if diff > diff_threshold:
            pfCands_that_differ.append(pfCand)    
    if len(pfCands_that_differ):
        if isFirst:        
            print("Differences in event %s, jet = '%s':" % (event, jet))
            isFirst = False
        for pfCand in pfCands_that_differ:
            print(" pfCand = '%s': value1 = %1.4f, value2 = %1.4f" % (pfCand, dict1[event_and_jet][pfCand]["output"], dict2[event_and_jet][pfCand]["output"]))

    if not ignore_missing_keys:
        if len(pfCands_only1) > 0 or len(pfCands_only2) > 0:
            if isFirst:
                print("Differences in event %s, jet = '%s':" % (event, jet))
                isFirst = False
            print("jet = '%s':" % jet)
            if len(pfCands_only1) > 0:
                print(" pfCands only in %s: %s" % (label1, pfCands_only1))
            if len(pfCands_only2) > 0:
                print(" pfCands only in %s: %s" % (label2, pfCands_only2))               

    if not isFirst:
        print("")

# make scatter plot of DNN output assigned to pfCands (only for common events & jets)
x = []
y = []
for event_and_jet in events_and_jets_common:
    ( event, jet ) = separate_event_and_jet(event_and_jet)
    #print("Processing event %s, jet = '%s':" % (event, jet))
    ( pfCands_common, pfCands_only1, pfCands_only2 ) = split_keys(dict1[event_and_jet], dict2[event_and_jet])
    #print("pfCands_common = %s" % pfCands_common)
    #print("pfCands_only1 = %s" % pfCands_only1)
    #print("pfCands_only2 = %s" % pfCands_only2)
    for pfCand in pfCands_common:
        value1 = dict1[event_and_jet][pfCand]["output"]
        value2 = dict2[event_and_jet][pfCand]["output"]
        #print("x = %1.3f, y = %1.3f" % (value1, value2))
        x.append(value1)
        y.append(value2)
plt.scatter(x, y, marker = 'o', s = 5)
plt.xlabel(label1)
plt.ylabel(label2)
plt.show()
plt.savefig('compareJSONs.png')
