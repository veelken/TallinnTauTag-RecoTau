#!/usr/bin/env python3

import functools
import json
import os
import matplotlib.pyplot as plt
plt.style.use('seaborn-whitegrid')

inputFilePath1 = "/home/veelken/gnnTauReco/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/"
jsonFileName1 = "TallinnTauProducer_forLaurits_v2.json"
label1 = "Training"

inputFilePath2 = "/home/veelken/gnnTauReco/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/"
jsonFileName2 = "TallinnTauProducer_forLaurits_v2.json"
label2 = "EDProducer"

ignore_missing_keys = True

diff_threshold = 1.e-2

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
        if key in keys1 and key in keys2:
            keys_common.append(key)
        elif key in keys1:
            keys_only1.append(key)
        elif key in keys2:
            keys_only2.append(key)
    return ( keys_common, keys_only1, keys_only2 )

def event_order(event1, event2):
    items1 = event1.split(":")
    items2 = event2.split(":")
    for idx in range(3):
        if int(items1[idx]) < int(items2[idx]):
            return True
        if int(items1[idx]) > int(items2[idx]):
            return False
    return True

( events_common, events_only1, events_only2 ) = split_keys(dict1.keys(), dict2.keys())
all_events = []
all_events.extend(events_common)
all_events.extend(events_only1)
all_events.extend(events_only2)
all_events.sort(key = functools.cmp_to_key(event_order))

# compare dictionaries and print differences to stdout
for event in all_events:
    ( jets_common, jets_only1, jets_only2 ) = split_keys(dict1[event], dict2[event])
    pfCands_that_differ = {} # key = jet
    for jet in jets_common:
        ( pfCands_common, pfCands_only1, pfCands_only2 ) = split_keys(dict1[event][jet], dict2[event][jet])
        for pfCand in pfCands_common:
            value1 = dict1[event][jet][pfCand]
            value2 = dict2[event][jet][pfCand]
            diff = value1 - value2
            mean = 0.5*(abs(value1) + abs(value2))
            if diff > diff_threshold:
                if not jet in pfCands_that_differ:
                    pfCands_that_differ[jet] = []
                pfCands_that_differ[jet].append(pfCand)
    isFirst = True
    if len(pfCands_that_differ):
        if isFirst:
            print("Differences in event %s:" % event)
            isFirst = False
        for jet in jets_common:
            if jet in pfCands_that_differ:
                for pfCand in pfCands_that_differ[jet]:
                    print("jet = '%s', pfCand = '%s': value1 = %1.4f, value2 = %1.4f" % (jet, pfCand, dict1[event][jet][pfCand], dict2[event][jet][pfCand]))

    if not ignore_missing_keys:
        if len(jets_only1) > 0 or len(jets_only2):
            if isFirst:
                print("Differences in event %s:" % event)
                isFirst = False
            if len(jets_only1) > 0:
                print("jets only in %s: %s" % (label1, jets_only1))
            if len(jets_only2) > 0:
                print("jets only in %s: %s" % (label2, jets_only2))
        for jet in jets_common:
            pfCands_only1 = dict1[event][jet].keys()
            sort(pfCands_only1)
            pfCands_only2 = dict2[event][jet].keys()
            sort(pfCands_only2)
            if len(pfCands_only1) > 0 or len(pfCands_only2) > 0:
                if isFirst:
                    print("Differences in event %s:" % event)
                    isFirst = False
                print("jet = '%s':" % jet)
                if len(pfCands_only1) > 0:
                    print(" pfCands only in %s: %s" % (label1, jets_only1))
                if len(pfCands_only2) > 0:
                    print(" pfCands only in %s: %s" % (label2, jets_only2))
               
    if not isFirst:
        print("")

# make scatter plot of DNN output assigned to pfCands (only for common events & jets)
x = []
y = []
for event in all_events:
    ( jets_common, jets_only1, jets_only2 ) = split_keys(dict1[event], dict2[event])
    for jet in jets_common:
        ( pfCands_common, pfCands_only1, pfCands_only2 ) = split_keys(dict1[event][jet], dict2[event][jet])
        for pfCand in pfCands_common:
            value1 = dict1[event][jet][pfCand]
            value2 = dict2[event][jet][pfCand]
            x.append(value1)
            y.append(value2)       
plt.scatter(x, y, marker = 'o', s = 5)
plt.xlabel(label1)
plt.ylabel(label2)
plt.show()
plt.savefig('compareJSONs.png')
