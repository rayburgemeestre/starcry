/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
var data = [
    ["r01i00compute00", 29693, "datanode", 33456],
    ["r01i00compute00", 29693, "datanode", 50010],
    ["r01i00compute00", 29693, "datanode", 50020],
    ["r01i00compute00", 29880, "journalnode", 8480],
    ["r01i00compute00", 29880, "journalnode", 8485],
    ["r01i00compute01", 23615, "datanode", 37513],
    ["r01i00compute01", 23615, "datanode", 50010],
    ["r01i00compute01", 23615, "datanode", 50020],
    ["r01i00compute01", 23919, "journalnode", 8480],
    ["r01i00compute01", 23919, "journalnode", 8485],
    ["r01i00compute02", 23478, "datanode", 50010],
    ["r01i00compute02", 23478, "datanode", 50020],
    ["r01i00compute02", 23666, "journalnode", 8480],
    ["r01i00compute02", 23666, "journalnode", 8485],
    ["r01i00compute03", 32033, "datanode", 50010],
    ["r01i00compute03", 32033, "datanode", 50020],
    ["r01i00compute03", 32033, "datanode", 55042],
    ["r01i00compute03", 32371, "journalnode", 8480],
    ["r01i00compute03", 32371, "journalnode", 8485],
    ["r01i00compute03", 32624, "nodemanager", 8040],
    ["r01i00compute03", 32624, "nodemanager", 8091],
    ["r01i00compute04", 31450, "datanode", 50010],
    ["r01i00compute04", 31450, "datanode", 50020],
    ["r01i00compute04", 31792, "journalnode", 8480],
    ["r01i00compute04", 31792, "journalnode", 8485],
    ["r01i00compute04", 32052, "nodemanager", 8040],
    ["r01i00compute04", 32052, "nodemanager", 8091],
    ["r01i00compute05", 2614, "datanode", 38330],
    ["r01i00compute05", 2614, "datanode", 50020],
    ["r01i00compute05", 2971, "namenode", 8020],
    ["r01i00compute05", 3218, "nodemanager", 8040],
    ["r01i00compute05", 3218, "nodemanager", 8091],
    ["r01i00compute05", 3765, "zkfc", 8019],
    ["r01i00compute06", 4700, "datanode", 50010],
    ["r01i00compute06", 4700, "datanode", 50020],
    ["r01i00compute06", 5179, "namenode", 8020],
    ["r01i00compute06", 5988, "nodemanager", 46418],
    ["r01i00compute06", 5988, "nodemanager", 8040],
    ["r01i00compute06", 5988, "nodemanager", 8091],
    ["r01i00compute06", 6494, "zkfc", 8019],
    ["r01i00compute07", 4079, "datanode", 41655],
    ["r01i00compute07", 4079, "datanode", 50010],
    ["r01i00compute07", 4079, "datanode", 50020],
    ["r01i00compute07", 4582, "nodemanager", 51967],
    ["r01i00compute07", 4582, "nodemanager", 8040],
    ["r01i00compute07", 4582, "nodemanager", 8091],
    ["r01i00compute07", 4894, "resourcemanager", 8033],
    ["r01i00compute07", 4894, "resourcemanager", 8088],
    ["r01i00compute07", 5058, "timelineserver", 10021],
    ["r01i00compute07", 5058, "timelineserver", 8188],
    ["r01i00compute08", 6321, "datanode", 50010],
    ["r01i00compute08", 6321, "datanode", 50020],
    ["r01i00compute08", 6729, "nodemanager", 49637],
    ["r01i00compute08", 6729, "nodemanager", 8040],
    ["r01i00compute08", 6729, "nodemanager", 8091],
    ["r01i00compute08", 7585, "resourcemanager", 8030],
    ["r01i00compute08", 7585, "resourcemanager", 8033],
    ["r01i00compute08", 7585, "resourcemanager", 8088],
    ["r01i00compute08", 7819, "timelineserver", 10021],
    ["r01i00compute08", 7819, "timelineserver", 8188],
    ["r01i00compute09", 29660, "datanode", 36291],
    ["r01i00compute09", 29660, "datanode", 50010],
    ["r01i00compute09", 30000, "KMS", 16000],
    ["r01i00compute09", 30000, "KMS", 16001],
    ["r01i00compute09", 30277, "nodemanager", 8040],
    ["r01i00compute09", 30277, "nodemanager", 8091],
    ["r01i00compute10", 32432, "datanode", 50010],
    ["r01i00compute10", 32432, "datanode", 50020],
    ["r01i00compute10", 32793, "nodemanager", 57942],
    ["r01i00compute10", 32793, "nodemanager", 8040],
    ["r01i00compute10", 32793, "nodemanager", 8091],
    ["r01i00compute10", 32970, "zookeeper", 2181],
    ["r01i00compute10", 32970, "zookeeper", 3888],
    ["r01i00compute10", 32970, "zookeeper", 44307],
    ["r01i00compute11", 32556, "datanode", 50010],
    ["r01i00compute11", 32556, "datanode", 50020],
    ["r01i00compute11", 32556, "datanode", 50075],
    ["r01i00compute11", 32556, "datanode", 52874],
    ["r01i00compute11", 32904, "nodemanager", 40728],
    ["r01i00compute11", 32904, "nodemanager", 8040],
    ["r01i00compute11", 32997, "zookeeper", 2181],
    ["r01i00compute11", 32997, "zookeeper", 3888],
    ["r01i00compute12", 31978, "datanode", 50010],
    ["r01i00compute12", 31978, "datanode", 50020],
    ["r01i00compute12", 32454, "nodemanager", 43959],
    ["r01i00compute12", 32454, "nodemanager", 8040],
    ["r01i00compute12", 32543, "zookeeper", 2181],
    ["r01i00compute12", 32543, "zookeeper", 3888],
    ["r01i00compute12", 32543, "zookeeper", 42258],
    ["r01i00compute13", 32899, "datanode", 50010],
    ["r01i00compute13", 32899, "datanode", 50020],
    ["r01i00compute13", 33248, "nodemanager", 8040],
    ["r01i00compute13", 33248, "nodemanager", 8091],
    ["r01i00compute13", 33341, "zookeeper", 2181],
    ["r01i00compute13", 33341, "zookeeper", 3888],
    ["r01i00compute13", 33341, "zookeeper", 40503],
    ["r01i00compute14", 32551, "datanode", 50010],
    ["r01i00compute14", 32551, "datanode", 50020],
    ["r01i00compute14", 32914, "nodemanager", 57082],
    ["r01i00compute14", 32914, "nodemanager", 8040],
    ["r01i00compute14", 32914, "nodemanager", 8042],
    ["r01i00compute14", 33005, "zookeeper", 2888],
    ["r01i00compute14", 33005, "zookeeper", 3888],
    ["r01i00compute14", 33005, "zookeeper", 59106],
    ["r01i00compute15", 19031, "datanode", 50010],
    ["r01i00compute15", 19031, "datanode", 50020],
    ["r01i00compute15", 19031, "datanode", 55628],
    ["r01i00compute15", 19395, "nodemanager", 8040],
    ["r01i00compute15", 19395, "nodemanager", 8042],
    ["r01i00compute15", 19395, "nodemanager", 8091],
    ["r01i00compute16", 19420, "datanode", 38840],
    ["r01i00compute16", 19420, "datanode", 50010],
    ["r01i00compute16", 19420, "datanode", 50020],
    ["r01i00compute16", 19600, "nodemanager", 43857],
    ["r01i00compute16", 19600, "nodemanager", 8040],
    ["r01i00compute16", 19600, "nodemanager", 8091],
    ["r01i00compute17", 18815, "datanode", 50010],
    ["r01i00compute17", 18815, "datanode", 50020],
    ["r01i00compute17", 18815, "datanode", 59266],
    ["r01i00compute17", 19190, "nodemanager", 40259],
    ["r01i00compute17", 19190, "nodemanager", 8040],
    ["r01i00compute17", 19190, "nodemanager", 8091],
    ["r01i00compute18", 18477, "datanode", 42766],
    ["r01i00compute18", 18477, "datanode", 50010],
    ["r01i00compute18", 18477, "datanode", 50020],
    ["r01i00compute18", 18825, "nodemanager", 40474],
    ["r01i00compute18", 18825, "nodemanager", 8040],
    ["r01i00compute19", 18543, "datanode", 38432],
    ["r01i00compute19", 18543, "datanode", 50010],
    ["r01i00compute19", 18543, "datanode", 50020],
    ["r01i00compute19", 18905, "nodemanager", 36998],
    ["r01i00compute19", 18905, "nodemanager", 8040],
    ["r01i00compute19", 18905, "nodemanager", 8091],
    ["r01i00compute20", 18363, "datanode", 50010],
    ["r01i00compute20", 18363, "datanode", 50020],
    ["r01i00compute20", 18716, "nodemanager", 36005],
    ["r01i00compute20", 18716, "nodemanager", 8040],
    ["r01i00compute20", 18716, "nodemanager", 8091],
    ["r01i00compute21", 18448, "datanode", 37910],
    ["r01i00compute21", 18448, "datanode", 50010],
    ["r01i00compute21", 18448, "datanode", 50020],
    ["r01i00compute21", 18800, "nodemanager", 8040],
    ["r01i00compute21", 18800, "nodemanager", 8091],
    ["r01i00compute22", 19303, "datanode", 50010],
    ["r01i00compute22", 19303, "datanode", 58851],
    ["r01i00compute22", 19653, "nodemanager", 60083],
    ["r01i00compute22", 19653, "nodemanager", 8040],
    ["r01i00compute22", 19653, "nodemanager", 8091],
    ["r01i00compute23", 19005, "datanode", 48250],
    ["r01i00compute23", 19005, "datanode", 50020],
    ["r01i00compute23", 19360, "nodemanager", 60862],
    ["r01i00compute23", 19360, "nodemanager", 8040],
    ["r01i00compute23", 19360, "nodemanager", 8091],
    ["r01i00compute24", 18343, "datanode", 42025],
    ["r01i00compute24", 18343, "datanode", 50010],
    ["r01i00compute24", 18343, "datanode", 50020],
    ["r01i00compute24", 18716, "nodemanager", 44624],
    ["r01i00compute24", 18716, "nodemanager", 8040],
    ["r01i00compute24", 18716, "nodemanager", 8091],
    ["r01i00compute25", 18769, "datanode", 50010],
    ["r01i00compute25", 18769, "datanode", 50020],
    ["r01i00compute25", 18769, "datanode", 52500],
    ["r01i00compute25", 19129, "nodemanager", 8040],
    ["r01i00compute25", 19129, "nodemanager", 8091],
    ["r01i00compute26", 19068, "datanode", 46962],
    ["r01i00compute26", 19068, "datanode", 50010],
    ["r01i00compute26", 19068, "datanode", 50020],
    ["r01i00compute26", 19421, "nodemanager", 8040],
    ["r01i00compute26", 19421, "nodemanager", 8091],
    ["r01i00compute27", 18226, "datanode", 43176],
    ["r01i00compute27", 18226, "datanode", 50010],
    ["r01i00compute27", 18226, "datanode", 50020],
    ["r01i00compute27", 18577, "nodemanager", 38979],
    ["r01i00compute27", 18577, "nodemanager", 8040],
    ["r01i00compute27", 18577, "nodemanager", 8091],
    ["r01i00compute28", 19089, "datanode", 50010],
    ["r01i00compute28", 19089, "datanode", 50020],
    ["r01i00compute28", 19089, "datanode", 55939],
    ["r01i00compute28", 19441, "nodemanager", 59605],
    ["r01i00compute28", 19441, "nodemanager", 8040],
    ["r01i00compute28", 19441, "nodemanager", 8091],
    ["r01i00compute29", 19762, "datanode", 41899],
    ["r01i00compute29", 19762, "datanode", 50010],
    ["r01i00compute29", 19762, "datanode", 50020],
    ["r01i00compute29", 20126, "nodemanager", 49839],
    ["r01i00compute29", 20126, "nodemanager", 8040],
    ["r01i00compute29", 20126, "nodemanager", 8091],
    ["r01i00compute30", 18109, "datanode", 50010],
    ["r01i00compute30", 18109, "datanode", 50020],
    ["r01i00compute30", 18794, "nodemanager", 8040],
    ["r01i00compute30", 18794, "nodemanager", 8091],
    ["r01i00compute31", 19083, "datanode", 50010],
    ["r01i00compute31", 19083, "datanode", 50020],
    ["r01i00compute31", 19446, "nodemanager", 52006],
    ["r01i00compute31", 19446, "nodemanager", 8040],
    ["r01i00compute31", 19446, "nodemanager", 8091],
    ["r01i00compute32", 19329, "datanode", 50010],
    ["r01i00compute32", 19329, "datanode", 50020],
    ["r01i00compute32", 19329, "datanode", 51756],
    ["r01i00compute32", 19509, "nodemanager", 8040],
    ["r01i00compute32", 19509, "nodemanager", 8091],
    ["r01i00compute33", 19412, "datanode", 48195],
    ["r01i00compute33", 19412, "datanode", 50010],
    ["r01i00compute33", 19591, "nodemanager", 60502],
    ["r01i00compute33", 19591, "nodemanager", 8040],
    ["r01i00compute33", 19591, "nodemanager", 8091],
    ["r01i00compute34", 19232, "datanode", 50010],
    ["r01i00compute34", 19232, "datanode", 50020],
    ["r01i00compute34", 19411, "nodemanager", 57138],
    ["r01i00compute34", 19411, "nodemanager", 8040],
    ["r01i00compute34", 19411, "nodemanager", 8091],
    ["r01i00compute35", 18915, "datanode", 50010],
    ["r01i00compute35", 18915, "datanode", 50020],
    ["r01i00compute35", 19103, "nodemanager", 52601],
    ["r01i00compute35", 19103, "nodemanager", 8040],
    ["r01i01compute00", 20338, "datanode", 41539],
    ["r01i01compute00", 20338, "datanode", 50010],
    ["r01i01compute00", 20531, "nodemanager", 8040],
    ["r01i01compute00", 20531, "nodemanager", 8091],
    ["r01i01compute01", 19916, "datanode", 50010],
    ["r01i01compute01", 19916, "datanode", 50020],
    ["r01i01compute01", 20104, "nodemanager", 8040],
    ["r01i01compute01", 20104, "nodemanager", 8091],
    ["r01i01compute02", 20176, "datanode", 39852],
    ["r01i01compute02", 20176, "datanode", 50010],
    ["r01i01compute02", 20176, "datanode", 50020],
    ["r01i01compute02", 20365, "nodemanager", 8040],
    ["r01i01compute02", 20365, "nodemanager", 8091],
    ["r01i01compute03", 4375, "datanode", 35440],
    ["r01i01compute03", 4375, "datanode", 50010],
    ["r01i01compute03", 4375, "datanode", 50020],
    ["r01i01compute03", 8960, "nodemanager", 36186],
    ["r01i01compute03", 8960, "nodemanager", 8040],
    ["r01i01compute04", 19593, "datanode", 47061],
    ["r01i01compute04", 19593, "datanode", 50010],
    ["r01i01compute04", 19593, "datanode", 50020],
    ["r01i01compute04", 19772, "nodemanager", 52506],
    ["r01i01compute04", 19772, "nodemanager", 8040],
    ["r01i01compute05", 20593, "datanode", 39690],
    ["r01i01compute05", 20593, "datanode", 50010],
    ["r01i01compute05", 20593, "datanode", 50020],
    ["r01i01compute05", 20782, "nodemanager", 40793],
    ["r01i01compute05", 20782, "nodemanager", 8040],
    ["r01i01compute06", 19594, "datanode", 50010],
    ["r01i01compute06", 19594, "datanode", 50020],
    ["r01i01compute06", 19773, "nodemanager", 50968],
    ["r01i01compute06", 19773, "nodemanager", 8040],
    ["r01i01compute07", 19976, "datanode", 48886],
    ["r01i01compute07", 19976, "datanode", 50010],
    ["r01i01compute07", 19976, "datanode", 50020],
    ["r01i01compute07", 20166, "nodemanager", 8040],
    ["r01i01compute07", 20166, "nodemanager", 8091],
    ["r01i01compute08", 19712, "datanode", 50010],
    ["r01i01compute08", 19712, "datanode", 50020],
    ["r01i01compute08", 19891, "nodemanager", 8040],
    ["r01i01compute08", 19891, "nodemanager", 8091],
    ["r01i01compute09", 19855, "datanode", 50010],
    ["r01i01compute09", 19855, "datanode", 50020],
    ["r01i01compute09", 20034, "nodemanager", 35481],
    ["r01i01compute09", 20034, "nodemanager", 8040],
    ["r01i01compute09", 20034, "nodemanager", 8091],
    ["r01i01compute10", 19873, "datanode", 48116],
    ["r01i01compute10", 19873, "datanode", 50010],
    ["r01i01compute10", 19873, "datanode", 50020],
    ["r01i01compute10", 20052, "nodemanager", 57457],
    ["r01i01compute10", 20052, "nodemanager", 8040],
    ["r01i01compute10", 20052, "nodemanager", 8042],
    ["r01i01compute10", 20052, "nodemanager", 8091],
    ["r01i01compute11", 19391, "datanode", 47865],
    ["r01i01compute11", 19391, "datanode", 50010],
    ["r01i01compute11", 19391, "datanode", 50020],
    ["r01i01compute11", 19580, "nodemanager", 41605],
    ["r01i01compute11", 19580, "nodemanager", 8040],
    ["r01i01compute11", 19580, "nodemanager", 8091],
    ["r01i01compute12", 19058, "datanode", 50010],
    ["r01i01compute12", 19058, "datanode", 50020],
    ["r01i01compute12", 19058, "datanode", 55597],
    ["r01i01compute12", 19237, "nodemanager", 8040],
    ["r01i01compute12", 19237, "nodemanager", 8091],
    ["r01i01compute13", 19456, "datanode", 34810],
    ["r01i01compute13", 19456, "datanode", 50020],
    ["r01i01compute13", 19635, "nodemanager", 33233],
    ["r01i01compute13", 19635, "nodemanager", 8040],
    ["r01i01compute13", 19635, "nodemanager", 8091],
    ["r01i01compute14", 19839, "datanode", 50010],
    ["r01i01compute14", 19839, "datanode", 50020],
    ["r01i01compute14", 20018, "nodemanager", 46645],
    ["r01i01compute14", 20018, "nodemanager", 8040],
    ["r01i01compute14", 20018, "nodemanager", 8042],
    ["r01i01compute14", 20018, "nodemanager", 8091],
    ["r01i01compute15", 19959, "datanode", 50010],
    ["r01i01compute15", 19959, "datanode", 50020],
    ["r01i01compute15", 19959, "datanode", 52878],
    ["r01i01compute15", 20139, "nodemanager", 42874],
    ["r01i01compute15", 20139, "nodemanager", 8040],
    ["r01i01compute16", 19567, "datanode", 50010],
    ["r01i01compute16", 19567, "datanode", 50020],
    ["r01i01compute16", 19746, "nodemanager", 8040],
    ["r01i01compute16", 19746, "nodemanager", 8091],
    ["r01i01compute17", 19549, "datanode", 41270],
    ["r01i01compute17", 19549, "datanode", 50010],
    ["r01i01compute17", 19549, "datanode", 50020],
    ["r01i01compute17", 19736, "nodemanager", 50325],
    ["r01i01compute17", 19736, "nodemanager", 8040],
    ["r01i01compute17", 19736, "nodemanager", 8091],
    ["r01i01compute18", 19723, "datanode", 50010],
    ["r01i01compute18", 19723, "datanode", 50020],
    ["r01i01compute18", 19723, "datanode", 60648],
    ["r01i01compute18", 19902, "nodemanager", 8040],
    ["r01i01compute18", 19902, "nodemanager", 8091],
    ["r01i01compute19", 19582, "datanode", 40756],
    ["r01i01compute19", 19582, "datanode", 50010],
    ["r01i01compute19", 19582, "datanode", 50020],
    ["r01i01compute19", 19760, "nodemanager", 59250],
    ["r01i01compute19", 19760, "nodemanager", 8040],
    ["r01i01compute19", 19760, "nodemanager", 8091],
    ["r01i01compute20", 19799, "datanode", 46234],
    ["r01i01compute20", 19799, "datanode", 50020],
    ["r01i01compute20", 19977, "nodemanager", 8040],
    ["r01i01compute20", 19977, "nodemanager", 8042],
    ["r01i01compute20", 19977, "nodemanager", 8091],
    ["r01i01compute21", 19430, "datanode", 46545],
    ["r01i01compute21", 19430, "datanode", 50010],
    ["r01i01compute21", 19430, "datanode", 50020],
    ["r01i01compute21", 19430, "datanode", 50075],
    ["r01i01compute21", 19616, "nodemanager", 47906],
    ["r01i01compute21", 19616, "nodemanager", 8040],
    ["r01i01compute21", 19616, "nodemanager", 8091],
    ["r01i01compute22", 19411, "datanode", 50010],
    ["r01i01compute22", 19411, "datanode", 50020],
    ["r01i01compute22", 19411, "datanode", 55120],
    ["r01i01compute22", 19600, "nodemanager", 57456],
    ["r01i01compute22", 19600, "nodemanager", 8040],
    ["r01i01compute22", 19600, "nodemanager", 8091],
    ["r01i01compute23", 19590, "datanode", 44241],
    ["r01i01compute23", 19590, "datanode", 50010],
    ["r01i01compute23", 19590, "datanode", 50020],
    ["r01i01compute23", 19772, "nodemanager", 8040],
    ["r01i01compute23", 19772, "nodemanager", 8091],
    ["r01i01compute24", 19871, "datanode", 50010],
    ["r01i01compute24", 19871, "datanode", 50020],
    ["r01i01compute24", 20051, "nodemanager", 8040],
    ["r01i01compute24", 20051, "nodemanager", 8091],
    ["r01i01compute25", 18299, "datanode", 50010],
    ["r01i01compute25", 18299, "datanode", 50020],
    ["r01i01compute25", 18299, "datanode", 51470],
    ["r01i01compute25", 18478, "nodemanager", 8040],
    ["r01i01compute25", 18478, "nodemanager", 8091],
    ["r01i01compute26", 19066, "datanode", 50010],
    ["r01i01compute26", 19066, "datanode", 50020],
    ["r01i01compute26", 19254, "nodemanager", 54262],
    ["r01i01compute26", 19254, "nodemanager", 8040],
    ["r01i01compute26", 19254, "nodemanager", 8091],
    ["r01i01compute27", 19650, "datanode", 35138],
    ["r01i01compute27", 19650, "datanode", 50010],
    ["r01i01compute27", 19650, "datanode", 50020],
    ["r01i01compute27", 19803, "nodemanager", 54906],
    ["r01i01compute27", 19803, "nodemanager", 8040],
    ["r01i01compute28", 20157, "datanode", 44424],
    ["r01i01compute28", 20157, "datanode", 50010],
    ["r01i01compute28", 20157, "datanode", 50020],
    ["r01i01compute28", 20336, "nodemanager", 58448],
    ["r01i01compute28", 20336, "nodemanager", 8040],
    ["r01i01compute28", 20336, "nodemanager", 8091],
    ["r01i01compute29", 20470, "datanode", 49585],
    ["r01i01compute29", 20470, "datanode", 50010],
    ["r01i01compute29", 20470, "datanode", 50020],
    ["r01i01compute29", 20648, "nodemanager", 57333],
    ["r01i01compute29", 20648, "nodemanager", 8040],
    ["r01i01compute29", 20648, "nodemanager", 8091],
    ["r01i01compute30", 20303, "datanode", 50010],
    ["r01i01compute30", 20303, "datanode", 50020],
    ["r01i01compute30", 20482, "nodemanager", 34806],
    ["r01i01compute30", 20482, "nodemanager", 8040],
    ["r01i01compute30", 20482, "nodemanager", 8091],
    ["r01i01compute31", 19327, "datanode", 50010],
    ["r01i01compute31", 19327, "datanode", 50020],
    ["r01i01compute31", 19327, "datanode", 58284],
    ["r01i01compute31", 19515, "nodemanager", 8040],
    ["r01i01compute31", 19515, "nodemanager", 8091],
    ["r01i01compute32", 19360, "datanode", 33613],
    ["r01i01compute32", 19360, "datanode", 50010],
    ["r01i01compute32", 19360, "datanode", 50020],
    ["r01i01compute32", 19538, "nodemanager", 60615],
    ["r01i01compute32", 19538, "nodemanager", 8040],
    ["r01i01compute32", 19538, "nodemanager", 8091],
    ["r01i01compute33", 19589, "datanode", 50010],
    ["r01i01compute33", 19589, "datanode", 50020],
    ["r01i01compute33", 19768, "nodemanager", 49750],
    ["r01i01compute33", 19768, "nodemanager", 8040],
    ["r01i01compute33", 19768, "nodemanager", 8091],
    ["r01i01compute34", 19341, "datanode", 50010],
    ["r01i01compute34", 19341, "datanode", 50020],
    ["r01i01compute34", 19341, "datanode", 60802],
    ["r01i01compute34", 19519, "nodemanager", 34885],
    ["r01i01compute34", 19519, "nodemanager", 8040],
    ["r01i01compute34", 19519, "nodemanager", 8091],
    ["r01i01compute35", 19387, "datanode", 50010],
    ["r01i01compute35", 19387, "datanode", 50020],
    ["r01i01compute35", 19576, "nodemanager", 48079],
    ["r01i01compute35", 19576, "nodemanager", 8040],
    ["r01i01compute35", 19576, "nodemanager", 8091],
    ["r01i02compute00", 20981, "datanode", 37027],
    ["r01i02compute00", 20981, "datanode", 50010],
    ["r01i02compute00", 20981, "datanode", 50020],
    ["r01i02compute00", 21174, "nodemanager", 50948],
    ["r01i02compute00", 21174, "nodemanager", 8040],
    ["r01i02compute00", 21174, "nodemanager", 8042],
    ["r01i02compute00", 21174, "nodemanager", 8091],
    ["r01i02compute01", 19279, "datanode", 50010],
    ["r01i02compute01", 19279, "datanode", 50020],
    ["r01i02compute01", 19279, "datanode", 55057],
    ["r01i02compute01", 19457, "nodemanager", 33367],
    ["r01i02compute01", 19457, "nodemanager", 8040],
    ["r01i02compute01", 19457, "nodemanager", 8042],
    ["r01i02compute02", 19049, "datanode", 49784],
    ["r01i02compute02", 19049, "datanode", 50010],
    ["r01i02compute02", 19049, "datanode", 50020],
    ["r01i02compute02", 19227, "nodemanager", 8040],
    ["r01i02compute02", 19227, "nodemanager", 8091],
    ["r01i02compute03", 19206, "datanode", 40949],
    ["r01i02compute03", 19206, "datanode", 50010],
    ["r01i02compute03", 19206, "datanode", 50020],
    ["r01i02compute03", 19385, "nodemanager", 8040],
    ["r01i02compute03", 19385, "nodemanager", 8091],
    ["r01i02compute04", 19162, "datanode", 50010],
    ["r01i02compute04", 19162, "datanode", 50020],
    ["r01i02compute04", 19162, "datanode", 56657],
    ["r01i02compute04", 19341, "nodemanager", 49117],
    ["r01i02compute04", 19341, "nodemanager", 8040],
    ["r01i02compute04", 19341, "nodemanager", 8091],
    ["r01i02compute05", 21314, "datanode", 41392],
    ["r01i02compute05", 21314, "datanode", 50010],
    ["r01i02compute05", 21314, "datanode", 50020],
    ["r01i02compute05", 21492, "nodemanager", 8040],
    ["r01i02compute05", 21492, "nodemanager", 8091],
    ["r01i02compute06", 20396, "datanode", 50010],
    ["r01i02compute06", 20396, "datanode", 50020],
    ["r01i02compute06", 20575, "nodemanager", 34436],
    ["r01i02compute06", 20575, "nodemanager", 8040],
    ["r01i02compute06", 20575, "nodemanager", 8091],
    ["r01i02compute07", 18876, "datanode", 49646],
    ["r01i02compute07", 18876, "datanode", 50010],
    ["r01i02compute07", 18876, "datanode", 50020],
    ["r01i02compute07", 19055, "nodemanager", 8040],
    ["r01i02compute07", 19055, "nodemanager", 8091],
    ["r01i02compute08", 19346, "datanode", 38445],
    ["r01i02compute08", 19346, "datanode", 50010],
    ["r01i02compute08", 19346, "datanode", 50020],
    ["r01i02compute08", 19527, "nodemanager", 41076],
    ["r01i02compute08", 19527, "nodemanager", 8040],
    ["r01i02compute08", 19527, "nodemanager", 8091],
    ["r01i02compute09", 18935, "datanode", 42897],
    ["r01i02compute09", 18935, "datanode", 50010],
    ["r01i02compute09", 18935, "datanode", 50020],
    ["r01i02compute09", 19438, "nodemanager", 35061],
    ["r01i02compute09", 19438, "nodemanager", 8040],
    ["r01i02compute09", 19438, "nodemanager", 8091],
    ["r01i02compute10", 19611, "datanode", 50010],
    ["r01i02compute10", 19611, "datanode", 50020],
    ["r01i02compute10", 19793, "nodemanager", 60214],
    ["r01i02compute10", 19793, "nodemanager", 8040],
    ["r01i02compute10", 19793, "nodemanager", 8091],
    ["r01i02compute11", 19333, "datanode", 50010],
    ["r01i02compute11", 19333, "datanode", 50020],
    ["r01i02compute11", 19333, "datanode", 56962],
    ["r01i02compute11", 19628, "nodemanager", 8040],
    ["r01i02compute11", 19628, "nodemanager", 8091],
    ["r01i02compute12", 19466, "datanode", 45131],
    ["r01i02compute12", 19466, "datanode", 50010],
    ["r01i02compute12", 19466, "datanode", 50020],
    ["r01i02compute12", 19648, "nodemanager", 39015],
    ["r01i02compute12", 19648, "nodemanager", 8040],
    ["r01i02compute12", 19648, "nodemanager", 8091],
    ["r01i02compute13", 18991, "datanode", 50010],
    ["r01i02compute13", 18991, "datanode", 50020],
    ["r01i02compute13", 19169, "nodemanager", 46343],
    ["r01i02compute13", 19169, "nodemanager", 8040],
    ["r01i02compute13", 19169, "nodemanager", 8091],
    ["r01i02compute14", 20699, "datanode", 50010],
    ["r01i02compute14", 20699, "datanode", 51363],
    ["r01i02compute14", 20886, "nodemanager", 8040],
    ["r01i02compute14", 20886, "nodemanager", 8091],
    ["r01i02compute15", 20683, "datanode", 39756],
    ["r01i02compute15", 20683, "datanode", 50010],
    ["r01i02compute15", 20683, "datanode", 50020],
    ["r01i02compute15", 21031, "nodemanager", 48417],
    ["r01i02compute15", 21031, "nodemanager", 8040],
    ["r01i02compute15", 21031, "nodemanager", 8091],
    ["r01i02compute16", 19158, "datanode", 43668],
    ["r01i02compute16", 19158, "datanode", 50010],
    ["r01i02compute16", 19158, "datanode", 50020],
    ["r01i02compute16", 19498, "nodemanager", 36911],
    ["r01i02compute16", 19498, "nodemanager", 8040],
    ["r01i02compute16", 19498, "nodemanager", 8091],
    ["r01i02compute17", 20018, "datanode", 50010],
    ["r01i02compute17", 20018, "datanode", 50020],
    ["r01i02compute17", 20018, "datanode", 55918],
    ["r01i02compute17", 20196, "nodemanager", 55425],
    ["r01i02compute17", 20196, "nodemanager", 8040],
    ["r01i02compute17", 20196, "nodemanager", 8091],
    ["r01i02compute18", 19408, "datanode", 50010],
    ["r01i02compute18", 19408, "datanode", 50020],
    ["r01i02compute18", 19586, "nodemanager", 34871],
    ["r01i02compute18", 19586, "nodemanager", 8040],
    ["r01i02compute19", 21133, "datanode", 45943],
    ["r01i02compute19", 21133, "datanode", 50010],
    ["r01i02compute19", 21133, "datanode", 50020],
    ["r01i02compute19", 21312, "nodemanager", 60057],
    ["r01i02compute19", 21312, "nodemanager", 8040],
    ["r01i02compute20", 17867, "datanode", 50010],
    ["r01i02compute20", 17867, "datanode", 50020],
    ["r01i02compute20", 18055, "nodemanager", 41584],
    ["r01i02compute20", 18055, "nodemanager", 8040],
    ["r01i02compute20", 18055, "nodemanager", 8091],
    ["r01i02compute21", 17974, "datanode", 50010],
    ["r01i02compute21", 17974, "datanode", 50020],
    ["r01i02compute21", 17974, "datanode", 55277],
    ["r01i02compute21", 18152, "nodemanager", 8040],
    ["r01i02compute21", 18152, "nodemanager", 8042],
    ["r01i02compute21", 18152, "nodemanager", 8091],
    ["r01i02compute22", 17795, "datanode", 40085],
    ["r01i02compute22", 17795, "datanode", 50010],
    ["r01i02compute22", 17795, "datanode", 50020],
    ["r01i02compute22", 17982, "nodemanager", 49428],
    ["r01i02compute22", 17982, "nodemanager", 8040],
    ["r01i02compute22", 17982, "nodemanager", 8091],
    ["r01i02compute23", 17970, "datanode", 38164],
    ["r01i02compute23", 17970, "datanode", 50010],
    ["r01i02compute23", 17970, "datanode", 50020],
    ["r01i02compute23", 18320, "nodemanager", 34074],
    ["r01i02compute23", 18320, "nodemanager", 8040],
    ["r01i02compute23", 18320, "nodemanager", 8042],
    ["r01i02compute24", 19093, "datanode", 33525],
    ["r01i02compute24", 19093, "datanode", 50010],
    ["r01i02compute24", 19093, "datanode", 50020],
    ["r01i02compute24", 19272, "nodemanager", 8040],
    ["r01i02compute24", 19272, "nodemanager", 8091],
    ["r01i02compute25", 19104, "datanode", 38578],
    ["r01i02compute25", 19104, "datanode", 50010],
    ["r01i02compute25", 19104, "datanode", 50020],
    ["r01i02compute25", 19283, "nodemanager", 8040],
    ["r01i02compute25", 19283, "nodemanager", 8091],
    ["r01i02compute26", 18865, "datanode", 50010],
    ["r01i02compute26", 18865, "datanode", 50020],
    ["r01i02compute26", 19044, "nodemanager", 8040],
    ["r01i02compute26", 19044, "nodemanager", 8091],
    ["r01i02compute27", 18519, "datanode", 50010],
    ["r01i02compute27", 18519, "datanode", 50020],
    ["r01i02compute27", 18858, "nodemanager", 36900],
    ["r01i02compute27", 18858, "nodemanager", 8040],
    ["r01i02compute27", 18858, "nodemanager", 8091],
    ["r01i02compute28", 18064, "datanode", 47145],
    ["r01i02compute28", 18064, "datanode", 50010],
    ["r01i02compute28", 18064, "datanode", 50020],
    ["r01i02compute28", 18252, "nodemanager", 36081],
    ["r01i02compute28", 18252, "nodemanager", 8040],
    ["r01i02compute28", 18252, "nodemanager", 8091],
    ["r01i02compute29", 18406, "datanode", 35523],
    ["r01i02compute29", 18406, "datanode", 50010],
    ["r01i02compute29", 18406, "datanode", 50075],
    ["r01i02compute29", 18753, "nodemanager", 41940],
    ["r01i02compute29", 18753, "nodemanager", 8040],
    ["r01i02compute29", 18753, "nodemanager", 8091],
    ["r01i02compute30", 18377, "datanode", 50010],
    ["r01i02compute30", 18377, "datanode", 50020],
    ["r01i02compute30", 18377, "datanode", 55728],
    ["r01i02compute30", 18555, "nodemanager", 41650],
    ["r01i02compute30", 18555, "nodemanager", 8040],
    ["r01i02compute30", 18555, "nodemanager", 8091],
    ["r01i02compute31", 18170, "datanode", 35600],
    ["r01i02compute31", 18170, "datanode", 50010],
    ["r01i02compute31", 18170, "datanode", 50020],
    ["r01i02compute31", 18349, "nodemanager", 8040],
    ["r01i02compute31", 18349, "nodemanager", 8091],
    ["r01i02compute32", 19290, "datanode", 50010],
    ["r01i02compute32", 19290, "datanode", 50020],
    ["r01i02compute32", 19551, "nodemanager", 8040],
    ["r01i02compute32", 19551, "nodemanager", 8091],
    ["r01i02compute33", 17893, "datanode", 50010],
    ["r01i02compute33", 17893, "datanode", 50020],
    ["r01i02compute33", 17893, "datanode", 60299],
    ["r01i02compute33", 18239, "nodemanager", 8040],
    ["r01i02compute33", 18239, "nodemanager", 8091],
    ["r01i02compute34", 17703, "datanode", 34595],
    ["r01i02compute34", 17703, "datanode", 50010],
    ["r01i02compute34", 18040, "nodemanager", 45795],
    ["r01i02compute34", 18040, "nodemanager", 8040],
    ["r01i02compute34", 18040, "nodemanager", 8091],
    ["r01i02compute35", 18101, "datanode", 48622],
    ["r01i02compute35", 18101, "datanode", 50010],
    ["r01i02compute35", 18101, "datanode", 50020],
    ["r01i02compute35", 18440, "nodemanager", 8040],
    ["r01i02compute35", 18440, "nodemanager", 8091],
    ["r01i03compute00", 18627, "datanode", 45512],
    ["r01i03compute00", 18627, "datanode", 50010],
    ["r01i03compute00", 18627, "datanode", 50020],
    ["r01i03compute00", 18966, "nodemanager", 36561],
    ["r01i03compute00", 18966, "nodemanager", 8040],
    ["r01i03compute00", 18966, "nodemanager", 8091],
    ["r01i03compute01", 17510, "datanode", 50010],
    ["r01i03compute01", 17510, "datanode", 50020],
    ["r01i03compute01", 17849, "nodemanager", 8040],
    ["r01i03compute01", 17849, "nodemanager", 8091],
    ["r01i03compute02", 18057, "datanode", 50010],
    ["r01i03compute02", 18057, "datanode", 50020],
    ["r01i03compute02", 18057, "datanode", 51635],
    ["r01i03compute02", 18395, "nodemanager", 8040],
    ["r01i03compute02", 18395, "nodemanager", 8091],
    ["r01i03compute03", 17811, "datanode", 50010],
    ["r01i03compute03", 17811, "datanode", 50020],
    ["r01i03compute03", 18470, "nodemanager", 33460],
    ["r01i03compute03", 18470, "nodemanager", 8040],
    ["r01i03compute03", 18470, "nodemanager", 8091],
    ["r01i03compute04", 17887, "datanode", 40952],
    ["r01i03compute04", 17887, "datanode", 50010],
    ["r01i03compute04", 17887, "datanode", 50020],
    ["r01i03compute04", 18226, "nodemanager", 47287],
    ["r01i03compute04", 18226, "nodemanager", 8040],
    ["r01i03compute04", 18226, "nodemanager", 8042],
    ["r01i03compute05", 19320, "datanode", 50010],
    ["r01i03compute05", 19320, "datanode", 50020],
    ["r01i03compute05", 19657, "nodemanager", 8040],
    ["r01i03compute05", 19657, "nodemanager", 8091],
    ["r01i03compute06", 17944, "datanode", 50010],
    ["r01i03compute06", 17944, "datanode", 50020],
    ["r01i03compute06", 17944, "datanode", 54932],
    ["r01i03compute06", 18282, "nodemanager", 8040],
    ["r01i03compute06", 18282, "nodemanager", 8091],
    ["r01i03compute07", 18711, "datanode", 50010],
    ["r01i03compute07", 18711, "datanode", 50020],
    ["r01i03compute07", 18711, "datanode", 53368],
    ["r01i03compute07", 19059, "nodemanager", 39877],
    ["r01i03compute07", 19059, "nodemanager", 8040],
    ["r01i03compute07", 19059, "nodemanager", 8091],
    ["r01i03compute08", 19063, "datanode", 35656],
    ["r01i03compute08", 19063, "datanode", 50010],
    ["r01i03compute08", 19063, "datanode", 50020],
    ["r01i03compute08", 19400, "nodemanager", 47797],
    ["r01i03compute08", 19400, "nodemanager", 8040],
    ["r01i03compute08", 19400, "nodemanager", 8091],
    ["r01i03compute09", 19179, "datanode", 44586],
    ["r01i03compute09", 19179, "datanode", 50010],
    ["r01i03compute09", 19179, "datanode", 50020],
    ["r01i03compute09", 19449, "nodemanager", 44697],
    ["r01i03compute09", 19449, "nodemanager", 8040],
    ["r01i03compute10", 18048, "datanode", 50010],
    ["r01i03compute10", 18048, "datanode", 50020],
    ["r01i03compute10", 18048, "datanode", 53994],
    ["r01i03compute10", 18386, "nodemanager", 43846],
    ["r01i03compute10", 18386, "nodemanager", 8040],
    ["r01i03compute10", 18386, "nodemanager", 8091],
    ["r01i03compute11", 19449, "datanode", 50010],
    ["r01i03compute11", 19449, "datanode", 50020],
    ["r01i03compute11", 19449, "datanode", 53739],
    ["r01i03compute11", 19634, "nodemanager", 49267],
    ["r01i03compute11", 19634, "nodemanager", 8040],
    ["r01i03compute11", 19634, "nodemanager", 8091],
    ["r01i03compute12", 18843, "datanode", 50010],
    ["r01i03compute12", 18843, "datanode", 50020],
    ["r01i03compute12", 19108, "nodemanager", 8040],
    ["r01i03compute12", 19108, "nodemanager", 8091],
    ["r01i03compute13", 18146, "datanode", 41635],
    ["r01i03compute13", 18146, "datanode", 50010],
    ["r01i03compute13", 18481, "nodemanager", 8040],
    ["r01i03compute13", 18481, "nodemanager", 8091],
    ["r01i03compute14", 19975, "datanode", 49232],
    ["r01i03compute14", 19975, "datanode", 50010],
    ["r01i03compute14", 19975, "datanode", 50020],
    ["r01i03compute14", 20235, "nodemanager", 8040],
    ["r01i03compute14", 20235, "nodemanager", 8091],
    ["r01i03compute15", 19645, "datanode", 33039],
    ["r01i03compute15", 19645, "datanode", 50010],
    ["r01i03compute15", 19645, "datanode", 50020],
    ["r01i03compute15", 19837, "nodemanager", 42902],
    ["r01i03compute15", 19837, "nodemanager", 8040],
    ["r01i03compute15", 19837, "nodemanager", 8091],
    ["r01i03compute16", 20154, "datanode", 50010],
    ["r01i03compute16", 20154, "datanode", 50020],
    ["r01i03compute16", 20526, "nodemanager", 56953],
    ["r01i03compute16", 20526, "nodemanager", 8040],
    ["r01i03compute17", 20222, "datanode", 50010],
    ["r01i03compute17", 20222, "datanode", 50020],
    ["r01i03compute17", 20222, "datanode", 50740],
    ["r01i03compute17", 20410, "nodemanager", 8040],
    ["r01i03compute17", 20410, "nodemanager", 8091],
    ["r01i03compute18", 19095, "datanode", 50010],
    ["r01i03compute18", 19095, "datanode", 51747],
    ["r01i03compute18", 19282, "nodemanager", 37589],
    ["r01i03compute18", 19282, "nodemanager", 8040],
    ["r01i03compute18", 19282, "nodemanager", 8091],
    ["r01i03compute19", 20271, "datanode", 50010],
    ["r01i03compute19", 20271, "datanode", 50020],
    ["r01i03compute19", 20271, "datanode", 52824],
    ["r01i03compute19", 20458, "nodemanager", 50771],
    ["r01i03compute19", 20458, "nodemanager", 8040],
    ["r01i03compute19", 20458, "nodemanager", 8091],
    ["r01i03compute20", 20676, "datanode", 50010],
    ["r01i03compute20", 20676, "datanode", 50020],
    ["r01i03compute20", 20864, "nodemanager", 59750],
    ["r01i03compute20", 20864, "nodemanager", 8040],
    ["r01i03compute20", 20864, "nodemanager", 8091],
    ["r01i03compute21", 20364, "datanode", 50010],
    ["r01i03compute21", 20364, "datanode", 50020],
    ["r01i03compute21", 20542, "nodemanager", 41363],
    ["r01i03compute21", 20542, "nodemanager", 8040],
    ["r01i03compute21", 20542, "nodemanager", 8091],
    ["r01i03compute22", 20811, "datanode", 50010],
    ["r01i03compute22", 20811, "datanode", 50020],
    ["r01i03compute22", 20811, "datanode", 53090],
    ["r01i03compute22", 20998, "nodemanager", 8040],
    ["r01i03compute22", 20998, "nodemanager", 8091],
    ["r01i03compute23", 20285, "datanode", 37657],
    ["r01i03compute23", 20285, "datanode", 50010],
    ["r01i03compute23", 20285, "datanode", 50020],
    ["r01i03compute23", 20463, "nodemanager", 49346],
    ["r01i03compute23", 20463, "nodemanager", 8040],
    ["r01i03compute23", 20463, "nodemanager", 8091],
    ["r01i03compute24", 20717, "datanode", 50010],
    ["r01i03compute24", 20717, "datanode", 50020],
    ["r01i03compute24", 20717, "datanode", 50466],
    ["r01i03compute24", 20897, "nodemanager", 8040],
    ["r01i03compute24", 20897, "nodemanager", 8091],
    ["r01i03compute25", 20494, "datanode", 50010],
    ["r01i03compute25", 20494, "datanode", 50020],
    ["r01i03compute25", 21058, "nodemanager", 8040],
    ["r01i03compute25", 21058, "nodemanager", 8091],
    ["r01i03compute26", 20080, "datanode", 44760],
    ["r01i03compute26", 20080, "datanode", 50010],
    ["r01i03compute26", 20080, "datanode", 50020],
    ["r01i03compute26", 20258, "nodemanager", 36277],
    ["r01i03compute26", 20258, "nodemanager", 8040],
    ["r01i03compute26", 20258, "nodemanager", 8091],
    ["r01i03compute27", 20139, "datanode", 50010],
    ["r01i03compute27", 20139, "datanode", 50020],
    ["r01i03compute27", 20317, "nodemanager", 8040],
    ["r01i03compute27", 20317, "nodemanager", 8091],
    ["r01i03compute28", 18846, "datanode", 44277],
    ["r01i03compute28", 18846, "datanode", 50010],
    ["r01i03compute28", 18846, "datanode", 50020],
    ["r01i03compute28", 19025, "nodemanager", 60071],
    ["r01i03compute28", 19025, "nodemanager", 8040],
    ["r01i03compute28", 19025, "nodemanager", 8091],
    ["r01i03compute29", 20184, "datanode", 50010],
    ["r01i03compute29", 20184, "datanode", 50020],
    ["r01i03compute29", 20364, "nodemanager", 42163],
    ["r01i03compute29", 20364, "nodemanager", 8040],
    ["r01i03compute29", 20364, "nodemanager", 8091],
    ["r01i03compute30", 20314, "datanode", 50010],
    ["r01i03compute30", 20314, "datanode", 50020],
    ["r01i03compute30", 20492, "nodemanager", 8040],
    ["r01i03compute30", 20492, "nodemanager", 8091],
    ["r01i03compute31", 19273, "datanode", 50010],
    ["r01i03compute31", 19273, "datanode", 50020],
    ["r01i03compute31", 19451, "nodemanager", 51477],
    ["r01i03compute31", 19451, "nodemanager", 8040],
    ["r01i03compute31", 19451, "nodemanager", 8091],
    ["r01i03compute32", 20397, "datanode", 50010],
    ["r01i03compute32", 20397, "datanode", 50020],
    ["r01i03compute32", 20575, "nodemanager", 8040],
    ["r01i03compute32", 20575, "nodemanager", 8091],
    ["r01i03compute33", 20390, "datanode", 33328],
    ["r01i03compute33", 20390, "datanode", 50010],
    ["r01i03compute33", 20390, "datanode", 50020],
    ["r01i03compute33", 20390, "datanode", 50075],
    ["r01i03compute33", 20568, "nodemanager", 8040],
    ["r01i03compute33", 20568, "nodemanager", 8091],
    ["r01i03compute34", 21296, "datanode", 46443],
    ["r01i03compute34", 21296, "datanode", 50010],
    ["r01i03compute34", 21296, "datanode", 50020],
    ["r01i03compute34", 21476, "nodemanager", 45318],
    ["r01i03compute34", 21476, "nodemanager", 8040],
    ["r01i03compute34", 21476, "nodemanager", 8042],
    ["r01i03compute34", 21476, "nodemanager", 8091],
    ["r01i03compute35", 21583, "datanode", 32962],
    ["r01i03compute35", 21583, "datanode", 50010],
    ["r01i03compute35", 21583, "datanode", 50020],
    ["r01i03compute35", 21761, "nodemanager", 60282],
    ["r01i03compute35", 21761, "nodemanager", 8040],
    ["r01i03compute35", 21761, "nodemanager", 8042],
    ["headnode", '',  "job", '']
];

