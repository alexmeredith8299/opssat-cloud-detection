#!/bin/sh
mkdir em_test
chmod x em_test
#img-> csv
./bin/opssat-rf-segment.out --input samples/img_msec_1597559030639_2_thumbnail.jpeg --mode 1 --output em_test/rf_test_to_csv.csv
#train rf
./bin/include/ranger/cpp_version/ranger --file em_test/rf_test_to_csv.csv --depvarname CLOUD --treetype 1 --ntree 10 --write
#test rf
./bin/include/ranger/cpp_version/ranger --file em_test/rf_test_to_csv.csv --predict ranger_out.forest
#write cloud mask
mv ranger_out.* em_test
mv em_test/ranger_out.prediction em_test/ranger_out.csv
./bin/opssat-rf-segment.out --input em_test/ranger_out.csv --mode 2 --output em_test/rf_test_cloud_mask.png
