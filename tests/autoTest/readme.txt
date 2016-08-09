-$ ./install-thirdparty.sh
-build autoTest from QtCreator in /build-auto-test folder (debug | release)
-Copy the Resources folder into the build folder
-run test

INPUT
-Resources/test_multi.VertexSubstance.json - list of subtances exported from PMATCH++
-Resources/TestMulti - GEMS4R exported files containing the same sbustances as in the above list

OUTPUT
-Writes in terminal - if there is a relative difference between GEMS and TCorrPT calculated properties larger than tolerance = 1e-05
-writes in csv files calculated properties 