/*
data.sort(function(a, b) {
    return a[3] > b[3];
});
*/

var ips = {
    '10.142.1.0'    : 'r01i00compute00',
    '10.142.1.1'    : 'r01i00compute01',
    '10.142.1.2'    : 'r01i00compute02',
    '10.142.1.3'    : 'r01i00compute03',
    '10.142.1.4'    : 'r01i00compute04',
    '10.142.1.5'    : 'r01i00compute05',
    '10.142.1.6'    : 'r01i00compute06',
    '10.142.1.7'    : 'r01i00compute07',
    '10.142.1.8'    : 'r01i00compute08',
    '10.142.1.9'    : 'r01i00compute09',
    '10.142.1.10'    : 'r01i00compute10',
    '10.142.1.11'    : 'r01i00compute11',
    '10.142.1.12'    : 'r01i00compute12',
    '10.142.1.13'    : 'r01i00compute13',
    '10.142.1.14'    : 'r01i00compute14',
    '10.142.1.15'    : 'r01i00compute15',
    '10.142.1.16'    : 'r01i00compute16',
    '10.142.1.17'    : 'r01i00compute17',
    '10.142.1.18'    : 'r01i00compute18',
    '10.142.1.19'    : 'r01i00compute19',
    '10.142.1.20'    : 'r01i00compute20',
    '10.142.1.21'    : 'r01i00compute21',
    '10.142.1.22'    : 'r01i00compute22',
    '10.142.1.23'    : 'r01i00compute23',
    '10.142.1.24'    : 'r01i00compute24',
    '10.142.1.25'    : 'r01i00compute25',
    '10.142.1.26'    : 'r01i00compute26',
    '10.142.1.27'    : 'r01i00compute27',
    '10.142.1.28'    : 'r01i00compute28',
    '10.142.1.29'    : 'r01i00compute29',
    '10.142.1.30'    : 'r01i00compute30',
    '10.142.1.31'    : 'r01i00compute31',
    '10.142.1.32'    : 'r01i00compute32',
    '10.142.1.33'    : 'r01i00compute33',
    '10.142.1.34'    : 'r01i00compute34',
    '10.142.1.35'    : 'r01i00compute35',
    '10.142.1.36'    : 'r01i01compute00',
    '10.142.1.37'    : 'r01i01compute01',
    '10.142.1.38'    : 'r01i01compute02',
    '10.142.1.39'    : 'r01i01compute03',
    '10.142.1.40'    : 'r01i01compute04',
    '10.142.1.41'    : 'r01i01compute05',
    '10.142.1.42'    : 'r01i01compute06',
    '10.142.1.43'    : 'r01i01compute07',
    '10.142.1.44'    : 'r01i01compute08',
    '10.142.1.45'    : 'r01i01compute09',
    '10.142.1.46'    : 'r01i01compute10',
    '10.142.1.47'    : 'r01i01compute11',
    '10.142.1.48'    : 'r01i01compute12',
    '10.142.1.49'    : 'r01i01compute13',
    '10.142.1.50'    : 'r01i01compute14',
    '10.142.1.51'    : 'r01i01compute15',
    '10.142.1.52'    : 'r01i01compute16',
    '10.142.1.53'    : 'r01i01compute17',
    '10.142.1.54'    : 'r01i01compute18',
    '10.142.1.55'    : 'r01i01compute19',
    '10.142.1.56'    : 'r01i01compute20',
    '10.142.1.57'    : 'r01i01compute21',
    '10.142.1.58'    : 'r01i01compute22',
    '10.142.1.59'    : 'r01i01compute23',
    '10.142.1.60'    : 'r01i01compute24',
    '10.142.1.61'    : 'r01i01compute25',
    '10.142.1.62'    : 'r01i01compute26',
    '10.142.1.63'    : 'r01i01compute27',
    '10.142.1.64'    : 'r01i01compute28',
    '10.142.1.65'    : 'r01i01compute29',
    '10.142.1.66'    : 'r01i01compute30',
    '10.142.1.67'    : 'r01i01compute31',
    '10.142.1.68'    : 'r01i01compute32',
    '10.142.1.69'    : 'r01i01compute33',
    '10.142.1.70'    : 'r01i01compute34',
    '10.142.1.71'    : 'r01i01compute35',
    '10.142.1.72'    : 'r01i02compute00',
    '10.142.1.73'    : 'r01i02compute01',
    '10.142.1.74'    : 'r01i02compute02',
    '10.142.1.75'    : 'r01i02compute03',
    '10.142.1.76'    : 'r01i02compute04',
    '10.142.1.77'    : 'r01i02compute05',
    '10.142.1.78'    : 'r01i02compute06',
    '10.142.1.79'    : 'r01i02compute07',
    '10.142.1.80'    : 'r01i02compute08',
    '10.142.1.81'    : 'r01i02compute09',
    '10.142.1.82'    : 'r01i02compute10',
    '10.142.1.83'    : 'r01i02compute11',
    '10.142.1.84'    : 'r01i02compute12',
    '10.142.1.85'    : 'r01i02compute13',
    '10.142.1.86'    : 'r01i02compute14',
    '10.142.1.87'    : 'r01i02compute15',
    '10.142.1.88'    : 'r01i02compute16',
    '10.142.1.89'    : 'r01i02compute17',
    '10.142.1.90'    : 'r01i02compute18',
    '10.142.1.91'    : 'r01i02compute19',
    '10.142.1.92'    : 'r01i02compute20',
    '10.142.1.93'    : 'r01i02compute21',
    '10.142.1.94'    : 'r01i02compute22',
    '10.142.1.95'    : 'r01i02compute23',
    '10.142.1.96'    : 'r01i02compute24',
    '10.142.1.97'    : 'r01i02compute25',
    '10.142.1.98'    : 'r01i02compute26',
    '10.142.1.99'    : 'r01i02compute27',
    '10.142.1.100'    : 'r01i02compute28',
    '10.142.1.101'    : 'r01i02compute29',
    '10.142.1.102'    : 'r01i02compute30',
    '10.142.1.103'    : 'r01i02compute31',
    '10.142.1.104'    : 'r01i02compute32',
    '10.142.1.105'    : 'r01i02compute33',
    '10.142.1.106'    : 'r01i02compute34',
    '10.142.1.107'    : 'r01i02compute35',
    '10.142.1.108'    : 'r01i03compute00',
    '10.142.1.109'    : 'r01i03compute01',
    '10.142.1.110'    : 'r01i03compute02',
    '10.142.1.111'    : 'r01i03compute03',
    '10.142.1.112'    : 'r01i03compute04',
    '10.142.1.113'    : 'r01i03compute05',
    '10.142.1.114'    : 'r01i03compute06',
    '10.142.1.115'    : 'r01i03compute07',
    '10.142.1.116'    : 'r01i03compute08',
    '10.142.1.117'    : 'r01i03compute09',
    '10.142.1.118'    : 'r01i03compute10',
    '10.142.1.119'    : 'r01i03compute11',
    '10.142.1.120'    : 'r01i03compute12',
    '10.142.1.121'    : 'r01i03compute13',
    '10.142.1.122'    : 'r01i03compute14',
    '10.142.1.123'    : 'r01i03compute15',
    '10.142.1.124'    : 'r01i03compute16',
    '10.142.1.125'    : 'r01i03compute17',
    '10.142.1.126'    : 'r01i03compute18',
    '10.142.1.127'    : 'r01i03compute19',
    '10.142.1.128'    : 'r01i03compute20',
    '10.142.1.129'    : 'r01i03compute21',
    '10.142.1.130'    : 'r01i03compute22',
    '10.142.1.131'    : 'r01i03compute23',
    '10.142.1.132'    : 'r01i03compute24',
    '10.142.1.133'    : 'r01i03compute25',
    '10.142.1.134'    : 'r01i03compute26',
    '10.142.1.135'    : 'r01i03compute27',
    '10.142.1.136'    : 'r01i03compute28',
    '10.142.1.137'    : 'r01i03compute29',
    '10.142.1.138'    : 'r01i03compute30',
    '10.142.1.139'    : 'r01i03compute31',
    '10.142.1.140'    : 'r01i03compute32',
    '10.142.1.141'    : 'r01i03compute33',
    '10.142.1.142'    : 'r01i03compute34',
    '10.142.1.143'    : 'r01i03compute35',
    '10.141.255.254'  : 'headnode',
};

