#! /usr/bin/env bash

echo "Running msh HW 6 Tests" 
echo "-------------------------" 
for file in *.in; do
     if [[ -e time.out ]]; then 
         rm time.out
     fi 
     ARGS=$(cat ${file%.in}.args)
     diff -w <(\time -o time.out -f "%e" ../../bin/msh $ARGS < ${file} | sed 's/msh>//g' | tr -d '\n' | sed  -e '$a\' ) ${file%.in}.ans &> /dev/null
     if [[ "$?" -eq 0 ]]; then 
          if [[ -e ${file%.in}.time ]]; then 
               GOT_TIME=$(cat time.out)
               GOT_TIME=${GOT_TIME%.*}
               EXPECTED_TIME=$(cat ${file%.in}.time)
               printf "Test (%s) passed (Got time=%s), however your time for this test must between %s to actually pass.\n" $file $GOT_TIME $EXPECTED_TIME 
          else 
               printf "Test (%s) passed\n" $file 
          fi 
     else 
          printf "Test (#%s) failed\n-----------\n" $file 
          diff -w <(../../bin/msh $ARGS < ${file} | sed 's/msh>//g' | tr -d '\n' | sed  -e '$a\') ${file%.in}.ans
          echo "-----------"
     fi 
done 