var fps            = 25;
var max_frames     = 10000000 * fps;
var realtime       = false;
var offset_date    = false;
var first_frame    = false;
var buffer         = [];
var frame          = 0;
var current_frame  = 0;
var previous_frame = 0;
var nodes          = {};

function draw_box(x, y, width, height) {
    var r = 1, g = 0, b = 0;
    width /= 2.0;
    height /= 2.0;

    add_line(x - width, y - height, 0,
             x - width, y + height, 0, 2.0, r, g, b);
    add_line(x - width, y + height, 0,
             x + width, y + height, 0, 2.0, r, g, b);
    add_line(x + width, y - height, 0,
             x + width, y + height, 0, 2.0, r, g, b);
    add_line(x + width, y - height, 0,
             x - width, y - height, 0, 2.0, r, g, b);
}

var coords = {};
function calculate_coords(node, x, y) {
    var offset = 0;
    for (service in nodes[node]) {
        //if (service.indexOf('datanode') != -1) continue;
        //if (service.indexOf('nodemanager') != -1) continue;
        //output(service);
        // service
        var service_x = x;
        var service_y = y + offset;

        // ports
        offset += 10;
        var ports = nodes[node][service].ports;
        var len = 0;
        for (port in ports) { len++; }
        var port_offset = -1 * (((len - 1) / 2.0) * 20);
        for (port in ports) {
            var port_x = x + port_offset;
            var port_y = y + offset;
            coords[node + port] = { x: port_x, y: port_y };
            port_offset += 20;
        }

        // extra spacing
        offset += 50;

        coords[node + service] = { x: service_x + 40 + 2.5, y: service_y - 2.5 };
    }
}
function draw_node(node, x, y) {
    var offset = 0;
    for (service in nodes[node]) {
        //if (service.indexOf('datanode') != -1) continue;
        //if (service.indexOf('nodemanager') != -1) continue;

        // service
        var service_x = x;
        var service_y = y + offset;
        add_text(service_x, service_y, 0, service, 'center');

        // ports
        offset += 10;
        var ports = nodes[node][service].ports;
        var len = 0;
        for (port in ports) { len++; }
        var port_offset = -1 * (((len - 1) / 2.0) * 20);
        for (port in ports) {
            var port_x = x + port_offset;
            var port_y = y + offset;
            add_text(port_x, port_y, 0, '' + port, 'center');
            draw_box(port_x, port_y, 18, 6);
            coords[node + port] = { x: port_x, y: port_y };
            port_offset += 20;
        }

        // extra spacing
        offset += 50;

        draw_box(service_x, service_y + 5, 70, 20);
        draw_box(service_x + 40 + 2.5, service_y - 2.5, 5, 5);
        coords[node + service] = { x: service_x + 40 + 2.5, y: service_y - 2.5 };
        draw_box(service_x + 40 - 2.5, service_y - 2.5, 5, 0);

    }
}

var activity = {};
var console = [];
var header = '';

function draw_header(x, y) {
    add_text(x, y, 0, header, 'left');
}

function draw_console(x, y) {
    console = console.slice(-30);
    var offset = 0;
    for (var i=0; i<console.length; i++) {
        add_text(x, y + offset, 0, console[i], 'left');
        offset += 10;
    }
}

function process() {
    if (frame != previous_frame) {
        while (current_frame < frame) {
            for (var a in activity) {
                if (activity[a].bytes > 20) {
                    activity[a].bytes /= 1.2;
                }
                else {
                    activity[a].bytes -= 1;
                }
                if (activity[a].bytes <= 0) {
                    activity[a].bytes = 0;
                    activity[a].shadow -= (0.2 / (25 * 4));
                }
                if (activity[a].shadow < 0) {
                    activity[a].shadow = 0;
                }
            }

            for (var i=0; i<buffer.length; i++) {
                var line = buffer[i];

                if (line.startsWith('@@usage@@')) {
                    line = line.substr(10);
                    //console.push('usage = ' + line);
                    continue;
                }
                else if (line.startsWith('@@job@@')) {
                    line = line.substr(8);
                    if (line.startsWith('INFO'))
                        header = line.substr(5);
                    else
                        console.push(line);
                    continue;
                }

                var c = line.split(' ');
                c[4] = c[4] == '127.0.0.1' ? c[0] : ips[c[4]];
                if (!c[4]) continue;
                var from = c[0] + c[1]; // node + service
                var to   = c[4] + c[5]; // node + port
                if (!(from in activity))
                    activity[from + to] = { 
                        from: from, to: to, bytes: 0, shadow: 0.2 
                    };

                activity[from + to].bytes += c[6];
            }
            buffer = [];
            current_frame++;

            //calculate_coords('node1',    -250, -125);
            //calculate_coords('node2',    -150, -125);
            //calculate_coords('node3',    -50,  -125);
            
            //mark
            for (var i=0; i<data.length; i++) {
                var node = data[i];
                if (node[0].indexOf('compute') !== -1 /*&& node[2] !== 'datanode'*/) {
                    var x = -250 + i;
                    var y = -125;
                    calculate_coords(node[0], x, y);
                }
            }

            calculate_coords('headnode', -150,   80);
            draw_console(10, -125);
            draw_header(-250, -150);
            add_circle(0, 0, 0, 100, 2.0);

            for (var a in activity) {
                var act = activity[a];
                var from = coords[act.from];
                var to = coords[act.to];
                output(act.from + " + " + act.to);
                if (act.to.endsWith('53') && from) {
                    if (act.bytes > 0) {
                        add_text(from.x - 90, from.y, 0, 'DNS', 'left');
                    }
                }
                /*
                if (act.to.endsWith('88') && from) {
                    to = coords['kerberos'];
                    if (act.bytes > 0) {
                        add_text(from.x - 90, from.y + 10, 0, 'KRB', 'left');
                    }
                }
                */
                if (from && to) {
                    var size = act.bytes / 10.0;
                    if (size > 100) size = 100.0;
                    if (act.bytes > 0) {
                        add_line(from.x, from.y, 0, to.x, to.y, 0, size + 1, 0, 1, 0);
                    }
                    else if (act.shadow > 0.1) {
                        add_line(from.x, from.y, 0, to.x, to.y, 0, 5.0, act.shadow, act.shadow, act.shadow);
                    }
                }
            }

            // TODO: refactor these functions so they use coords[] instead!
            //draw_node('node1',    -250, -125);
            //draw_node('node2',    -150, -125);
            //draw_node('node3',    -50,  -125);
            //mark
            for (var i=0; i<data.length; i++) {
                var node = data[i];
                if (node[0].indexOf('compute') !== -1) {
                    var x = -250 + (i * 5);
                    var y = -125;
                    draw_node(node[0], x, y);
                }
            }


            draw_node('headnode', -150,   100);

            write_frame();
        }
    }
}

function input(line) {
    var matches = line.match(/(\d+):(\d+):(\d+).(\d+) (.*)/ );
    var time    = matches[1] + ':' + matches[2] + ':'  + matches[3];
    var dt      = new Date('1984-02-23 ' + time); // add a date for Mr. Javascript

    frame       = Math.floor(parseInt(matches[4]) / (1000000 / fps));
    offset_date = offset_date || dt;
    first_frame = first_frame || frame;

    frame += parseInt(((dt - offset_date) / 1000) * 25) - first_frame;

    process();

    buffer.push(matches[5]);
    previous_frame = frame;
}

function initialize() {
    for (var i=0; i<data.length; i++) {
        var node    = data[i][0];
        var pid     = data[i][1];
        var service = data[i][2];
        var port    = data[i][3];

        if (!(node in nodes)) nodes[node] = {};
        if (!(service in nodes[node])) nodes[node][service] = {
            pid : pid,
            ports : {},
        };
        nodes[node][service].ports[port] = {};
    }
}

function close() { frame++; process(); }

